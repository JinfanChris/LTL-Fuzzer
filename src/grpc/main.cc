// src/grpc/main.cc
#include "ltlfuzzServiceImpl.cc"
#include <grpcpp/grpcpp.h>

#define DEBUG

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  LTLFuzzServiceImpl service;

  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

int main() {
  RunServer();
  return 0;
}
