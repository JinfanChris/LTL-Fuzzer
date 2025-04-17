// src/grpc/LTLFuzzServiceImpl.cc
#include "automata.h"
#include "automata_handler.h" // Existing logic
#include "ltlfuzz.grpc.pb.h"
#include <exception>
#include <iostream>
#include <sstream>
#include <vector>

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
    autometa_list_.clear();

    for (const auto &prop : request->properties()) {
      std::cout << "Received LTL properties: ";
      std::cout << " - " << prop << std::endl;
      try {

        lfz::automata::Automata *atm = new lfz::automata::Automata();
        // atm->set_formula(prop, "a,b,c,d");
        atm->set_formula(prop, "");
        autometa_list_.push_back(atm);
      } catch (const lfz::automata::AutomataException &e) {
        std::cerr << "Error creating automata: " << e.what() << std::endl;
        reply->set_message("Failed to create automata.");
        return Status(grpc::INVALID_ARGUMENT, e.what());
      }
    }
    // Pass request->properties() to your automata handler
    reply->set_message("Properties loaded.");
    return Status::OK;
  }

  Status SubmitTrace(ServerContext *context, const TraceInput *request,
                     CheckResult *result) override {

    std::string trace_str = request->trace();
    std::cout << "Received trace: " << request->trace() << std::endl;

    std::vector<std::string> trace_events;
    std::stringstream ss(trace_str);
    std::string item;

    while (std::getline(ss, item, ',')) {
      trace_events.push_back(item);
    }

    bool all_satisfied = true;
    for (auto &automata : autometa_list_) {
      std::vector<lfz::automata::MCState> trace_states;
      try {

        automata->model_check_events(trace_events, trace_states);
      } catch (std::exception &e) {
        std::cerr << "Error during model checking: " << e.what() << std::endl;
        result->add_violations("Model checking failed.");
        return Status(grpc::INVALID_ARGUMENT, e.what());
      }

      if (!trace_states.empty() && trace_states.back().state == -1) {
        all_satisfied = false;
        std::string vio = automata->formula() + ", ";
        // std::cout << vio << " " << automata->formula() << "<<<" << std::endl;
        result->add_violations(vio);
      }
    }

    // Call LTL checker logic on request->trace()
    result->set_satisfied(all_satisfied); // mock
    return Status::OK;
  }

private:
  std::vector<lfz::automata::Automata *> autometa_list_;
};
