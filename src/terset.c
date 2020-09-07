//
//  terset.c
//  WDSat
//
//  Created by Gilles Dequen and Monika Trimoska on 15/12/2018.
//  Copyright © 2018 Gilles Dequen. All rights reserved.
//

#include "terset.h"
#include "dimacs.h"
#include "wdsat_utils.h"


// unit propagation stack
int_t terset_up_stack[__ID_SIZE__];
int_t terset_up_top_stack;

boolean_t terset_assignment_buffer[__SIGNED_ID_SIZE__];
boolean_t *terset_assignment;

// undo structures
int_t terset_history[__ID_SIZE__];
int_t terset_history_top;
int_t terset_step[__ID_SIZE__];
int_t terset_step_top;
// number of variables, number of equations
static int_t terset_nb_of_vars;
static int_t terset_nb_of_equations;

// binary implication lists
static int_t terset_size_of_binary_implications_buffer[__SIGNED_ID_SIZE__];
static int_t *terset_size_of_binary_implications;
static int_t *terset_binary_implication_buffer[__SIGNED_ID_SIZE__];
static int_t **terset_binary_implication;

// ternary implication lists
static int_t terset_size_of_ternary_implications_buffer[__SIGNED_ID_SIZE__], *terset_size_of_ternary_implications;
static int_t *terset_ternary_implication_buffer[__SIGNED_ID_SIZE__], **terset_ternary_implication;

// quaternary implication lists
static int_t terset_size_of_quaternary_implications_buffer[__SIGNED_ID_SIZE__], *terset_size_of_quaternary_implications;
static int_t *terset_quaternary_implication_buffer[__SIGNED_ID_SIZE__], **terset_quaternary_implication;

// main buffer
static int_t terset_main_top_buffer;
static int_t terset_main_buffer[__MAX_BUFFER_SIZE__];

static int_t terset_history_top_it;

static boolean_t terset_status;

extern byte_t thread_digits;

inline int_t get_terset_binary_implication(const int_t i, const int_t j) { return terset_binary_implication[i][j]; }

/// @fn void _print_terset(FILE *f);
/// @brief Debugging function allowing to visualize internal structures
void terset_fprint() {
	int_t _i, _j;
    const int_t _n_v = terset_nb_of_vars;
    
    for(_i = -_n_v; _i <= _n_v; ++_i) {
        if(!_i) continue;
        const int_t _b_sz = terset_size_of_binary_implications[_i];
        _cid_cout("Binary[%ld].[%s]{#%ld}:", _i, _terset_is_true(_i) ? "T" : _terset_is_false(_i) ? "F" : "?", _b_sz);
        for(_j = 0; _j < _b_sz; ++_j) {
            const int_t _l = terset_binary_implication[_i][_j];
			printf(" %lld.[%s]", _l, _terset_is_true(_l) ? "T" : _terset_is_false(_l) ? "F" : "?");
        }
        printf("\n");
    }
    for(_i = -_n_v; _i <= _n_v; ++_i) {
        const int_t _t_sz = terset_size_of_ternary_implications[_i];
       _cid_cout("Ternary[%ld].[%s]{#%ld}:", _i, _terset_is_true(_i) ? "T" : _terset_is_false(_i) ? "F" : "?", _t_sz);
        for(_j = 0; _j < _t_sz; _j += 2) {
            const int_t _l1 = terset_ternary_implication[_i][_j];
            const int_t _l2 = terset_ternary_implication[_i][_j + 1];
			printf(" { %lld.[%s] %lld.[%s] }", _l1, _terset_is_true(_l1) ? "T" : _terset_is_false(_l1) ? "F" : "?", _l2, _terset_is_true(_l2) ? "T" : _terset_is_false(_l2) ? "F" : "?");
        }
        printf("\n");
    }
	for(_i = -_n_v; _i <= _n_v; ++_i) {
		const int_t _t_sz = terset_size_of_quaternary_implications[_i];
		_cid_cout("Quaternary[%ld].[%s]{#%ld}:", _i, _terset_is_true(_i) ? "T" : _terset_is_false(_i) ? "F" : "?", _t_sz);
		for(_j = 0; _j < _t_sz; _j += 3) {
			const int_t _l1 = terset_quaternary_implication[_i][_j];
			const int_t _l2 = terset_quaternary_implication[_i][_j + 1];
			const int_t _l3 = terset_quaternary_implication[_i][_j + 2];
			printf(" { %lld.[%s] %lld.[%s] %lld.[%s] }", _l1, _terset_is_true(_l1) ? "T" : _terset_is_false(_l1) ? "F" : "?", _l2, _terset_is_true(_l2) ? "T" : _terset_is_false(_l2) ? "F" : "?", _l3, _terset_is_true(_l3) ? "T" : _terset_is_false(_l3) ? "F" : "?");
		}
		printf("\n");
	}
}

int_t terset_occurrence_binary(int_t l)
{
    return terset_size_of_binary_implications[l];
}

int_t terset_occurrence_ternary(int_t l)
{
    return terset_size_of_ternary_implications[l]/2;
}

int_t terset_occurrence_quaternary(int_t l)
{
	return terset_size_of_quaternary_implications[l]/3;
}

inline bool terset_infer(void) {
    static int_t l, i, l1, l2, l3;
    static boolean_t t1, t2, t3;

    // here propagation stack is initialized
    while(terset_up_top_stack) {
		l = terset_up_stack[--terset_up_top_stack];
		if(_terset_is_true(l)) continue;
        else if(_terset_is_false(l)) {
            terset_up_top_stack = 0;
            return(false);
        } else {
            _terset_set(l, __TRUE__);
            terset_history[terset_history_top++] = l;
            // push propagation thanks to binary implications
            const int_t _sz_bin_imp = terset_size_of_binary_implications[l];
            int_t * const _bin_imp = terset_binary_implication[l];
			for(i = 0; i < _sz_bin_imp; ++i) terset_up_stack[terset_up_top_stack++] = _bin_imp[i];
            // push propagation thanks to ternary implications
            const int_t _sz_ter_imp = terset_size_of_ternary_implications[l];
            int_t * const _ter_imp = terset_ternary_implication[l];
            for(i = 0; i < _sz_ter_imp; i += 2) {
                l1 = _ter_imp[i];
                l2 = _ter_imp[i + 1];
                t1 = terset_assignment[l1];
                t2 = terset_assignment[l2];
                /// is clause already SAT ?
                if((t1 & (boolean_t) 1) || (t2 & (boolean_t) 1)) continue;
				if(!t1) terset_up_stack[terset_up_top_stack++] = l2;
				else if(!t2) terset_up_stack[terset_up_top_stack++] = l1;
            }
			// push propagation thanks to quaternary implications
			const int_t _sz_quater_imp = terset_size_of_quaternary_implications[l];
			int_t * const _quater_imp = terset_quaternary_implication[l];
			for(i = 0; i < _sz_quater_imp; i += 3) {
				l1 = _quater_imp[i];
				l2 = _quater_imp[i + 1];
				l3 = _quater_imp[i + 2];
				t1 = terset_assignment[l1];
				t2 = terset_assignment[l2];
				t3 = terset_assignment[l3];
				/// is clause already SAT ?
				if((t1 & (boolean_t) 1) || (t2 & (boolean_t) 1) || (t3 & (boolean_t) 1)) continue;
				if((!t1) && (!t2)) terset_up_stack[terset_up_top_stack++] = l3;
				if((!t1) && (!t3)) terset_up_stack[terset_up_top_stack++] = l2;
				if((!t2) && (!t3)) terset_up_stack[terset_up_top_stack++] = l1;
			}
        }
    }
    // is it SAT
    if(terset_history_top == terset_nb_of_vars) terset_status = __TRUE__;
    return(true);
}

/// @fn bool terset_set_true(const int_t l);
/// @brief assign and propagate l to true.
/// @param l is the literal (can be negative) that will be assigned to true
/// @return true if assignment ends right and false if inconsistency occurs
inline bool terset_set_true(const int_t l) {
    assert(abs((int) l) <= terset_nb_of_vars);
	
    terset_up_stack[terset_up_top_stack++] = l;
    return(terset_infer());
}

bool terset_set_unitary(void) {
	return(terset_infer());
}

bool terset_initiate_from_dimacs(void) {
    int_t i, j, sz;
    const int_t _n_v = dimacs_nb_vars();
    const int_t _n_e = dimacs_nb_equations();
    terset_nb_of_equations = _n_e;
    terset_nb_of_vars = _n_v;
    assert(terset_nb_of_vars <= __MAX_ID__);
    assert(terset_nb_of_equations <= __MAX_EQ__);
    
    // initiate all literals to UNDETERMINED
    // and build empty lists of propagation
    terset_assignment = terset_assignment_buffer + _n_v + 1LL;
    terset_size_of_binary_implications = terset_size_of_binary_implications_buffer + _n_v + 1LL;
    terset_binary_implication = terset_binary_implication_buffer + _n_v + 1LL;
    terset_size_of_ternary_implications = terset_size_of_ternary_implications_buffer + _n_v + 1LL;
    terset_ternary_implication = terset_ternary_implication_buffer + _n_v + 1LL;
	terset_size_of_quaternary_implications = terset_size_of_quaternary_implications_buffer + _n_v + 1LL;
	terset_quaternary_implication = terset_quaternary_implication_buffer + _n_v + 1LL;
    terset_assignment[0LL] = __UNDEF__;
    terset_size_of_binary_implications[0LL] = terset_size_of_ternary_implications[0LL] = terset_size_of_quaternary_implications[0LL] = 0LL;
    terset_binary_implication[0LL] = terset_ternary_implication[0LL] = terset_quaternary_implication[0LL] = NULL;
    for(i = 1LL; i <= _n_v; ++i) {
        _terset_unset(i);
        terset_size_of_binary_implications[i] = terset_size_of_binary_implications[-i] = 0LL;
		terset_size_of_ternary_implications[i] = terset_size_of_ternary_implications[-i] = 0LL;
		terset_size_of_quaternary_implications[i] = terset_size_of_quaternary_implications[-i] = 0LL;
        terset_binary_implication[i] = terset_binary_implication[-i] = NULL;
		terset_ternary_implication[i] = terset_ternary_implication[-i] = NULL;
		terset_quaternary_implication[i] = terset_quaternary_implication[-i] = NULL;
    }
    terset_main_top_buffer = terset_up_top_stack = 0LL;
    // checking equations
    int_t * const _dimacs_size_of_equations = dimacs_size_of_equations();
    /// temporary structures for binary and ternary and quaternary equations
    int_t **tmp_binary_implications = NULL, **_tmp_binary_implications;
	int_t **tmp_ternary_implications = NULL, **_tmp_ternary_implications;
	int_t **tmp_quaternary_implications = NULL, **_tmp_quaternary_implications;
    _get_mem(int_t *, tmp_binary_implications, ((terset_nb_of_vars + 1LL) << 1LL));
	_get_mem(int_t *, tmp_ternary_implications, ((terset_nb_of_vars + 1LL) << 1LL));
	_get_mem(int_t *, tmp_quaternary_implications, ((terset_nb_of_vars + 1LL) << 1LL));
    _tmp_binary_implications = tmp_binary_implications + terset_nb_of_vars + 1LL;
	_tmp_ternary_implications = tmp_ternary_implications + terset_nb_of_vars + 1LL;
	_tmp_quaternary_implications = tmp_quaternary_implications + terset_nb_of_vars + 1LL;
    for(i = 1LL; i <= terset_nb_of_vars; ++i) {
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
                terset_up_stack[terset_up_top_stack++] = _dimacs_equation[0];
                assert(terset_up_top_stack < __ID_SIZE__);
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
    terset_main_top_buffer = 0LL;
    for(i = 1LL; i <= terset_nb_of_vars; ++i) {
        terset_size_of_binary_implications[i] = sz = _int_vector_size(_tmp_binary_implications[i]);
        terset_binary_implication[i] = terset_main_buffer + terset_main_top_buffer;
        for(j = 0LL; j < sz; ++j) {
            terset_main_buffer[terset_main_top_buffer++] = _tmp_binary_implications[i][j];
            assert(terset_main_top_buffer < __MAX_BUFFER_SIZE__);
        }
        terset_size_of_binary_implications[-i] = sz = _int_vector_size(_tmp_binary_implications[-i]);
        terset_binary_implication[-i] = terset_main_buffer + terset_main_top_buffer;
        for(j = 0LL; j < sz; ++j) {
            terset_main_buffer[terset_main_top_buffer++] = _tmp_binary_implications[-i][j];
            assert(terset_main_top_buffer < __MAX_BUFFER_SIZE__);
        }
        terset_size_of_ternary_implications[i] = sz = _int_vector_size(_tmp_ternary_implications[i]);
        terset_ternary_implication[i] = terset_main_buffer + terset_main_top_buffer;
        for(j = 0LL; j < sz; ++j) {
			terset_main_buffer[terset_main_top_buffer++] = _tmp_ternary_implications[i][j];
            assert(terset_main_top_buffer < __MAX_BUFFER_SIZE__);
        }
		terset_size_of_ternary_implications[-i] = sz = _int_vector_size(_tmp_ternary_implications[-i]);
        terset_ternary_implication[-i] = terset_main_buffer + terset_main_top_buffer;
        for(j = 0LL; j < sz; ++j) {
            terset_main_buffer[terset_main_top_buffer++] = _tmp_ternary_implications[-i][j];
            assert(terset_main_top_buffer < __MAX_BUFFER_SIZE__);
        }
		terset_size_of_quaternary_implications[i] = sz = _int_vector_size(_tmp_quaternary_implications[i]);
		terset_quaternary_implication[i] = terset_main_buffer + terset_main_top_buffer;
		for(j = 0LL; j < sz; ++j) {
			terset_main_buffer[terset_main_top_buffer++] = _tmp_quaternary_implications[i][j];
			assert(terset_main_top_buffer < __MAX_BUFFER_SIZE__);
		}
		terset_size_of_quaternary_implications[-i] = sz = _int_vector_size(_tmp_quaternary_implications[-i]);
		terset_quaternary_implication[-i] = terset_main_buffer + terset_main_top_buffer;
		for(j = 0LL; j < sz; ++j) {
			terset_main_buffer[terset_main_top_buffer++] = _tmp_quaternary_implications[-i][j];
			assert(terset_main_top_buffer < __MAX_BUFFER_SIZE__);
		}
    }
	/// freeing temporary structures
    for(i = 1LL; i <= terset_nb_of_vars; ++i) {
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
    terset_step_top = terset_history_top = terset_history_top_it = 0LL;
    terset_status = __UNDEF__;
	return true;
}

void terset_undo() {
    static int_t _l;
    const int_t top_step = (terset_step_top) ? terset_step[--terset_step_top] : 0;
    while(terset_history_top != top_step) {
        _l = terset_history[--terset_history_top];
        _terset_unset(_l);
    }
	terset_history_top_it = terset_history_top;
}

/// @fn int_t terset_last_assigned_breakpoint(int_t *up_stack);
/// @brief return the list of literals that have been assigned since
/// the last breakpoint
/// @param up_stack is the list that will be filled
/// @return the last element in the stack 
int_t terset_last_assigned_breakpoint(int_t *up_stack) {
	int_t up_stack_top = 0LL;
	int_t _l;
	terset_history_top_it = (terset_step_top) ? terset_step[terset_step_top - 1LL] : 0;
	while(terset_history_top_it < terset_history_top) {
		_l = terset_history[terset_history_top_it++];
		up_stack[up_stack_top++] = _l;
	}
	return up_stack_top;
}

/// @fn int_t terset_last_assigned(int_t *up_stack);
/// @brief return the list of literals that have been assigned since
/// the last time this function (or the other last_assigned) was called
/// @param up_stack is the list that will be filled
/// @return the last element in the stack
int_t terset_last_assigned(int_t *up_stack) {
	int_t up_stack_top = 0LL;
	int_t _l;
	while(terset_history_top_it < terset_history_top) {
		_l = terset_history[terset_history_top_it++];
		up_stack[up_stack_top++] = _l;
	}
	return up_stack_top;
}

/// @fn const int_t terset_number_of_assigned_variables();
/// @brief return the number of assigned variables.
inline const int_t terset_number_of_assigned_variables() { return terset_history_top; }



