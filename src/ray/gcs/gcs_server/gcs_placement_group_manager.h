// Copyright 2017 The Ray Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include <queue>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "ray/common/id.h"
#include "ray/common/task/task_execution_spec.h"
#include "ray/common/task/task_spec.h"
#include "ray/gcs/gcs_server/gcs_placement_group_scheduler.h"
#include "ray/gcs/gcs_server/gcs_table_storage.h"
#include "ray/gcs/pubsub/gcs_pub_sub.h"
#include "ray/rpc/worker/core_worker_client.h"
#include "src/ray/protobuf/gcs_service.pb.h"

namespace ray {
namespace gcs {

/// GcsPlacementGroup just wraps `PlacementGroupTableData` and provides some convenient
/// interfaces to access the fields inside `PlacementGroupTableData`. This class is not
/// thread-safe.
class GcsPlacementGroup {
 public:
  /// Create a GcsPlacementGroup by placement_group_table_data.
  ///
  /// \param placement_group_table_data Data of the placement_group (see gcs.proto).
  explicit GcsPlacementGroup(rpc::PlacementGroupTableData placement_group_table_data)
      : placement_group_table_data_(std::move(placement_group_table_data)) {}

  /// Create a GcsPlacementGroup by CreatePlacementGroupRequest.
  ///
  /// \param request Contains the placement group creation task specification.
  explicit GcsPlacementGroup(const ray::rpc::CreatePlacementGroupRequest &request) {
    const auto &placement_group_spec = request.placement_group_spec();
    placement_group_table_data_.set_placement_group_id(
        placement_group_spec.placement_group_id());
    placement_group_table_data_.set_name(placement_group_spec.name());
    placement_group_table_data_.set_state(rpc::PlacementGroupTableData::PENDING);
    placement_group_table_data_.mutable_bundles()->CopyFrom(
        placement_group_spec.bundles());
    placement_group_table_data_.set_strategy(placement_group_spec.strategy());
  }

  /// Get the immutable PlacementGroupTableData of this placement group.
  const rpc::PlacementGroupTableData &GetPlacementGroupTableData();

  /// Get the mutable bundle of this placement group.
  rpc::Bundle *GetMutableBundle(int bundle_index);

  /// Update the state of this placement_group.
  void UpdateState(rpc::PlacementGroupTableData::PlacementGroupState state);

  /// Get the state of this gcs placement_group.
  rpc::PlacementGroupTableData::PlacementGroupState GetState() const;

  /// Get the id of this placement_group.
  PlacementGroupID GetPlacementGroupID() const;

  /// Get the name of this placement_group.
  std::string GetName() const;

  /// Get the bundles of this placement_group (including unplaced).
  std::vector<std::shared_ptr<BundleSpecification>> GetBundles() const;

  /// Get the unplaced bundles of this placement group.
  std::vector<std::shared_ptr<BundleSpecification>> GetUnplacedBundles() const;

  /// Get the Strategy
  rpc::PlacementStrategy GetStrategy() const;

  // Get debug string for the placement group.
  const std::string DebugString() const;

 private:
  /// The placement_group meta data which contains the task specification as well as the
  /// state of the gcs placement_group and so on (see gcs.proto).
  rpc::PlacementGroupTableData placement_group_table_data_;
};

using RegisterPlacementGroupCallback =
    std::function<void(std::shared_ptr<GcsPlacementGroup>)>;

/// GcsPlacementGroupManager is responsible for managing the lifecycle of all placement
/// group. This class is not thread-safe.
/// The placementGroup will be added into queue and set the status as pending first and
/// use SchedulePendingPlacementGroups(). The SchedulePendingPlacementGroups() will get
/// the head of the queue and schedule it. If schedule success, using the
/// SchedulePendingPlacementGroups() Immediately. else wait for a short time beforw using
/// SchedulePendingPlacementGroups() next time.
class GcsPlacementGroupManager : public rpc::PlacementGroupInfoHandler {
 public:
  /// Create a GcsPlacementGroupManager
  ///
  /// \param io_context The event loop to run the monitor on.
  /// \param scheduler Used to schedule placement group creation tasks.
  /// \param gcs_table_storage Used to flush placement group data to storage.
  explicit GcsPlacementGroupManager(
      boost::asio::io_context &io_context,
      std::shared_ptr<GcsPlacementGroupSchedulerInterface> scheduler,
      std::shared_ptr<gcs::GcsTableStorage> gcs_table_storage);

  ~GcsPlacementGroupManager() = default;

  void HandleCreatePlacementGroup(const rpc::CreatePlacementGroupRequest &request,
                                  rpc::CreatePlacementGroupReply *reply,
                                  rpc::SendReplyCallback send_reply_callback) override;

  void HandleRemovePlacementGroup(const rpc::RemovePlacementGroupRequest &request,
                                  rpc::RemovePlacementGroupReply *reply,
                                  rpc::SendReplyCallback send_reply_callback) override;

  void HandleGetPlacementGroup(const rpc::GetPlacementGroupRequest &request,
                               rpc::GetPlacementGroupReply *reply,
                               rpc::SendReplyCallback send_reply_callback) override;

  /// Register placement_group asynchronously.
  ///
  /// \param request Contains the meta info to create the placement_group.
  /// \param callback Will be invoked after the placement_group is created successfully or
  /// be invoked immediately if the placement_group is already registered to
  /// `registered_placement_groups_` and its state is `CREATED`. The callback will not be
  /// called in this case.
  void RegisterPlacementGroup(const rpc::CreatePlacementGroupRequest &request,
                              StatusCallback callback);

  /// Schedule placement_groups in the `pending_placement_groups_` queue.
  void SchedulePendingPlacementGroups();

  /// Get the placement_group ID for the named placement_group. Returns nil if the
  /// placement_group was not found.
  /// \param name The name of the  placement_group to look up.
  /// \returns PlacementGroupID The ID of the placement_group. Nil if the
  /// placement_group was not found.
  PlacementGroupID GetPlacementGroupIDByName(const std::string &name);

  /// Handle placement_group creation task failure. This should be called when scheduling
  /// an placement_group creation task is infeasible.
  ///
  /// \param placement_group The placement_group whose creation task is infeasible.
  void OnPlacementGroupCreationFailed(std::shared_ptr<GcsPlacementGroup> placement_group);

  /// Handle placement_group creation task success. This should be called when the
  /// placement_group creation task has been scheduled successfully.
  ///
  /// \param placement_group The placement_group that has been created.
  void OnPlacementGroupCreationSuccess(
      const std::shared_ptr<GcsPlacementGroup> &placement_group);

  /// TODO-SANG Fill it up.
  void RemovePlacementGroup(const PlacementGroupID &placement_group_id,
                            StatusCallback on_placement_group_removed);

  /// Handle a node death. This will reschedule all bundles associated with the
  /// specified node id.
  ///
  /// \param node_id The specified node id.
  void OnNodeDead(const ClientID &node_id);

 private:
  /// Scheduling priority of placement group: RESCHEDULING > PENDING.
  struct PlacementGroupCompare {
    bool operator()(const std::shared_ptr<GcsPlacementGroup> &left,
                    const std::shared_ptr<GcsPlacementGroup> &right) {
      return left->GetState() < right->GetState();
    }
  };

  /// Try to create placement group after a short time.
  void RetryCreatingPlacementGroup();

  /// Mark the manager that there's a placement group scheduling going on.
  void MarkSchedulingStarted(const PlacementGroupID placement_group_id) {
    scheduling_in_progress_id_ = placement_group_id;
  }

  /// Mark the manager that there's no more placement group scheduling going on.
  void MarkSchedulingDone() { scheduling_in_progress_id_ = PlacementGroupID::Nil(); }

  /// Check if the placement group of a given id is scheduling.
  bool IsSchedulingInProgress(const PlacementGroupID &placement_group_id) const {
    return scheduling_in_progress_id_ == placement_group_id;
  }

  /// Check if there's any placement group scheduling going on.
  bool IsSchedulingInProgress() const {
    return scheduling_in_progress_id_ != PlacementGroupID::Nil();
  }

  /// The io loop that is used to delay execution of tasks (e.g.,
  /// execute_after).
  boost::asio::io_context &io_context_;

  /// Callback of placement_group registration requests that are not yet flushed.
  absl::flat_hash_map<PlacementGroupID, StatusCallback>
      placement_group_to_register_callback_;

  /// All registered placement_groups (pending placement_groups are also included).
  absl::flat_hash_map<PlacementGroupID, std::shared_ptr<GcsPlacementGroup>>
      registered_placement_groups_;

  /// The pending placement_groups which will not be scheduled until there's a resource
  /// change.
  std::priority_queue<std::shared_ptr<GcsPlacementGroup>,
                      std::vector<std::shared_ptr<GcsPlacementGroup>>,
                      PlacementGroupCompare>
      pending_placement_groups_;

  /// The scheduler to schedule all registered placement_groups.
  std::shared_ptr<gcs::GcsPlacementGroupSchedulerInterface>
      gcs_placement_group_scheduler_;

  /// Used to update placement group information upon creation, deletion, etc.
  std::shared_ptr<gcs::GcsTableStorage> gcs_table_storage_;

  /// The placement group id that is in progress of scheduling bundles.
  /// TODO(sang): Currently, only one placement group can be scheduled at a time.
  /// We should probably support concurrenet creation (or batching).
  PlacementGroupID scheduling_in_progress_id_ = PlacementGroupID::Nil();
};

}  // namespace gcs
}  // namespace ray
