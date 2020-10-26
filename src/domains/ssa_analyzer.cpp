/*******************************************************************\

Module: SSA Analyzer

Author: Peter Schrammel

\*******************************************************************/

/// \file
/// SSA Analyzer

#ifdef DEBUG
#include <iostream>
#endif

#include <solvers/sat/satcheck.h>
#include <solvers/flattening/bv_pointers.h>
#include <util/find_symbols.h>
#include <util/arith_tools.h>
#include <util/simplify_expr.h>
#include <util/mp_arith.h>
#include <util/options.h>
#include <util/expr.h>
#include <goto-programs/goto_program.h>

#include "strategy_solver_base.h"
#include "strategy_solver_binsearch.h"
#include "strategy_solver_binsearch2.h"
#include "strategy_solver_binsearch3.h"
#include "linrank_domain.h"
#include "equality_domain.h"
#include "lexlinrank_domain.h"
#include "predabs_domain.h"
#include "template_generator_ranking.h"
#include "ssa_analyzer.h"
#include "strategy_solver_sympath.h"
#include "strategy_solver_simple.h"
#include "heap_domain.h"
#include "strategy_solver_product.h"

// NOLINTNEXTLINE(*)
#define SIMPLE_SOLVER(domain_ptr, domain_type) strategy_solver_simplet(\
  *dynamic_cast<domain_type *>(domain_ptr), \
  solver, \
  SSA, \
  get_message_handler())
#define BINSEARCH_SOLVER strategy_solver_binsearcht(\
  *dynamic_cast<tpolyhedra_domaint *>(simple_domains[next_domain++]), \
  solver, \
  SSA, \
  get_message_handler())
#if 0
// NOLINTNEXTLINE(*)
#define BINSEARCH_SOLVER strategy_solver_binsearch2t(\
  *static_cast<tpolyhedra_domaint *>(domain), solver, SSA.ns)
// NOLINTNEXTLINE(*)
#define BINSEARCH_SOLVER strategy_solver_binsearch3t(\
  *static_cast<tpolyhedra_domaint *>(domain), solver, SSA, SSA.ns)
#endif

void ssa_analyzert::operator()(
  incremental_solvert &solver,
  local_SSAt &SSA,
  const exprt &precondition,
  template_generator_baset &template_generator)
{
  if(SSA.goto_function.body.instructions.empty())
    return;

  solver << SSA;
  SSA.mark_nodes();

  solver.new_context();
  solver << SSA.get_enabling_exprs();

  // add precondition (or conjunction of asssertion in backward analysis)
  solver << precondition;

  domain=template_generator.domain();

  // Get a strategy solver and a new abstract value (invariant)
  auto s_solver=domain->new_strategy_solver(solver, SSA, get_message_handler());
  result=domain->new_value();

  // initialize inv
  domain->initialize_value(*result);

  // iterate
  while(s_solver->iterate(*result)) {}

  solver.pop_context();

  // statistics
  solver_instances+=s_solver->get_number_of_solver_instances();
  solver_calls+=s_solver->get_number_of_solver_calls();
  solver_instances+=s_solver->get_number_of_solver_instances();

  if(template_generator.options.get_bool_option("export-invariants"))
  {
    std::vector<std::string> invariants=domain->get_invariants(*result);
    export_invariants(SSA, invariants);
  }
}

void ssa_analyzert::get_result(exprt &_result, const var_sett &vars)
{
  domain->project_on_vars(*result, vars, _result);
}

void ssa_analyzert::export_invariants(
    local_SSAt &SSA,
    const std::vector<std::string> &invariants)
{
  goto_programt exported;
  exported.copy_from(SSA.goto_function.body);
  std::cout << "Before\n";
  exported.output(std::cout);
  for(auto const& inv: invariants)
  {
    int node_loc=get_name_node_loc(inv);
    if(node_loc!=-1)
    {
      local_SSAt::nodest::iterator lb_node=SSA.find_node(
        SSA.get_location(static_cast<unsigned>(node_loc)));

      // Get loophead node
      local_SSAt::nodest::iterator lh_node=lb_node->loophead;
      // Find loophead node in exported, making use of uniqueness of
      // location_number attribute across the program to match.
      unsigned location=(*lh_node->location).location_number;
      goto_programt::instructionst::iterator it=exported.instructions.begin();
      for(; it!=exported.instructions.end(); it++)
      {
        if(it->location_number==location)
          break;
      }
      // Insert the assume call
      goto_programt::targett inst=exported.insert_after(it);
      exprt func_expr=exprt();
      code_function_callt func_call;
      func_call.lhs().make_nil();
      func_call.function()=symbol_exprt("__CPROVER_assume", typet());
      func_call.arguments().resize(1);
      func_call.arguments()[0]=predicate_exprt(inv);
      inst->make_function_call(func_call);
    }
  }
  std::cout << "\n\nAfter\n";
  exported.output(std::cout);
}

int ssa_analyzert::get_name_node_loc(const std::string &name)
{
  size_t idx=name.rfind("#lb");
  if(idx==std::string::npos)
    return -1;
  std::string loc_str=name.substr(idx+3);
  assert(!loc_str.empty());
  return std::stoi(loc_str);
}
