#include <boost/optional/optional.hpp>

#include "ray/core_worker/task_execution.h"
#include "ray/core_worker/context.h"
#include "ray/core_worker/core_worker.h"
#include "ray/core_worker/transport/direct_actor_transport.h"
#include "ray/core_worker/transport/raylet_transport.h"

namespace ray {

CoreWorkerTaskExecutionInterface::CoreWorkerTaskExecutionInterface(
    WorkerContext &worker_context, std::unique_ptr<RayletClient> &raylet_client,
    CoreWorkerObjectInterface &object_interface, const TaskExecutor &executor,
    bool use_asio_rpc)
    : worker_context_(worker_context),
      object_interface_(object_interface),
      execution_callback_(executor),
      main_service_(std::make_shared<boost::asio::io_service>()),
      main_work_(*main_service_) {
  RAY_CHECK(execution_callback_ != nullptr);

  auto func = std::bind(&CoreWorkerTaskExecutionInterface::ExecuteTask, this,
                        std::placeholders::_1, std::placeholders::_2);

  boost::optional<rpc::GrpcServer &> grpc_server;
  boost::optional<rpc::AsioRpcServer &> asio_server;

  if (use_asio_rpc) {
    // TODO: fix the port.
    std::unique_ptr<rpc::AsioRpcServer> server(new rpc::AsioRpcServer("Worker", 0 /* let grpc choose port */, *main_service_));
    asio_server = *server;
    worker_server_ = std::move(server);
  } else {
    std::unique_ptr<rpc::GrpcServer> server(new rpc::GrpcServer("Worker", 0 /* let grpc choose port */));
    grpc_server = *server;
    worker_server_ = std::move(server);
  }

  task_receivers_.emplace(
      TaskTransportType::RAYLET,
      use_asio_rpc ?
      std::unique_ptr<CoreWorkerRayletTaskReceiver>(
          new RayletAsioTaskReceiver(raylet_client, object_interface_, asio_server.get(), func)) :
      std::unique_ptr<CoreWorkerRayletTaskReceiver>(new RayletGrpcTaskReceiver(
          raylet_client, object_interface_, *main_service_, grpc_server.get(), func)));
  task_receivers_.emplace(
      TaskTransportType::DIRECT_ACTOR,
      use_asio_rpc ?
      std::unique_ptr<CoreWorkerDirectActorTaskReceiver>(
          new DirectActorAsioTaskReceiver(object_interface_, asio_server.get(), func)) :
      std::unique_ptr<CoreWorkerDirectActorTaskReceiver>(
          new DirectActorGrpcTaskReceiver(object_interface_, *main_service_, grpc_server.get(), func))); 

  // Start RPC server after all the task receivers are properly initialized.
  worker_server_->Run();
}

Status CoreWorkerTaskExecutionInterface::ExecuteTask(
    const TaskSpecification &task_spec,
    std::vector<std::shared_ptr<RayObject>> *results) {
  RAY_LOG(DEBUG) << "Executing task " << task_spec.TaskId();

  worker_context_.SetCurrentTask(task_spec);

  RayFunction func{task_spec.GetLanguage(), task_spec.FunctionDescriptor()};

  std::vector<std::shared_ptr<RayObject>> args;
  RAY_CHECK_OK(BuildArgsForExecutor(task_spec, &args));

  auto num_returns = task_spec.NumReturns();
  if (task_spec.IsActorCreationTask() || task_spec.IsActorTask()) {
    RAY_CHECK(num_returns > 0);
    // Decrease to account for the dummy object id.
    num_returns--;
  }

  auto status = execution_callback_(func, args, num_returns, results);
  // TODO(zhijunfu):
  // 1. Check and handle failure.
  // 2. Save or load checkpoint.
  return status;
}

void CoreWorkerTaskExecutionInterface::Run() {
  // Run main IO service.
  main_service_->run();
}

void CoreWorkerTaskExecutionInterface::Stop() {
  // Stop main IO service.
  std::shared_ptr<boost::asio::io_service> main_service = main_service_;
  // Delay the execution of io_service::stop() to avoid deadlock if
  // CoreWorkerTaskExecutionInterface::Stop is called inside a task.
  main_service_->post([main_service]() { main_service->stop(); });
}

Status CoreWorkerTaskExecutionInterface::BuildArgsForExecutor(
    const TaskSpecification &task, std::vector<std::shared_ptr<RayObject>> *args) {
  auto num_args = task.NumArgs();
  (*args).resize(num_args);

  std::vector<ObjectID> object_ids_to_fetch;
  std::vector<int> indices;

  for (size_t i = 0; i < task.NumArgs(); ++i) {
    int count = task.ArgIdCount(i);
    if (count > 0) {
      // pass by reference.
      RAY_CHECK(count == 1);
      object_ids_to_fetch.push_back(task.ArgId(i, 0));
      indices.push_back(i);
    } else {
      // pass by value.
      (*args)[i] = std::make_shared<RayObject>(
          std::make_shared<LocalMemoryBuffer>(const_cast<uint8_t *>(task.ArgVal(i)),
                                              task.ArgValLength(i)),
          nullptr);
    }
  }

  std::vector<std::shared_ptr<RayObject>> results;
  auto status = object_interface_.Get(object_ids_to_fetch, -1, &results);
  if (status.ok()) {
    for (size_t i = 0; i < results.size(); i++) {
      (*args)[indices[i]] = results[i];
    }
  }

  return status;
}

}  // namespace ray
