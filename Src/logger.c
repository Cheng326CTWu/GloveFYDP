/*
 * logger.c
 *
 *  Created on: May 26, 2019
 *      Author: Ben
 */

#include "logger.h"
#include <time.h>
#include <string.h>

const char* LogLevelString [] =
{
    "INFO   :",
    "DEBUG  :",
    "WARNING:",
    "ERROR  :",
    "FATAL  :"
};

char logBuffer[LOG_MAX_LENGTH];
size_t logBufferSize = 0;

void LogMessage(int logLevel, const char* filename, int line, const char* format, ...)
{
    // if log buffer >= max log size; reset it
    if(logBufferSize >= LOG_MAX_LENGTH)
    {
        logBufferSize = 0;
    }

    size_t offset = 0;
    char messageBuffer[MESSAGE_MAX_LENGTH];

    time_t t = time(NULL);
    struct tm *timeInfo = localtime(&t);

    // memset the 256 bytes block to null termination
    memset(logBuffer+logBufferSize, '\0', MESSAGE_MAX_LENGTH);

    // print timestamp to message buffer
    offset = strftime(messageBuffer, MESSAGE_MAX_LENGTH, "[%T] ", timeInfo);

    offset += snprintf(messageBuffer+offset, MESSAGE_MAX_LENGTH-offset, "%s ", LogLevelString[logLevel]);

    if(logLevel == LOG_LVL_ERROR || logLevel == LOG_LVL_FATAL)
    {
        offset += snprintf(messageBuffer+offset, MESSAGE_MAX_LENGTH-offset, "(File: %s at line %i) ", filename, line);
    }

    va_list args;
    va_start(args, format);
    size_t tempOffset = vsnprintf(messageBuffer+offset, MESSAGE_MAX_LENGTH-offset, format, args);
    // check if bytes that can be written if size is sufficiently large
    // is greater than max message buffer length - offset - 3 (\r\n\0)
    // then set offset to max length - offset - 3 such that
    // \r\n\0 will replace the last 2 index in the buffer to further
    // truncate message
    // Otherwise, add to offset as is
    if(tempOffset > MESSAGE_MAX_LENGTH-(offset+3))
    {
        offset += MESSAGE_MAX_LENGTH-(offset+3);
    }
    else
    {
        offset += tempOffset;
    }
    va_end(args);

    offset += snprintf(messageBuffer+offset, MESSAGE_MAX_LENGTH-offset, "\r\n");

    // print the message buffer content into log buffer
    strncpy(logBuffer+logBufferSize, messageBuffer, MESSAGE_MAX_LENGTH);

    // increment log buffer size to next block
    logBufferSize += MESSAGE_MAX_LENGTH;
}

void flushLog()
{
    for(int i = 0; i < 4; ++i)
    {
        printf("%s", logBuffer+(i*MESSAGE_MAX_LENGTH));
    }

    // When log buffer is flushed, clear the entire buffer
    // so no duplicate messages are flushed out when the next
    // flush happens
    memset(logBuffer, '\0', LOG_MAX_LENGTH);
    logBufferSize = 0;
}