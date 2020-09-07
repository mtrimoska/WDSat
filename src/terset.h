//
//  terset.h
//  WDSat
//
//  Created by Gilles Dequen and Monika Trimoska on 15/12/2018.
//  Copyright © 2018 Gilles Dequen. All rights reserved.
//

#ifndef __terset_h__
#define __terset_h__

#include <stdio.h>
#include <stdbool.h>

#include "wdsat_utils.h"

extern boolean_t terset_assignment_buffer[__SIGNED_ID_SIZE__];
extern boolean_t *terset_assignment;

// undo structures
extern int_t terset_history[__ID_SIZE__];
extern int_t terset_history_top;
extern int_t terset_step[__ID_SIZE__];
extern int_t terset_step_top;


// unit propagation stack
extern int_t terset_up_stack[__ID_SIZE__];
extern int_t terset_up_top_stack;


#define _terset_set(_v, _tv) \
{ \
    terset_assignment[_v] = (boolean_t) _tv; \
    terset_assignment[-_v] = (boolean_t) _tv ^ (boolean_t) __TRUE__; \
}

#define _terset_unset(_v) \
{ \
    terset_assignment[_v] = terset_assignment[-_v] = __UNDEF__; \
}

#define _terset_is_true(_v) (terset_assignment[_v] & (boolean_t) 1)
#define _terset_is_false(_v) (!terset_assignment[_v])
#define _terset_is_undef(_v) (terset_assignment[_v] & 2)

/// @def terset_breakpoint
/// @brief set a breakpoint during resolution
#define _terset_breakpoint terset_step[terset_step_top++] = terset_history_top;

/// @def terset_mergepoint
/// @brief merge last pushed context to previous one
#define _terset_mergepoint terset_step_top && --terset_step_top

bool terset_initiate_from_dimacs(void);
void terset_fprint(void);
bool terset_infer(void);
bool terset_set_true(const int_t l);
void terset_undo(void);
int_t terset_last_assigned_breakpoint(int_t *up_stack);
int_t terset_last_assigned(int_t *up_stack);
const int_t terset_number_of_assigned_variables(void);
void terset_occurrence(void);
int_t terset_occurrence_binary(int_t l);
int_t terset_occurrence_ternary(int_t l);
bool terset_set_unitary(void);
#endif /* terset_h */
