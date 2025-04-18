#include <cassert>
#include <fstream>
#include <spot/parseaut/public.hh>
#include <spot/tl/exclusive.hh>
#include <spot/tl/formula.hh>
#include <spot/tl/parse.hh>
#include <spot/twa/bdddict.hh>
#include <spot/twa/bddprint.hh>
#include <spot/twaalgos/hoa.hh>
#include <spot/twaalgos/translate.hh>

#include <automata.h>
#include <string>
#include <vector>

void custom_print(std::ostream &out, const spot::twa_graph_ptr &aut,
                  const spot::formula pf);

namespace lfz {
namespace automata {

static const std::string DELIMETER = ",";
static bool check_cond_res;
static bool is_true_cond_res;
static const std::set<int> *check_cond_events;
static std::vector<std::vector<std::pair<int, bool>>> transition_cond;

static void is_true_cond_helper(char *cond, int size) {
  for (int i = 0; i < size; i++) {
    if (cond[i] >= 0) {
      is_true_cond_res = false;
      return;
    }
  }
  is_true_cond_res = true;
}

static void check_cond_helper(char *cond, int size) {
  int n_true = 0;

  for (int i = 0; i < size; i++) {
    if (cond[i] == 1) {
      n_true++;
    }
  }
  int n_match = 0;
  for (int index : *check_cond_events) {
    assert(index < size);
    if (cond[index] == 0) {
      check_cond_res = false;
      return;
    }
    if (cond[index] == 1) {
      n_match++;
    }
  }
  assert(n_match <= n_true);
  check_cond_res = n_match == n_true;
}

static void get_cond_helper(char *cond, int size) {
  std::vector<std::pair<int, bool>> conditions;
  for (int i = 0; i < size; i++) {
    if (cond[i] == 0) {
      conditions.push_back(std::make_pair(i, false));
    } else if (cond[i] == 1) {
      conditions.push_back(std::make_pair(i, true));
    }
  }
  transition_cond.push_back(conditions);
}

AutomataException::AutomataException(const std::string &msg) noexcept {
  this->msg_.append("[Automata Error] ");
  this->msg_.append(msg);
}

AutomataException::AutomataException(const AutomataException &e) noexcept
    : msg_(e.msg_) {}

AutomataException &
AutomataException::operator=(const AutomataException &e) noexcept {
  this->msg_ = e.msg_;
  return *this;
}

AutomataException::~AutomataException() {}

const char *AutomataException::what() const noexcept {
  return this->msg_.c_str();
}

State::State(const spot::state *state, state_num_t state_num)
    : state_(state), state_num_(state_num) {}

State::State(State &&s) : state_(s.state_), state_num_(s.state_num_) {
  s.state_ = nullptr;
}

State &State::operator=(State &&s) {
  if (this != &s) {
    if (this->state_ != nullptr) {
      this->state_->destroy();
    }
    this->state_ = s.state_;
    this->state_num_ = s.state_num_;
    s.state_ = nullptr;
  }
  return *this;
}

State::~State() {
  if (this->state_ != nullptr) {
    this->state_->destroy();
  }
}

const spot::state *State::state() const { return this->state_; }

state_num_t State::state_num() const { return this->state_num_; }

Iterator::Iterator(spot::twa_succ_iterator *iter,
                   spot::const_twa_graph_ptr graph,
                   const std::unordered_map<std::string, int> *bdd_map,
                   const std::unordered_map<int, std::string> *reverse_bdd_map)
    : iter_(iter), graph_(graph), bdd_map_(bdd_map),
      reverse_bdd_map_(reverse_bdd_map) {}

Iterator::Iterator(Iterator &&i)
    : iter_(i.iter_), graph_(i.graph_), bdd_map_(i.bdd_map_),
      reverse_bdd_map_(i.reverse_bdd_map_) {
  i.iter_ = nullptr;
}

Iterator &Iterator::operator=(Iterator &&i) {
  if (this != &i) {
    delete this->iter_;
    this->iter_ = i.iter_;
    this->graph_ = i.graph_;
    this->bdd_map_ = i.bdd_map_;
    this->reverse_bdd_map_ = i.reverse_bdd_map_;
    i.iter_ = nullptr;
  }
  return *this;
}

Iterator::~Iterator() { delete this->iter_; }

bool Iterator::first() { return this->iter_->first(); }

bool Iterator::next() { return this->iter_->next(); }

bool Iterator::done() const { return this->iter_->done(); }

State Iterator::dst() const {
  const spot::state *st = this->iter_->dst();
  state_num_t num = this->graph_->state_number(st);
  return State(st, num);
}

bool Iterator::is_true_cond() const {
  bdd_allsat(this->iter_->cond(), is_true_cond_helper);
  return is_true_cond_res;
}

bool Iterator::check_cond(const std::set<std::string> &events) const {
  std::set<int> event_indices;
  for (const auto &e : events) {
    if (this->bdd_map_->count(e) > 0) {
      event_indices.insert(this->bdd_map_->at(e));
    }
  }
  check_cond_events = &event_indices;
  bdd_allsat(this->iter_->cond(), check_cond_helper);
  return check_cond_res;
}

void Iterator::get_cond(std::vector<std::set<std::string>> &events) const {
  transition_cond.clear();
  bdd_allsat(this->iter_->cond(), get_cond_helper);
  for (const auto &sat : transition_cond) {
    std::set<std::string> s;
    for (const auto &e : sat) {
      assert(this->reverse_bdd_map_->count(e.first) > 0);
      if (e.second) {
        s.insert(this->reverse_bdd_map_->at(e.first));
      } else {
        std::string ne = "!";
        s.insert(ne.append(this->reverse_bdd_map_->at(e.first)));
      }
    }
    events.push_back(s);
  }
}

MCState::MCState(int state, int distance, bool acceptance)
    : state(state), distance(distance), acceptance(acceptance) {}

Automata::Automata() {}

Automata::Automata(const std::string &formula, const std::string &exclusive) {
  set_formula(formula, exclusive);
}

Automata::Automata(const Automata &a) : automata_(a.automata_) {}

Automata &Automata::operator=(const Automata &a) {
  this->automata_ = a.automata_;
  return *this;
}

Automata::~Automata() {}

spot::formula Automata::formula() const { return pf.f; }

void Automata::set_formula(const std::string &formula,
                           const std::string &exclusive) {
  // std::cout << "Set formula: >" << formula << "<" << std::endl;
  // std::cout << "Set exclusive: >" << exclusive << "<" << std::endl;
  std::lock_guard<std::mutex> lock(mtx_);

  std::ostringstream error;
  pf = spot::parse_infix_psl(formula);
  if (pf.format_errors(error)) {
    throw AutomataException("Failed to parse formula " + formula + '\n' +
                            error.str());
  }

  std::cout << "Parsed formula: " << pf.f << std::endl;

  spot::translator translator;
  translator.set_type(spot::postprocessor::BA);
  translator.set_pref(spot::postprocessor::SBAcc);
  this->automata_ = translator.run(pf.f);

  // spot::print_hoa(std::cout, this->automata_) << "\n";

  /*
   * Add exclusive APs constraint
   */
  std::vector<spot::formula> excl_formulas;
  size_t next = 0, last = 0;
  while ((next = exclusive.find(DELIMETER, last)) != std::string::npos) {
    excl_formulas.push_back(
        spot::parse_infix_psl(exclusive.substr(last, next - last)).f);
    last = next + 1;
  }
  if (last < exclusive.length()) {
    excl_formulas.push_back(spot::parse_infix_psl(exclusive.substr(last)).f);
  }

  if (!excl_formulas.empty()) {
    spot::exclusive_ap excl;
    excl.add_group(excl_formulas);
    this->automata_ = excl.constrain(this->automata_, true);
  }

  custom_print(std::cout, this->automata_, this->pf.f);

  // std::cout << "After Exclude" << std::endl;
  // custom_print(std::cout, this->automata_, this->pf.f);

  /* Construct BDD variable map */
  this->bdd_map_.clear();
  this->reverse_bdd_map_.clear();
  for (const auto &kv : this->automata_->get_dict()->var_map) {
    this->bdd_map_[kv.first.ap_name()] = kv.second;
    this->reverse_bdd_map_[kv.second] = kv.first.ap_name();
  }

  if (!automata_->prop_state_acc())
    throw AutomataException("Automaton is not using state-based acceptance!");

  if (!automata_->acc().is_buchi())
    throw AutomataException("Automaton is not Büchi!");
}

bool Automata::valid() const { return this->automata_.get() != nullptr; }

State Automata::get_init_state() const {
  const spot::state *st = this->automata_->get_init_state();
  state_num_t num = this->automata_->get_init_state_number();
  return State(st, num);
}

Iterator Automata::get_iterator(const State &state) const {
  spot::twa_succ_iterator *iter = this->automata_->succ_iter(state.state());
  return Iterator(iter, this->automata_, &this->bdd_map_,
                  &this->reverse_bdd_map_);
}

// void Automata::checkrun(std::vector<std::string> &events, bool &acc) {
//   std::vector<bdd> trace_bdds;
//   for (const auto &e : events) {
//     bdd label = bddfalse;
//     auto it = bdd_map_.find(e);
//     if (it != bdd_map_.end()) {
//       label = automata_->get_dict()->bdd_ithvar(it->second);
//     }
//     trace_bdds.push_back(label);
//   }
//
//   auto run = spot::accepting_run(automata_, trace_bdds);
//   if (run) {
//     std::cout << "Trace is ACCEPTED by the automaton (Spot-level)."
//               << std::endl;
//   } else {
//     std::cout << "Trace is REJECTED by the automaton." << std::endl;
//   }
// }

void Automata::model_check_events(const std::vector<std::string> &events,
                                  std::vector<MCState> &states) const {

  std::lock_guard<std::mutex> lock(mtx_);

  std::cout << "--------------------- Model Checking --------------------"
            << std::endl;
  // std::cout << "\tTrace:";
  // for (const auto &e : events) {
  //   std::cout << " " << e;
  // }
  // std::cout << std::endl;

  std::cout << "\tLTL: " << this->formula() << std::endl;
  // std::cout << ">>>>>>>>>>>> Automaton:" << std::endl;
  // print_hoa(std::cout, this->automata_) << "\n";
  // custom_print(std::cout, this->automata_, this->pf.f);

  states.clear();
  State s = get_init_state();

  // std::cout << "Initial state: " << s.state_num() << std::endl;

  for (const auto &e : events) {
    bool cond_sat = false;
    Iterator it = get_iterator(s);
    for (it.first(); !it.done(); it.next()) {
      // std::cout << "Transition: " << it.dst().state_num() << std::endl;
      if (it.is_true_cond()) {
        // Condition is TRUE. Only make this transition if
        // there is no other satisfied transitions.
        cond_sat = true;
        s = it.dst();
      } else {
        /*
         * Currently only check single event
         */
        std::set<std::string> es;
        es.insert(e);
        if (it.check_cond(es)) {
          cond_sat = true;
          s = it.dst();
          break;
        }
      }
    }

    if (cond_sat) {
      bool accepting = this->automata_->state_is_accepting(s.state());
      states.push_back(MCState(s.state_num(), 0, accepting));
    } else {
      states.push_back(MCState(-1, 0, false));
      break;
    }
  }

  std::cout << "\tStates: ";
  for (auto &s : states) {
    std::cout << "{s:" << s.state << ", acc:" << s.acceptance << "} ";
  }

  std::cout << std::endl;
  // std::cout << "====================" << std::endl;
}

void Automata::get_state_transitions(int state,
                                     transitions_t &transitions) const {
  transitions.clear();
  // Get state iterator
  State s(this->automata_->state_from_number(state), state);
  Iterator it = get_iterator(s);

  // Iterate through all transitions
  for (it.first(); !it.done(); it.next()) {
    std::pair<std::vector<std::set<std::string>>, int> t;
    it.get_cond(t.first);
    t.second = it.dst().state_num();
    transitions.push_back(t);
  }
}

void Automata::get_state_events(int curState, int nextState,
                                events_t &events) const {
  State s(this->automata_->state_from_number(curState), curState);
  Iterator it = get_iterator(s);
  for (it.first(); !it.done(); it.next()) {
    int tempState = it.dst().state_num();
    if (tempState == nextState) {
      it.get_cond(events);
      break;
    }
  }
}

void Automata::get_state_set(int curState, stateSet_t &stateSet) const {
  stateSet.clear();
  State s(this->automata_->state_from_number(curState), curState);
  Iterator it = get_iterator(s);

  for (it.first(); !it.done(); it.next()) {
    int nextState = it.dst().state_num();
    if (nextState != curState) {
      stateSet.insert(nextState);
    }
  }
}

void Automata::find_paths(std::stack<int> &sstack, paths_t &paths,
                          std::vector<int> &isVisited,
                          std::stack<int> &path) const {
  if (sstack.empty()) {
    return;
  }

  int stop = sstack.top();
  isVisited[stop] = 1;
  path.push(stop);

  stateSet_t stateSet;
  get_state_set(stop, stateSet);

  for (stateSet_t::iterator it = stateSet.begin(); it != stateSet.end(); it++) {
    bool accepting = this->automata_->state_is_accepting(*it);
    if (accepting) {
      path.push(*it);
      paths.push_back(path);
      path.pop();
    } else {
      if (isVisited[*it] == 0) {
        sstack.push(*it);
        find_paths(sstack, paths, isVisited, path);
      }
    }
  }

  int pt = sstack.top();
  sstack.pop();
  path.pop();
  isVisited[pt] = 0;
  return;
}

void Automata::get_state_paths(int curState, paths_t &paths,
                               std::vector<int> aPath) const {
  std::vector<int> isVisited(10);
  for (size_t i = 0; i < aPath.size(); i++) {
    isVisited[i] = 1;
  }

  std::stack<int> sstack;
  sstack.push(curState);

  std::stack<int> path;

  find_paths(sstack, paths, isVisited, path);
}

} // namespace automata
} // namespace lfz
//

void custom_print(std::ostream &out, const spot::twa_graph_ptr &aut,
                  const spot::formula f) {
  // We need the dictionary to print the BDDs that label the edges
  const spot::bdd_dict_ptr &dict = aut->get_dict();

  // Some meta-data...
  out << ">>>>>>>>>>>>>>>>>>>>>>>>>> Cutom Print Start" << "\n";
  out << "LTL: " << f << "\n";
  out << "Acceptance: " << aut->get_acceptance() << '\n';
  out << "Number of sets: " << aut->num_sets() << '\n';
  out << "Number of states: " << aut->num_states() << '\n';
  out << "Number of edges: " << aut->num_edges() << '\n';
  out << "Initial state: " << aut->get_init_state_number() << '\n';
  out << "Atomic propositions:";
  for (spot::formula ap : aut->ap())
    out << ' ' << ap << " (=" << dict->varnum(ap) << ')';
  out << '\n';

  // Arbitrary data can be attached to automata, by giving them
  // a type and a name.  The HOA parser and printer both use the
  // "automaton-name" to name the automaton.
  if (auto name = aut->get_named_prop<std::string>("automaton-name"))
    out << "Name: " << *name << '\n';

  // For the following prop_*() methods, the return value is an
  // instance of the spot::trival class that can represent
  // yes/maybe/no.  These properties correspond to bits stored in the
  // automaton, so they can be queried in constant time.  They are
  // only set whenever they can be determined at a cheap cost: for
  // instance an algorithm that always produces deterministic automata
  // would set the deterministic property on its output.  In this
  // example, the properties that are set come from the "properties:"
  // line of the input file.
  out << "Complete: " << aut->prop_complete() << '\n';
  out << "Deterministic: " << (aut->prop_universal() && aut->is_existential())
      << '\n';
  out << "Unambiguous: " << aut->prop_unambiguous() << '\n';
  out << "State-Based Acc: " << aut->prop_state_acc() << '\n';
  out << "Terminal: " << aut->prop_terminal() << '\n';
  out << "Weak: " << aut->prop_weak() << '\n';
  out << "Inherently Weak: " << aut->prop_inherently_weak() << '\n';
  out << "Stutter Invariant: " << aut->prop_stutter_invariant() << '\n';

  // States are numbered from 0 to n-1
  unsigned n = aut->num_states();
  for (unsigned s = 0; s < n; ++s) {
    out << "State " << s << ":\n";

    // The out(s) method returns a fake container that can be
    // iterated over as if the contents was the edges going
    // out of s.  Each of these edges is a quadruplet
    // (src,dst,cond,acc).  Note that because this returns
    // a reference, the edge can also be modified.
    for (auto &t : aut->out(s)) {
      out << "  edge(" << t.src << " -> " << t.dst << ")\n    label = ";
      spot::bdd_print_formula(out, dict, t.cond);
      out << "\n    acc sets = " << t.acc << '\n';
    }
  }
  out << "<<<<<<<<<<<<<<<<<<<<<<<<< Custom Print End " << "\n";
}
