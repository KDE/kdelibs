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
#ifndef QTESTRESULT_H
#define QTESTRESULT_H

#include "QtTest/qttest_global.h"

class QtTestResultPrivate;
class QtTestData;

class QtTestResult
{
public:
    enum TestLocation { NoWhere = 0, DataFunc = 1, InitFunc = 2, Func = 3, CleanupFunc = 4 };

    static const char *currentTestObjectName();
    static bool currentTestFailed();
    static bool allDataPassed();
    static QtTestData *currentTestData();
    static QtTestData *currentGlobalTestData();
    static const char *currentTestFunction();
    static TestLocation currentTestLocation();
    static const char *currentDataTag();
    static const char *currentGlobalDataTag();
    static void finishedCurrentTestFunction();

    static int passCount();
    static int failCount();
    static int skipCount();

    static void ignoreMessage(QtMsgType type, const char *msg);

    static void addFailure(const char *message, const char *file, int line);
    static bool compare(bool success, const char *msg, const char *file, int line);
    static bool compare(bool success, const char *msg, char *val1, char *val2,
                        const char *file, int line);

    static void setCurrentGlobalTestData(QtTestData *data);
    static void setCurrentTestData(QtTestData *data);
    static void setCurrentTestFunction(const char *func);
    static void setCurrentTestLocation(TestLocation loc);
    static void setCurrentTestObject(const char *name);
    static void addSkip(const char *message, QtTest::SkipMode mode,
                        const char *file, int line);
    static bool expectFail(const char *dataIndex, const char *comment,
                           QtTest::TestFailMode mode, const char *file, int line);
    static bool verify(bool statement, const char *statementStr, const char *extraInfo,
                       const char *file, int line);
};

#endif

