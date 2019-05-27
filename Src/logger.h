/*
 * logger.h
 *
 *  Created on: May 26, 2019
 *      Author: Ben
 */

#ifndef LOGGER_H
#define LOGGER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOG_MAX_LENGTH 4096
#define MESSAGE_MAX_LENGTH 256

enum LogLevel
{
    LOG_LVL_INFO,
    LOG_LVL_DEBUG,
    LOG_LVL_WARNING,
    LOG_LVL_ERROR,
    LOG_LVL_FATAL
};

void LogMessage(int logLevel, const char* filename, int line, const char* format, ...);
void flushLog();

#define LOG(level, ...) LogMessage(level, __FILENAME__, __LINE__, __VA_ARGS__)
#define FLUSHLOG() flushLog()

#ifdef __cplusplus
}
#endif

#endif