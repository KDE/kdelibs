    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
   
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#include "debug.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "thread.h"

static int arts_debug_level = Arts::Debug::lInfo;
static bool arts_debug_abort = false;
static const char *arts_debug_prefix = "";
static char *messageAppName = 0;
static Arts::Mutex *arts_debug_mutex = 0;

namespace Arts {

/*
 * Call the graphical application to display a message, if
 * defined. Otherwise, send to standard error. Debug messages are
 * always sent to standard error because they tend to be very verbose.
 * Note that the external application is run in the background to
 * avoid blocking the sound server.
 */
void output_message(Debug::Level level, const char *msg) {
	char buff[1024];

	/* default to text output if no message app is defined or if it is a debug message. */
	if (messageAppName == 0 || !strcmp(messageAppName, "") || (level == Debug::lDebug))
	{
		fprintf(stderr, "%s\n", msg);
		return;
	}

	switch (level) {
	  case Debug::lFatal:
		  sprintf(buff, "%s -e \"Sound server fatal error:\n\n%s\" &", messageAppName, msg);
		  break;
	  case Debug::lWarning:
		  sprintf(buff, "%s -w \"Sound server warning message:\n\n%s\" &", messageAppName, msg);
		  break;
	  case Debug::lInfo:
		  sprintf(buff, "%s -i \"Sound server informational message:\n\n%s\" &", messageAppName, msg);
		  break;
	  default:
		  break; // avoid compile warning
	}
	system(buff);
}

/*
 * Display a message using output_message. If the message is the same
 * as the previous one, just increment a count but don't display
 * it. This prevents flooding the user with duplicate warnings. If the
 * message is not the same as the previous one, then we report the
 * previously repeated message (if any) and reset the last message and
 * count.
 */
void display_message(Debug::Level level, const char *msg) {
	static char lastMsg[1024];
	static Debug::Level lastLevel;
	static int msgCount = 0;

	if(arts_debug_mutex)
		arts_debug_mutex->lock();

	if (!strncmp(msg, lastMsg, 1024))
	{
		msgCount++;
	} else {
		if (msgCount > 0)
		{
			char buff[1024];
			sprintf(buff, "%s\n(The previous message was repeated %d times.)", lastMsg, msgCount);
			output_message(lastLevel, buff);
		}
		strncpy(lastMsg, msg, 1024);
		lastLevel = level;
		msgCount = 0;
		output_message(level, msg);
	}

	if(arts_debug_mutex)
		arts_debug_mutex->unlock();
}

static class DebugInitFromEnv {
public:
	DebugInitFromEnv() {
		const char *env = getenv("ARTS_DEBUG");
		if(env)
		{
			if(strcmp(env,"debug") == 0)
				arts_debug_level = Debug::lDebug;
			else if(strcmp(env,"info") == 0)
				arts_debug_level = Debug::lInfo;
			else if(strcmp(env,"warning") == 0)
				arts_debug_level = Debug::lWarning;
			else if(strcmp(env,"quiet") == 0)
				arts_debug_level = Debug::lFatal;
			else
			{
				fprintf(stderr,
					"ARTS_DEBUG must be one of debug,info,warning,quiet\n");
			}
		}
		env = getenv("ARTS_DEBUG_ABORT");
		if(env)
			arts_debug_abort = true;
	}
} debugInitFromEnv;

};

void Arts::Debug::init(const char *prefix, Level level)
{
	arts_debug_level = level;
	arts_debug_prefix = prefix;
}

void Arts::Debug::fatal(const char *fmt, ...)
{
	char buff[1024];
    va_list ap;
    va_start(ap, fmt);
	vsprintf(buff, fmt, ap);
    va_end(ap);
	display_message(Debug::lFatal, buff);

	if(arts_debug_abort) abort();
	exit(1);
}

void Arts::Debug::warning(const char *fmt, ...)
{
	if(lWarning >= arts_debug_level)
	{
		char buff[1024];
		va_list ap;
		va_start(ap, fmt);
		vsprintf(buff, fmt, ap);
		va_end(ap);
		display_message(Debug::lWarning, buff);
	}
}

void Arts::Debug::info(const char *fmt, ...)
{
	if(lInfo >= arts_debug_level)
	{
		char buff[1024];
		va_list ap;
		va_start(ap, fmt);
		vsprintf(buff, fmt, ap);
		va_end(ap);
		display_message(Debug::lInfo, buff);
	}
}

void Arts::Debug::debug(const char *fmt, ...)
{
	if(lDebug >= arts_debug_level)
	{
		char buff[1024];
		va_list ap;
		va_start(ap, fmt);
		vsprintf(buff, fmt, ap);
		va_end(ap);
		display_message(Debug::lDebug, buff);
	}
}

void Arts::Debug::messageApp(const char *appName)
{
	messageAppName = (char*) realloc(messageAppName, strlen(appName)+1);
	strcpy(messageAppName, appName);
}

void Arts::Debug::initMutex()
{
	arts_return_if_fail(arts_debug_mutex == 0);

	arts_debug_mutex = new Arts::Mutex();
}

void Arts::Debug::freeMutex()
{
	arts_return_if_fail(arts_debug_mutex != 0);

	delete arts_debug_mutex;
	arts_debug_mutex = 0;
}
