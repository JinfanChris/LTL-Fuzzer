// src/grpc/LTLFuzzServiceImpl.cc
#include "automata_handler.h" // Existing logic
#include "ltlfuzz.grpc.pb.h"
#include <iostream>

using grpc::ServerContext;
using grpc::Status;
using ltlfuzz::CheckResult;
using ltlfuzz::FuzzService;
using ltlfuzz::LTLList;
using ltlfuzz::TraceInput;

class LTLFuzzServiceImpl final : public FuzzService::Service {
public:
  Status PrepareLTLProperties(ServerContext *context, const LTLList *request,
                              ::ltlfuzz::Status *reply) override {
    std::cout << "Received LTL properties: " << std::endl;
    for (const auto &prop : request->properties()) {
      std::cout << " - " << prop << std::endl;
    }
    // Pass request->properties() to your automata handler
    reply->set_message("Properties loaded.");
    return Status::OK;
  }

  Status SubmitTrace(ServerContext *context, const TraceInput *request,
                     CheckResult *result) override {

    std::cout << "Received trace: " << request->trace() << std::endl;
    // Call LTL checker logic on request->trace()
    result->set_satisfied(true); // mock
    return Status::OK;
  }
};
