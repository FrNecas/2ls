/*******************************************************************\

Module: ACDL Solver

Author: Rajdeep Mukherjee

\*******************************************************************/


#include <langapi/language_util.h>
#include <util/find_symbols.h>
#include "acdl_solver.h"
#include "acdl_domain.h"

#define DEBUG


#ifdef DEBUG
#include <iostream>
#endif

/*******************************************************************\

Function: acdl_solvert::operator()

  Inputs:

 Outputs:

 Purpose:

 \*******************************************************************/


void
acdl_solvert::initialize_worklist (const local_SSAt &SSA, worklistt &worklist)
{
  // check for equalitites or constraints or next node
  if (SSA.nodes.empty ())
    return;
  assert(!SSA.nodes.front ().equalities.empty ());
  // insert the first element on to the worklist
  worklist.insert (SSA.nodes.front ().equalities.front ());
  #ifdef DEBUG
  std::cout << "The first statement of worklist is " << from_expr (SSA.ns, "", SSA.nodes.front().equalities.front ()) << std::endl;
  #endif
}

/*******************************************************************\

Function: acdl_solvert::check_statement()

  Inputs:

 Outputs:

 Purpose:

 \*******************************************************************/

bool
acdl_solvert::check_statement (const exprt &expr,
                               const acdl_domaint::varst &vars)
{

  std::set<symbol_exprt> symbols;
  // find all variables in a statement
  find_symbols (expr, symbols);
  // check if vars appears in the symbols set,
  // if there is a non-empty intersection, then insert the
  // equality statement in the worklist
  for (acdl_domaint::varst::const_iterator it = vars.begin ();
      it != vars.end (); it++)
  {
    if (symbols.find (*it) != symbols.end ())
    {
      return true;
    }
  }
  return false;
}


/*******************************************************************\

Function: acdl_solvert::update_worklist()

  Inputs:

 Outputs:

 Purpose:

 \*******************************************************************/


void
acdl_solvert::update_worklist (const local_SSAt &SSA,
                               const acdl_domaint::varst &vars,
                               worklistt &worklist,
                               const acdl_domaint::statementt &current_statement)
{
  // dependency analysis loop for equalities
  for (local_SSAt::nodest::const_iterator n_it = SSA.nodes.begin ();
      n_it != SSA.nodes.end (); n_it++)
  {

    for (local_SSAt::nodet::equalitiest::const_iterator e_it =
        n_it->equalities.begin (); e_it != n_it->equalities.end (); e_it++)
    {
      // the statement has already been processed, so no action needed
      if(*e_it == current_statement) continue;

      if (check_statement (*e_it, vars)) {
        worklist.insert (*e_it);
        #ifdef DEBUG
        std::cout << "The statement that is inserted in worklist is an equality: " << from_expr (SSA.ns, "", *e_it) << std::endl;
        #endif
      }
    }
    for (local_SSAt::nodet::constraintst::const_iterator e_it =
        n_it->constraints.begin (); e_it != n_it->constraints.end (); e_it++)
    {
      if(*e_it == current_statement) continue;
      if (check_statement (*e_it, vars)) {
        worklist.insert (*e_it);
        #ifdef DEBUG
        std::cout << "The statement that is inserted in worklist is a constraint: " << from_expr (SSA.ns, "", *e_it) << std::endl;
        #endif
      }
    }
    for (local_SSAt::nodet::assertionst::const_iterator e_it =
        n_it->assertions.begin (); e_it != n_it->assertions.end (); e_it++)
    {
      if(*e_it == current_statement) continue;
      if (check_statement (*e_it, vars)) {
        worklist.insert (not_exprt (*e_it));
        #ifdef DEBUG
        std::cout << "The statement that is inserted in worklist is an assertion: " << from_expr (SSA.ns, "", *e_it) << std::endl;
        #endif
      }
    }
  }
}


/*******************************************************************
 Function: acdl_solvert::select_vars()

 Inputs:

 Outputs:

 Purpose:

 \*******************************************************************/

void
acdl_solvert::select_vars (const exprt &statement, acdl_domaint::varst &vars)
{
  // If it is an equality, then select the lhs for post-condition computation
  exprt lhs;
  if (statement.id () == ID_equal)
  {
    lhs = to_equal_expr (statement).lhs ();
    if (lhs.id () == ID_symbol)
    {
      #ifdef DEBUG
      std::cout << "The symbol that is selected now is " << lhs << std::endl;
      #endif
      vars.push_back (to_symbol_expr (lhs));
    }
    else
      assert(false);
  }
  else // for constraints
  {
    #ifdef DEBUG
    std::cout << "The symbols are pushed from a constraint " << std::endl;
    #endif
    std::set<symbol_exprt> symbols;
    find_symbols(statement,symbols);
    vars.insert(vars.end(),symbols.begin(), symbols.end());
  }
}
/*******************************************************************\

Function: acdl_solvert::operator()

  Inputs:

 Outputs:

 Purpose: Worklist algorithm sketch 
 list<statementt> worklist;
 valuet v = true_exprt();
 // Initialize worklist
 // wl <-- first_statement in localSSA.nodes;
 do {
  s = worklist_pop();
  post(s,v); // this will update v
  // Find statements where s.lhs appears in RHS of SSA nodes, insert the whole statement in worklist
  // To do this, iterate over the localSSA.nodes and collect all these statements
   populate_worklist(s.lhs); 
 } while(worklist != 0);

 In ACDCL, we do gfp computation, so we start with TOP and perform
 forward abstract analysis to compute the post-condition of a statement
\************************************************************************/

property_checkert::resultt acdl_solvert::operator()(const local_SSAt &SSA)
{
  unsigned iteration_number=0;
  bool change;

  worklistt worklist;
  acdl_domaint::valuet v = true_exprt();
  
  initialize_worklist(SSA, worklist);

#if 1
  exprt expression;
  std::list<acdl_domaint::statementt> equalities_expr;
   std::list<acdl_domaint::statementt> constraints_expr;
   std::list<acdl_domaint::statementt> assertions_expr;
  // collect all equalities, constraints and assertions
  for(local_SSAt::nodest::const_iterator n_it = SSA.nodes.begin();
      n_it != SSA.nodes.end(); n_it++) {
    for(local_SSAt::nodet::equalitiest::const_iterator e_it =
	 	  n_it->equalities.begin(); e_it != n_it->equalities.end(); e_it++) {
       expression = *e_it;
       assert(e_it->id()==ID_equal);
       //std::cout<< "The expression is " << e_it->pretty() << std::endl;
       equalities_expr.push_back(expression);
    }
    for(local_SSAt::nodet::assertionst::const_iterator a_it =
    	  n_it->assertions.begin(); a_it != n_it->assertions.end(); a_it++) {
         expression = *a_it;
         assertions_expr.push_back(expression);
    }
    
    for(local_SSAt::nodet::constraintst::const_iterator c_it =
    	  n_it->constraints.begin(); c_it != n_it->constraints.end(); c_it++) {
         expression = *c_it;
         constraints_expr.push_back(expression);
    }
  }
#endif
  while (!worklist.empty())
  {
    worklistt::iterator it = worklist.begin ();
    const exprt statement = *it;
    worklist.erase (it);
    #ifdef DEBUG
    std::cout << "The statement just popped from worklist is " << from_expr (SSA.ns, "", statement)
        << std::endl;
    #endif
    acdl_domaint::varst vars;
    std::vector<acdl_domaint::valuet> new_v;
    new_v.resize (1);
    // TODO: this is a workaround to handle booleans,
    //       must be implemented using a product domain
    if (statement.id () == ID_equal
        && to_equal_expr (statement).lhs ().type ().id () == ID_bool)
    {
      new_v[0] = statement;
      // collect variables for dependencies
      std::set<symbol_exprt> symbols;
      find_symbols(statement,symbols);
      vars.insert(vars.end(),symbols.begin(), symbols.end());
    }
    else
    {
      // select vars according to iteration strategy
      select_vars (statement, vars);
      // compute update of abstract value
      domain (statement, vars, v, new_v[0]);
    }
    // terminating condition check for populating worklist
    if(domain.contains(new_v[0], v)) {
      #ifdef DEBUG
      std::cout << "The old value of is " << from_expr (SSA.ns, "", new_v[0])
        << std::endl;
      #endif
      // meet is computed because we are doing gfp
      // v will get the new value of new_v
      domain.meet (new_v, v);
      #ifdef DEBUG
       std::cout << "The new value is " << from_expr (SSA.ns, "", v)
         << std::endl;
      #endif
      update_worklist(SSA, vars, worklist, statement);
    }
  }

  return property_checkert::UNKNOWN;
}
