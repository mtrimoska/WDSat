//
//  xorgauss.c
//  WDSat
//
//  Created by Gilles Dequen and Monika Trimoska on 15/12/2018.
//  Copyright © 2018 Gilles Dequen. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "wdsat_utils.h"
#include "xorgauss.h"
#include "dimacs.h"


int_t xorgauss_up_stack[__ID_SIZE__];
int_t xorgauss_up_top_stack;

/// @var uint_xorgauss_nb_of_vars;
/// @var uint_t xorgauss_nb_of_equations;
/// @brief resp. number of variables, number of equations
static uint_t xorgauss_nb_of_vars;
static uint_t xorgauss_nb_of_equations;

/// @var uint_t xorgauss_equivalency[__ID_SIZE__][__SZ_GAUSS__];
/// @brief equivalency list according to variable
uint_t xorgauss_equivalency[__ID_SIZE__][__SZ_GAUSS__];
bool xorgauss_equivalent[__ID_SIZE__];

boolean_t xorgauss_assignment_buffer[__SIGNED_ID_SIZE__];
boolean_t *xorgauss_assignment;

// undo structures
int_t xorgauss_history[__ID_SIZE__];
int_t xorgauss_history_top;
int_t xorgauss_step[__ID_SIZE__];
int_t xorgauss_step_top;

#ifdef __XG_ENHANCED__
uint_t xorgauss_equivalency_history[__MAX_ANF_ID__][__ID_SIZE__][__SZ_GAUSS__];
bool xorgauss_equivalent_history[__MAX_ANF_ID__][__ID_SIZE__];
boolean_t xorgauss_assignment_buffer_history[__MAX_ANF_ID__][__SIGNED_ID_SIZE__];
boolean_t xorgauss_current_degree[__ID_SIZE__];
boolean_t xorgauss_current_degree_history[__MAX_ANF_ID__][__ID_SIZE__];
#else
uint_t xorgauss_mask[__ID_SIZE__][__SZ_GAUSS__];
int_t xorgauss_mask_top;
int_t xorgauss_step_mask[__ID_SIZE__];
int_t xorgauss_step_mask_top;
int_t xorgauss_mask_list[__ID_SIZE__][__ID_SIZE__ + 1];
int_t xorgauss_mask_list_top;
int_t xorgauss_reset[__ID_SIZE__];
int_t xorgauss_reset_top;
#endif

void xorgauss_fprint() {
	uint_t v;
	for(v = 1ULL; v <= xorgauss_nb_of_vars; ++v) {
		printf(" [%lld][%s] <=> ", v, xorgauss_equivalent[v] ? "EQU" : "NEQ");
		_boolean_vector_fprint(xorgauss_equivalency[v], xorgauss_nb_of_vars);
		printf("\n");
	}
}

void xorgauss_fprint_system() {
	uint_t v;
	for(v = 1ULL; v <= xorgauss_nb_of_vars; ++v) {
		if(xorgauss_equivalent[v])
		{
			printf("{");
			for(uint_t _boolean_vector_fprint_i = 1ULL; _boolean_vector_fprint_i <= xorgauss_nb_of_vars; ++_boolean_vector_fprint_i)
			{
				if(_boolean_vector_fprint_i == v)
				{
					printf("1,");
				}
				else
				{
					if(_boolean_vector_get(xorgauss_equivalency[v], _boolean_vector_fprint_i))
					{
						printf("1,");
					}
					else
					{
						printf("0,");
					}
				}
				
			}
			printf("},\n");
		}
	}
}

void xorgauss_fprint_for_xorset() {
	uint_t v;
	for(v = 1ULL; v <= xorgauss_nb_of_vars; ++v) {
		if(xorgauss_equivalent[v])
		{
			if(_boolean_vector_get(xorgauss_equivalency[v], 0))
				printf("x %lld ", v);
			else
				printf("x %lld ", -v);
			for(uint_t _boolean_vector_fprint_i = 1ULL; _boolean_vector_fprint_i <= xorgauss_nb_of_vars; ++_boolean_vector_fprint_i)
			{
				if(_boolean_vector_get(xorgauss_equivalency[v], _boolean_vector_fprint_i))
				{
					printf("%llu ", _boolean_vector_fprint_i);
				}
				
			}
			printf("0\n");
		}
	}
}

inline void xorgauss_reset_boolean_vector(uint_t *v) {
	for(uint_t i = 0ULL; i < __SZ_GAUSS__; ++i)
		/// different writing for same aim
		/// v[i] = 0ULL;
		v[i] ^= v[i];
}

inline uint_t xorgauss_get_size_of_boolean_vector(uint_t *v, uint_t *lt) {
	uint_t sz = 0ULL, mask, _id, i;
	_id = 0ULL;
	bool is_cst = false;
	if(_boolean_vector_get_constant(v)) {
		is_cst = true;
		_boolean_vector_flip_constant(v);
	}
	for(i = 0ULL; i < __SZ_GAUSS__; ++i) {
		if(!v[i]) {
			_id += __SZ_ATOM__;
		} else {
			const uint_t _offset = v[i];
			mask = __XOR_CONSTANT_MASK__;
			while(mask) {
				if(_offset & mask) {
					++sz;
					*lt = _id;
				}
				mask >>= 1ULL;
				++_id;
			}
		}
	}
	if(is_cst)
		_boolean_vector_flip_constant(v);
	return(sz);
}

inline uint_t xorgauss_get_last_id_from_boolean_vector(uint_t *v) {
	uint_t i, _id, mask;
	_id = 0ULL;
	_id = __SZ_ATOM__ * __SZ_GAUSS__;
	for(i = __SZ_GAUSS__; i >= 1ULL; --i) {
		if(v[i-1]) break;
		_id -= __SZ_ATOM__;
	}
	--_id;
	//assert(_id < __MAX_ID__);
	if(_id > __MAX_ID__ + __SZ_ATOM__) return 0;
	const uint_t _offset = v[i-1];
	mask = 1ULL;
	while(!(_offset & mask)) {
		--_id;
		mask <<= 1ULL;
	}
	return(_id);
}

inline uint_t xorgauss_get_first_id_from_boolean_vector(uint_t *v) {
	uint_t i, _id, mask;
	_id = 0ULL;
	bool is_cst = false;
	if(_boolean_vector_get_constant(v)) {
		is_cst = true;
		_boolean_vector_flip_constant(v);
	}
	for(i = 0ULL; i < __SZ_GAUSS__; ++i) {
		if(v[i]) break;
		_id += __SZ_ATOM__;
	}
	//assert(_id < __MAX_ID__);
	if(_id > __MAX_ID__)
	{
		if(is_cst)
			_boolean_vector_flip_constant(v);
		return 0;
	}
	const uint_t _offset = v[i];
	mask = __XOR_CONSTANT_MASK__;
	while(!(_offset & mask)) {
		++_id;
		mask >>= 1ULL;
	}
	if(is_cst)
		_boolean_vector_flip_constant(v);
	return(_id);
}

inline void xorgauss_xor_it(uint_t *dst, uint_t *src) {
	for(uint_t i = 0ULL; i < __SZ_GAUSS__; ++i) dst[i] ^= src[i];
}

inline bool xorgauss_xor_it_and_check(uint_t *dst, uint_t *src) {
	static bool out_xor;
	out_xor = true;
	if((dst[0] ^= src[0]) & (~__XOR_CONSTANT_MASK__)) out_xor = false;
	for(uint_t i = 1ULL; i < __SZ_GAUSS__; ++i)
		if(dst[i] ^= src[i]) out_xor = false;
	return(out_xor);
}

inline bool xorgauss_is_constant(uint_t *v) {
	uint_t i;
	if(v[0] & (~__XOR_CONSTANT_MASK__)) return(false);
	for(i = 1ULL; i < __SZ_GAUSS__; ++i)
		if(v[i]) return(false);
	return(true);
}

void aff_bin(uint_t v)
{
	int i;
	for(i=63; i>=0; i--)
	{
		printf("%d,",v & (1ULL << i)? 63-i : 0);
	}
	printf("\n");
}

bool xorgauss_from_dimacs() {
	static uint_t i, j, u_lt;
	static int_t lt;
	const uint_t _n_x = (uint_t) dimacs_nb_xor_equations();
	/// Temporary structures
	uint_t **_t_eq = NULL;
	
	/// Allocate and initiate temporary structures
	/// this will lead to build main XOR reasoning structures
	_get_mem(uint_t *, _t_eq, _n_x);
	for(i = 0ULL; i < _n_x; ++i) {
		_t_eq[i] = NULL;
		_get_mem(uint_t, _t_eq[i], __SZ_GAUSS__);
		xorgauss_reset_boolean_vector(_t_eq[i]);
		/// initiate constant value. Constant has the id 0
		/// it corresponds to _boolean_vector_set(_t_eq[i], 0);
		_boolean_vector_flip_constant(_t_eq[i]);
	}
	/// Normalize dimacs XOR equations
	int_t * const _dimacs_size_of_xor_equations = dimacs_size_of_xor_equations();
	for(i = 0ULL; i < _n_x; ++i) {
		int_t * const _dimacs_xor = dimacs_xor(i);
		const uint_t sz = (uint_t) _dimacs_size_of_xor_equations[i];
		for(j = 0ULL; j < sz; ++j) {
			lt = _dimacs_xor[j];
			if(lt < 0LL) {
				u_lt = (uint_t) (-lt);
				_boolean_vector_flip_constant(_t_eq[i]);
			} else u_lt = (uint_t) lt;
			if(_boolean_vector_get(_t_eq[i], u_lt)) //duplicates
				_boolean_vector_reset(_t_eq[i], u_lt);
			else
				_boolean_vector_set(_t_eq[i], u_lt);
		}
	}

	/// for all dimacs equations
	for(i = 0ULL; i < _n_x; ++i) {
		/// conflict occurs
		/// iff the only constant is assigned to true meaning TRUE = FALSE -> conflict
		if((xorgauss_is_constant(_t_eq[i])) && (_t_eq[i][0])) return(false);
		u_lt = xorgauss_get_first_id_from_boolean_vector(_t_eq[i]);
		if(u_lt == 0) continue;
		/// compute equivalency of selected literal
		uint_t * const _xeq_lt = xorgauss_equivalency[u_lt];
		xorgauss_xor_it(_xeq_lt, _t_eq[i]);
		for(j = xorgauss_nb_of_vars; j > 0ULL; --j) {
			if(!xorgauss_equivalent[j]) continue;
			uint_t * const _xeq_lt_j = xorgauss_equivalency[j];
			if(!_boolean_vector_get(_xeq_lt_j, u_lt)) continue;
			xorgauss_xor_it(_xeq_lt_j, _xeq_lt);
		}
		xorgauss_equivalent[u_lt] = true;
		for(j = i + 1ULL; j < _n_x; ++j) {
			if(!_boolean_vector_get(_t_eq[j], u_lt)) continue;
			xorgauss_xor_it(_t_eq[j], _xeq_lt);
		}
		_boolean_vector_reset(_xeq_lt, u_lt);
	}
	/// Free temporary structures
	for(i = 0; i < _n_x; ++i) _free_mem(_t_eq[i]);
	_free_mem(_t_eq);
	/// Get the assigned literals
	for(j = xorgauss_nb_of_vars; j > 0ULL; --j) {
		if(!xorgauss_equivalent[j]) continue;
		uint_t * const _xeq_lt = xorgauss_equivalency[j];
		if(xorgauss_is_constant(_xeq_lt)) {
			xorgauss_history[xorgauss_history_top++] = j;
			const bool _tv = ((_boolean_vector_get_constant(_xeq_lt)) == __XOR_CONSTANT_MASK__);
			_xorgauss_set(j, _tv);
		}
	}
	return(true);
}

bool xorgauss_replace(const int_t v_bin, const int_t v_mon)
{
	static uint_t i, _to_subst;
	static bool _is_unary_subst;
	
	if(!_xorgauss_is_undef(v_bin))
	{
		if(!_xorgauss_is_undef(v_mon))
		{
			return (xorgauss_assignment[v_mon] == xorgauss_assignment[v_bin]);
		}
		else
		{
			if(xorgauss_assignment[v_bin])
			{
				xorgauss_up_stack[xorgauss_up_top_stack++] = v_mon;
				assert(xorgauss_up_top_stack < __ID_SIZE__);
				return true;
			}
			else
			{
				xorgauss_up_stack[xorgauss_up_top_stack++] = -v_mon;
				assert(xorgauss_up_top_stack < __ID_SIZE__);
				return true;
			}
		}
	}
	if(!_xorgauss_is_undef(v_mon))
	{
		if(xorgauss_assignment[v_mon])
		{
			xorgauss_up_stack[xorgauss_up_top_stack++] = v_bin;
			assert(xorgauss_up_top_stack < __ID_SIZE__);
			return true;
		}
		else
		{
			xorgauss_up_stack[xorgauss_up_top_stack++] = -v_bin;
			assert(xorgauss_up_top_stack < __ID_SIZE__);
			return true;
		}
	}
	
	_is_unary_subst = false;
	if(xorgauss_equivalent[v_mon]) // 4,5,6
	{
		if(xorgauss_equivalent[v_bin]) // 5,6
		{
			uint_t * const _xeq_bin = xorgauss_equivalency[v_bin];
			if(xorgauss_is_constant(_xeq_bin)) // 6
			{
				if((_boolean_vector_get_constant(_xeq_bin)) == __XOR_CONSTANT_MASK__)
					xorgauss_up_stack[xorgauss_up_top_stack++] = v_mon;
				else
					xorgauss_up_stack[xorgauss_up_top_stack++] = -v_mon;
			}
			else // 5
			{
				uint_t * const _xeq_bin = xorgauss_equivalency[v_bin];
				uint_t * const _xeq_mon = xorgauss_equivalency[v_mon];
				if(!xorgauss_xor_it_and_check(_xeq_bin, _xeq_mon))
				{
					_to_subst = xorgauss_get_first_id_from_boolean_vector(_xeq_bin);
					uint_t * const _xeq_to_subst = xorgauss_equivalency[_to_subst];
					_boolean_vector_reset(_xeq_bin, _to_subst);
					if(xorgauss_xor_it_and_check(_xeq_to_subst, _xeq_bin)) _is_unary_subst = true;
					_boolean_vector_set(_xeq_to_subst, _to_subst);
					xorgauss_reset_boolean_vector(_xeq_bin);
					for(i = xorgauss_nb_of_vars; i > 0ULL; --i) {
						if(!xorgauss_equivalent[i]) continue;
						uint_t * const _xeq_i = xorgauss_equivalency[i];
						if(!_boolean_vector_get(_xeq_i, _to_subst)) continue;
						if(xorgauss_xor_it_and_check(_xeq_i, _xeq_to_subst)) {
							const bool _tv_i = ((_boolean_vector_get_constant(_xeq_i)) == __XOR_CONSTANT_MASK__);
							xorgauss_history[xorgauss_history_top++] = (_tv_i ? (int_t) i : -(int_t) i);
							xorgauss_up_stack[xorgauss_up_top_stack++] = (_tv_i ? (int_t) i : -(int_t) i);
							assert(xorgauss_history_top < __ID_SIZE__);
							assert(xorgauss_up_top_stack < __ID_SIZE__);
							_xorgauss_set(i, _tv_i);
						}
					}
					xorgauss_equivalent[_to_subst] = true;
					_boolean_vector_reset(_xeq_to_subst, _to_subst);
					if(_is_unary_subst) {
						const bool _tv_to_subst = ((_boolean_vector_get_constant(_xeq_to_subst)) == __XOR_CONSTANT_MASK__);
						xorgauss_history[xorgauss_history_top++] = (_tv_to_subst ? (int_t) _to_subst : -(int_t) _to_subst);
						xorgauss_up_stack[xorgauss_up_top_stack++] = (_tv_to_subst ? (int_t) _to_subst : -(int_t) _to_subst);
						assert(xorgauss_history_top < __ID_SIZE__);
						assert(xorgauss_up_top_stack < __ID_SIZE__);
						_xorgauss_set(_to_subst, _tv_to_subst);
					}
				}
				else //there was already an evident equivalence and we deduced nothing or we found a contradiction
				{
					if((_boolean_vector_get_constant(_xeq_bin)) == __XOR_CONSTANT_MASK__)
						return false;
				}
				xorgauss_equivalent[v_bin] = false;
				xorgauss_reset_boolean_vector(xorgauss_equivalency[v_bin]); //pas nécessaire mais propre
			}
		}
		else // 4
		{
			uint_t * const _xeq_mon = xorgauss_equivalency[v_mon];
			
			// *
			if(_boolean_vector_get(_xeq_mon, v_bin))
			{
				_boolean_vector_reset(_xeq_mon, v_bin);
				_to_subst = xorgauss_get_first_id_from_boolean_vector(_xeq_mon);
				if(_to_subst == 0)
				{
					if((_boolean_vector_get_constant(_xeq_mon)) == __XOR_CONSTANT_MASK__) return false;
				}
				else
				{
					uint_t * const _xeq_to_subst = xorgauss_equivalency[_to_subst];
					_boolean_vector_reset(_xeq_mon, _to_subst);
					if(xorgauss_xor_it_and_check(_xeq_to_subst, _xeq_mon)) _is_unary_subst = true;
					_boolean_vector_set(_xeq_to_subst, _to_subst);
					xorgauss_reset_boolean_vector(_xeq_mon);
					for(i = xorgauss_nb_of_vars; i > 0ULL; --i) {
						if(!xorgauss_equivalent[i]) continue;
						uint_t * const _xeq_i = xorgauss_equivalency[i];
						if(!_boolean_vector_get(_xeq_i, _to_subst)) continue;
						if(xorgauss_xor_it_and_check(_xeq_i, _xeq_to_subst)) {
							const bool _tv_i = ((_boolean_vector_get_constant(_xeq_i)) == __XOR_CONSTANT_MASK__);
							xorgauss_history[xorgauss_history_top++] = (_tv_i ? (int_t) i : -(int_t) i);
							xorgauss_up_stack[xorgauss_up_top_stack++] = (_tv_i ? (int_t) i : -(int_t) i);
							assert(xorgauss_history_top < __ID_SIZE__);
							assert(xorgauss_up_top_stack < __ID_SIZE__);
							_xorgauss_set(i, _tv_i);
						}
					}
					xorgauss_equivalent[_to_subst] = true;
					_boolean_vector_reset(_xeq_to_subst, _to_subst);
					if(_is_unary_subst) {
						const bool _tv_to_subst = ((_boolean_vector_get_constant(_xeq_to_subst)) == __XOR_CONSTANT_MASK__);
						xorgauss_history[xorgauss_history_top++] = (_tv_to_subst ? (int_t) _to_subst : -(int_t) _to_subst);
						xorgauss_up_stack[xorgauss_up_top_stack++] = (_tv_to_subst ? (int_t) _to_subst : -(int_t) _to_subst);
						assert(xorgauss_history_top < __ID_SIZE__);
						assert(xorgauss_up_top_stack < __ID_SIZE__);
						_xorgauss_set(_to_subst, _tv_to_subst);
					}
				}
				// do 1
				_boolean_vector_set(_xeq_mon, v_mon);
				_boolean_vector_set(_xeq_mon, v_bin);
				xorgauss_equivalent[v_mon] = false;
				for(i = xorgauss_nb_of_vars; i > 0ULL; --i) {
					if(!xorgauss_equivalent[i]) continue;
					uint_t * const _xeq_i = xorgauss_equivalency[i];
					if(!_boolean_vector_get(_xeq_i, v_bin)) continue;
					if(xorgauss_xor_it_and_check(_xeq_i, _xeq_mon))
					{
						const bool _tv_i = ((_boolean_vector_get_constant(_xeq_i)) == __XOR_CONSTANT_MASK__);
						xorgauss_history[xorgauss_history_top++] = (_tv_i ? (int_t) i : -(int_t) i);
						xorgauss_up_stack[xorgauss_up_top_stack++] = (_tv_i ? (int_t) i : -(int_t) i);
						assert(xorgauss_history_top < __ID_SIZE__);
						assert(xorgauss_up_top_stack < __ID_SIZE__);
						_xorgauss_set(i, _tv_i);
					}
				}
				xorgauss_reset_boolean_vector(xorgauss_equivalency[v_mon]); //pas nécessaire mais propre
				
			}
			else
			{
				_boolean_vector_set(_xeq_mon, v_bin);
				for(i = xorgauss_nb_of_vars; i > 0ULL; --i) {
					if(!xorgauss_equivalent[i]) continue;
					if(i == v_mon) continue;
					uint_t * const _xeq_i = xorgauss_equivalency[i];
					if(!_boolean_vector_get(_xeq_i, v_bin)) continue;
					if(xorgauss_xor_it_and_check(_xeq_i, _xeq_mon))
					{
						const bool _tv_i = ((_boolean_vector_get_constant(_xeq_i)) == __XOR_CONSTANT_MASK__);
						xorgauss_history[xorgauss_history_top++] = (_tv_i ? (int_t) i : -(int_t) i);
						xorgauss_up_stack[xorgauss_up_top_stack++] = (_tv_i ? (int_t) i : -(int_t) i);
						assert(xorgauss_history_top < __ID_SIZE__);
						assert(xorgauss_up_top_stack < __ID_SIZE__);
						_xorgauss_set(i, _tv_i);
					}
				}
				_boolean_vector_reset(_xeq_mon, v_bin);
			}
		}
	}
	else // 1,2,3
	{
		if(xorgauss_equivalent[v_bin]) // 2,3
		{
			uint_t * const _xeq_bin = xorgauss_equivalency[v_bin];
			uint_t * const _xeq_mon = xorgauss_equivalency[v_mon]; // is zero vector (otherwise do or instead of xor)
			
			// *
			if(_boolean_vector_get(_xeq_bin, v_mon))
			{
				_boolean_vector_reset(_xeq_bin, v_mon);
				_to_subst = xorgauss_get_first_id_from_boolean_vector(_xeq_bin);
				if(_to_subst == 0)
				{
					xorgauss_equivalent[v_bin] = false;
					return ((_boolean_vector_get_constant(_xeq_bin)) != __XOR_CONSTANT_MASK__);
				}
				uint_t * const _xeq_to_subst = xorgauss_equivalency[_to_subst];
				_boolean_vector_reset(_xeq_bin, _to_subst);
				if(xorgauss_xor_it_and_check(_xeq_to_subst, _xeq_bin)) _is_unary_subst = true;
				_boolean_vector_set(_xeq_to_subst, _to_subst);
				xorgauss_reset_boolean_vector(_xeq_bin);
				for(i = xorgauss_nb_of_vars; i > 0ULL; --i) {
					if(!xorgauss_equivalent[i]) continue;
					uint_t * const _xeq_i = xorgauss_equivalency[i];
					if(!_boolean_vector_get(_xeq_i, _to_subst)) continue;
					if(xorgauss_xor_it_and_check(_xeq_i, _xeq_to_subst)) {
						const bool _tv_i = ((_boolean_vector_get_constant(_xeq_i)) == __XOR_CONSTANT_MASK__);
						xorgauss_history[xorgauss_history_top++] = (_tv_i ? (int_t) i : -(int_t) i);
						xorgauss_up_stack[xorgauss_up_top_stack++] = (_tv_i ? (int_t) i : -(int_t) i);
						assert(xorgauss_history_top < __ID_SIZE__);
						assert(xorgauss_up_top_stack < __ID_SIZE__);
						_xorgauss_set(i, _tv_i);
					}
				}
				xorgauss_equivalent[_to_subst] = true;
				_boolean_vector_reset(_xeq_to_subst, _to_subst);
				if(_is_unary_subst) {
					const bool _tv_to_subst = ((_boolean_vector_get_constant(_xeq_to_subst)) == __XOR_CONSTANT_MASK__);
					xorgauss_history[xorgauss_history_top++] = (_tv_to_subst ? (int_t) _to_subst : -(int_t) _to_subst);
					xorgauss_up_stack[xorgauss_up_top_stack++] = (_tv_to_subst ? (int_t) _to_subst : -(int_t) _to_subst);
					assert(xorgauss_history_top < __ID_SIZE__);
					assert(xorgauss_up_top_stack < __ID_SIZE__);
					_xorgauss_set(_to_subst, _tv_to_subst);
				}
			}
			else
			{
				if(xorgauss_xor_it_and_check(_xeq_mon, _xeq_bin))
				{
					const bool _tv_i = ((_boolean_vector_get_constant(_xeq_mon)) == __XOR_CONSTANT_MASK__);
					xorgauss_history[xorgauss_history_top++] = (_tv_i ? (int_t) v_mon : -(int_t) v_mon);
					xorgauss_up_stack[xorgauss_up_top_stack++] = (_tv_i ? (int_t) v_mon : -(int_t) v_mon);
					assert(xorgauss_history_top < __ID_SIZE__);
					assert(xorgauss_up_top_stack < __ID_SIZE__);
					_xorgauss_set(v_mon, _tv_i);
				}
				
				_boolean_vector_set(_xeq_mon, v_mon);
				for(i = xorgauss_nb_of_vars; i > 0ULL; --i) {
					if(!xorgauss_equivalent[i]) continue;
					uint_t * const _xeq_i = xorgauss_equivalency[i];
					if(!_boolean_vector_get(_xeq_i, v_mon)) continue;
					if(xorgauss_xor_it_and_check(_xeq_i, _xeq_mon))
					{
						const bool _tv_i = ((_boolean_vector_get_constant(_xeq_i)) == __XOR_CONSTANT_MASK__);
						xorgauss_history[xorgauss_history_top++] = (_tv_i ? (int_t) i : -(int_t) i);
						xorgauss_up_stack[xorgauss_up_top_stack++] = (_tv_i ? (int_t) i : -(int_t) i);
						assert(xorgauss_history_top < __ID_SIZE__);
						assert(xorgauss_up_top_stack < __ID_SIZE__);
						_xorgauss_set(i, _tv_i);
					}
				}
				_boolean_vector_reset(_xeq_mon, v_mon);
				xorgauss_equivalent[v_mon] = true;
			}
			xorgauss_equivalent[v_bin] = false;
			xorgauss_reset_boolean_vector(xorgauss_equivalency[v_bin]); //pas nécessaire mais propre
		}
		else // 1
		{
			uint_t * const _xeq = xorgauss_equivalency[v_bin]; // is zero vector
			_boolean_vector_set(_xeq, v_mon);
			_boolean_vector_set(_xeq, v_bin);
			for(i = xorgauss_nb_of_vars; i > 0ULL; --i) {
				if(!xorgauss_equivalent[i]) continue;
				uint_t * const _xeq_i = xorgauss_equivalency[i];
				if(!_boolean_vector_get(_xeq_i, v_bin)) continue;
				if(xorgauss_xor_it_and_check(_xeq_i, _xeq))
				{
					const bool _tv_i = ((_boolean_vector_get_constant(_xeq_i)) == __XOR_CONSTANT_MASK__);
					xorgauss_history[xorgauss_history_top++] = (_tv_i ? (int_t) i : -(int_t) i);
					xorgauss_up_stack[xorgauss_up_top_stack++] = (_tv_i ? (int_t) i : -(int_t) i);
					assert(xorgauss_history_top < __ID_SIZE__);
					assert(xorgauss_up_top_stack < __ID_SIZE__);
					_xorgauss_set(i, _tv_i);
				}
			}
			xorgauss_reset_boolean_vector(xorgauss_equivalency[v_bin]);
		}
	}
	return true;
}

bool xorgauss_set_true(const int_t v)
{
	assert(abs((int) v) <= xorgauss_nb_of_vars);
	int_t _v;
	xorgauss_up_stack[xorgauss_up_top_stack++] = v;
	while(xorgauss_up_top_stack)
	{
		_v = xorgauss_up_stack[--xorgauss_up_top_stack];
		if(!xorgauss_infer(_v)) return false;
#ifdef __XG_ENHANCED__
		const bool _tf = (_v < 0) ? false : true;
		const uint_t _uv = (uint_t) ((_v < 0) ? -_v : _v);
		if(_uv <= dimacs_nb_unary_vars())
		{
			if(_tf == true)
			{
				int_t i = 0;
				while(monomials_to_column[_uv][i][0] > 0)
				{
					xorgauss_current_degree[monomials_to_column[_uv][i][0]]--;
					if(xorgauss_current_degree[monomials_to_column[_uv][i][0]] == 1)
					{
						int_t j = 1;
						while(_xorgauss_is_true(monomials_to_column[_uv][i][j])) j++;
						if(j > __MAX_DEGREE__ - 2 || monomials_to_column[_uv][i][j] == 0) //all of the terms are set to 1
						{
							xorgauss_up_stack[xorgauss_up_top_stack++] = monomials_to_column[_uv][i][0]; //so set monomial to 1
							assert(xorgauss_up_top_stack < __ID_SIZE__);
						}
						else
						{
							if(!xorgauss_replace(monomials_to_column[_uv][i][0], monomials_to_column[_uv][i][j]))
							{
								xorgauss_up_top_stack = 0;
								return false;
							}
						}
					}
					i++;
				}
			}
			else
			{
				int_t i = 0;
				while(monomials_to_column[_uv][i][0] > 0)
				{
					xorgauss_current_degree[monomials_to_column[_uv][i][0]] = 0;
					i++;
				}
			}
		}
#endif
	}
	return true;
}

bool xorgauss_infer(int_t v) {
	// get the literal to assign and its truth value
	const bool _tf = (v < 0) ? false : true;
	bool _is_unary_subst;
	const uint_t _uv = (uint_t) ((v < 0) ? -v : v);
	// literals to substitute
	uint_t _to_subst, i;
	_is_unary_subst = false;
	// is it equivalent to a constant or not ?
	if(xorgauss_equivalent[_uv]) {
		uint_t * const _xeq_uv = xorgauss_equivalency[_uv];
		if(xorgauss_is_constant(_xeq_uv))
		{
			if(((_boolean_vector_get_constant(_xeq_uv)) == __XOR_CONSTANT_MASK__) == _tf) return true;
			else
			{
				xorgauss_up_top_stack = 0;
				return false;
			}
		}
		_to_subst = xorgauss_get_first_id_from_boolean_vector(_xeq_uv);
		//printf("%lu->%lu\n",_uv,_to_subst);
		uint_t * const _xeq_to_subst = xorgauss_equivalency[_to_subst];
		_boolean_vector_reset(_xeq_uv, _to_subst);
		if(xorgauss_xor_it_and_check(_xeq_to_subst, _xeq_uv)) _is_unary_subst = true;
		_boolean_vector_set(_xeq_to_subst, _to_subst);
		xorgauss_reset_boolean_vector(_xeq_uv);
		if(_tf) {
			_boolean_vector_flip_constant(_xeq_uv);
			_boolean_vector_flip_constant(_xeq_to_subst);
		}
#ifndef __XG_ENHANCED__
		xorgauss_mask_list[xorgauss_mask_list_top][1] = _uv; ///undo
		xorgauss_mask_list[xorgauss_mask_list_top][0] = 2; ///undo
#endif
		// if there is a no current equivalency (unassigned variable)
	} else {
		uint_t * const _xeq_to_subst = xorgauss_equivalency[_uv];
		_boolean_vector_set(_xeq_to_subst, _uv);
		if(_tf) _boolean_vector_flip_constant(_xeq_to_subst);
		_to_subst = _uv;
#ifndef __XG_ENHANCED__
		xorgauss_mask_list[xorgauss_mask_list_top][0] = 1; ///undo
#endif
	}
	xorgauss_history[xorgauss_history_top++] = v;
	_xorgauss_set(v, true);
	// proceed substitution
	uint_t * const _xeq = xorgauss_equivalency[_to_subst];
#ifndef __XG_ENHANCED__
	xorgauss_reset[xorgauss_reset_top++] = _to_subst; ///undo
	memcpy(xorgauss_mask[xorgauss_mask_top++], xorgauss_equivalency[_to_subst], __SZ_GAUSS__ * sizeof(uint_t)); ///undo
#endif
	for(i = xorgauss_nb_of_vars; i > 0ULL; --i) {
		if(!xorgauss_equivalent[i]) continue;
		uint_t * const _xeq_i = xorgauss_equivalency[i];
		if(!_boolean_vector_get(_xeq_i, _to_subst)) continue;
#ifndef __XG_ENHANCED__
		xorgauss_mask_list[xorgauss_mask_list_top][xorgauss_mask_list[xorgauss_mask_list_top][0]++] = i; ///undo
#endif
		if(xorgauss_xor_it_and_check(_xeq_i, _xeq)) {
			const bool _tv_i = ((_boolean_vector_get_constant(_xeq_i)) == __XOR_CONSTANT_MASK__);
			xorgauss_history[xorgauss_history_top++] = (_tv_i ? (int_t) i : -(int_t) i);
			_xorgauss_set(i, _tv_i);
			xorgauss_up_stack[xorgauss_up_top_stack++] = (_tv_i ? (int_t) i : -(int_t) i);
			//assert(xorgauss_history_top < __ID_SIZE__);
			assert(xorgauss_up_top_stack < __ID_SIZE__);
		}
	}
#ifndef __XG_ENHANCED__
	xorgauss_mask_list_top++;
#endif
	xorgauss_equivalent[_to_subst] = true;
	_boolean_vector_reset(_xeq, _to_subst);
	if(_is_unary_subst) {
		const bool _tv_to_subst = ((_boolean_vector_get_constant(_xeq)) == __XOR_CONSTANT_MASK__);
		xorgauss_history[xorgauss_history_top++] = (_tv_to_subst ? (int_t) _to_subst : -(int_t) _to_subst);
		_xorgauss_set(_to_subst, _tv_to_subst);
		xorgauss_up_stack[xorgauss_up_top_stack++] = (_tv_to_subst ? (int_t) _to_subst : -(int_t) _to_subst);
		assert(xorgauss_history_top < __ID_SIZE__);
		assert(xorgauss_up_top_stack < __ID_SIZE__);
	}
	return true;
}

bool xorgauss_initiate_from_dimacs() {
	const int_t _n_v = dimacs_nb_vars();
	const int_t _n_x = dimacs_nb_xor_equations();
	xorgauss_nb_of_equations = _n_x;
	xorgauss_nb_of_vars = _n_v;
	xorgauss_assignment = xorgauss_assignment_buffer + _n_v + 1LL;
	assert(xorgauss_nb_of_vars <= __MAX_ID__);
	assert(xorgauss_nb_of_equations <= __MAX_XEQ__);
	/// initiate static structures
	for(int_t i = 0LL; i <= _n_v; ++i) {
		xorgauss_reset_boolean_vector(xorgauss_equivalency[i]);
		xorgauss_equivalent[i] = false;
		_xorgauss_unset(i);
#ifdef __XG_ENHANCED__
		xorgauss_current_degree[i] = dimacs_get_current_degree(i);
#endif
	}
	xorgauss_history_top = xorgauss_step_top = xorgauss_up_top_stack = 0LL;
#ifndef __XG_ENHANCED__
	xorgauss_mask_top = xorgauss_mask_list_top = xorgauss_reset_top = 0LL;
#endif
	return(xorgauss_from_dimacs());
}

void xorgauss_undo() {
	int_t _l;
	const int_t top_step = (xorgauss_step_top) ? xorgauss_step[--xorgauss_step_top] : 0;
	while(xorgauss_history_top != top_step) {
		_l = xorgauss_history[--xorgauss_history_top];
		_xorgauss_unset(_l);
	}
	xorgauss_up_top_stack = 0LL;
#ifdef __XG_ENHANCED__
	memcpy(xorgauss_equivalency, xorgauss_equivalency_history[xorgauss_step_top], sizeof(int_t)*__ID_SIZE__*__SZ_GAUSS__);
	memcpy(xorgauss_equivalent, xorgauss_equivalent_history[xorgauss_step_top], sizeof(bool)*__ID_SIZE__);
	memcpy(xorgauss_assignment_buffer, xorgauss_assignment_buffer_history[xorgauss_step_top], sizeof(boolean_t)*__SIGNED_ID_SIZE__);
	memcpy(xorgauss_current_degree, xorgauss_current_degree_history[xorgauss_step_top], sizeof(boolean_t)*__ID_SIZE__);
#else
	const int_t top_step_mask = (xorgauss_step_mask_top) ? xorgauss_step_mask[--xorgauss_step_mask_top] : 0;
	while(xorgauss_mask_top != top_step_mask)
	{
		uint_t * const _xeq_mask = xorgauss_mask[--xorgauss_mask_top];
		xorgauss_mask_list_top--;
		xorgauss_mask_list[xorgauss_mask_list_top][0]--;
		while(xorgauss_mask_list[xorgauss_mask_list_top][0] != 0)
		{
			int_t const clause = xorgauss_mask_list[xorgauss_mask_list_top][xorgauss_mask_list[xorgauss_mask_list_top][0]--];
			uint_t * const _xeq_i = xorgauss_equivalency[clause];
			xorgauss_xor_it_and_check(_xeq_i, _xeq_mask);
		}
		xorgauss_reset_boolean_vector(xorgauss_equivalency[xorgauss_reset[--xorgauss_reset_top]]);
		xorgauss_equivalent[xorgauss_reset[xorgauss_reset_top]] = false;
	}
#endif
}

