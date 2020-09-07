//
//  xorset.h
//  WDSat
//
//  Created by Gilles Dequen and Monika Trimoska on 15/12/2018.
//  Copyright © 2018 Gilles Dequen. All rights reserved.
//

#ifndef __xorset_h__
#define __xorset_h__

#include <stdio.h>
#include <stdbool.h>

#include "wdsat_utils.h"

extern boolean_t xorset_assignment_buffer[__SIGNED_ID_SIZE__];
extern boolean_t *xorset_assignment;
// number of variables, number of equations
extern int_t xorset_nb_of_vars;
extern int_t xorset_nb_of_equations;
// undo structures
extern int_t xorset_history[__ID_SIZE__];
extern int_t xorset_history_top;
extern int_t xorset_step[__ID_SIZE__];
extern int_t xorset_step_top;

extern int_t xorset_history_s[__ID_SIZE__ * __MAX_XEQ_SIZE__];
extern int_t xorset_history_s_top;
extern int_t xorset_step_s[__ID_SIZE__];
extern int_t xorset_step_s_top;
extern int_t xorset_history_u[__ID_SIZE__ * __MAX_XEQ_SIZE__];
extern int_t xorset_history_u_top;
extern int_t xorset_step_u[__ID_SIZE__];
extern int_t xorset_step_u_top;
// how many literals are true/false in
// each equation
extern int_t xorset_degree_s[__MAX_XEQ__];
extern int_t xorset_degree_u[__MAX_XEQ__];
extern int_t xor_equation[__MAX_XEQ__][__MAX_XEQ_SIZE__];
extern int_t size_of_xor_equation[__MAX_XEQ__];
extern int_t xorset_occurrence(int_t l);
extern int_t xorset_up_stack[__ID_SIZE__];
extern int_t xorset_up_top_stack;
#define _xorset_unset(_v) \
{ \
  xorset_assignment[_v] = xorset_assignment[-_v] = __UNDEF__; \
}

#define _xorset_set(_v, _tv) \
{ \
  xorset_assignment[_v] = (boolean_t) _tv; \
  xorset_assignment[-_v] = (boolean_t) _tv ^ (boolean_t) __TRUE__; \
}

#define _xorset_breakpoint \
{ \
  xorset_step[xorset_step_top++] = xorset_history_top; \
  xorset_step_s[xorset_step_s_top++] = xorset_history_s_top; \
  xorset_step_u[xorset_step_u_top++] = xorset_history_u_top; \
}

#define _xorset_mergepoint \
{ \
  xorset_step_top && --xorset_step_top; \
  xorset_step_s_top && --xorset_step_s_top; \
  xorset_step_u_top && --xorset_step_u_top; \
}

#define _xorset_is_true(_v) (xorset_assignment[_v] & (boolean_t) 1)
#define _xorset_is_false(_v) (!xorset_assignment[_v])
#define _xorset_is_undef(_v) (xorset_assignment[_v] & 2)

#define _xorset_clause_sat(_c) ((!(dimacs_size_of_xor(_c) - xorset_degree_s[_c] - xorset_degree_u[_c])) && (xorset_degree_s[_c] & 1))
#define _xorset_clause_unsat(_c) ((!(dimacs_size_of_xor(_c) - xorset_degree_s[_c] - xorset_degree_u[_c])) && (!(xorset_degree_s[_c] & 1)))
 


bool xorset_initiate_from_dimacs(void);
void xorset_fprint(void);
bool xorset_infer(void);
void xorset_undo(void);
bool xorset_set_true(const int_t);
bool xorset_set_unitary(void);
int_t xorset_last_assigned_breakpoint(int_t *up_stack);
int_t xorset_last_assigned(int_t *up_stack);
const int_t xorset_number_of_assigned_variables(void);
void cpy_from_dimacs(void);
int_t neg_undef(int_t *neg, int_t c);
void xorset_set_deg(int_t l);
#endif /* xorset_h */
