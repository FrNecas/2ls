/*******************************************************************\

Module: Summarizer for Forward Analysis

Author: Peter Schrammel

\*******************************************************************/

#ifndef CPROVER_SUMMARIZER_FW_REC_H
#define CPROVER_SUMMARIZER_FW_REC_H

#include <util/message.h>
#include <util/options.h>
#include <util/time_stopping.h>

#include <ssa/ssa_inliner.h>
#include <ssa/ssa_unwinder.h>
#include <ssa/local_ssa.h>
#include <ssa/ssa_db.h>

#include "summarizer_base.h"

class summarizer_fw_rect : public summarizer_baset
{
public:
  explicit summarizer_fw_rect(optionst &_options, 
                              summary_dbt &_summary_db,
                              ssa_dbt &_ssa_db,
                              ssa_unwindert &_ssa_unwinder,
                              ssa_inlinert &_ssa_inliner) : 
  summarizer_baset(_options,_summary_db,_ssa_db,_ssa_unwinder,_ssa_inliner)
  {}

  virtual void summarize(const function_namet &entry_function); 

protected:

  virtual void compute_summary_rec(const function_namet &function_name,
                                   const exprt &precondition,
                                   bool context_sensitive);

  void inline_summaries(const function_namet &function_name, 
                        local_SSAt &SSA,
                        const exprt &precondition,
                        bool context_sensitive); 

  void do_summary(const function_namet &function_name, 
                  local_SSAt &SSA, 
                  summaryt &summary, 
                  exprt cond, //additional constraints
                  bool forward);
};


#endif
