/*******************************************************************\

Module: ACDL Solver

Author: Rajdeep Mukherjee, Peter Schrammel

\*******************************************************************/

#ifndef CPROVER_ACDL_SOLVER_H
#define CPROVER_ACDL_SOLVER_H

#include <util/options.h>
#include <util/find_symbols.h>
#include <goto-programs/property_checker.h>

#include "../ssa/local_ssa.h"

#include "acdl_domain.h"
#include "acdl_decision_heuristics_base.h"
#include "acdl_worklist_base.h"
#include "acdl_conflict_analysis_base.h"

class acdl_solvert : public messaget
{
public:
  
  explicit acdl_solvert(const optionst &_options,
			acdl_domaint &_domain,
			acdl_decision_heuristics_baset &_decision_heuristics,
                        acdl_worklist_baset &_worklist,
                        acdl_conflict_analysis_baset &_conflict_analysis)
    : 
    options(_options),
    domain(_domain),
    decision_heuristics(_decision_heuristics),
    worklist(_worklist),
    conflict_analysis(_conflict_analysis)
    {
    }  

  ~acdl_solvert() 
    {
    }

  property_checkert::resultt operator()(const local_SSAt &SSA,
					const exprt &assertion, const exprt &additional_constraint);

  std::set<exprt> decision_variables;

protected:
  const optionst &options;
  acdl_domaint &domain;
  acdl_decision_heuristics_baset &decision_heuristics;
  acdl_worklist_baset &worklist; 
  acdl_conflict_analysis_baset &conflict_analysis;

  acdl_implication_grapht implication_graph;
  std::vector<acdl_domaint::valuet> learned_clauses;
  unsigned ITERATION_LIMIT=999999; 
  
  property_checkert::resultt propagate(const local_SSAt &SSA, const exprt& assertion);

  bool decide(const local_SSAt &SSA, const exprt& assertion);
  acdl_domaint::varst value_to_vars(const acdl_domaint::valuet &value)
  {
    acdl_domaint::varst vars;
    for(acdl_domaint::valuet::const_iterator it = value.begin(); 
       it != value.end(); it++) {
      std::set<symbol_exprt> symbols;
      find_symbols(*it,symbols);
      for(std::set<symbol_exprt>::const_iterator 
        it1 = symbols.begin(); it1 != symbols.end(); ++it1) 
      vars.insert(*it1);
    }
    return vars;
  }
    
  
  void init();
  bool analyze_conflict(const local_SSAt &SSA, const exprt& assertion);
};


#endif
 
