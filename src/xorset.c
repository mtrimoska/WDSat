//
//  xorset.c
//  WDSat
//
//  Created by Gilles Dequen and Monika Trimoska on 15/12/2018.
//  Copyright © 2018 Gilles Dequen. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "xorset.h"
#include "dimacs.h"
#include "wdsat_utils.h"

//CHECK**
boolean_t xorset_assignment_buffer[__SIGNED_ID_SIZE__];
boolean_t *xorset_assignment;
// number of variables, number of equations
int_t xorset_nb_of_vars;
int_t xorset_nb_of_equations;
// undo structures
int_t xorset_history[__ID_SIZE__];
int_t xorset_history_top;
int_t xorset_step[__ID_SIZE__];
int_t xorset_step_top;

int_t xorset_history_s[__ID_SIZE__ * __MAX_XEQ_SIZE__];
int_t xorset_history_s_top;
int_t xorset_step_s[__ID_SIZE__];
int_t xorset_step_s_top;
int_t xorset_history_u[__ID_SIZE__ * __MAX_XEQ_SIZE__];
int_t xorset_history_u_top;
int_t xorset_step_u[__ID_SIZE__];
int_t xorset_step_u_top;
// how many literals are true/false in
// each equation
int_t xorset_degree_s[__MAX_XEQ__];
int_t xorset_degree_u[__MAX_XEQ__];
int_t xor_equation[__MAX_XEQ__][__MAX_XEQ_SIZE__];
int_t size_of_xor_equation[__MAX_XEQ__];
int_t xorset_occurrence(int_t l);

// unit propagation stack
int_t xorset_up_stack[__ID_SIZE__];
int_t xorset_up_top_stack;

// index of clauses per literals
static int_t xorset_size_of_index_buffer[__SIGNED_ID_SIZE__], *xorset_size_of_index;
static int_t *xorset_index_buffer[__SIGNED_ID_SIZE__], **xorset_index;
static int_t xorset_main_buffer[__MAX_BUFFER_SIZE__];
static int_t xorset_main_top_buffer;

static int_t xorset_history_top_it;

static boolean_t xorset_status;

extern byte_t thread_digits;

//SOLVER ONLY VARIABLES
static bool exists_xor_equation[__MAX_XEQ__];
static int_t const_xor_equation[__MAX_XEQ__];

void xorset_fprint() {
    static int_t i, j, l;
    const int_t n_x = xorset_nb_of_equations;
    const int_t log_n_x = fast_int_log10(xorset_nb_of_equations);
    for(i = 0; i < n_x; ++i) {
        _cid_cout("[%0*ld] s[%ld] u[%ld]:", log_n_x, i, xorset_degree_s[i], xorset_degree_u[i]);
        for(j = 0; j < size_of_xor_equation[i]; ++j) {
            l = xor_equation[i][j];
			printf(" %lld.[%s]", l, _xorset_is_true(l) ? "T" : _xorset_is_false(l) ? "F" : "?");
        }
        printf("\n");
    }
}

/// @fn bool xorset_set_true(const atoms_t v);
/// @brief assign and propagate v to true.
/// @param v is the literal (can be negative) that will be assigned to true
/// @return true if assignment ends right and false if inconsistency occurs
bool xorset_set_true(const int_t v) {
    assert(abs((int) v) <= xorset_nb_of_vars); //CHECK** < to <=
    xorset_up_stack[xorset_up_top_stack++] = v;
    return(xorset_infer());
}

bool xorset_set_unitary(void) {
	return(xorset_infer());
}

int_t xorset_occurrence(int_t l)
{
    return xorset_size_of_index[l];
}

bool xorset_infer() {
    static int_t l, i, j, c, up_l;
    while(xorset_up_top_stack) {
        l = xorset_up_stack[--xorset_up_top_stack];
        if(_xorset_is_true(l)) continue;
        else if(_xorset_is_false(l)) {
            xorset_up_top_stack = 0;
            return(false);
        } else {
            _xorset_set(l, __TRUE__)
            xorset_history[xorset_history_top++] = l;
            // for clauses where _l is assigned to TRUE
            const int_t xt_sz = xorset_size_of_index[l];
            int_t * const idx_t = xorset_index[l];
            for(i = 0; i < xt_sz; ++i) {
                c = idx_t[i];
                ++xorset_degree_s[c];
                xorset_history_s[xorset_history_s_top++] = c;
                const int_t d_s = xorset_degree_s[c];
                const int_t sz_eq = size_of_xor_equation[c];
                const int_t unset_lt = sz_eq - d_s - xorset_degree_u[c];
                /// below is same as 'if(_xorset_clause_unsat(c)) return(false);'
				if((!unset_lt) && (!(d_s & 1LL))) {xorset_up_top_stack = 0; return(false);}
                if(unset_lt == 1) {
                    for(j = 0; j < sz_eq; ++j) {
                        up_l = xor_equation[c][j];
                        if(_xorset_is_undef(up_l)) {
                            xorset_up_stack[xorset_up_top_stack++] = (d_s & 1) ? -up_l : up_l;
							break;
                        }
                    }
                }
            }
            // for clauses where _l is assigned to FALSE
            const int_t xf_sz = xorset_size_of_index[-l];
            int_t * const idx_f = xorset_index[-l];
            for(i = 0; i < xf_sz; ++i) {
                c = idx_f[i];
                ++xorset_degree_u[c];
                xorset_history_u[xorset_history_u_top++] = c; //CHECK** changed from: xorset_history_s[xorset_history_s_top++] = c;
                const int_t d_s = xorset_degree_s[c];
                const int_t sz_eq = size_of_xor_equation[c];
                const int_t unset_lt = sz_eq - d_s - xorset_degree_u[c];
                /// below is same as 'if(_xorset_clause_unsat(c)) return(false);'
				if((!unset_lt) && (!(d_s & 1LL))) {xorset_up_top_stack = 0; return(false);}
                if(unset_lt == 1) {
                    for(j = 0; j < sz_eq; ++j) {
                        up_l = xor_equation[c][j];
                        if(_xorset_is_undef(up_l)) {
                            xorset_up_stack[xorset_up_top_stack++] = (d_s & 1) ? -up_l : up_l;
							break;
                        }
                    }
                }
            }
        }
    }
    // is it SAT
    if(xorset_history_top == xorset_nb_of_vars) xorset_status = __TRUE__;
    return(true);
}

void xorset_set_deg(int_t l) {
	static int_t i, c;
			_xorset_set(l, __TRUE__)
			const int_t xt_sz = xorset_size_of_index[l];
			int_t * const idx_t = xorset_index[l];
			for(i = 0; i < xt_sz; ++i) {
				c = idx_t[i];
				++xorset_degree_s[c];
			}
			// for clauses where _l is assigned to FALSE
			const int_t xf_sz = xorset_size_of_index[-l];
			int_t * const idx_f = xorset_index[-l];
			for(i = 0; i < xf_sz; ++i) {
				c = idx_f[i];
				++xorset_degree_u[c];
			}
}

bool xorset_initiate_from_dimacs() {
    static int_t i, j, sz;
    const int_t _n_v = dimacs_nb_vars();
    const int_t _n_x = dimacs_nb_xor_equations();
    xorset_nb_of_equations = _n_x;
    xorset_nb_of_vars = _n_v;
    assert(xorset_nb_of_vars <= __MAX_ID__);
    assert(xorset_nb_of_equations <= __MAX_XEQ__);

    xorset_up_top_stack = 0LL;
    xorset_history_top = xorset_history_s_top = xorset_history_u_top = xorset_history_top_it = 0LL;
    xorset_step_top = xorset_step_s_top = xorset_step_u_top = 0LL;
    xorset_assignment = xorset_assignment_buffer + _n_v + 1LL;
    xorset_size_of_index = xorset_size_of_index_buffer + _n_v + 1LL;
    xorset_index = xorset_index_buffer + _n_v + 1LL;
    
    /// all sat degrees to 0
    for(i = 0; i < _n_x; ++i) xorset_degree_s[i] = xorset_degree_u[i] = 0LL;
    /// all variables are unassigned
    /// no indexed clause per variable
    xorset_assignment[0LL] = __UNDEF__;
    xorset_size_of_index[0LL] = 0;
    xorset_index[0LL] = NULL;
    for(i = 1LL; i <= _n_v; ++i) {
        _xorset_unset(i);
        xorset_size_of_index[i] = xorset_size_of_index[-i] = 0LL;
    }
    /// temporary structures for indexing clauses
    int_t **tmp_idx = NULL, **_tmp_idx;
    _get_mem(int_t *, tmp_idx, ((xorset_nb_of_vars + 1LL) << 1LL));
    _tmp_idx = tmp_idx + xorset_nb_of_vars + 1LL;
    for(i = 1LL; i <= _n_v; ++i) {
        _tmp_idx[i] = (int_t *) NULL;
        _tmp_idx[-i] = (int_t *) NULL;
        _reset_int_vector(_tmp_idx[i]);
        _reset_int_vector(_tmp_idx[-i]);
    }
    /// building temporary indexes
    int_t * const _dimacs_size_of_xor_equations = dimacs_size_of_xor_equations();
    for(i = 0LL; i < _n_x; ++i) {
        int_t * const _dimacs_xor = dimacs_xor(i);
        sz = _dimacs_size_of_xor_equations[i];
        switch (sz) {
            case 1:
                xorset_up_stack[xorset_up_top_stack++] = _dimacs_xor[0];
                assert(xorset_up_top_stack < __ID_SIZE__);
                continue;
            default:
                for(j = 0; j < sz; ++j) {
                    _int_vector_append(_tmp_idx[_dimacs_xor[j]], i);
                }
                break;
        }
    }
    /// from temporary to static structures
    xorset_main_top_buffer = 0LL;
    for(i = 1LL; i <= xorset_nb_of_vars; ++i) {
        xorset_size_of_index[i] = sz = _int_vector_size(_tmp_idx[i]);
        xorset_index[i] = xorset_main_buffer + xorset_main_top_buffer;
        for(j = 0LL; j < sz; ++j) {
            xorset_main_buffer[xorset_main_top_buffer++] = _tmp_idx[i][j];
            assert(xorset_main_top_buffer < __MAX_BUFFER_SIZE__);
        }
        xorset_size_of_index[-i] = sz = _int_vector_size(_tmp_idx[-i]);
        xorset_index[-i] = xorset_main_buffer + xorset_main_top_buffer;
        for(j = 0LL; j < sz; ++j) {
            xorset_main_buffer[xorset_main_top_buffer++] = _tmp_idx[-i][j];
            assert(xorset_main_top_buffer < __MAX_BUFFER_SIZE__);
        }
    }
    /// freeing temporary structures
    for(i = 1LL; i <= xorset_nb_of_vars; ++i) {
        _free_int_vector(_tmp_idx[i]);
        _free_int_vector(_tmp_idx[-i]);
    }
    _free_mem(tmp_idx);
    xorset_status = __UNDEF__;
    
    for(i = 0; i < xorset_nb_of_equations; ++i) {
        exists_xor_equation[i] = true;
        const_xor_equation[i] = 1;
    }
    return(true);
}

void xorset_undo() {
    static int_t _l;
    const int_t top_step = (xorset_step_top) ? xorset_step[--xorset_step_top] : 0;
    while(xorset_history_top != top_step) {
        _l = xorset_history[--xorset_history_top];
        _xorset_unset(_l);
    }
	xorset_history_top_it = xorset_history_top;
    const int_t top_step_s = (xorset_step_s_top) ? xorset_step_s[--xorset_step_s_top] : 0;
    while(xorset_history_s_top != top_step_s)
        --xorset_degree_s[xorset_history_s[--xorset_history_s_top]];
    const int_t top_step_u = (xorset_step_u_top) ? xorset_step_u[--xorset_step_u_top] : 0;
    while(xorset_history_u_top != top_step_u)
        --xorset_degree_u[xorset_history_u[--xorset_history_u_top]];
}

//CHECK**
/// @fn int_t xorset_last_assigned_breakpoint(int_t *up_stack);
/// @brief return the list of literals that have been assigned since
/// the last breakpoint
/// @param up_stack is the list that will be filled
/// @return the last element in the stack 
int_t xorset_last_assigned_breakpoint(int_t *up_stack) {
	int_t up_stack_top = 0LL;
	int_t _l;
	xorset_history_top_it = (xorset_step_top) ? xorset_step[xorset_step_top - 1LL] : 0;
	while(xorset_history_top_it < xorset_history_top) {
		_l = xorset_history[xorset_history_top_it++];
		up_stack[up_stack_top++] = _l;
	}
	return up_stack_top;
}

//CHECK**
/// @fn int_t xorset_last_assigned(int_t *up_stack);
/// @brief return the list of literals that have been assigned since
/// the last time this function (or the other last_assigned) was called
/// @param up_stack is the list that will be filled
/// @return the last element in the stack
int_t xorset_last_assigned(int_t *up_stack) {
	int_t up_stack_top = 0LL;
	int_t _l;
	while(xorset_history_top_it < xorset_history_top) {
		_l = xorset_history[xorset_history_top_it++];
		up_stack[up_stack_top++] = _l;
	}
	return up_stack_top;
}

/// @fn const int_t xorset_number_of_assigned_variables();
/// @brief return the number of assigned variables.
inline const int_t xorset_number_of_assigned_variables() { return xorset_history_top; }

void cpy_from_dimacs()
{
    int_t **_dimacs_xor = get_dimacs_xor_equation();
    int_t i;
    for(i = 0; i < __MAX_XEQ__; i++) {
        size_of_xor_equation[i] = dimacs_size_of_xor(i);
    }
    memcpy(xor_equation, _dimacs_xor, __MAX_XEQ__ * __MAX_XEQ_SIZE__ * sizeof(int_t));
}
