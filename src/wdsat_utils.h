//
//  wdsat_utils.h
//  WDSat
//
//  Created by Gilles Dequen on 15/12/2018.
//  Copyright © 2018 Gilles Dequen. All rights reserved.
//

#ifndef wdsat_utils_h
#define wdsat_utils_h

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#define __XG_ENHANCED__
//#define __DEBUG__

#define __INT_SIZE__ 64
#define __MAX_INT_SIZE__ ~0ULL
#define __END_OF_CLAUSE__ 0
#define __CID_VERSION__ "1.0"
#define __CID_NAME__ "CID"
#define __OFFSET_VECTOR__ 15
#define __ON__ 1
#define __OFF__ 0
#define __TRUE__ 1
#define __FALSE__ 0
#define __UNDEF__ 2
#define __STATIC_STRING_SIZE__ 250
#define __LONG_STATIC_STRING_SIZE__ 1000
#define __STATIC_CLAUSE_STRING_SIZE__ 30000
#define __EXIT_SAT__ 10
#define __EXIT_UNSAT__ 20
#define __EXIT_UNKNOWN__ 0

/// ------------------------------------------
/// this should be modified in order to manage
/// more models

/** Trivium - min*/
/*#ifdef __XG_ENHANCED__
#define __MAX_ANF_ID__ 289
#define __MAX_DEGREE__ 3 // make it +1
#endif
#define __MAX_ID__ 531
#define __MAX_BUFFER_SIZE__ 30000
#define __MAX_EQ__ 6000
#define __MAX_EQ_SIZE__ 5
#define __MAX_XEQ__ 200
#define __MAX_XEQ_SIZE__ 500*/

/** n23l11*/
/*#ifdef __XG_ENHANCED__
#define __MAX_ANF_ID__ 23
#define __MAX_DEGREE__ 3 // make it +1
#endif
#define __MAX_ID__ 143
#define __MAX_BUFFER_SIZE__ 2500
#define __MAX_EQ__ 365
#define __MAX_EQ_SIZE__ 4
#define __MAX_XEQ__ 23
#define __MAX_XEQ_SIZE__ 200*/
 

// n37-l18
/**#ifdef __XG_ENHANCED__
#define __MAX_ANF_ID__ 37
#define __MAX_DEGREE__ 3 // make it +1
#endif
#define __MAX_ID__ 360
#define __MAX_BUFFER_SIZE__ 7000
#define __MAX_EQ__ 980
#define __MAX_EQ_SIZE__ 4
#define __MAX_XEQ__ 38
#define __MAX_XEQ_SIZE__ 350*/

// n41-l20
/*#ifdef __XG_ENHANCED__
#define __MAX_ANF_ID__ 41
#define __MAX_DEGREE__ 3 // make it +1
#endif
#define __MAX_ID__ 440
#define __MAX_BUFFER_SIZE__ 11000
#define __MAX_EQ__ 1300
#define __MAX_EQ_SIZE__ 4
#define __MAX_XEQ__ 42
#define __MAX_XEQ_SIZE__ 450*/

// n43-l21
/**#ifdef __XG_ENHANCED__
#define __MAX_ANF_ID__ 43
#define __MAX_DEGREE__ 3 // make it +1
#endif
#define __MAX_ID__ 483
#define __MAX_BUFFER_SIZE__ 14000
#define __MAX_EQ__ 1330
#define __MAX_EQ_SIZE__ 4
#define __MAX_XEQ__ 44
#define __MAX_XEQ_SIZE__ 650*/

/** S4: l=6 **/
/*#ifdef __XG_ENHANCED__
#define __MAX_ANF_ID__ 52
#define __MAX_DEGREE__ 4 // make it +1
#endif
#define __MAX_ID__ 767
#define __MAX_BUFFER_SIZE__ 60000
#define __MAX_EQ__ 3000
#define __MAX_EQ_SIZE__ 5 //make it +1
#define __MAX_XEQ__ 52
#define __MAX_XEQ_SIZE__ 800*/

/** MQ : n=20 m=40 **/
#ifdef __XG_ENHANCED__
 #define __MAX_ANF_ID__ 21 // make it +1
 #define __MAX_DEGREE__ 3 // make it +1
 #endif
 #define __MAX_ID__ 210
 #define __MAX_BUFFER_SIZE__ 5000
 #define __MAX_EQ__ 600
 #define __MAX_EQ_SIZE__ 4 //make it +1
 #define __MAX_XEQ__ 40
 #define __MAX_XEQ_SIZE__ 200

/** MQ : n=25 m=50 **/
/*#ifdef __XG_ENHANCED__
#define __MAX_ANF_ID__ 26 // make it +1
#define __MAX_DEGREE__ 3 // make it +1
#endif
#define __MAX_ID__ 325
#define __MAX_BUFFER_SIZE__ 10000
#define __MAX_EQ__ 950
#define __MAX_EQ_SIZE__ 4 //make it +1
#define __MAX_XEQ__ 50
#define __MAX_XEQ_SIZE__ 900*/

/** MQ : n=55 m=110 **/
/*#ifdef __XG_ENHANCED__
#define __MAX_ANF_ID__ 56 // make it +1
#define __MAX_DEGREE__ 3 // make it +1
#endif
#define __MAX_ID__ 1541
#define __MAX_BUFFER_SIZE__ 100000
#define __MAX_EQ__ 4456
#define __MAX_EQ_SIZE__ 4 //make it +1
#define __MAX_XEQ__ 110
#define __MAX_XEQ_SIZE__ 1500*/

/// ------------------------------------------

#define __ID_SIZE__ (__MAX_ID__ + 1) //CHECK** paranthèses
#define __SIGNED_ID_SIZE__ ((__MAX_ID__ + 1) << 1)

#ifndef _OPENMP

#define omp_get_thread_num() 0
#define omp_get_max_threads() 1
#define omp_set_num_threads() 1

#else

#include<omp.h>

#endif

typedef int_fast64_t  int_t;
typedef int_fast8_t   byte_t;
typedef int_fast8_t   boolean_t;
typedef uint_fast64_t  uint_t;
typedef uint_fast8_t   ubyte_t;

/* ---------------------------------------------------------------- */
// *** General Memory Management

/**
 * @def _free_mem(_p)
 * @author Gilles Dequen
 * @brief Free the memory allocated with _get_mem macro
 * @param _p is a pointer
 * @warning _free_mem should not free memory got from 'malloc' class functions
 */
#define _free_mem(_p) \
{ \
 if(_p != NULL) { \
  free(_p); \
  _p = NULL; \
 } \
}

/**
 * @def _get_stack_mem(_type, _p, _elt)
 * @author Gilles Dequen
 * @brief Allocates the memory needed to store _elt of type _type. Contrary to _get_mem thet gets memory from heap, this macro gets memory from stack. Consequently, it should not be freed and is only available within a local context.
 * @param _type is needed to set the offset adressing value
 * @param _p is a pointer
 * @param _elt is the number of values of size '_type'
 * @warning _p must have a value (NULL or a already allocated)
 * @warning This function cannot resize memory
 */
#define _get_stack_mem(_type, _p, _elt) \
{ \
 if(_p == NULL) { \
  _p = (_type*) alloca(_elt * sizeof(_type)); \
 } \
}

/**
 * @def _get_mem(_type, _p, _elt)
 * @author Gilles Dequen
 * @brief Allocates or resizes the memory needed to store _elt of type _type.
 * @param _type is needed to set the offset adressing value
 * @param _p is a pointer
 * @param _elt is the number of values of size '_type'
 * @warning _p must have a value (NULL or a already allocated)
 */
#define _get_mem(_type, _p, _elt) \
{ \
 if(_p == NULL) { \
  _p = (_type*) malloc((_elt) * sizeof(_type)); \
 } else { \
  if(!(_elt)) { \
    _p = NULL; \
  } else { \
    _p = (_type*) realloc(_p, ((_elt) * sizeof(_type))); \
  } \
 } \
}

/* ---------------------------------------------------------------- */
// *** One-Block int Vector Macros

#define _int_vector_id(_v) int_t *_v = NULL;

#define _reset_int_vector(_v) \
{ \
    _free_mem(_v); \
    _get_mem(int_t, _v, (__OFFSET_VECTOR__ + 2)); \
    _v += 2; \
    _v[-1] = 0; \
    _v[-2] = __OFFSET_VECTOR__; \
}

#define _free_int_vector(_v) \
{ \
    _v -= 2; \
    _free_mem(_v); \
}

#define _int_vector_append(_v, _i) \
{ \
    if(_v[-1] >= _v[-2]) { \
        _v -= 2; \
        _v[0] <<= 1; \
        _v = (int_t *) realloc(_v, ((2 + _v[0]) * sizeof(int_t))); \
        _v += 2; \
    } \
   _v[_v[-1]++] = _i; \
}

#define _int_vector_size(_v) _v[-1]

#define _cout_int_vector(_v) \
{ \
    int_t __idx; \
    printf(" [#%p][%lld/%lld]:", _v, _v[-1], _v[-2]); \
    for(__idx = 0; __idx < _v[-1]; ++__idx) { \
        printf(" %lld", _v[__idx]); \
    } \
}

/* ---------------------------------------------------------------- */
// *** Vector Management Macros

#define _vector(_v) _v

/**
 * @def _vector_id(_type, _v)
 * @author Gilles Dequen
 * @brief Declaration of an object Vector.
 * @param _type characterize each element of _vector
 * @param _v is the keyword of the object
 */
#define _vector_id(_type, _v) \
_type* _v = NULL; \
int _ ## _v = 0, _ ## _v ## _sz = 0

/**
 * @def _extern_vector_id(_type, _vector)
 * @author Gilles Dequen
 * @brief External declaration of a vector
 * @param _type characterize each element of _vector
 * @param _vector is the keyword of the object
 */
#define _extern_vector_id(_type, _v) \
extern _type* _v; \
extern int _ ## _v, _ ## _v ## _sz

/**
 * @def _no_init_vector_id(_type, _vector)
 * @author Gilles Dequen
 * @brief Declaration of a vector without any initialization
 * @param _type characterize each element of _vector
 * @param _vector is the keyword of the object
 */
#define _no_init_vector_id(_type, _v) \
_type* _v; \
int _ ## _v, _ ## _vector ## _sz

/**
 * @def _init_vector(_type, _vector)
 * @author Gilles Dequen
 * @brief Assign vector before allocation. Needed if declared with _NO_INIT_VectorID
 * @param _type characterize each element of _vector
 * @param _vector is the keyword of the object
 */
#define _init_vector(_type, _v) _v = NULL; _ ## _v = _ ## _v ## _sz = 0

/**
 * @def _vector_reset(_vector)
 * @author Gilles Dequen
 * @brief Resetting a generic vector. Not free process is done.
 * @param _vector is the keyword of the object
 */
#define _vector_reset(_v) _ ## _v = 0;


#define _free_vector(_vector) \
{ \
  _ ## _vector = _ ## _vector ## _sz = 0; \
  _free_mem(_vector); \
}

#define _vector_size(_vector) _ ## _vector

/**
 * @def _vector_append(_type, _vector, _value)
 * @author Gilles Dequen
 * @brief Add a value in a generic vector
 * @param _type characterize each element of _vector
 * @param _vector is the keyword of the object
 * @param _value is the value to add
 */
#define _vector_append(_type, _vector, _value) \
{ \
  if(_ ## _vector >= _ ## _vector ## _sz) { \
    _ ## _vector ## _sz = ((_ ## _vector ## _sz << 1) + __OFFSET_VECTOR__); \
    _get_mem(_type, _vector, _ ## _vector ## _sz); \
  } \
  _vector[_ ## _vector++] = _value; \
}

/**
 * \def _vector_copy(_type, _vDst, _vSrc)
 * \author Gilles Dequen
 * \brief copy _vSrc vector to _vDst vector. _vDst should be an empty vector.
 * \param _type characterize each element of _vDst and _vSrc
 */
#define _vector_copy(_type, _vDst, _vSrc) \
{ \
  _ ## _vDst = _ ## _Src; \
  _ ## _vDst ## _sz = _ ## _Src ## _sz; \
  _get_mem(_type, _vDst, _ ## _vDst ## _sz); \
  memcpy(_vDst, _vSrc, (_ ## _vSrc * sizeof(_type))); \
}

#define _cid_cout(f, ...) \
{ \
  char tmpprefix[__STATIC_STRING_SIZE__]; \
  char tmpstr[__STATIC_STRING_SIZE__]; \
  sprintf(tmpprefix, "c [%s-%s][#%0*d]", __CID_NAME__, __CID_VERSION__, (int) thread_digits, (int) omp_get_thread_num()); \
  sprintf(tmpstr, "%-15s %s", tmpprefix, f); \
  fprintf(stdout, tmpstr, __VA_ARGS__); \
} 

int_t fast_int_log2(int_t v);
int_t fast_int_log10(int_t v);
void print_bin(const uint_t v);


#endif 
