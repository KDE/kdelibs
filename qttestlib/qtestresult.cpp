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
#include "QtTest/private/qtestresult_p.h"
#include <QtCore/qglobal.h>

#include "QtTest/private/qtestlog_p.h"
#include "QtTest/qttestdata.h"
#include "QtTest/qttestassert.h"

#include <stdio.h>
#include <string.h>

namespace QtTest
{
    static QtTestData *currentTestData = 0;
    static QtTestData *currentGlobalTestData = 0;
    static const char *currentTestFunc = 0;
    static const char *currentTestObjectName = 0;
    static bool failed = false;
    static bool dataFailed = false;
    static QtTestResult::TestLocation location = QtTestResult::NoWhere;

    static int fails = 0;
    static int passes = 0;
    static int skips = 0;

    static const char *expectFailComment = 0;
    static int expectFailMode = 0;
};

bool QtTestResult::allDataPassed()
{
    return !QtTest::failed;
}

bool QtTestResult::currentTestFailed()
{
    return QtTest::dataFailed;
}

QtTestData *QtTestResult::currentGlobalTestData()
{
    return QtTest::currentGlobalTestData;
}

QtTestData *QtTestResult::currentTestData()
{
    return QtTest::currentTestData;
}

void QtTestResult::setCurrentGlobalTestData(QtTestData *data)
{
    QtTest::currentGlobalTestData = data;
}

void QtTestResult::setCurrentTestData(QtTestData *data)
{
    QtTest::currentTestData = data;
    QtTest::dataFailed = false;
}

void QtTestResult::setCurrentTestFunction(const char *func)
{
    QtTest::currentTestFunc = func;
    QtTest::failed = false;
    if (!func)
        QtTest::location = NoWhere;
    if (func)
        QtTestLog::enterTestFunction(func);
}

static void clearExpectFail()
{
    QtTest::expectFailMode = 0;
    QtTest::expectFailComment = 0;
}

void QtTestResult::finishedCurrentTestFunction()
{
    if (!QtTest::failed && QtTestLog::unhandledIgnoreMessages()) {
        QtTestLog::printUnhandledIgnoreMessages();
        addFailure("Not all expected messages were received", 0, 0);
    }

    if (!QtTest::failed) {
        QtTestLog::addPass("");
        ++QtTest::passes;
    }
    QtTest::currentTestFunc = 0;
    QtTest::failed = false;
    QtTest::dataFailed = false;
    QtTest::location = NoWhere;

    QtTestLog::leaveTestFunction();

    clearExpectFail();
}

const char *QtTestResult::currentTestFunction()
{
    return QtTest::currentTestFunc;
}

const char *QtTestResult::currentDataTag()
{
    return QtTest::currentTestData ? QtTest::currentTestData->dataTag()
                                   : static_cast<const char *>(0);
}

const char *QtTestResult::currentGlobalDataTag()
{
    return QtTest::currentGlobalTestData ? QtTest::currentGlobalTestData->dataTag()
                                         : static_cast<const char *>(0);
}

static bool isExpectFailData(const char *dataIndex)
{
    if (!dataIndex || dataIndex[0] == '\0')
        return true;
    if (!QtTest::currentTestData)
        return false;
    if (strcmp(dataIndex, QtTest::currentTestData->dataTag()) == 0)
        return true;
    return false;
}

bool QtTestResult::expectFail(const char *dataIndex, const char *comment,
                             QtTest::TestFailMode mode, const char *file, int line)
{
    QTEST_ASSERT(comment);
    QTEST_ASSERT(mode > 0);

    if (!isExpectFailData(dataIndex))
        return true; // we don't care

    if (QtTest::expectFailMode) {
        clearExpectFail();
        addFailure("Already expecting a fail", file, line);
        return false;
    }

    QtTest::expectFailMode = mode;
    QtTest::expectFailComment = comment;
    return true;
}

static bool checkStatement(bool statement, const char *msg, const char *file, int line)
{
    if (statement) {
        if (QtTest::expectFailMode) {
            QtTestLog::addXPass(msg, file, line);
            clearExpectFail();
            QtTest::failed = true;
            ++QtTest::fails;
            return QtTest::expectFailMode == QtTest::Continue;
        }
        return true;
    }

    if (QtTest::expectFailMode) {
        QtTestLog::addXFail(QtTest::expectFailComment, file, line);
        clearExpectFail();
        return QtTest::expectFailMode == QtTest::Continue;
    }

    QtTestResult::addFailure(msg, file, line);
    return false;
}

bool QtTestResult::verify(bool statement, const char *statementStr,
                         const char *description, const char *file, int line)
{
    char msg[1024];

    if (QtTestLog::verboseLevel() >= 2) {
        QtTest::qt_snprintf(msg, 1024, "VERIFY(%s)", statementStr);
        QtTestLog::info(msg, file, line);
    }

    QtTest::qt_snprintf(msg, 1024, "'%s' returned FALSE. (%s)", statementStr, description);

    return checkStatement(statement, msg, file, line);
}

bool QtTestResult::compare(bool success, const char *msg, const char *file, int line)
{
    if (QtTestLog::verboseLevel() >= 2) {
        QtTestLog::info(msg, file, line);
    }

    return checkStatement(success, msg, file, line);
}

bool QtTestResult::compare(bool success, const char *msg, char *val1, char *val2,
                          const char *file, int line)
{
    if (!val1 && !val2)
        return compare(success, msg, file, line);

    char buf[1024];
    QtTest::qt_snprintf(buf, 1024, "%s\n   Actual: %s\n   Expected: %s", msg,
                       val1 ? val1 : "<null>", val2 ? val2 : "<null>");
    delete [] val1;
    delete [] val2;
    return compare(success, buf, file, line);
}

void QtTestResult::addFailure(const char *message, const char *file, int line)
{
    clearExpectFail();

    QtTestLog::addFail(message, file, line);
    QtTest::failed = true;
    QtTest::dataFailed = true;
    ++QtTest::fails;
}

void QtTestResult::addSkip(const char *message, QtTest::SkipMode mode,
                          const char *file, int line)
{
    clearExpectFail();

    QtTestLog::addSkip(message, mode, file, line);
    ++QtTest::skips;
}

QtTestResult::TestLocation QtTestResult::currentTestLocation()
{
    return QtTest::location;
}

void QtTestResult::setCurrentTestLocation(TestLocation loc)
{
    QtTest::location = loc;
}

void QtTestResult::setCurrentTestObject(const char *name)
{
    QtTest::currentTestObjectName = name;
}

const char *QtTestResult::currentTestObjectName()
{
    return QtTest::currentTestObjectName ? QtTest::currentTestObjectName : "";
}

int QtTestResult::passCount()
{
    return QtTest::passes;
}

int QtTestResult::failCount()
{
    return QtTest::fails;
}

int QtTestResult::skipCount()
{
    return QtTest::skips;
}

void QtTestResult::ignoreMessage(QtMsgType type, const char *msg)
{
    QtTestLog::addIgnoreMessage(type, msg);
}

