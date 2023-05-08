/**
*******************************************************************************
* @file           : log.h
* @brief          : Description of header file
* @author         : Gonzalo Rivera
* @date           : 04/05/2023
*******************************************************************************
* @attention
*
* Copyright (c) <date> Gonzalo Rivera. All rights reserved.
*
*/
#ifndef __LOG_H__
#define __LOG_H__
/******************************************************************************
        Includes
 ******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
/******************************************************************************
        Constants
 ******************************************************************************/

/******************************************************************************
        Data types
 ******************************************************************************/
/**
 * @brief Log level
 *
 */
typedef enum {
    LOG_NONE,       /*!< No log output */
    LOG_ERROR,      /*!< Critical errors, software module can not recover on its own */
    LOG_WARN,       /*!< Error conditions from which recovery measures have been taken */
    LOG_INFO,       /*!< Information messages which describe normal flow of events */
    LOG_DEBUG,      /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    LOG_VERBOSE,     /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
	LOG_MAX
} log_level_t;

#define LOG_NO_TIMESTAMP 0
#define LOG_TIMESTAMP 1

typedef int (*vprintf_like_t)(const char *, va_list);
typedef uint32_t (*get_timestamp_t)(void);
typedef int (*log_out_t)(uint8_t *data, uint16_t size);
/******************************************************************************
        Public function prototypes
 ******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set function used to output log entries
 *
 * By default, log output goes to UART0. This function can be used to redirect log
 * output to some other destination, such as file or network. Returns the original
 * log handler, which may be necessary to return output to the previous destination.
 *
 * @note Please note that function callback here must be re-entrant as it can be
 * invoked in parallel from multiple thread context.
 *
 * @param func new Function used for output. Must have same signature as vprintf.
 *
 * @return func old Function used for output.
 */
vprintf_like_t log_set_vprintf(vprintf_like_t func);

/*
 * @brief Set function used to set get timestamp function
 *
 * @param get_timestamp func: new function to get ticks/timestamp
 *
 * @reiturn none
 *
 * */
void log_set_timestamp(get_timestamp_t func);

/**
 * @brief Function which returns timestamp to be used in log output
 *
 * This function is used in expansion of ESP_LOGx macros.
 * In the 2nd stage bootloader, and at early application startup stage
 * this function uses CPU cycle counter as time source. Later when
 * FreeRTOS scheduler start running, it switches to FreeRTOS tick count.
 *
 * For now, we ignore millisecond counter overflow.
 *
 * @return timestamp, in milliseconds
 */
uint32_t log_timestamp(void);

/*
 * @brief Function to set uart output for logging
 *
 * @return none
 *
 * */
void log_set_output(log_out_t func);

/**
 * @brief Write message into the log
 *
 * This function is not intended to be used directly. Instead, use one of
 * ESP_LOGE, ESP_LOGW, ESP_LOGI, ESP_LOGD, ESP_LOGV macros.
 *
 * This function or these macros should not be used from an interrupt.
 */
void log_write(log_level_t level, const char* tag, const char* format, ...) __attribute__ ((format (printf, 3, 4)));

/**
 * @brief Write message into the log, va_list variant
 * @see esp_log_write()
 *
 * This function is provided to ease integration toward other logging framework,
 * so that esp_log can be used as a log sink.
 */
void log_writev(log_level_t level, const char* tag, const char* format, va_list args);

#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL  LOG_MAX
#endif

//CONFIG_LOG_COLORS
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define LOG_COLOR_E ANSI_COLOR_RED
#define LOG_COLOR_W ANSI_COLOR_YELLOW
#define LOG_COLOR_I ANSI_COLOR_GREEN
#define LOG_COLOR_D ANSI_COLOR_MAGENTA
#define LOG_COLOR_V ANSI_COLOR_CYAN
#define LOG_RESET_COLOR ANSI_COLOR_RESET

#if LOG_CONFIG_TIMESTAMP
#define LOG_FORMAT(letter, format)  LOG_COLOR_ ## letter #letter " (%" PRIu32 ") %s: " format LOG_RESET_COLOR "\r\n"
#define LOG_SYSTEM_TIME_FORMAT(letter, format)  LOG_COLOR_ ## letter #letter " (%s) %s: " format LOG_RESET_COLOR "\r\n"
#else
#define LOG_FORMAT(letter, format)  LOG_COLOR_ ## letter #letter " %s: " format LOG_RESET_COLOR "\r\n"
#define LOG_SYSTEM_TIME_FORMAT(letter, format)  LOG_COLOR_ ## letter #letter " %s: " format LOG_RESET_COLOR "\r\n"
#endif

#define LOGE( tag, format, ... ) LOG_LEVEL_LOCAL(LOG_ERROR,   tag, format, ##__VA_ARGS__)
#define LOGW( tag, format, ... ) LOG_LEVEL_LOCAL(LOG_WARN,    tag, format, ##__VA_ARGS__)
#define LOGI( tag, format, ... ) LOG_LEVEL_LOCAL(LOG_INFO,    tag, format, ##__VA_ARGS__)
#define LOGD( tag, format, ... ) LOG_LEVEL_LOCAL(LOG_DEBUG,   tag, format, ##__VA_ARGS__)
#define LOGV( tag, format, ... ) LOG_LEVEL_LOCAL(LOG_VERBOSE, tag, format, ##__VA_ARGS__)

#if LOG_CONFIG_TIMESTAMP
#define LOG_LEVEL(level, tag, format, ...) do {                     \
        if (level==LOG_ERROR )          { log_write(LOG_ERROR,      tag, LOG_FORMAT(E, format), log_timestamp(), tag, ##__VA_ARGS__); } \
        else if (level==LOG_WARN )      { log_write(LOG_WARN,       tag, LOG_FORMAT(W, format), log_timestamp(), tag, ##__VA_ARGS__); } \
        else if (level==LOG_DEBUG )     { log_write(LOG_DEBUG,      tag, LOG_FORMAT(D, format), log_timestamp(), tag, ##__VA_ARGS__); } \
        else if (level==LOG_VERBOSE )   { log_write(LOG_VERBOSE,    tag, LOG_FORMAT(V, format), log_timestamp(), tag, ##__VA_ARGS__); } \
        else                            { log_write(LOG_INFO,   	tag, LOG_FORMAT(I, format), log_timestamp(), tag, ##__VA_ARGS__); } \
    } while(0)
#else
#define LOG_LEVEL(level, tag, format, ...) do {                     \
        if (level==LOG_ERROR )          { log_write(LOG_ERROR,      tag, LOG_FORMAT(E, format), tag, ##__VA_ARGS__); } \
        else if (level==LOG_WARN )      { log_write(LOG_WARN,       tag, LOG_FORMAT(W, format), tag, ##__VA_ARGS__); } \
        else if (level==LOG_DEBUG )     { log_write(LOG_DEBUG,      tag, LOG_FORMAT(D, format), tag, ##__VA_ARGS__); } \
        else if (level==LOG_VERBOSE )   { log_write(LOG_VERBOSE,    tag, LOG_FORMAT(V, format), tag, ##__VA_ARGS__); } \
        else                            { log_write(LOG_INFO,   	tag, LOG_FORMAT(I, format), tag, ##__VA_ARGS__); } \
    } while(0)
#endif

/** runtime macro to output logs at a specified level. Also check the level with ``LOG_LOCAL_LEVEL``.
 *
 * @see ``printf``, ``LOG_LEVEL``
 */
#define LOG_LEVEL_LOCAL(level, tag, format, ...) do {               \
        if ( LOG_LOCAL_LEVEL >= level ) LOG_LEVEL(level, tag, format, ##__VA_ARGS__); \
    } while(0)

#ifdef __cplusplus
} // extern "C"
#endif

#endif // EOF __LOG_H__
