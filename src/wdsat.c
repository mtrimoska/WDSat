//
//  wdsat.c
//  WDSat
//
//  Created by Monika Trimoska on 15/12/2018.
//  Copyright © 2018 Monika Trimoska. All rights reserved.
//

#include <string.h>
#include<stdio.h>
#include<time.h>
#include <stdlib.h>

#include "wdsat.h"
#include "terset.h"
#include "xorset.h"
#include "xorgauss.h"
#include "dimacs.h"

#define _LA_LEVEL_ 17

int_t l;
int_t m;
int test[100] = {0};

/// @var uint_t nb_of_vars;
/// @brief number of variables
static int_t nb_of_vars;
	
/// @var int_t wdsat_terset_up_stack[__ID_SIZE__];
/// @brief unit propagation terset stack
static int_t wdsat_terset_up_stack[__ID_SIZE__];

/// @var int_t wdsat_terset_up_top_stack;
/// @brief unit propagation terset stack top
static int_t wdsat_terset_up_top_stack;

/// @var int_t wdsat_xorset_up_stack[__ID_SIZE__];
/// @brief unit propagation xorset stack
static int_t wdsat_xorset_up_stack[__ID_SIZE__];

/// @var int_t wdsat_terset_up_top_stack;
/// @brief unit propagation terset stack top
static int_t wdsat_xorset_up_top_stack;

static int_t set[__ID_SIZE__];

bool wdsat_set_true(const int_t l) {
    /*printf("Setting:%ld\n",l);
	for(int i = 1; i <= 15; i++)
		printf("-%d%d-",terset_assignment[i], xorset_assignment[i]);
	printf("\n");*/
    bool _next_loop;
    int_t _l;
    wdsat_terset_up_top_stack = 0LL;
    wdsat_terset_up_stack[wdsat_terset_up_top_stack++] = l;
    wdsat_xorset_up_top_stack = 0LL;
    wdsat_xorset_up_stack[wdsat_xorset_up_top_stack++] = l;
    _next_loop = true;
    while(_next_loop) {
		_next_loop = false;
		while(wdsat_terset_up_top_stack) {
			_l = wdsat_terset_up_stack[--wdsat_terset_up_top_stack];
            if(_terset_is_undef(_l)) _next_loop = true;
			if(!terset_set_true(_l)) {/*printf("ter contr %lld\n",_l);*/return false;}
		}
		while(wdsat_xorset_up_top_stack) {
			_l = wdsat_xorset_up_stack[--wdsat_xorset_up_top_stack];
			if(_xorset_is_undef(_l)) _next_loop = true;
			if(!xorset_set_true(_l)) {/*printf("xor contr %lld\n",_l);*/return false;}
		}
		wdsat_terset_up_top_stack = xorset_last_assigned(wdsat_terset_up_stack);
		wdsat_xorset_up_top_stack = terset_last_assigned(wdsat_xorset_up_stack);
	}
    return true;
}

bool wdsat_set_unitary(void) {
	bool _next_loop;
	int_t _l;
	wdsat_terset_up_top_stack = 0LL;
	wdsat_xorset_up_top_stack = 0LL;
	
	if(!terset_set_unitary()) return false;
	if(!xorset_set_unitary()) return false;
	wdsat_terset_up_top_stack = xorset_last_assigned(wdsat_terset_up_stack);
	wdsat_xorset_up_top_stack = terset_last_assigned(wdsat_xorset_up_stack);
	_next_loop = true;
	while(_next_loop) {
		_next_loop = false;
		while(wdsat_terset_up_top_stack) {
			_l = wdsat_terset_up_stack[--wdsat_terset_up_top_stack];
			if(_terset_is_undef(_l)) _next_loop = true;
			if(!terset_set_true(_l)) return false;
		}
		while(wdsat_xorset_up_top_stack) {
			_l = wdsat_xorset_up_stack[--wdsat_xorset_up_top_stack];
			if(_xorset_is_undef(_l)) _next_loop = true;
			if(!xorset_set_true(_l)) return false;
		}
		wdsat_terset_up_top_stack = xorset_last_assigned(wdsat_terset_up_stack);
		wdsat_xorset_up_top_stack = terset_last_assigned(wdsat_xorset_up_stack);
	}
	return true;
}

bool wdsat_solve_rest(int_t l, int_t set_end, int_t conf[]) {
	if(l > set_end)
	{
#ifdef __FIND_ALL_SOLUTIONS__
		printf("SAT:\n");
		for(int i = 1; i <= dimacs_nb_unary_vars(); i++)
			printf("%d", terset_assignment[i]);
		printf("\nconf:%lld\n", conf[0]);
		return false;
#endif
		return true;
	}
	if(!_terset_is_undef(set[l])) return wdsat_solve_rest(l + 1, set_end,conf);
	_terset_breakpoint;
	_xorset_breakpoint;
	conf[0]++;
	if(!wdsat_set_true(-set[l]))
	{
		terset_undo();
		xorset_undo();
		if(!wdsat_set_true(set[l])) return false;
		return wdsat_solve_rest(l + 1, set_end,conf);
		
	}
	else
	{
		if(!wdsat_solve_rest(l + 1, set_end,conf))
		{
			terset_undo();
			xorset_undo();
			if(!wdsat_set_true(set[l])) return false;
			return wdsat_solve_rest(l + 1, set_end,conf);
		}
		else
		{
			_terset_mergepoint;
			_xorset_mergepoint;
			return true;
		}
	}
}

bool wdsat_solve_rest_XG(int_t l, int_t nb_min_vars, int_t conf[]) {
	if(l > nb_min_vars)
	{
#ifdef __FIND_ALL_SOLUTIONS__
		printf("SAT:\n");
		for(int i = 1; i <= dimacs_nb_unary_vars(); i++)
			printf("%d", xorgauss_assignment[i]);
		printf("\nconf:%lld\n", conf[0]);
		return false;
#endif
		return true;
	}
#ifdef __DEBUG__
	printf("\nSetting:%d\n",set[l]);
	for(int i = 1; i <= dimacs_nb_unary_vars(); i++)
		printf("%d", xorgauss_assignment[i]);
	printf("\n");
#endif
	if(!_terset_is_undef(set[l])) return wdsat_solve_rest_XG(l + 1, nb_min_vars, conf);
	_terset_breakpoint;
	_xorset_breakpoint;
	_xorgauss_breakpoint;
	conf[0]++;
	if(!wdsat_infer(-set[l]))
	{
		terset_undo();
		xorset_undo();
		xorgauss_undo();
#ifdef __DEBUG__
		printf("lev:%d--undo on 0\n",set[l]);
		for(int i = 1; i <= dimacs_nb_unary_vars(); i++)
			printf("%d", xorgauss_assignment[i]);
		printf("\n");
#endif
	}
	else
	{
		if(!wdsat_solve_rest_XG(l + 1, nb_min_vars, conf))
		{
			terset_undo();
			xorset_undo();
			xorgauss_undo();
#ifdef __DEBUG__
			printf("lev:%d--undo on 0 profond\n",set[l]);
			for(int i = 1; i <= dimacs_nb_unary_vars(); i++)
				printf("%d", xorgauss_assignment[i]);
			printf("\n");
#endif
		}
		else
		{
#ifdef __DEBUG__
			printf("lev:%d--ok on 0\n",set[l]);
#endif
			_terset_mergepoint;
			_xorset_mergepoint;
			_xorgauss_mergepoint;
			return true;
		}
	}
	if(!wdsat_infer(set[l]))
	{
#ifdef __DEBUG__
		printf("lev:%d--undo on 1\n",set[l]);
#endif
		return false;
	}
#ifdef __DEBUG__
	for(int i = 1; i <= dimacs_nb_unary_vars(); i++)
		printf("%d", xorgauss_assignment[i]);
	printf("\n");
#endif
	return wdsat_solve_rest_XG(l + 1, nb_min_vars, conf);
}

bool go_left(int_t h, int_t h_end, bool search_prune_point, int_t conf[])
{
	conf[0]++;
	if(!wdsat_set_true(-set[h]))
		return false;
	
	return wdsat_solve_rest_sym(h + 1, h_end, search_prune_point, conf);
}

bool go_right(int_t h, int_t h_end, bool search_prune_point, int_t conf[])
{
	if(!wdsat_set_true(set[h]))
		return false;
	
	return wdsat_solve_rest_sym(h + 1, h_end, search_prune_point, conf);
}

/// @fn bool wdsat_solve_rest()
/// @brief look for any model
bool wdsat_solve_rest_sym(int_t h, int_t h_end, bool search_prune_point, int_t conf[]) {
	if(h > h_end)
	{
#ifdef __FIND_ALL_SOLUTIONS__
		printf("SAT:\n");
		for(int i = 1; i <= dimacs_nb_unary_vars(); i++)
			printf("%d", xorgauss_assignment[i]);
		printf("\nconf:%lld\n", conf[0]);
		return false;
#endif
		return true;
	}
	if(h % l == 0)
		search_prune_point = true;
	if(search_prune_point)
	{
		if((h - l < 0) || _terset_is_false(set[h - l]))
		{
			_terset_breakpoint;
			_xorset_breakpoint;
			if(!go_left(h, h_end, search_prune_point, conf))
			{
				terset_undo();
				xorset_undo();
				search_prune_point = false;
			}
			else
			{
				_terset_mergepoint;
				_xorset_mergepoint;
				return true;
			}
		}
		else// à supp
		{
			search_prune_point = true;
		}
	}
	else
	{
		_terset_breakpoint;
		_xorset_breakpoint;
		if(!go_left(h, h_end, search_prune_point, conf))
		{
			terset_undo();
			xorset_undo();
		}
		else
		{
			_terset_mergepoint;
			_xorset_mergepoint;
			return true;
		}
	}
	return go_right(h, h_end, search_prune_point, conf);
}

bool go_left_XG(int_t h, int_t h_end, bool search_prune_point, int_t conf[])
{
	conf[0]++;
	if(!wdsat_infer(-set[h]))
		return false;

	return wdsat_solve_rest_XG_sym(h + 1, h_end, search_prune_point, conf);
}

bool go_right_XG(int_t h, int_t h_end, bool search_prune_point, int_t conf[])
{
	if(!wdsat_infer(set[h]))
		return false;
	
	return wdsat_solve_rest_XG_sym(h + 1, h_end, search_prune_point, conf);
}

/// @fn bool wdsat_solve_rest()
/// @brief look for any model
bool wdsat_solve_rest_XG_sym(int_t h, int_t h_end, bool search_prune_point, int_t conf[]) {
	if(h > h_end)
	{
#ifdef __FIND_ALL_SOLUTIONS__
		printf("SAT:\n");
		for(int i = 1; i <= dimacs_nb_unary_vars(); i++)
			printf("%d", xorgauss_assignment[i]);
		printf("\nconf:%lld\n", conf[0]);
		return false;
#endif
		return true;
	}
	if(h % l == 0)
		search_prune_point = true;
	if(search_prune_point)
	{
		if((h - l < 0) || _terset_is_false(set[h - l]))
		{
			_terset_breakpoint;
			_xorset_breakpoint;
			_xorgauss_breakpoint;
			if(!go_left_XG(h, h_end, search_prune_point, conf))
			{
				terset_undo();
				xorset_undo();
				xorgauss_undo();
				search_prune_point = false;
			}
			else
			{
				_terset_mergepoint;
				_xorset_mergepoint;
				_xorgauss_mergepoint;
				return true;
			}
		}
		else// à supp
		{
			search_prune_point = true;
		}
	}
	else
	{
		_terset_breakpoint;
		_xorset_breakpoint;
		_xorgauss_breakpoint;
		if(!go_left_XG(h, h_end, search_prune_point, conf))
		{
			terset_undo();
			xorset_undo();
			xorgauss_undo();
		}
		else
		{
			_terset_mergepoint;
			_xorset_mergepoint;
			_xorgauss_mergepoint;
			return true;
		}
	}
	return go_right_XG(h, h_end, search_prune_point, conf);
}

bool wdsat_infer(const int_t l) {
	bool _loop_pass = true;
	bool _continue;
	int_t terset_history_it;
	int_t terset_history_last = terset_history_top;
	int_t xorgauss_history_it;
	int_t xorgauss_history_last = xorgauss_history_top;
	int_t _l;
	
	if(!wdsat_set_true(l)) return false;
	while(_loop_pass) {
		// finalyse with XORGAUSS
		_continue = false;
		terset_history_it = terset_history_top;
		while(terset_history_it > terset_history_last) {
			_l = terset_history[--terset_history_it];
			if(_xorgauss_is_undef(_l)) {
				if(!xorgauss_set_true(_l)) return false;
				_continue = true;
			}
		}
		terset_history_last = terset_history_top;
		_loop_pass = false;
		if(_continue) {
			// get list of literal set thanks to XORGAUSS
			xorgauss_history_it = xorgauss_history_top;
			while(xorgauss_history_it > xorgauss_history_last) {
				_l = xorgauss_history[--xorgauss_history_it];
				if(_terset_is_false(_l)) return false;
				if(_terset_is_undef(_l)) {
					_loop_pass = true;
					if(!wdsat_set_true(_l)) return false;
				}
			}
			xorgauss_history_last = xorgauss_history_top;
		}
	}
	return true;
}

bool wdsat_infer_unitary() {
	bool _loop_pass = true;
	bool _continue;
	int_t terset_history_it;
	int_t terset_history_last = terset_history_top;
	int_t xorgauss_history_it;
	int_t xorgauss_history_last = xorgauss_history_top;
	int_t _l;
	
	if(!wdsat_set_unitary()) return false;
	while(_loop_pass) {
		// finalyse with XORGAUSS
		_continue = false;
		terset_history_it = terset_history_top;
		while(terset_history_it > terset_history_last) {
			_l = terset_history[--terset_history_it];
			if(_xorgauss_is_undef(_l)) {
				if(!xorgauss_set_true(_l)) return false;
				_continue = true;
			}
		}
		terset_history_last = terset_history_top;
		_loop_pass = false;
		if(_continue) {
			// get list of literal set thanks to XORGAUSS
			xorgauss_history_it = xorgauss_history_top;
			while(xorgauss_history_it > xorgauss_history_last) {
				_l = xorgauss_history[--xorgauss_history_it];
				if(_terset_is_false(_l)) return false;
				if(_terset_is_undef(_l)) {
					_loop_pass = true;
					if(!wdsat_set_true(_l)) return false;
				}
			}
			xorgauss_history_last = xorgauss_history_top;
		}
	}
	return true;
}

/// @fn solve();
/// @return false if formula is unsatisfiable and true otherwise
bool wdsat_solve(int_t n, int_t new_l, int_t new_m, char *irr, char *X3, int_t xg, char mvc_graph[1000], char thread[1000]) {
	int_t i, j;
	int_t nb_min_vars;
	int_t conf[1]={0};
	bool seen[50]={0};
	terset_initiate_from_dimacs();
	xorset_initiate_from_dimacs();
	if(!xorgauss_initiate_from_dimacs())
	{
		printf("UNSAT on XORGAUSS init\n");
		return false;
	}
	cpy_from_dimacs();
	//terset_fprint();
	//xorset_fprint();
	//xorgauss_fprint();
	
	//check allocated memory
	if(dimacs_nb_vars() < __ID_SIZE__)
		printf("\n!!! Running times are not optimal with these parameters. Set the __ID_SIZE__ constant to %d !!!\n\n", dimacs_nb_vars());
	
	//code for multithread: assign prefix for this thread
	if(strlen(thread) > 0)
	{
		char *str_l;
		int_t l;
		str_l = strtok (thread, ",");
		while(str_l != NULL)
		{
			l = atoi(str_l);
			terset_up_stack[terset_up_top_stack++] = l;
			assert(terset_up_top_stack < __ID_SIZE__);
			str_l = strtok (NULL, ",");
		}
	}
	// end code for multithread (this has to be done before wdsat_infer_unitary();
	
	wdsat_infer_unitary();
	if(strlen(mvc_graph) > 0)
	{
		nb_min_vars = 0;
		char *str_l;
		j = 0;
		str_l = strtok (mvc_graph, ",");
		while(str_l != NULL)
		{
			set[j] = atoi(str_l);
			str_l = strtok (NULL, ",");
			nb_min_vars++;
			j++;
		}
	}
	else
	{
#ifdef __XG_ENHANCED__
		nb_min_vars = dimacs_nb_unary_vars();
#else
		nb_min_vars = dimacs_nb_vars();
#endif
		for(j = 1; j <= nb_min_vars; j++)
		{
			set[j - 1] = j;
		}
	}
	
	if(xg == 0)
	{
		if(!wdsat_solve_rest(0, nb_min_vars - 1, conf)) {printf("UNSAT\n");printf("%lld\n",conf[0]);return false;}
	}
	if(xg == 1)
	{
		if(!wdsat_solve_rest_XG(0, nb_min_vars - 1, conf)) {printf("UNSAT\n");printf("%lld\n",conf[0]);return false;}
	}
	for(j = 1; j <= dimacs_nb_unary_vars(); j++)
	{
		printf("%d", terset_assignment[j]);
	}
	printf("\n");
	
	printf("%lld\n",conf[0]);
	
	return (true);

}
