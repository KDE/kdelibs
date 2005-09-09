/***************************************************************************
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
*/
#ifndef QTESTLOG_H
#define QTESTLOG_H

#include "QtTest/qttest_global.h"

class QtTestLog
{
public:
    enum LogMode { Plain = 0, XML };

    static void enterTestFunction(const char* function);
    static void leaveTestFunction();

    static void addPass(const char *msg);
    static void addFail(const char *msg, const char *file, int line);
    static void addXFail(const char *msg, const char *file, int line);
    static void addXPass(const char *msg, const char *file, int line);
    static void addSkip(const char *msg, QtTest::SkipMode mode,
                        const char *file, int line);
    static void addIgnoreMessage(QtMsgType type, const char *msg);
    static int unhandledIgnoreMessages();
    static void printUnhandledIgnoreMessages();

    static void warn(const char *msg);
    static void info(const char *msg, const char *file, int line);

    static void startLogging();
    static void stopLogging();

    static void setLogMode(LogMode mode);
    static LogMode logMode();

    static void setVerboseLevel(int level);
    static int verboseLevel();

    static void redirectOutput(const char *fileName);
    static const char *outputFileName();

private:
    QtTestLog();
    ~QtTestLog();
};

#endif

