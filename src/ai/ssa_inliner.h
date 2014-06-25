/*******************************************************************\

Module: SSA Inliner

Author: Peter Schrammel

\*******************************************************************/

#ifndef CPROVER_DELTACHECK_SSA_INLINER_H
#define CPROVER_DELTACHECK_SSA_INLINER_H

#include <util/message.h>

#include "summary_store.h"
#include "../ssa/local_ssa.h"

class summary_storet;

class ssa_inlinert : public messaget
{
 public:
  ssa_inlinert() : counter(0) {}

  void replace(local_SSAt::nodest &nodes,
	       local_SSAt::nodest::iterator node,
               local_SSAt::nodet::equalitiest::iterator equ_it, 
	       const local_SSAt::var_sett &cs_globals_in, //incoming globals at call site
	       const local_SSAt::var_sett &cs_globals_out, //outgoing globals at call site
               const summaryt &summary);

  void replace(local_SSAt &SSA,
               const summary_storet &summary_store);

  void replace(local_SSAt::nodest &nodes,
	       local_SSAt::nodest::iterator node, 
               local_SSAt::nodet::equalitiest::iterator equ_it, 
	       const local_SSAt::var_sett &cs_globals_in, //incoming globals at call site
	       const local_SSAt::var_sett &cs_globals_out, //outgoing globals at call site
               const local_SSAt &function);

  void replace(local_SSAt &SSA,
               const std::map<irep_idt, local_SSAt*> &functions, 
               bool recursive=false,
               bool rename=true);

  void havoc(local_SSAt::nodet &node, 
	     local_SSAt::nodet::equalitiest::iterator &equ_it);

  //apply changes to node, must be called after replace and havoc
  void commit_node(local_SSAt::nodest::iterator node);
  void commit_nodes(local_SSAt::nodest &nodes);

 protected:
  unsigned counter;
  local_SSAt::nodest new_nodes;
  local_SSAt::nodet::equalitiest new_equs;
  std::set<local_SSAt::nodet::equalitiest::iterator> rm_equs;

 private:
  void replace_globals_in(const local_SSAt::var_sett &globals_in, 
                          const local_SSAt::var_sett &globals);
  void replace_params(const local_SSAt::var_listt &params,
                      const function_application_exprt &funapp_expr);
  void replace_globals_out(const local_SSAt::var_sett &globals_out, 
			   const local_SSAt::var_sett &cs_globals_in,  
			   const local_SSAt::var_sett &cs_globals_out);

  void merge_into_nodes(local_SSAt::nodest &nodes, 
			const local_SSAt::locationt &loc, 
                        const local_SSAt::nodet &new_n);

  void rename(exprt &expr);

  bool find_corresponding_symbol(const symbol_exprt &s, 
				 const local_SSAt::var_sett &globals,
                                 symbol_exprt &s_found);

  irep_idt get_original_identifier(const symbol_exprt &s);

};


#endif
