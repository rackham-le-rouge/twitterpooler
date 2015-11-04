
/*******************************************************

  Program configuration
  Set parameters here and
  recompile in order to apply
  Need a 'make clean' !!!

 *******************************************************/



#ifndef CONFIG_H
#define CONFIG_H

/* Some includes */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include <sys/ioctl.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <openssl/md5.h>
#include <fcntl.h>
#include <ctype.h>

/* Some constants */
#define DEBUG_LEVEL	                    5
#define USE_COLORED_LOG                 1
#define URL_LENGTH                      9 /* At least add 1 to the real URL lenght for the \0 */
#define URL_PREFIX                      "https://twitter.com/"
#define TOKEN_DELIMITER_FOR_DATA_START  "js-tweet-text tweet-text\""
#define TOKEN_DELIMITER_FOR_DATA_END    "</p>"
#define CHECKSUM_DIRECTORY              "checksum"
#define CONFIGURATION_FILE              "pooler.cfg"
#define WAIT_TIME_MAX                   15
#define WAIT_TIME_MIN                   5
#define WAIT_TIME_URL_MAX               2
#define WAIT_TIME_URL_MIN               1
#define MAX_URL_BEFORE_SAVING_INIT      9
#define MAX_URL_NUMBER_OF_TURN_TO_FIX   10
#define MAX_URL_MAXIMUM_PER_TURN        50
#define PROGRESS_BAR_BODY_CHARACTER     '-'
#define PROGRESS_BAR_HEAD_CHARACTER     '>' 
#define SCREEN_SIZE_COLUMN_MIN          20
#define PIPE_NAME                       "extractedTwitter"
#define WORD_CORRELATION                0.80        /* Means that we can have 20% of difference between the read word and the keyword */


#ifdef USE_COLORED_LOG
    #define ANSI_COLOR_RED     "\x1b[31m"
    #define ANSI_COLOR_GREEN   "\x1b[32m"
    #define ANSI_COLOR_YELLOW  "\x1b[33m"
    #define ANSI_COLOR_BLUE    "\x1b[34m"
    #define ANSI_COLOR_MAGENTA "\x1b[35m"
    #define ANSI_COLOR_CYAN    "\x1b[36m"
    #define ANSI_COLOR_RESET   "\x1b[0m"
#else
    #define ANSI_COLOR_RED     ""
    #define ANSI_COLOR_GREEN   ""
    #define ANSI_COLOR_YELLOW  ""
    #define ANSI_COLOR_BLUE    ""
    #define ANSI_COLOR_MAGENTA ""
    #define ANSI_COLOR_CYAN    ""
    #define ANSI_COLOR_RESET   ""
#endif

/* Logic stuff */
enum boolean
{
    TRUE,
    FALSE,
    DONT_KNOW
};


/* Structures */
struct MemoryStruct {
    char *memory;
    size_t size;
};

/*** Padding is for avoid multi-thread concurent access to the same memory sector - speed improvement */
typedef struct ThreadStateInfo_
{
    int iQuoteTreated;
    char bHaveToDie;
    char padding[64];
}threadStateInfo;

/*** Padding is for avoid multi-thread concurent access to the same memory sector - speed improvement */
struct PagePoolingInitData {
    char* sName;
    char* sKeyWords;
    threadStateInfo* structThreadStateInfo;
    char padding[64];
};

typedef struct PagePoolingInitData structPagePoolingInitData;

enum checksumFileAction
{
    INIT,
    UPDATE,
    CHECK_EXIST,
    CLOSE
};

typedef struct linkedListKeywords_
{
    struct linkedListKeywords_* structNext;
    char* sKeyword;
    char cSpecialWordLevel;
    int iWordLevel;
}linkedListKeywords;



/* Debug part */
#define UNUSED(x) (void)(x)

#if(DEBUG_LEVEL > 0)
#define LOG_ERROR(p_sStr, ...)      fprintf(stderr, ANSI_COLOR_MAGENTA "%s " ANSI_COLOR_RED "ERR " ANSI_COLOR_BLUE "[%d]:" ANSI_COLOR_RESET p_sStr "\n", __FILE__, __LINE__, __VA_ARGS__);
#else
#define LOG_ERROR(p_sStr, ...)
#endif

#if(DEBUG_LEVEL > 1)
#define LOG_WARNING(p_sStr, ...)      fprintf(stderr, ANSI_COLOR_MAGENTA "%s " ANSI_COLOR_YELLOW "WARN " ANSI_COLOR_BLUE "[%d]:" ANSI_COLOR_RESET p_sStr "\n", __FILE__, __LINE__, __VA_ARGS__);
#else
#define LOG_WARNING(p_sStr, ...)
#endif

#if(DEBUG_LEVEL > 2)
#define LOG_INFO(p_sStr, ...)      fprintf(stderr, ANSI_COLOR_MAGENTA "%s " ANSI_COLOR_GREEN "INFO " ANSI_COLOR_BLUE "[%d]:" ANSI_COLOR_RESET p_sStr "\n", __FILE__, __LINE__, __VA_ARGS__);
#else
#define LOG_INFO(p_sStr, ...)
#endif


#if(DEBUG_LEVEL > 3)
#define LOG_MSG(p_sStr)      fprintf(stderr, "%s INFO [%d]:" p_sStr "\n", __FILE__, __LINE__);
#else
#define LOG_MSG(p_sStr)
#endif

#define LOG_PRINT(p_sStr, ...)      fprintf(stdout,  p_sStr "\n", __VA_ARGS__);


#include "parser.h"
#include "network.h"
#include "io.h"
#include "main.h"



#endif
