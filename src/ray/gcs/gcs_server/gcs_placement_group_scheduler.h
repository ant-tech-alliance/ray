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

#ifndef RAY_GCS_PLACEMRNT_GROUP_SCHEDULER_H
#define RAY_GCS_PLACEMRNT_GROUP_SCHEDULER_H

#include <ray/common/id.h>
#include <ray/gcs/accessor.h>
#include <ray/protobuf/gcs_service.pb.h>
#include <ray/raylet/raylet_client.h>
#include <ray/rpc/node_manager/node_manager_client.h>
#include <ray/rpc/worker/core_worker_client.h>
#include <queue>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "gcs_node_manager.h"

namespace ray {
namespace gcs {

using LeaseClientFactoryFn =
    std::function<std::shared_ptr<WorkerLeaseInterface>(const rpc::Address &address)>;

class GcsPlacementGroup;

class GcsPlacementGroupSchedulerInterface {
 public:
  /// Schedule the specified placement_group.
  ///
  /// \param placement_group to be scheduled.
  virtual void Schedule(std::shared_ptr<GcsPlacementGroup> placement_group) = 0;

  virtual ~GcsPlacementGroupSchedulerInterface() {}
};

/// GcsPlacementGroupScheduler is responsible for scheduling placement_groups registered to GcsPlacementGroupManager.
/// This class is not thread-safe.
class GcsPlacementGroupScheduler : public GcsPlacementGroupSchedulerInterface {
 public:
  /// Create a GcsPlacementGroupScheduler
  ///
  /// \param io_context The main event loop.
  /// \param placement_group_info_accessor Used to flush placement_group info to storage.
  /// \param gcs_node_manager The node manager which is used when scheduling.
  /// \param schedule_failure_handler Invoked when there are no available nodes to
  /// schedule placement_groups.
  /// \param schedule_success_handler Invoked when placement_groups are created on the worker
  /// successfully.
  explicit GcsPlacementGroupScheduler(
      boost::asio::io_context &io_context, gcs::PlacementGroupInfoAccessor &placement_group_info_accessor,
      const GcsNodeManager &gcs_node_manager,std::shared_ptr<gcs::GcsPubSub> gcs_pub_sub,
      std::function<void(std::shared_ptr<GcsPlacementGroup>)> schedule_failure_handler,
      std::function<void(std::shared_ptr<GcsPlacementGroup>)> schedule_success_handler);
  virtual ~GcsPlacementGroupScheduler() = default;

  /// Schedule the specified placement_group.
  /// If there is no available nodes then the `schedule_failed_handler_` will be
  /// triggered, otherwise the placement_group will be scheduled until succeed or canceled.
  ///
  /// \param placement_group to be scheduled.
  void Schedule(std::shared_ptr<GcsPlacementGroup> placement_group) override;


 protected:
  /// The GcsLeasedWorker is kind of abstraction of remote leased worker inside raylet. It
  /// contains the address of remote leased worker as well as the leased resources and the
  /// ID of the placement_group associated with this worker. Through this class, we can easily get
  /// the WorkerID, Endpoint, NodeID and the associated PlacementGroupID of the remote worker.
  class GcsLeasedWorker {
   public:
    /// Create a GcsLeasedWorker
    ///
    /// \param address the Address of the remote leased worker.
    /// \param resources the resources that leased from the remote node(raylet).
    /// \param placement_group_id ID of the placement_group associated with this leased worker.
    explicit GcsLeasedWorker(rpc::Address address,
                             std::vector<rpc::ResourceMapEntry> resources,
                             const PlacementGroupID &placement_group_id)
        : address_(std::move(address)),
          resources_(std::move(resources)),
          assigned_placement_group_id_(placement_group_id) {}
    virtual ~GcsLeasedWorker() = default;

    /// Get the Address of this leased worker.
    const rpc::Address &GetAddress() const { return address_; }

    /// Get the ip address of this leased worker.
    const std::string &GetIpAddress() const { return address_.ip_address(); }

    /// Get the listening port of the leased worker at remote side.
    uint16_t GetPort() const { return address_.port(); }

    /// Get the id of the placement_group which is assigned to this leased worker.
    PlacementGroupID GetAssignedPlacementGroupID() const { return assigned_placement_group_id_; }

    /// Get the leased resources.
    const std::vector<rpc::ResourceMapEntry> &GetLeasedResources() const {
      return resources_;
    }

   protected:
    /// The address of the remote leased worker.
    rpc::Address address_;
    /// The resources leased from remote node.
    std::vector<rpc::ResourceMapEntry> resources_;
    /// Id of the placement_group assigned to this worker.
    PlacementGroupID assigned_placement_group_id_;
  };

  /// Lease a worker from the specified node for the specified bundle.
  ///
  /// \param bundle A description of the placement_group to create. This object has the resource
  /// specification needed to lease workers from the specified node.
  /// \param node The node that the worker will be leased from.
    Status LeaseWorkerFromNode(const rpc::Bundle &bundle, std::shared_ptr<rpc::GcsNodeInfo> node);

  /// Handler to process a granted lease.
  ///
  /// \param bundle Contains the resources needed to lease workers from the specified node.
  /// \param reply The reply of `RequestWorkerLeaseForBundleRequest`.
    Status HandleWorkerLeasedForBundleReply(const rpc::Bundle &bundle, 
                                            const ray::rpc::RequestWorkerLeaseForBundleReply &reply);

  /// Get an existing lease client or connect a new one.
  std::shared_ptr<WorkerLeaseInterface> GetOrConnectLeaseClient(
      const rpc::Address &raylet_address);

 protected:
  /// The io loop that is used to delay execution of tasks (e.g.,
  /// execute_after).
  boost::asio::io_context &io_context_;
  /// The placement_group info accessor.
  gcs::PlacementGroupInfoAccessor &placement_group_info_accessor_;
  /// Reference of GcsNodeManager.
  const GcsNodeManager &gcs_node_manager_;
  /// A publisher for publishing gcs messages.
  std::shared_ptr<gcs::GcsPubSub> gcs_pub_sub_;
  /// The handler to handle the scheduling failures.
  std::function<void(std::shared_ptr<GcsPlacementGroup>)> schedule_failure_handler_;
  /// The handler to handle the successful scheduling.
  std::function<void(std::shared_ptr<GcsPlacementGroup>)> schedule_success_handler_;
  /// Fplacement_groupy for producing new clients to request leases from remote nodes.
  LeaseClientFactoryFn lease_client_fplacement_groupy_;
  /// Fplacement_groupy for producing new core worker clients.
  rpc::ClientFactoryFn client_fplacement_groupy_;
    /// The cached node clients which are used to communicate with raylet to lease workers.
  absl::flat_hash_map<ClientID, std::shared_ptr<WorkerLeaseInterface>>
      remote_lease_clients_;
  /// Factory for producing new clients to request leases from remote nodes.
  LeaseClientFactoryFn lease_client_factory_;

};

}
}

#endif