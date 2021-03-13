
//enable the XG-ext module (must use anf as input)
#define __XG_ENHANCED__


//find all solutions instead of only one
//#define __FIND_ALL_SOLUTIONS__


//outputs some intermediate results for fixing errors
//#define __DEBUG__


/*** static allocation ***/
/// these params should be modified in order to manage
/// more models.
/// several config example are given here
/// for different parameter sizes for Trivium, MQ, Index calculus S3 and S'4

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

/** IC-S3: n23l11*/
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
 

/** IC-S3: n37-l18 **/
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

/** IC-S3: n41-l20 **/
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

/** IC-S3: n43-l21 **/
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

/** IC-S4: l=6 **/
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
/*#ifdef __XG_ENHANCED__
 #define __MAX_ANF_ID__ 21 // make it +1
 #define __MAX_DEGREE__ 3 // make it +1
 #endif
 #define __MAX_ID__ 210
 #define __MAX_BUFFER_SIZE__ 5000
 #define __MAX_EQ__ 600
 #define __MAX_EQ_SIZE__ 4 //make it +1
 #define __MAX_XEQ__ 40
 #define __MAX_XEQ_SIZE__ 200*/

/** MQ : n=25 m=50 **/
#ifdef __XG_ENHANCED__
#define __MAX_ANF_ID__ 26 // make it +1
#define __MAX_DEGREE__ 3 // make it +1
#endif
#define __MAX_ID__ 325
#define __MAX_BUFFER_SIZE__ 10000
#define __MAX_EQ__ 950
#define __MAX_EQ_SIZE__ 4 //make it +1
#define __MAX_XEQ__ 50
#define __MAX_XEQ_SIZE__ 900

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

/*** END static allocation ***/
