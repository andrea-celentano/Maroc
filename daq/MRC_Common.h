#ifndef __MRC_COMMON__H
#define __MRC_COMMON__H

// Define common macros and objects of general use

#define FE_REV1	0
#define FE_REV2	2
#define FE_REV3	3

#define MRC_VERBOSE 1 // 0= information, 1= +error, 2= +warning, 3= +message, 4= +debug message, 5= +dump 


#define MRC_STR "%s[%d] "
#define MRC_ARG __FUNCTION__, __LINE__
#define MRC_PRINT(...) fprintf(stderr, ##__VA_ARGS__)


#if (MRC_VERBOSE>4)
#define MRC_DUM(_fmt, ...) MRC_PRINT("DUM:" MRC_STR _fmt, MRC_ARG, ##__VA_ARGS__)
#else
#define MRC_DUM(...)
#endif

#if (MRC_VERBOSE>3)
#define MRC_DBG(_fmt, ...) MRC_PRINT("DBG:" MRC_STR _fmt, MRC_ARG, ##__VA_ARGS__)
#else
#define MRC_DBG(...)
#endif

#if (MRC_VERBOSE>2)
#define MRC_MSG(_fmt, ...) MRC_PRINT("MSG:" MRC_STR _fmt, MRC_ARG, ##__VA_ARGS__)
#else
#define MRC_MSG(...)
#endif

#if (MRC_VERBOSE>1)
#define MRC_WRN(_fmt, ...) MRC_PRINT("WRN:" MRC_STR _fmt, MRC_ARG, ##__VA_ARGS__)
#else
#define MRC_WRN(...)
#endif

#if (MRC_VERBOSE>0)
#define MRC_ERR(_fmt, ...) MRC_PRINT("ERR:" MRC_STR _fmt, MRC_ARG, ##__VA_ARGS__)
#else
#define MRC_ERR(...)
#endif

#define MRC_INF(_fmt, ...) MRC_PRINT("INF:" MRC_STR _fmt, MRC_ARG, ##__VA_ARGS__)

#endif
