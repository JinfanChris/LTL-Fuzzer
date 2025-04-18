// src/grpc/LTLFuzzServiceImpl.cc
#include "automata.h"
#include "automata_handler.h" // Existing logic
#include "ltlfuzz.grpc.pb.h"
#include <exception>
#include <iostream>
#include <iterator>
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

    auto properties = request->properties();
    auto exclude = request->exclude();
    for (int i = 0; i < properties.size(); i++) {
      auto &prop = properties[i];
      auto &excl = exclude[i];

      // std::cout << "Received LTL properties: ";
      // std::cout << " - " << prop << std::endl;
      try {

        lfz::automata::Automata *atm = new lfz::automata::Automata();
        atm->set_formula(prop, excl);
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

    std::cout << "<<<<<<<<<<<<<<<<<<<< Submitted Trace >>>>>>>>>>>>>>>>>>>>"
              << "\n\t" << request->trace() << std::endl;
    std::string trace_str = request->trace();
    // std::cout << "Received trace: " << request->trace() << std::endl;

    std::vector<std::string> trace_events;
    std::stringstream ss(trace_str);
    std::string item;

    while (std::getline(ss, item, ',')) {
      trace_events.push_back(item);
    }

    bool satisfied_one = false;
    for (auto &automata : autometa_list_) {
      std::vector<lfz::automata::MCState> trace_states;
      try {
        automata->model_check_events(trace_events, trace_states);
      } catch (std::exception &e) {
        std::cerr << "Error during model checking: " << e.what() << std::endl;
        result->add_violations("Model checking failed.");
        return Status(grpc::INVALID_ARGUMENT, e.what());
      }
      bool seen_accepting = false;
      bool last_accepting = false;
      int last_state = -1;

      for (const auto &s : trace_states) {
        if (s.acceptance) {
          seen_accepting = true;
        }
        last_state = s.state;
      }

      last_accepting = trace_states.back().acceptance;

      if (last_state == -1) {
        std::cout << "Trace is REJECTED (dead end)." << std::endl;
      } else if (seen_accepting && last_accepting) {
        std::cout << "Trace is ACCEPTED under Büchi (infinite extension)."
                  << std::endl;
        satisfied_one = true;
        std::ostringstream oss;
        oss << automata->formula() << ", ";
        result->add_violations(oss.str());
      } else {
        std::cout << "Trace is NOT accepted under Büchi semantics."
                  << std::endl;
      }
    }

    // Call LTL checker logic on request->trace()
    result->set_satisfied(satisfied_one); // mock
    return Status::OK;
  }

private:
  std::vector<lfz::automata::Automata *> autometa_list_;
};
