//
//  cnf.h
//  WDSat
//
//  Created by Gilles Dequen and Monika Trimoska on 15/12/2018.
//  Copyright © 2018 Gilles Dequen. All rights reserved.
//

#ifndef __cnf_h__
#define __cnf_h__

#include <stdio.h>
#include <stdbool.h>

#include "wdsat_utils.h"

extern boolean_t cnf_assignment_buffer[__SIGNED_ID_SIZE__];
extern boolean_t *cnf_assignment;

// undo structures
extern int_t cnf_history[__ID_SIZE__];
extern int_t cnf_history_top;
extern int_t cnf_step[__ID_SIZE__];
extern int_t cnf_step_top;


// unit propagation stack
extern int_t cnf_up_stack[__ID_SIZE__];
extern int_t cnf_up_top_stack;


#define _cnf_set(_v, _tv) \
{ \
    cnf_assignment[_v] = (boolean_t) _tv; \
    cnf_assignment[-_v] = (boolean_t) _tv ^ (boolean_t) __TRUE__; \
}

#define _cnf_unset(_v) \
{ \
    cnf_assignment[_v] = cnf_assignment[-_v] = __UNDEF__; \
}

#define _cnf_is_true(_v) (cnf_assignment[_v] & (boolean_t) 1)
#define _cnf_is_false(_v) (!cnf_assignment[_v])
#define _cnf_is_undef(_v) (cnf_assignment[_v] & 2)

/// @def cnf_breakpoint
/// @brief set a breakpoint during resolution
#define _cnf_breakpoint cnf_step[cnf_step_top++] = cnf_history_top;

/// @def cnf_mergepoint
/// @brief merge last pushed context to previous one
#define _cnf_mergepoint cnf_step_top && --cnf_step_top

bool cnf_initiate_from_dimacs(void);
void cnf_fprint(void);
bool cnf_infer(void);
bool cnf_set_true(const int_t l);
void cnf_undo(void);
int_t cnf_last_assigned_breakpoint(int_t *up_stack);
int_t cnf_last_assigned(int_t *up_stack);
const int_t cnf_number_of_assigned_variables(void);
void cnf_occurrence(void);
int_t cnf_occurrence_binary(int_t l);
int_t cnf_occurrence_ternary(int_t l);
bool cnf_set_unitary(void);
#endif /* cnf_h */
