//
//  cnf.c
//  WDSat
//
//  Created by Gilles Dequen and Monika Trimoska on 15/12/2018.
//  Copyright © 2018 Gilles Dequen. All rights reserved.
//

#include "cnf.h"
#include "dimacs.h"
#include "wdsat_utils.h"


// unit propagation stack
int_t cnf_up_stack[__ID_SIZE__];
int_t cnf_up_top_stack;

boolean_t cnf_assignment_buffer[__SIGNED_ID_SIZE__];
boolean_t *cnf_assignment;

// undo structures
int_t cnf_history[__ID_SIZE__];
int_t cnf_history_top;
int_t cnf_step[__ID_SIZE__];
int_t cnf_step_top;
// number of variables, number of equations
static int_t cnf_nb_of_vars;
static int_t cnf_nb_of_equations;

// binary implication lists
static int_t cnf_size_of_binary_implications_buffer[__SIGNED_ID_SIZE__];
static int_t *cnf_size_of_binary_implications;
static int_t *cnf_binary_implication_buffer[__SIGNED_ID_SIZE__];
static int_t **cnf_binary_implication;

// ternary implication lists
static int_t cnf_size_of_ternary_implications_buffer[__SIGNED_ID_SIZE__], *cnf_size_of_ternary_implications;
static int_t *cnf_ternary_implication_buffer[__SIGNED_ID_SIZE__], **cnf_ternary_implication;

// quaternary implication lists
static int_t cnf_size_of_quaternary_implications_buffer[__SIGNED_ID_SIZE__], *cnf_size_of_quaternary_implications;
static int_t *cnf_quaternary_implication_buffer[__SIGNED_ID_SIZE__], **cnf_quaternary_implication;

// main buffer
static int_t cnf_main_top_buffer;
static int_t cnf_main_buffer[__MAX_BUFFER_SIZE__];

static int_t cnf_history_top_it;

static boolean_t cnf_status;

extern byte_t thread_digits;

inline int_t get_cnf_binary_implication(const int_t i, const int_t j) { return cnf_binary_implication[i][j]; }

/// @fn void _print_cnf(FILE *f);
/// @brief Debugging function allowing to visualize internal structures
void cnf_fprint() {
	int_t _i, _j;
    const int_t _n_v = cnf_nb_of_vars;
    
    for(_i = -_n_v; _i <= _n_v; ++_i) {
        if(!_i) continue;
        const int_t _b_sz = cnf_size_of_binary_implications[_i];
        _cid_cout("Binary[%ld].[%s]{#%ld}:", _i, _cnf_is_true(_i) ? "T" : _cnf_is_false(_i) ? "F" : "?", _b_sz);
        for(_j = 0; _j < _b_sz; ++_j) {
            const int_t _l = cnf_binary_implication[_i][_j];
			printf(" %lld.[%s]", _l, _cnf_is_true(_l) ? "T" : _cnf_is_false(_l) ? "F" : "?");
        }
        printf("\n");
    }
    for(_i = -_n_v; _i <= _n_v; ++_i) {
        const int_t _t_sz = cnf_size_of_ternary_implications[_i];
       _cid_cout("Ternary[%ld].[%s]{#%ld}:", _i, _cnf_is_true(_i) ? "T" : _cnf_is_false(_i) ? "F" : "?", _t_sz);
        for(_j = 0; _j < _t_sz; _j += 2) {
            const int_t _l1 = cnf_ternary_implication[_i][_j];
            const int_t _l2 = cnf_ternary_implication[_i][_j + 1];
			printf(" { %lld.[%s] %lld.[%s] }", _l1, _cnf_is_true(_l1) ? "T" : _cnf_is_false(_l1) ? "F" : "?", _l2, _cnf_is_true(_l2) ? "T" : _cnf_is_false(_l2) ? "F" : "?");
        }
        printf("\n");
    }
	for(_i = -_n_v; _i <= _n_v; ++_i) {
		const int_t _t_sz = cnf_size_of_quaternary_implications[_i];
		_cid_cout("Quaternary[%ld].[%s]{#%ld}:", _i, _cnf_is_true(_i) ? "T" : _cnf_is_false(_i) ? "F" : "?", _t_sz);
		for(_j = 0; _j < _t_sz; _j += 3) {
			const int_t _l1 = cnf_quaternary_implication[_i][_j];
			const int_t _l2 = cnf_quaternary_implication[_i][_j + 1];
			const int_t _l3 = cnf_quaternary_implication[_i][_j + 2];
			printf(" { %lld.[%s] %lld.[%s] %lld.[%s] }", _l1, _cnf_is_true(_l1) ? "T" : _cnf_is_false(_l1) ? "F" : "?", _l2, _cnf_is_true(_l2) ? "T" : _cnf_is_false(_l2) ? "F" : "?", _l3, _cnf_is_true(_l3) ? "T" : _cnf_is_false(_l3) ? "F" : "?");
		}
		printf("\n");
	}
}

int_t cnf_occurrence_binary(int_t l)
{
    return cnf_size_of_binary_implications[l];
}

int_t cnf_occurrence_ternary(int_t l)
{
    return cnf_size_of_ternary_implications[l]/2;
}

int_t cnf_occurrence_quaternary(int_t l)
{
	return cnf_size_of_quaternary_implications[l]/3;
}

inline bool cnf_infer(void) {
    static int_t l, i, l1, l2, l3;
    static boolean_t t1, t2, t3;

    // here propagation stack is initialized
    while(cnf_up_top_stack) {
		l = cnf_up_stack[--cnf_up_top_stack];
		if(_cnf_is_true(l)) continue;
        else if(_cnf_is_false(l)) {
            cnf_up_top_stack = 0;
            return(false);
        } else {
            _cnf_set(l, __TRUE__);
            cnf_history[cnf_history_top++] = l;
            // push propagation thanks to binary implications
            const int_t _sz_bin_imp = cnf_size_of_binary_implications[l];
            int_t * const _bin_imp = cnf_binary_implication[l];
			for(i = 0; i < _sz_bin_imp; ++i) cnf_up_stack[cnf_up_top_stack++] = _bin_imp[i];
            // push propagation thanks to ternary implications
            const int_t _sz_ter_imp = cnf_size_of_ternary_implications[l];
            int_t * const _ter_imp = cnf_ternary_implication[l];
            for(i = 0; i < _sz_ter_imp; i += 2) {
                l1 = _ter_imp[i];
                l2 = _ter_imp[i + 1];
                t1 = cnf_assignment[l1];
                t2 = cnf_assignment[l2];
                /// is clause already SAT ?
                if((t1 & (boolean_t) 1) || (t2 & (boolean_t) 1)) continue;
				if(!t1) cnf_up_stack[cnf_up_top_stack++] = l2;
				else if(!t2) cnf_up_stack[cnf_up_top_stack++] = l1;
            }
			// push propagation thanks to quaternary implications
			const int_t _sz_quater_imp = cnf_size_of_quaternary_implications[l];
			int_t * const _quater_imp = cnf_quaternary_implication[l];
			for(i = 0; i < _sz_quater_imp; i += 3) {
				l1 = _quater_imp[i];
				l2 = _quater_imp[i + 1];
				l3 = _quater_imp[i + 2];
				t1 = cnf_assignment[l1];
				t2 = cnf_assignment[l2];
				t3 = cnf_assignment[l3];
				/// is clause already SAT ?
				if((t1 & (boolean_t) 1) || (t2 & (boolean_t) 1) || (t3 & (boolean_t) 1)) continue;
				if((!t1) && (!t2)) cnf_up_stack[cnf_up_top_stack++] = l3;
				if((!t1) && (!t3)) cnf_up_stack[cnf_up_top_stack++] = l2;
				if((!t2) && (!t3)) cnf_up_stack[cnf_up_top_stack++] = l1;
			}
        }
    }
    // is it SAT
    if(cnf_history_top == cnf_nb_of_vars) cnf_status = __TRUE__;
    return(true);
}

/// @fn bool cnf_set_true(const int_t l);
/// @brief assign and propagate l to true.
/// @param l is the literal (can be negative) that will be assigned to true
/// @return true if assignment ends right and false if inconsistency occurs
inline bool cnf_set_true(const int_t l) {
    assert(abs((int) l) <= cnf_nb_of_vars);
	
    cnf_up_stack[cnf_up_top_stack++] = l;
    return(cnf_infer());
}

bool cnf_set_unitary(void) {
	return(cnf_infer());
}

bool cnf_initiate_from_dimacs(void) {
    int_t i, j, sz;
    const int_t _n_v = dimacs_nb_vars();
    const int_t _n_e = dimacs_nb_equations();
    cnf_nb_of_equations = _n_e;
    cnf_nb_of_vars = _n_v;
    assert(cnf_nb_of_vars <= __MAX_ID__);
    assert(cnf_nb_of_equations <= __MAX_EQ__);
    
    // initiate all literals to UNDETERMINED
    // and build empty lists of propagation
    cnf_assignment = cnf_assignment_buffer + _n_v + 1LL;
    cnf_size_of_binary_implications = cnf_size_of_binary_implications_buffer + _n_v + 1LL;
    cnf_binary_implication = cnf_binary_implication_buffer + _n_v + 1LL;
    cnf_size_of_ternary_implications = cnf_size_of_ternary_implications_buffer + _n_v + 1LL;
    cnf_ternary_implication = cnf_ternary_implication_buffer + _n_v + 1LL;
	cnf_size_of_quaternary_implications = cnf_size_of_quaternary_implications_buffer + _n_v + 1LL;
	cnf_quaternary_implication = cnf_quaternary_implication_buffer + _n_v + 1LL;
    cnf_assignment[0LL] = __UNDEF__;
    cnf_size_of_binary_implications[0LL] = cnf_size_of_ternary_implications[0LL] = cnf_size_of_quaternary_implications[0LL] = 0LL;
    cnf_binary_implication[0LL] = cnf_ternary_implication[0LL] = cnf_quaternary_implication[0LL] = NULL;
    for(i = 1LL; i <= _n_v; ++i) {
        _cnf_unset(i);
        cnf_size_of_binary_implications[i] = cnf_size_of_binary_implications[-i] = 0LL;
		cnf_size_of_ternary_implications[i] = cnf_size_of_ternary_implications[-i] = 0LL;
		cnf_size_of_quaternary_implications[i] = cnf_size_of_quaternary_implications[-i] = 0LL;
        cnf_binary_implication[i] = cnf_binary_implication[-i] = NULL;
		cnf_ternary_implication[i] = cnf_ternary_implication[-i] = NULL;
		cnf_quaternary_implication[i] = cnf_quaternary_implication[-i] = NULL;
    }
    cnf_main_top_buffer = cnf_up_top_stack = 0LL;
    // checking equations
    int_t * const _dimacs_size_of_equations = dimacs_size_of_equations();
    /// temporary structures for binary and ternary and quaternary equations
    int_t **tmp_binary_implications = NULL, **_tmp_binary_implications;
	int_t **tmp_ternary_implications = NULL, **_tmp_ternary_implications;
	int_t **tmp_quaternary_implications = NULL, **_tmp_quaternary_implications;
    _get_mem(int_t *, tmp_binary_implications, ((cnf_nb_of_vars + 1LL) << 1LL));
	_get_mem(int_t *, tmp_ternary_implications, ((cnf_nb_of_vars + 1LL) << 1LL));
	_get_mem(int_t *, tmp_quaternary_implications, ((cnf_nb_of_vars + 1LL) << 1LL));
    _tmp_binary_implications = tmp_binary_implications + cnf_nb_of_vars + 1LL;
	_tmp_ternary_implications = tmp_ternary_implications + cnf_nb_of_vars + 1LL;
	_tmp_quaternary_implications = tmp_quaternary_implications + cnf_nb_of_vars + 1LL;
    for(i = 1LL; i <= cnf_nb_of_vars; ++i) {
        _tmp_binary_implications[i] = _tmp_binary_implications[-i] = NULL;
		_tmp_ternary_implications[i] = _tmp_ternary_implications[-i] = NULL;
		_tmp_quaternary_implications[i] = _tmp_quaternary_implications[-i] = NULL;
        _reset_int_vector(_tmp_binary_implications[i]);
        _reset_int_vector(_tmp_binary_implications[-i]);
		_reset_int_vector(_tmp_ternary_implications[i]);
		_reset_int_vector(_tmp_ternary_implications[-i]);
		_reset_int_vector(_tmp_quaternary_implications[i]);
		_reset_int_vector(_tmp_quaternary_implications[-i]);
    }
    /// building temporary sets
    for(i = 0LL; i < _n_e; ++i) {
        int_t * const _dimacs_equation = dimacs_equation(i);
        switch (_dimacs_size_of_equations[i]) {
            case 1:
                cnf_up_stack[cnf_up_top_stack++] = _dimacs_equation[0];
                assert(cnf_up_top_stack < __ID_SIZE__);
                continue;
            case 2:
                _int_vector_append(_tmp_binary_implications[-_dimacs_equation[0]], _dimacs_equation[1]);
                _int_vector_append(_tmp_binary_implications[-_dimacs_equation[1]], _dimacs_equation[0]);
                break;
            case 3:
				_int_vector_append(_tmp_ternary_implications[-_dimacs_equation[0]], _dimacs_equation[1]);
                _int_vector_append(_tmp_ternary_implications[-_dimacs_equation[0]], _dimacs_equation[2]);
                _int_vector_append(_tmp_ternary_implications[-_dimacs_equation[1]], _dimacs_equation[0]);
                _int_vector_append(_tmp_ternary_implications[-_dimacs_equation[1]], _dimacs_equation[2]);
                _int_vector_append(_tmp_ternary_implications[-_dimacs_equation[2]], _dimacs_equation[0]);
                _int_vector_append(_tmp_ternary_implications[-_dimacs_equation[2]], _dimacs_equation[1]);
                break;
			case 4:
				_int_vector_append(_tmp_quaternary_implications[-_dimacs_equation[0]], _dimacs_equation[1]);
				_int_vector_append(_tmp_quaternary_implications[-_dimacs_equation[0]], _dimacs_equation[2]);
				_int_vector_append(_tmp_quaternary_implications[-_dimacs_equation[0]], _dimacs_equation[3]);
				_int_vector_append(_tmp_quaternary_implications[-_dimacs_equation[1]], _dimacs_equation[0]);
				_int_vector_append(_tmp_quaternary_implications[-_dimacs_equation[1]], _dimacs_equation[2]);
				_int_vector_append(_tmp_quaternary_implications[-_dimacs_equation[1]], _dimacs_equation[3]);
				_int_vector_append(_tmp_quaternary_implications[-_dimacs_equation[2]], _dimacs_equation[0]);
				_int_vector_append(_tmp_quaternary_implications[-_dimacs_equation[2]], _dimacs_equation[1]);
				_int_vector_append(_tmp_quaternary_implications[-_dimacs_equation[2]], _dimacs_equation[3]);
				_int_vector_append(_tmp_quaternary_implications[-_dimacs_equation[3]], _dimacs_equation[0]);
				_int_vector_append(_tmp_quaternary_implications[-_dimacs_equation[3]], _dimacs_equation[1]);
				_int_vector_append(_tmp_quaternary_implications[-_dimacs_equation[3]], _dimacs_equation[2]);
				break;
        }
    }
    /// from temporary to static structures
    cnf_main_top_buffer = 0LL;
    for(i = 1LL; i <= cnf_nb_of_vars; ++i) {
        cnf_size_of_binary_implications[i] = sz = _int_vector_size(_tmp_binary_implications[i]);
        cnf_binary_implication[i] = cnf_main_buffer + cnf_main_top_buffer;
        for(j = 0LL; j < sz; ++j) {
            cnf_main_buffer[cnf_main_top_buffer++] = _tmp_binary_implications[i][j];
            assert(cnf_main_top_buffer < __MAX_BUFFER_SIZE__);
        }
        cnf_size_of_binary_implications[-i] = sz = _int_vector_size(_tmp_binary_implications[-i]);
        cnf_binary_implication[-i] = cnf_main_buffer + cnf_main_top_buffer;
        for(j = 0LL; j < sz; ++j) {
            cnf_main_buffer[cnf_main_top_buffer++] = _tmp_binary_implications[-i][j];
            assert(cnf_main_top_buffer < __MAX_BUFFER_SIZE__);
        }
        cnf_size_of_ternary_implications[i] = sz = _int_vector_size(_tmp_ternary_implications[i]);
        cnf_ternary_implication[i] = cnf_main_buffer + cnf_main_top_buffer;
        for(j = 0LL; j < sz; ++j) {
			cnf_main_buffer[cnf_main_top_buffer++] = _tmp_ternary_implications[i][j];
            assert(cnf_main_top_buffer < __MAX_BUFFER_SIZE__);
        }
		cnf_size_of_ternary_implications[-i] = sz = _int_vector_size(_tmp_ternary_implications[-i]);
        cnf_ternary_implication[-i] = cnf_main_buffer + cnf_main_top_buffer;
        for(j = 0LL; j < sz; ++j) {
            cnf_main_buffer[cnf_main_top_buffer++] = _tmp_ternary_implications[-i][j];
            assert(cnf_main_top_buffer < __MAX_BUFFER_SIZE__);
        }
		cnf_size_of_quaternary_implications[i] = sz = _int_vector_size(_tmp_quaternary_implications[i]);
		cnf_quaternary_implication[i] = cnf_main_buffer + cnf_main_top_buffer;
		for(j = 0LL; j < sz; ++j) {
			cnf_main_buffer[cnf_main_top_buffer++] = _tmp_quaternary_implications[i][j];
			assert(cnf_main_top_buffer < __MAX_BUFFER_SIZE__);
		}
		cnf_size_of_quaternary_implications[-i] = sz = _int_vector_size(_tmp_quaternary_implications[-i]);
		cnf_quaternary_implication[-i] = cnf_main_buffer + cnf_main_top_buffer;
		for(j = 0LL; j < sz; ++j) {
			cnf_main_buffer[cnf_main_top_buffer++] = _tmp_quaternary_implications[-i][j];
			assert(cnf_main_top_buffer < __MAX_BUFFER_SIZE__);
		}
    }
	/// freeing temporary structures
    for(i = 1LL; i <= cnf_nb_of_vars; ++i) {
        _free_int_vector(_tmp_binary_implications[i]);
        _free_int_vector(_tmp_binary_implications[-i]);
		_free_int_vector(_tmp_ternary_implications[i]);
		_free_int_vector(_tmp_ternary_implications[-i]);
		_free_int_vector(_tmp_quaternary_implications[i]);
		_free_int_vector(_tmp_quaternary_implications[-i]);
    }
    _free_mem(tmp_binary_implications);
	_free_mem(tmp_ternary_implications);
	_free_mem(tmp_quaternary_implications);
    cnf_step_top = cnf_history_top = cnf_history_top_it = 0LL;
    cnf_status = __UNDEF__;
	return true;
}

void cnf_undo() {
    static int_t _l;
    const int_t top_step = (cnf_step_top) ? cnf_step[--cnf_step_top] : 0;
    while(cnf_history_top != top_step) {
        _l = cnf_history[--cnf_history_top];
        _cnf_unset(_l);
    }
	cnf_history_top_it = cnf_history_top;
}

/// @fn int_t cnf_last_assigned_breakpoint(int_t *up_stack);
/// @brief return the list of literals that have been assigned since
/// the last breakpoint
/// @param up_stack is the list that will be filled
/// @return the last element in the stack 
int_t cnf_last_assigned_breakpoint(int_t *up_stack) {
	int_t up_stack_top = 0LL;
	int_t _l;
	cnf_history_top_it = (cnf_step_top) ? cnf_step[cnf_step_top - 1LL] : 0;
	while(cnf_history_top_it < cnf_history_top) {
		_l = cnf_history[cnf_history_top_it++];
		up_stack[up_stack_top++] = _l;
	}
	return up_stack_top;
}

/// @fn int_t cnf_last_assigned(int_t *up_stack);
/// @brief return the list of literals that have been assigned since
/// the last time this function (or the other last_assigned) was called
/// @param up_stack is the list that will be filled
/// @return the last element in the stack
int_t cnf_last_assigned(int_t *up_stack) {
	int_t up_stack_top = 0LL;
	int_t _l;
	while(cnf_history_top_it < cnf_history_top) {
		_l = cnf_history[cnf_history_top_it++];
		up_stack[up_stack_top++] = _l;
	}
	return up_stack_top;
}

/// @fn const int_t cnf_number_of_assigned_variables();
/// @brief return the number of assigned variables.
inline const int_t cnf_number_of_assigned_variables() { return cnf_history_top; }



