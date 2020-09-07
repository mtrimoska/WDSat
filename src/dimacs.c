//
//  dimacs.c
//  WDSat
//
//  Created by Gilles Dequen and Monika Trimoska on 15/12/2018.
//  Copyright © 2018 Gilles Dequen. All rights reserved.
//

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>

#include "dimacs.h"
#include "wdsat_utils.h"

static bool _dimacs_header_read = false;
static bool _dimacs_read = false;

/// Number of atoms
static int_t dimacs_nb_of_vars = 0LL;
/// Number of rounds
static int_t dimacs_nb_of_rounds = 0LL;
/// Number of models
static int_t dimacs_nb_of_models = 0LL;
/// Number of atoms
static int_t dimacs_nb_of_atoms_be = 0LL;
static int_t dimacs_nb_of_atoms_xe = 0LL;

/// from meaning
static char * dimacs_meaning[__ID_SIZE__];
static byte_t dimacs_truth_value[__ID_SIZE__];

/// number of boolean equations
static int_t dimacs_nb_of_boolean_equations;
static int_t dimacs_boolean_equation[__MAX_EQ__][__MAX_EQ_SIZE__];
static int_t dimacs_size_of_boolean_equation[__MAX_EQ__];
static int_t dimacs_size_max_among_boolean_equations;
static int_t dimacs_size_min_among_boolean_equations;

/// number of xor equations
static int_t dimacs_nb_of_xor_equations;
static int_t dimacs_xor_equation[__MAX_XEQ__][__MAX_XEQ_SIZE__];
static int_t dimacs_size_of_xor_equation[__MAX_XEQ__];
static int_t dimacs_size_max_among_xor_equations;
static int_t dimacs_size_min_among_xor_equations;
static bool dimacs_xor_equation_constant[__MAX_XEQ__];

extern byte_t thread_digits;

static int_t dimacs_nb_of_unary_vars = 0LL;
inline const int_t dimacs_nb_unary_vars() { return dimacs_nb_of_unary_vars; }

#ifdef __XG_ENHANCED__
static int_t dimacs_nb_of_eq = 0LL;
//monomials degree > 1
uint_t monomials_to_column[__MAX_ANF_ID__][__ID_SIZE__][__MAX_DEGREE__ - 1];
boolean_t dimacs_current_degree[__ID_SIZE__];
#endif

/* ----------------- Getters, Setters */
inline const int_t dimacs_nb_vars() { return dimacs_nb_of_vars; }
inline const int_t dimacs_nb_equations() { return dimacs_nb_of_boolean_equations; }
inline const int_t dimacs_nb_atoms() { return dimacs_nb_of_atoms_be; }
inline const int_t dimacs_nb_xor_equations() { return dimacs_nb_of_xor_equations; }
inline const int_t dimacs_nb_xor_atoms() { return dimacs_nb_of_atoms_xe; }
inline int_t * dimacs_equation(const int_t i) { return dimacs_boolean_equation[i]; }
inline int_t * dimacs_size_of_equations() { return dimacs_size_of_boolean_equation; }
inline const int_t dimacs_size_of_equation(const int_t i) { return dimacs_size_of_boolean_equation[i]; }
inline int_t * dimacs_xor(const int_t i) { return dimacs_xor_equation[i]; }
inline int_t * dimacs_size_of_xor_equations() { return dimacs_size_of_xor_equation; }
inline const int_t dimacs_size_of_xor(const int_t i) { return dimacs_size_of_xor_equation[i]; }
inline const bool dimacs_is_header_read() { return(_dimacs_header_read); }
inline const bool dimacs_is_read() { return(_dimacs_read); }
inline int_t ** get_dimacs_xor_equation() { return dimacs_xor_equation; }
inline bool dimacs_xor_constant(const int_t i) { return dimacs_xor_equation_constant[i]; }
#ifdef __XG_ENHANCED__
inline boolean_t dimacs_get_current_degree(const int_t i) { return dimacs_current_degree[i]; }
#endif
/* ----------------- */

void dimacs_generate_meaning() {
	assert(dimacs_nb_of_vars > 0);
	int_t i, n_vsz = fast_int_log10(dimacs_nb_of_vars);
	char * tmp_str;
	dimacs_meaning[0] = NULL;
	for(i = 1; i <= dimacs_nb_of_vars; ++i) {
		tmp_str = NULL;
		_get_mem(char, tmp_str, (n_vsz + 2));
		sprintf(tmp_str, "x%0*lld", (int) n_vsz, i);
		dimacs_meaning[i] = tmp_str;
		dimacs_truth_value[i] = __UNDEF__;
	}
}

#ifdef __XG_ENHANCED__
void dimacs_read_formula(FILE *f) {
	char str_clause[__STATIC_CLAUSE_STRING_SIZE__] = {0};
	int_t l, i, j, deg, a_temp, d, k;
	int_t a[__MAX_DEGREE__];
	char *str_l;
	
	if(f == NULL) return;
	
	//set all values to 0. 0 will be used to delimit a list
	//this can cause a problem if __MAX_DEGREE__ is not set properly
	memset(monomials_to_column, 0, sizeof(uint_t)*__MAX_ANF_ID__*__ID_SIZE__*(__MAX_DEGREE__ - 1));
	
	fgets(str_clause, sizeof(str_clause), f);
	str_l = strtok (str_clause, " ");
	str_l = strtok (NULL, " ");
	str_l = strtok (NULL, " ");
	dimacs_nb_of_unary_vars = atoi(str_l);
	str_l = strtok (NULL, " ");
	dimacs_nb_of_eq = atoi(str_l);
	
	assert(dimacs_nb_of_unary_vars <= __MAX_ANF_ID__);
	
	/// start reading the file
	dimacs_nb_of_xor_equations = 0ULL;
	memset((void *) dimacs_size_of_xor_equation, 0, __MAX_XEQ__ * sizeof(int_t));
	
	dimacs_nb_of_vars = dimacs_nb_of_unary_vars;
	while (fgets(str_clause, sizeof(str_clause), f) != NULL )
	{
		assert(dimacs_nb_of_xor_equations < dimacs_nb_of_eq);
		dimacs_xor_equation_constant[dimacs_nb_of_xor_equations] = false;
		str_l = strtok (str_clause, " ");
		if(str_l[0] == 'x')
		{
			str_l = strtok (NULL, " ");
			while(str_l != NULL)
			{
				if(str_l[0] == '0')
					break;
				if(strcmp(str_l, "T") == 0)
				{
					dimacs_xor_equation_constant[dimacs_nb_of_xor_equations] = (dimacs_xor_equation_constant[dimacs_nb_of_xor_equations] + 1) % 2;
					str_l = strtok (NULL, " ");
					continue;
				}
				if(str_l[0] == '.')
				{
					deg = str_l[1] - '0'; //att if __MAX_DEGREE__ > 9
					assert(deg < __MAX_DEGREE__);
					for(i = 0; i < deg; i++)
					{
						str_l = strtok (NULL, " ");
						assert(str_l != NULL);
						a_temp = atoi(str_l);
						assert(a_temp <= dimacs_nb_of_unary_vars);
						for(j = i; j > 0; j--)
						{
							if(a[j - 1] > a_temp)
							{
								a[j] = a[j - 1];
							}
							else
							{
								break;
							}
						}
						a[j] = a_temp;
					}
					
					//check if monomial already occured
					l = 0;
					i = 0;
					while(monomials_to_column[a[0]][i][0])
					{
						for(j = 1; j < deg; j++)
						{
							if(monomials_to_column[a[0]][i][j] != a[j])
							{
								break;
							}
						}
						if(j == deg)
						{
							if(j == __MAX_DEGREE__ - 1 || monomials_to_column[a[0]][i][j] == 0) //check if right degree monomial
							{
								l = monomials_to_column[a[0]][i][0];
								break;
							}
						}
						i++;
					}
					
					if(!l)
					{
						l = ++dimacs_nb_of_vars;
						assert(dimacs_nb_of_vars <= __ID_SIZE__);
						for(j = 0; j < deg; j++)
						{
							i = 0;
							while(monomials_to_column[a[j]][i][0] > 0) i++;
							monomials_to_column[a[j]][i][0] = l;
							k = 1;
							for(d = 0; d < deg; d++)
							{
								if(d != j)
								{
									monomials_to_column[a[j]][i][k] = a[d];
									k++;
								}
							}
							//cnf part
							dimacs_boolean_equation[dimacs_nb_of_boolean_equations + j][0] = -l;
							++dimacs_nb_of_atoms_be;
							dimacs_boolean_equation[dimacs_nb_of_boolean_equations + j][1] = a[j];
							++dimacs_nb_of_atoms_be;
							dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations + j] = 2;
							
							dimacs_boolean_equation[dimacs_nb_of_boolean_equations + deg][j] = -a[j];
							++dimacs_nb_of_atoms_be;
						}
						dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations + deg] = deg + 1;
						dimacs_boolean_equation[dimacs_nb_of_boolean_equations + deg][deg] = l;
						++dimacs_nb_of_atoms_be;
						assert(dimacs_nb_of_atoms_be < __MAX_BUFFER_SIZE__);
						dimacs_nb_of_boolean_equations += (deg + 1);
						assert(dimacs_nb_of_boolean_equations < __MAX_EQ__);
						assert(deg + 1 < __MAX_EQ_SIZE__);
						if(deg + 1 > dimacs_size_max_among_boolean_equations) dimacs_size_max_among_boolean_equations = deg + 1;
						if((!dimacs_size_min_among_boolean_equations) || (2 < dimacs_size_min_among_boolean_equations))
							dimacs_size_min_among_boolean_equations = 2;
						dimacs_current_degree[l] = deg;
						
						//transform to dimacs for other solvers
						/*for(j = 0; j < deg; j++)
						{
							fprintf(stderr,"%lld %lld 0\n", -l, a[j]);
						}
						fprintf(stderr,"%lld ", l);
						for(j = 0; j < deg; j++)
						{
							fprintf(stderr,"%lld ", -a[j]);
						}
						fprintf(stderr,"0\n");*/
					}
				}
				else
				{
					l = atoi(str_l);
					assert(l <= dimacs_nb_of_unary_vars);
				}
				dimacs_xor_equation[dimacs_nb_of_xor_equations][dimacs_size_of_xor_equation[dimacs_nb_of_xor_equations]++] = l;
				++dimacs_nb_of_atoms_xe;
				assert(dimacs_nb_of_atoms_xe < __MAX_BUFFER_SIZE__);
				assert(dimacs_size_of_xor_equation[dimacs_nb_of_xor_equations] < __MAX_XEQ_SIZE__);
				
				str_l = strtok (NULL, " ");
			}
			if(dimacs_size_of_xor_equation[dimacs_nb_of_xor_equations] > dimacs_size_max_among_xor_equations) dimacs_size_max_among_xor_equations = dimacs_size_of_xor_equation[dimacs_nb_of_xor_equations];
			if((!dimacs_size_min_among_xor_equations) || (dimacs_size_of_xor_equation[dimacs_nb_of_xor_equations] < dimacs_size_min_among_xor_equations)) dimacs_size_min_among_xor_equations = dimacs_size_of_xor_equation[dimacs_nb_of_xor_equations];
			if(dimacs_xor_constant(dimacs_nb_of_xor_equations))
			{
				dimacs_xor_equation[dimacs_nb_of_xor_equations][0] = -dimacs_xor_equation[dimacs_nb_of_xor_equations][0];
			}
			assert(dimacs_nb_of_xor_equations < __MAX_XEQ__);
			dimacs_nb_of_xor_equations++;
		}
		else
		{
			dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations] = 1;
			dimacs_boolean_equation[dimacs_nb_of_boolean_equations++][0] = atoi(str_l);
			++dimacs_nb_of_atoms_be;
			dimacs_size_min_among_boolean_equations = 1;
			assert(dimacs_nb_of_atoms_be < __MAX_BUFFER_SIZE__);
			assert(dimacs_nb_of_boolean_equations < __MAX_EQ__);
			str_l = strtok (NULL, " ");
			assert(str_l[0] == '0');
		}
	}
	assert(dimacs_nb_of_vars <= __ID_SIZE__);
}
void dimacs_read_header(FILE *f) {}
#else

void dimacs_print_assigned_meaning() {
    const int_t nbv = dimacs_nb_of_vars;
    for(int_t i = 1LL; i <= nbv; ++i) {
        if(dimacs_truth_value[i] == __UNDEF__) continue;
        _cid_cout("%s : %s\n", dimacs_meaning[i],
                 (dimacs_truth_value[i] == __TRUE__) ? "TRUE" : "FALSE");
    }
}

uint_t dimacs_get_ids_from_key(uint_t *buffer, const char *key) {
    static uint_t _buffer;
    _buffer = 0;
    for(int_t i = 1LL; i <= dimacs_nb_of_vars; ++i) {
        if(strstr(dimacs_meaning[i], key) == NULL) continue;
        buffer[_buffer++] = (uint_t) i;
    }
    buffer[_buffer] = 0ULL;
    return(_buffer);
}

/// @fn int_t dimacs_get_assigned_ids(int_t *buffer)
/// @brief It returns the list of literals that are assigend to TRUE at the
/// begin of the process (i.e. those that are set into the dimacs header)
int_t dimacs_get_assigned_ids(int_t *buffer) {
    static int_t _buffer;;
    _buffer = 0;
    for(int_t i = 1LL; i <= dimacs_nb_of_vars; ++i) {
        const boolean_t _tv = dimacs_truth_value[i];
        if(_tv == __UNDEF__) continue;
        buffer[_buffer++] = (_tv == __TRUE__) ? i : -i;
    }
    buffer[_buffer] = 0LL;
    return(_buffer);
}

char *dimacs_get_meaning(const int_t i) { return(dimacs_meaning[i]); }

inline boolean_t dimacs_get_truth_value(const int_t i) { return(dimacs_truth_value[i]); }

void dimacs_free_meaning() {
    int_t v;
    for(v = 0LL; v <= __MAX_ID__; ++v)
        _free_mem(dimacs_meaning[v]);
}

/**
 * @fn void read_dimacs_header(FILE *f)
 * @brief read and build a meaning structure that associates any information to variables or others
 */
void dimacs_read_header(FILE *f) {
    char c;
    int_t status = MN_START, v = 0LL;
    boolean_t end_reading = __OFF__;
    _vector_id(char, buf);
    
    if(f == NULL) return;
    dimacs_nb_of_vars = (int_t) 0LL;
    _dimacs_header_read = false;
    /// reset meaning
    for(v = 0LL; v <= __MAX_ID__; ++v) dimacs_meaning[v] = NULL;
    while(!end_reading) {
        if(feof(f)) {
            end_reading = __ON__;
            break;
        }
        fscanf(f, "%c", &c);
        switch(status) {
            case MN_START:
                switch(c) {
                    case 'p' :
                        fseek(f, (long) (-1), SEEK_CUR);
                        return;
                    case 'c' :
                        status = MN_IN;
                        break;
                    default:
                        goto end_free_meaning;
                }
                break;
            case MN_IN:
                switch(c) {
                    case 'v' :
                        status = MN_1;
                        _vector_reset(buf);
                        _dimacs_header_read = true;
                        break;
                    case 'n' :
                        status = MN_VAR_1;
                        _dimacs_header_read = true;
                        break;
                    case 'm' :
                        status = MN_MODEL_1;
                        _dimacs_header_read = true;
                        break;
                    case 'r' :
                        status = MN_ROUND_1;
                        _dimacs_header_read = true;
                        break;
                    case '\n' :
                        status = MN_START;
                        _dimacs_header_read = true;
                        break;
                    default:
                        status = MN_NORMAL;
                        break;
                }
                break;
            case MN_NORMAL:
                switch(c) {
                    case '\n' :
                        status = MN_START;
                    default:
                        break;
                }
                break;
            case MN_1:
                switch(c) {
                    case ' ':
                        break;
                    case '0' :
                    case '1' :
                    case '2' :
                    case '3' :
                    case '4' :
                    case '5' :
                    case '6' :
                    case '7' :
                    case '8' :
                    case '9' :
                        if(!dimacs_nb_of_vars) {
                            _cid_cout("Bad input format", "");
                            return;
                        };
                        _vector_append(char, buf, c);
                        status = MN_2;
                        break;
                    case '\n' :
                        status = MN_START;
                        break;
                    default:
                        goto end_free_meaning;
                }
                break;
            case MN_2:
                switch(c) {
                    case '\n' :
                        status = MN_START;
                        break;
                    case '0' :
                    case '1' :
                    case '2' :
                    case '3' :
                    case '4' :
                    case '5' :
                    case '6' :
                    case '7' :
                    case '8' :
                    case '9' :
                        if(!dimacs_nb_of_vars) {
                            _cid_cout("Bad input format", "");
                            return;
                        };
                        _vector_append(char, buf, c);
                        break;
                    case ' ' :
                        status = MN_TMP_1;
                        if(!dimacs_nb_of_vars) {
                            _cid_cout("Bad input format", "");
                            return;
                        }
                        _vector_append(char, buf, '\0');
                        v = atoll(buf);
                        _vector_reset(buf);
                        break;
                    default:
                        goto end_free_meaning;
                }
                break;
            case MN_TMP_1:
                switch(c) {
                    case ' ' :
                        break;
                    default :
                        status = MN_3;
                        if(!dimacs_nb_of_vars) {
                            _cid_cout("Bad input format", "");
                            return;
                        }
                        _vector_append(char, buf, c);
                        break;
                }
                break;
            case MN_3:
                switch(c) {
                    case ' ' :
                        status = MN_TMP_2;
                        _vector_append(char, buf, '\0');
                        dimacs_meaning[v] = _vector(buf);
                        _init_vector(char, buf);
                        break;
                    case '\n' :
                        _vector_append(char, buf, '\0');
                        dimacs_meaning[v] = _vector(buf);
                        _init_vector(char, buf);
                        status = MN_START;
                        break;
                    default:
                        if(!dimacs_nb_of_vars) {
                            _cid_cout("Bad input format", "");
                            return;
                        }
                        _vector_append(char, buf, c);
                }
                break;
            case MN_TMP_2:
                switch(c) {
                    case ' ' : break;
                    default  :
                        status = MN_4;
                        if(!dimacs_nb_of_vars) {
                            _cid_cout("Bad input format", "");
                            return;
                        }
                        _vector_append(char, buf, c);
                        break;
                }
                break;
            case MN_4 :
                switch(c) {
                    case '\n' :
                        status = MN_START;
                        _vector_append(char, buf, '\0');
                        if(!strcmp(_vector(buf), "TRUE")) dimacs_truth_value[v] = __ON__;
                        else if(!strcmp(_vector(buf), "FALSE")) dimacs_truth_value[v] = __OFF__;
                        else dimacs_truth_value[v] = __UNDEF__;
                        _vector_reset(buf);
                        break;
                    default:
                        _vector_append(char, buf, c);
                        break;
                }
                break;
            case MN_VAR_1:
                switch(c) {
                    case ' ' :
                        break;
                    case '0' :
                    case '1' :
                    case '2' :
                    case '3' :
                    case '4' :
                    case '5' :
                    case '6' :
                    case '7' :
                    case '8' :
                    case '9' :
                        _vector_append(char, buf, c);
                        status = MN_VAR_2;
                        break;
                    default:
                        goto end_free_meaning;
                }
                break;
            case MN_VAR_2:
                switch(c) {
                    case '\n' :
                        _vector_append(char, buf, '\0');
                        dimacs_nb_of_vars = atoll(_vector(buf));
                        _vector_reset(buf);
                        status = MN_START;
                        assert(dimacs_nb_of_vars < __MAX_ID__);
                        break;
                    case '0' :
                    case '1' :
                    case '2' :
                    case '3' :
                    case '4' :
                    case '5' :
                    case '6' :
                    case '7' :
                    case '8' :
                    case '9' :
                        _vector_append(char, buf, c);
                        break;
                    default:
                        goto end_free_meaning;
                }
                break;
            case MN_MODEL_1:
                switch(c) {
                    case ' ' :
                        break;
                    case '0' :
                    case '1' :
                    case '2' :
                    case '3' :
                    case '4' :
                    case '5' :
                    case '6' :
                    case '7' :
                    case '8' :
                    case '9' :
                        _vector_append(char, buf, c);
                        status = MN_MODEL_2;
                        break;
                    default:
                        goto end_free_meaning;
                }
                break;
            case MN_MODEL_2:
                switch(c) {
                    case '\n' :
                        _vector_append(char, buf, '\0');
                        dimacs_nb_of_models = atoll(_vector(buf));
                        _vector_reset(buf);
                        status = MN_START;
                        break;
                    case '0' :
                    case '1' :
                    case '2' :
                    case '3' :
                    case '4' :
                    case '5' :
                    case '6' :
                    case '7' :
                    case '8' :
                    case '9' :
                        _vector_append(char, buf, c);
                        break;
                    default:
                        goto end_free_meaning;
                }
                break;
            case MN_ROUND_1:
                switch(c) {
                    case ' ' :
                        break;
                    case '0' :
                    case '1' :
                    case '2' :
                    case '3' :
                    case '4' :
                    case '5' :
                    case '6' :
                    case '7' :
                    case '8' :
                    case '9' :
                        _vector_append(char, buf, c);
                        status = MN_ROUND_2;
                        break;
                    default:
                        goto end_free_meaning;
                }
                break;
            case MN_ROUND_2:
                switch(c) {
                    case '\n' :
                        _vector_append(char, buf, '\0');
                        dimacs_nb_of_rounds = atoll(_vector(buf));
                        _vector_reset(buf);
                        status = MN_START;
                        break;
                    case '0' :
                    case '1' :
                    case '2' :
                    case '3' :
                    case '4' :
                    case '5' :
                    case '6' :
                    case '7' :
                    case '8' :
                    case '9' :
                        _vector_append(char, buf, c);
                        break;
                    default:
                        goto end_free_meaning;
                }
                break;
        }
    }
    goto end_meaning;
end_free_meaning:
    dimacs_free_meaning();
    return;
end_meaning:
    return;
}

/**
 * @fn void read_dimacs(FILE *f)
 * @author Gilles Dequen
 * @brief Read a dimacs input file and store it into a sat_formula structure.
 */
void dimacs_read_formula(FILE *f) {
    boolean_t end_reading = __OFF__;
    int_t n_c, v, status = SAT_START;
    char tmp_str[__STATIC_STRING_SIZE__], c;
    int _tmp_str = 0;
    boolean_t xor_mode = __OFF__;
    
    if(f == NULL) return;

    _dimacs_read = false;
    dimacs_nb_of_atoms_be = dimacs_nb_of_atoms_xe = 0LL;
    
    /// get number of atoms and check bad format if occurs
    if (!fscanf(f, "p cnf %llu %llu", &dimacs_nb_of_vars, &n_c) || (feof(f))) {
        _cid_cout("Bad input format", "");
        return;
    }
	assert(dimacs_nb_of_vars <= __MAX_ID__);
    assert(n_c < (__MAX_EQ__ + __MAX_XEQ__));
    
    /// start reading the file
    dimacs_nb_of_boolean_equations = dimacs_nb_of_xor_equations = 0ULL;
    dimacs_size_max_among_boolean_equations = dimacs_size_max_among_xor_equations = 0ULL;
    dimacs_size_min_among_boolean_equations = dimacs_size_min_among_xor_equations = 0ULL;
    memset((void *) dimacs_size_of_boolean_equation, 0, __MAX_EQ__ * sizeof(int_t));
    memset((void *) dimacs_size_of_xor_equation, 0, __MAX_XEQ__ * sizeof(int_t));
    
    while(!end_reading) {
        if(feof(f)) {
            end_reading = __ON__;
            break;
        }
        fscanf(f, "%c", &c);
        switch(status) {
            case SAT_START :
                _tmp_str = 0;
                switch(c) {
                    case '\n' :
                    case ' '  :
                        break;
                    case 'x':
                        status = SAT_XMODE;
                        xor_mode = __ON__;
                        break;
                    case '-' :
                    case '0' :
                    case '1' :
                    case '2' :
                    case '3' :
                    case '4' :
                    case '5' :
                    case '6' :
                    case '7' :
                    case '8' :
                    case '9' :
                        status = SAT_BMODE;
                        tmp_str[_tmp_str++] = c;
                        break;
                    default:
                        return;
                }
                break;
            case SAT_XMODE :
                switch(c) {
                    case '\t' :
                    case ' '  :
                        break;
                    case '\n' :
                        status = SAT_START;
                        xor_mode = __OFF__;
                        break;
                    case '-' :
                    case '0' :
                    case '1' :
                    case '2' :
                    case '3' :
                    case '4' :
                    case '5' :
                    case '6' :
                    case '7' :
                    case '8' :
                    case '9' :
                        status = SAT_IN_ATOM;
                        tmp_str[_tmp_str++] = c;
                        break;
                    default:
                        return;
                }
                break;
            case SAT_BMODE :
                switch(c) {
                    case '\n' :
                        status = SAT_START;
                        tmp_str[_tmp_str] = 0;
                        v = atoll(tmp_str);
                        if(v) {
                            dimacs_boolean_equation[dimacs_nb_of_boolean_equations][dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations]++] = v;
                            ++dimacs_nb_of_atoms_be;
                            assert(dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations] < __MAX_EQ_SIZE__);
                            assert(dimacs_nb_of_atoms_be < __MAX_BUFFER_SIZE__);
                        }
                        if(dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations] > dimacs_size_max_among_boolean_equations) dimacs_size_max_among_boolean_equations = dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations];
                        if((!dimacs_size_min_among_boolean_equations) || (dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations] < dimacs_size_min_among_boolean_equations))
                            dimacs_size_min_among_boolean_equations = dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations];
                        dimacs_nb_of_boolean_equations++;
                        assert(dimacs_nb_of_boolean_equations < __MAX_EQ__);
                        _tmp_str = 0;
                        break;
                    case '\t' :
                    case ' '  :
                        status = SAT_IN_EQ;
                        tmp_str[_tmp_str] = 0;
                        v = atoll(tmp_str);
                        if(v) {
                            dimacs_boolean_equation[dimacs_nb_of_boolean_equations][dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations]++] = v;
                            ++dimacs_nb_of_atoms_be;
                            assert(dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations] < __MAX_EQ_SIZE__);
                            assert(dimacs_nb_of_atoms_be < __MAX_BUFFER_SIZE__);
                        }
                        _tmp_str = 0;
                        break;
                    case '-' :
                    case '0' :
                    case '1' :
                    case '2' :
                    case '3' :
                    case '4' :
                    case '5' :
                    case '6' :
                    case '7' :
                    case '8' :
                    case '9' :
                        status = SAT_IN_ATOM;
                        tmp_str[_tmp_str++] = c;
                        break;
                    default:
                        return;
                }
                break;
            case SAT_IN_ATOM :
                switch(c) {
                    case '\t' :
                    case ' '  :
                        status = SAT_IN_EQ;
                        tmp_str[_tmp_str] = 0;
                        v = atoll(tmp_str);
                        if(v) {
                            if(!xor_mode) {
                                dimacs_boolean_equation[dimacs_nb_of_boolean_equations][dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations]++] = v;
                                ++dimacs_nb_of_atoms_be;
                                assert(dimacs_nb_of_atoms_be < __MAX_BUFFER_SIZE__);
                                assert(dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations] < __MAX_EQ_SIZE__);
                            } else {
                                dimacs_xor_equation[dimacs_nb_of_xor_equations][dimacs_size_of_xor_equation[dimacs_nb_of_xor_equations]++] = v;
                                ++dimacs_nb_of_atoms_xe;
                                assert(dimacs_nb_of_atoms_xe < __MAX_BUFFER_SIZE__);
                                assert(dimacs_size_of_xor_equation[dimacs_nb_of_xor_equations] < __MAX_XEQ_SIZE__);
                            }
                        }
                        _tmp_str = 0;
                        break;
                    case '-' :
                    case '0' :
                    case '1' :
                    case '2' :
                    case '3' :
                    case '4' :
                    case '5' :
                    case '6' :
                    case '7' :
                    case '8' :
                    case '9' :
                        tmp_str[_tmp_str++] = c;
                        break;
                    case '\n':
                        status = SAT_START;
                        tmp_str[_tmp_str] = 0;
                        v = atoll(tmp_str);
                        if(!xor_mode) {
                            if(v) {
                                dimacs_boolean_equation[dimacs_nb_of_boolean_equations][dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations]++] = v;
                                ++dimacs_nb_of_atoms_be;
                                assert(dimacs_nb_of_atoms_be < __MAX_BUFFER_SIZE__);
                                assert(dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations] < __MAX_EQ_SIZE__);
                            }
                            if(dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations] > dimacs_size_max_among_boolean_equations) dimacs_size_max_among_boolean_equations = dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations];
                            if((!dimacs_size_min_among_boolean_equations) || (dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations] < dimacs_size_min_among_boolean_equations)) dimacs_size_min_among_boolean_equations = dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations];
                            ++dimacs_nb_of_boolean_equations;
                            assert(dimacs_nb_of_boolean_equations < __MAX_EQ__);
                        } else {
                            if(v) {
                                dimacs_xor_equation[dimacs_nb_of_xor_equations][dimacs_size_of_xor_equation[dimacs_nb_of_xor_equations]++] = v;
                                ++dimacs_nb_of_atoms_xe;
                                assert(dimacs_nb_of_atoms_xe < __MAX_BUFFER_SIZE__);
                                assert(dimacs_size_of_xor_equation[dimacs_nb_of_xor_equations] < __MAX_EQ_SIZE__);
                            }
                            if(dimacs_size_of_xor_equation[dimacs_nb_of_xor_equations] > dimacs_size_max_among_xor_equations) dimacs_size_max_among_xor_equations = dimacs_size_of_xor_equation[dimacs_nb_of_xor_equations];
                            if((!dimacs_size_min_among_xor_equations) || (dimacs_size_of_xor_equation[dimacs_nb_of_xor_equations] < dimacs_size_min_among_xor_equations)) dimacs_size_min_among_xor_equations = dimacs_size_of_xor_equation[dimacs_nb_of_xor_equations];
                            ++dimacs_nb_of_xor_equations;
                            assert(dimacs_nb_of_xor_equations < __MAX_EQ__);
                        }
                        _tmp_str = 0;
                        xor_mode = __OFF__;
                        break;
                    default:
                        return;
                }
                break;
            case SAT_IN_EQ :
                switch(c) {
                    case '\t' :
                    case ' '  :
                        break;
                    case '-' :
                    case '0' :
                    case '1' :
                    case '2' :
                    case '3' :
                    case '4' :
                    case '5' :
                    case '6' :
                    case '7' :
                    case '8' :
                    case '9' :
                        status = SAT_IN_ATOM;
                        tmp_str[_tmp_str++] = c;
                        break;
                    case '\n':
                        status = SAT_START;
                        tmp_str[_tmp_str] = 0;
                        v = atoll(tmp_str);
                        if(!xor_mode) {
                            if(v) {
                                dimacs_boolean_equation[dimacs_nb_of_boolean_equations][dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations]++] = v;
                                ++dimacs_nb_of_atoms_be;
                                assert(dimacs_nb_of_atoms_be < __MAX_BUFFER_SIZE__);
                                assert(dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations] < __MAX_EQ_SIZE__);
                            }
                            if(dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations] > dimacs_size_max_among_boolean_equations) dimacs_size_max_among_boolean_equations = dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations];
                            if((!dimacs_size_min_among_boolean_equations) || (dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations] < dimacs_size_min_among_boolean_equations)) dimacs_size_min_among_boolean_equations = dimacs_size_of_boolean_equation[dimacs_nb_of_boolean_equations];
                            ++dimacs_nb_of_boolean_equations;
                        } else {
                            if(v) {
                                dimacs_xor_equation[dimacs_nb_of_xor_equations][dimacs_size_of_xor_equation[dimacs_nb_of_xor_equations]++] = v;
                                ++dimacs_nb_of_atoms_xe;
                                assert(dimacs_nb_of_atoms_xe < __MAX_BUFFER_SIZE__);
                                assert(dimacs_size_of_xor_equation[dimacs_nb_of_xor_equations] < __MAX_EQ_SIZE__);
                            }
                            if(dimacs_size_of_xor_equation[dimacs_nb_of_xor_equations] > dimacs_size_max_among_xor_equations) dimacs_size_max_among_xor_equations = dimacs_size_of_xor_equation[dimacs_nb_of_xor_equations];
                            if((!dimacs_size_min_among_xor_equations) || (dimacs_size_of_xor_equation[dimacs_nb_of_xor_equations] < dimacs_size_min_among_xor_equations)) dimacs_size_min_among_xor_equations = dimacs_size_of_xor_equation[dimacs_nb_of_xor_equations];
                            ++dimacs_nb_of_xor_equations;
                        }
                        _tmp_str = 0;
                        xor_mode = __OFF__;
                        break;
                    default:
                        return;
                }
                break;
            default:
                return;
        }
    }
	//printf("atoms: %d, %d\n", dimacs_nb_of_atoms_be, dimacs_nb_of_atoms_xe);
    _dimacs_read = true;
	dimacs_nb_of_unary_vars = dimacs_nb_of_vars;
}

#endif

void dimacs_print_header() {
	int_t i;
	int_t n_vsz = fast_int_log10(dimacs_nb_of_vars);
	_cid_cout("#atoms : %d\n", dimacs_nb_of_vars);
	_cid_cout("#rounds: %d\n", dimacs_nb_of_rounds);
	_cid_cout("#models: %d\n", dimacs_nb_of_models);
	_cid_cout("#IDs:%s\n", "");
	for(i = 1LL; i <= dimacs_nb_of_vars; ++i) {
		if(dimacs_meaning[i] == NULL) continue;
		_cid_cout("   %0*d [%s]: %s\n", n_vsz, i, dimacs_meaning[i], (dimacs_truth_value[i] == __UNDEF__) ? "undef" :  (dimacs_truth_value[i] == __ON__) ? "true" : (dimacs_truth_value[i] == __OFF__) ? "false" : "n/a");
	}
}

void dimacs_print_formula() {
	int_t i, j;
	int_t n_besz = fast_int_log10(dimacs_nb_of_boolean_equations);
	int_t n_xesz = fast_int_log10(dimacs_nb_of_xor_equations);
	int_t mx_xesz = fast_int_log10(dimacs_size_max_among_xor_equations);
	int_t mx_besz = fast_int_log10(dimacs_size_max_among_boolean_equations);
	_cid_cout("#atoms         : %d\n", dimacs_nb_of_vars);
	_cid_cout("#OR equations  : %d\n", dimacs_nb_of_boolean_equations);
	_cid_cout("#XOR equations : %d\n", dimacs_nb_of_xor_equations);
	_cid_cout("#EQ: [%llu]\n", dimacs_nb_of_boolean_equations);
	for(i = 0LL; i < dimacs_nb_of_boolean_equations; ++i) {
		_cid_cout("   [%0*lld][%0*lld]:", n_besz, i, mx_besz, dimacs_size_of_boolean_equation[i]);
		for(j = 0LL; j < dimacs_size_of_boolean_equation[i]; ++j) printf(" %lld", dimacs_boolean_equation[i][j]);
		printf("\n");
	}
	_cid_cout("#XEQ: [%llu]\n", dimacs_nb_of_xor_equations);
	for(i = 0LL; i < dimacs_nb_of_xor_equations; ++i) {
		_cid_cout("   [%0*lld][%0*lld]:", n_xesz, i, mx_xesz, dimacs_size_of_xor_equation[i]);
		for(j = 0LL; j < dimacs_size_of_xor_equation[i]; ++j) printf(" %lld", dimacs_xor_equation[i][j]);
		printf("\n");
	}
}
