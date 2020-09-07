//
//  dimacs.h
//  WDSat
//
//  Created by Gilles Dequen and Monika Trimoska on 15/12/2018.
//  Copyright © 2018 Gilles Dequen. All rights reserved.
//

#ifndef __dimacs_h__
#define __dimacs_h__

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "wdsat_utils.h"

/// States of reading automata
#define ADOS   0
#define ADOI   1
#define ADOSNR 2
#define ADONR  3

/// States of reading automata for header
#define MN_START   0
#define MN_IN      1
#define MN_VAR_1   2
#define MN_VAR_2   3
#define MN_ROUND_1 4
#define MN_ROUND_2 5
#define MN_MODEL_1 6
#define MN_MODEL_2 7
#define MN_1       8
#define MN_2       9
#define MN_3       10
#define MN_TMP_1   11
#define MN_4       12
#define MN_TMP_2   13
#define MN_NORMAL  14

#define __MAX_VARS__  __MAX_ID__

/// States of reading automata for formula
#define SAT_START   0
#define SAT_XMODE   1
#define SAT_BMODE   2
#define SAT_IN_ATOM 3
#define SAT_IN_EQ   4


#ifdef __XG_ENHANCED__
extern uint_t monomials_to_column[__MAX_ANF_ID__][__ID_SIZE__][__MAX_DEGREE__ - 1];
#endif

void dimacs_read_header(FILE * f);
void dimacs_print_header(void);

void dimacs_read_formula(FILE * f);
void dimacs_print_formula(void);

void dimacs_free_meaning(void);
void dimacs_generate_meaning(void);
void dimacs_print_assigned_meaning(void);
char *dimacs_get_meaning(const int_t i);
boolean_t dimacs_get_truth_value(const int_t);
int_t dimacs_get_assigned_ids(int_t *);
uint_t dimacs_get_ids_from_key(uint_t *, const char *);


const int_t dimacs_nb_vars(void);
const int_t dimacs_nb_equations(void);
const int_t dimacs_nb_atoms(void);
const int_t dimacs_nb_xor_equations(void);
const int_t dimacs_nb_xor_atoms(void);
int_t * dimacs_equation(const int_t);
int_t * dimacs_size_of_equations(void);
const int_t dimacs_size_of_equation(const int_t);
int_t * dimacs_xor(const int_t);
int_t * dimacs_size_of_xor_equations(void);
const int_t dimacs_size_of_xor(const int_t);
int_t ** get_dimacs_xor_equation(void);
bool dimacs_xor_constant(const int_t i);
boolean_t dimacs_get_current_degree(const int_t i);

const bool dimacs_is_header_read(void);
const bool dimacs_is_read(void);
const int_t dimacs_nb_unary_vars(void);
#endif /* dimacs_h */
