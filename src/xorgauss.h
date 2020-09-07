//
//  xorgauss.h
//  WDSat
//
//  Created by Gilles Dequen and Monika Trimoska on 15/12/2018.
//  Copyright © 2018 Gilles Dequen. All rights reserved.
//

#ifndef xorgauss_h
#define xorgauss_h

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "wdsat_utils.h"


#define __SZ_ATOM__ ((uint_t) sizeof(uint_t) << 3ULL)
#define __SZ_GAUSS__ ((__ID_SIZE__ / __SZ_ATOM__) + 1ULL)
#define __XOR_CONSTANT_MASK__ (1ULL << (__SZ_ATOM__ - 1ULL))

extern uint_t xorgauss_equivalency[__ID_SIZE__][__SZ_GAUSS__];
extern bool xorgauss_equivalent[__ID_SIZE__];
extern boolean_t xorgauss_assignment_buffer[__SIGNED_ID_SIZE__];
extern boolean_t *xorgauss_assignment;

// undo structures
extern int_t xorgauss_history[__ID_SIZE__];
extern int_t xorgauss_history_top;
extern int_t xorgauss_step[__ID_SIZE__];
extern int_t xorgauss_step_top;
#ifdef __XG_ENHANCED__
extern uint_t xorgauss_equivalency_history[__MAX_ANF_ID__][__ID_SIZE__][__SZ_GAUSS__];
extern bool xorgauss_equivalent_history[__MAX_ANF_ID__][__ID_SIZE__];
extern boolean_t xorgauss_assignment_buffer_history[__MAX_ANF_ID__][__SIGNED_ID_SIZE__];
extern boolean_t xorgauss_current_degree[__ID_SIZE__];
extern boolean_t xorgauss_current_degree_history[__MAX_ANF_ID__][__ID_SIZE__];
#else
extern uint_t xorgauss_mask[__ID_SIZE__][__SZ_GAUSS__];
extern int_t xorgauss_mask_top;
extern int_t xorgauss_step_mask[__ID_SIZE__];
extern int_t xorgauss_step_mask_top;
extern int_t xorgauss_mask_list[__ID_SIZE__][__ID_SIZE__ + 1];
extern int_t xorgauss_mask_list_top;
extern int_t xorgauss_reset[__ID_SIZE__];
extern int_t xorgauss_reset_top;
#endif

bool xorgauss_from_dimacs(void);
bool xorgauss_initiate_from_dimacs(void);
void xorgauss_reset_boolean_vector(uint_t *);
uint_t xorgauss_get_first_id_from_boolean_vector(uint_t *);
inline uint_t xorgauss_get_last_id_from_boolean_vector(uint_t *v);
uint_t xorgauss_get_size_of_boolean_vector(uint_t *, uint_t *);
bool xorgauss_is_constant(uint_t *);
void xorgauss_xor_it(uint_t *, uint_t *);
bool xorgauss_xor_it_and_check(uint_t *, uint_t *);
void xorgauss_fprint(void);
bool xorgauss_set_true(const int_t);
void aff_bin(uint_t v);
void xorgauss_undo(void);
void set_from_outside(void);
bool xorgauss_infer(int_t v);
void xorgauss_fprint_system(void);
void xorgauss_fprint_for_xorset(void);

#ifdef __XG_ENHANCED__ //if XG-enhanced

#define _xorgauss_breakpoint \
{ \
memcpy(xorgauss_equivalency_history[xorgauss_step_top], xorgauss_equivalency, sizeof(int_t)*__ID_SIZE__*__SZ_GAUSS__); \
memcpy(xorgauss_equivalent_history[xorgauss_step_top], xorgauss_equivalent, sizeof(bool)*__ID_SIZE__); \
memcpy(xorgauss_assignment_buffer_history[xorgauss_step_top], xorgauss_assignment_buffer, sizeof(boolean_t)*__SIGNED_ID_SIZE__); \
memcpy(xorgauss_current_degree_history[xorgauss_step_top], xorgauss_current_degree, sizeof(boolean_t)*__ID_SIZE__); \
xorgauss_step[xorgauss_step_top++] = xorgauss_history_top; \
}

#define _xorgauss_mergepoint \
{ \
xorgauss_step_top && --xorgauss_step_top; \
}

#else //else

#define _xorgauss_breakpoint \
{ \
xorgauss_step[xorgauss_step_top++] = xorgauss_history_top; \
xorgauss_step_mask[xorgauss_step_mask_top++] = xorgauss_mask_top; \
}

#define _xorgauss_mergepoint \
{ \
xorgauss_step_top && --xorgauss_step_top; \
xorgauss_step_mask_top && --xorgauss_step_mask_top; \
}

#endif //endif

#define _xorgauss_unset(_v) xorgauss_assignment[_v] = xorgauss_assignment[-_v] = __UNDEF__

#define _xorgauss_set(_v, _tv) \
{ \
/*printf("--------Setting %d to %d\n", _v, _tv);*/ \
xorgauss_assignment[_v] = (boolean_t) _tv; \
xorgauss_assignment[-_v] = (boolean_t) _tv ^ (boolean_t) __TRUE__; \
}

#define _xorgauss_is_true(_v) (xorgauss_assignment[_v] & (boolean_t) 1)
#define _xorgauss_is_false(_v) (!xorgauss_assignment[_v])
#define _xorgauss_is_undef(_v) (xorgauss_assignment[_v] & 2)

#define _boolean_vector_get(_vect, _id) \
(_vect[(_id / __SZ_ATOM__)] & (1ULL << ((__SZ_ATOM__ - 1ULL) - (_id % __SZ_ATOM__))))

#define _boolean_vector_set(_vect, _id) \
_vect[(_id / __SZ_ATOM__)] |= (1ULL << ((__SZ_ATOM__ - 1ULL) - (_id % __SZ_ATOM__)))

#define _boolean_vector_reset(_vect, _id) \
_vect[(_id / __SZ_ATOM__)] &= ((~0ULL) ^ (1ULL << ((__SZ_ATOM__ - 1ULL) - (_id % __SZ_ATOM__))))

#define _boolean_vector_flip_constant(_vect) \
_vect[0] ^= __XOR_CONSTANT_MASK__

#define _boolean_vector_get_constant(_vect) \
_vect[0] & __XOR_CONSTANT_MASK__

#define _boolean_vector_fprint_bin(_vect, _sz) \
{ \
for(uint_t _bv_i = 0ULL; _bv_i <= sz; ++_bv_i) \
print_bin(_vect[_bv_i]); \
}

#define _boolean_vector_fprint(_vect, _sz) \
{ \
for(uint_t _boolean_vector_fprint_i = 0ULL; _boolean_vector_fprint_i <= _sz; ++_boolean_vector_fprint_i) { \
if(_boolean_vector_get(_vect, _boolean_vector_fprint_i)) { \
if(!_boolean_vector_fprint_i) fprintf(stdout, " T"); \
else fprintf(stdout, " %llu", _boolean_vector_fprint_i); \
} else { \
if(!_boolean_vector_fprint_i) fprintf(stdout, " F"); \
} \
} \
}
#endif /* xorgauss_h */

