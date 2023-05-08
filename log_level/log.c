/**
*******************************************************************************
* @file           : log.c
* @brief          : Description of C implementation module
* @author         : Gonzalo Rivera
* @date           : 04/05/2023
*******************************************************************************
* @attention
*
* Copyright (c) <date> Gonzalo Rivera. All rights reserved.
*
*/
/******************************************************************************
    Includes
******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <inttypes.h>

#include "log.h"
/******************************************************************************
    Defines and constants
******************************************************************************/

/******************************************************************************
    Data types
******************************************************************************/

/******************************************************************************
    Local variables
******************************************************************************/
static vprintf_like_t s_log_print_func = &vprintf;
static get_timestamp_t s_log_get_timestamp_func = NULL;
static log_out_t s_log_out_func = NULL;
/******************************************************************************
    Local function prototypes
******************************************************************************/

/******************************************************************************
    Local function definitions
******************************************************************************/

/******************************************************************************
    Public function definitions
******************************************************************************/
vprintf_like_t log_set_vprintf(vprintf_like_t func)
{
    vprintf_like_t orig_func = s_log_print_func;
    s_log_print_func = func;
    return orig_func;
}

void log_set_timestamp(get_timestamp_t func)
{
	s_log_get_timestamp_func = func;
}

uint32_t log_timestamp(void)
{
    return (uint32_t) s_log_get_timestamp_func();
}

void log_set_output(log_out_t func)
{
	s_log_out_func = func;
}

void log_write(log_level_t level, const char *tag, const char *format, ...)
{
    va_list list;
    va_start(list, format);
    log_writev(level, tag, format, list);
    va_end(list);
}

void log_writev(log_level_t level, const char *tag, const char *format, va_list args)
{
    (*s_log_print_func)(format, args);
}

/**
  * @note This function is a method override for using printf for uart debugging.
  */
int _write(int file, char *ptr, int len)
{
	int data_inx;

	for(data_inx = 0; data_inx < len; data_inx++)
	{
		s_log_out_func((uint8_t *)ptr++, 1);
	}

	return len;
}

