#ifndef RAY_RPC_NODE_MANAGER_SERVER_H
#define RAY_RPC_NODE_MANAGER_SERVER_H

#include "ray/rpc/grpc_server.h"
#include "ray/rpc/server_call.h"

#include "src/ray/protobuf/test.grpc.pb.h"
#include "src/ray/protobuf/test.pb.h"

namespace ray {
namespace rpc {

/// Interface of the `TestServiceHandler`, see `src/ray/protobuf/test.proto`.
class TestServiceHandler {
 public:
  /// Handle a `DebugEcho` request.
  virtual void HandleEcho(const EchoRequest &request, EchoReply *reply,
                          SendReplyCallback send_reply_callback) = 0;
};

/// The `GrpcService` for `TestService`.
class TestService : public GrpcService {
 public:
  /// Constructor.
  ///
  /// \param[in] io_service See super class.
  /// \param[in] handler The service handler that actually handle the requests.
  TestService(boost::asio::io_service &io_service, TestServiceHandler &service_handler)
      : GrpcService(io_service), service_handler_(service_handler){};

 protected:
  grpc::Service &GetGrpcService() override { return service_; }

  void InitServerCallFactories(
      const std::unique_ptr<grpc::ServerCompletionQueue> &cq,
      std::vector<std::pair<std::unique_ptr<ServerCallFactory>, int>>
          *server_call_factories_and_concurrencies) override {
    // Initialize the factory for `DebugEcho` requests.
    std::unique_ptr<ServerCallFactory> debug_echo_call_factory(
        new ServerCallFactoryImpl<DebugEchoService, TestServiceHandler, EchoRequest,
                                  EchoReply>(
            service_, &DebugEchoService::AsyncService::RequestDebugEcho, service_handler_,
            &TestServiceHandler::HandleEcho, cq, main_service_));
    // Set `DebugEcho`'s accept concurrency.
    server_call_factories_and_concurrencies->emplace_back(
        std::move(debug_echo_call_factory), 3);
  }

 private:
  /// The grpc async service object.
  DebugEchoService::AsyncService service_;

  /// The service handler that actually handle the requests.
  TestServiceHandler &service_handler_;
};

}  // namespace rpc
}  // namespace ray

#endif
