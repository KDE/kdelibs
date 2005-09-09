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
#include "QtTest/qttestassert.h"
#include "QtTest/private/qtestlog_p.h"

#include "QtTest/private/qplaintestlogger_p.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef Q_OS_WIN
#include "windows.h"
#endif

namespace QtTest {

#ifdef Q_OS_WIN

    static CRITICAL_SECTION outputCriticalSection;
    static HANDLE hConsole = INVALID_HANDLE_VALUE;
    static WORD consoleAttributes = 0;

    static const char *qWinColoredMsg(int prefix, int color, const char *msg)
    {
        if (!hConsole)
            return msg;

        WORD attr = consoleAttributes & ~(FOREGROUND_GREEN | FOREGROUND_BLUE
                  | FOREGROUND_RED | FOREGROUND_INTENSITY);
        if (prefix)
            attr |= FOREGROUND_INTENSITY;
        if (color == 32)
            attr |= FOREGROUND_GREEN;
        if (color == 31)
            attr |= FOREGROUND_RED | FOREGROUND_INTENSITY;
        if (color == 37)
            attr |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        if (color == 33)
            attr |= FOREGROUND_RED | FOREGROUND_GREEN;
        SetConsoleTextAttribute(hConsole, attr);
        printf(msg);
        SetConsoleTextAttribute(hConsole, consoleAttributes);
        return "";
    }

# define COLORED_MSG(prefix, color, msg) colored ? qWinColoredMsg(prefix, color, msg) : msg
#else
# define COLORED_MSG(prefix, color, msg) colored ? "\033["#prefix";"#color"m" msg "\033[0m" : msg
#endif

    static const char *incidentType2String(QAbstractTestLogger::IncidentTypes type)
    {
        static bool colored = (getenv("QTEST_COLORED") != 0);
        switch (type) {
        case QAbstractTestLogger::Pass:
            return COLORED_MSG(0, 32, "PASS   "); //green
        case QAbstractTestLogger::XFail:
            return COLORED_MSG(1, 32, "XFAIL  "); //light green
        case QAbstractTestLogger::Fail:
            return COLORED_MSG(0, 31, "FAIL!  "); //red
        case QAbstractTestLogger::XPass:
            return COLORED_MSG(0, 31, "XPASS  "); //red, too
        }
        return "??????";
    }



    static const char *messageType2String(QAbstractTestLogger::MessageTypes type)
    {
        static bool colored = (getenv("QTEST_COLORED") != 0);
        switch (type) {
        case QAbstractTestLogger::Skip:
            return COLORED_MSG(0, 37, "SKIP   "); //white
        case QAbstractTestLogger::Warn:
            return COLORED_MSG(0, 33, "WARNING"); // yellow
        case QAbstractTestLogger::QWarning:
            return COLORED_MSG(1, 33, "QWARN  ");
        case QAbstractTestLogger::QDebug:
            return COLORED_MSG(1, 33, "QDEBUG ");
        case QAbstractTestLogger::QSystem:
            return COLORED_MSG(1, 33, "QSYSTEM");
        case QAbstractTestLogger::QFatal:
            return COLORED_MSG(0, 31, "QFATAL "); // red
        case QAbstractTestLogger::Info:
            return "INFO   "; // no coloring
        }
        return "??????";
    }

    static void outputMessage(const char *str)
    {
#ifdef Q_OS_WIN
        EnterCriticalSection(&outputCriticalSection);
        // OutputDebugString is not threadsafe
        OutputDebugStringA(str);
        LeaveCriticalSection(&outputCriticalSection);
#endif
        QAbstractTestLogger::outputString(str);
    }

    static void printMessage(const char *type, const char *msg, const char *file = 0, int line = 0)
    {
        QTEST_ASSERT(type);
        QTEST_ASSERT(msg);

        char buf[1024];

        const char *fn = QtTestResult::currentTestFunction() ? QtTestResult::currentTestFunction()
            : "UnknownTestFunc";
        const char *tag = QtTestResult::currentDataTag() ? QtTestResult::currentDataTag() : "";
        const char *gtag = QtTestResult::currentGlobalDataTag()
                         ? QtTestResult::currentGlobalDataTag()
                         : "";
        const char *filler = (tag[0] && gtag[0]) ? ":" : "";
        if (file) {
            QtTest::qt_snprintf(buf, sizeof(buf), "%s: %s::%s(%s%s%s)%s%s\n"
#ifdef Q_OS_WIN
                          "%s(%d) : failure location\n"
#else
                          "    Loc: [%s(%d)]\n"
#endif
                          , type, QtTestResult::currentTestObjectName(), fn, gtag, filler, tag,
                          msg[0] ? " " : "", msg, file, line);
        } else {
            QtTest::qt_snprintf(buf, sizeof(buf), "%s: %s::%s(%s%s%s)%s%s\n",
                    type, QtTestResult::currentTestObjectName(), fn, gtag, filler, tag,
                    msg[0] ? " " : "", msg);
        }
        memcpy(buf, type, strlen(type));
        outputMessage(buf);
    }
}

QPlainTestLogger::QPlainTestLogger()
{
#ifdef Q_OS_WIN
    InitializeCriticalSection(&QtTest::outputCriticalSection);
    QtTest::hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (QtTest::hConsole != INVALID_HANDLE_VALUE) {
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (GetConsoleScreenBufferInfo(QtTest::hConsole, &info)) {
            QtTest::consoleAttributes = info.wAttributes;
        } else {
            QtTest::hConsole = INVALID_HANDLE_VALUE;
        }
    }
#endif
}

QPlainTestLogger::~QPlainTestLogger()
{
#ifdef Q_OS_WIN
    DeleteCriticalSection(&QtTest::outputCriticalSection);
#endif
}

void QPlainTestLogger::startLogging()
{
    QAbstractTestLogger::startLogging();

    char buf[1024];
    QtTest::qt_snprintf(buf, sizeof(buf),
                         "********* Start testing of %s *********\n"
                         "Config: Using QtTest library " QTTEST_VERSION_STR
                         ", Qt %s\n", QtTestResult::currentTestObjectName(), qVersion());
    QtTest::outputMessage(buf);
}

void QPlainTestLogger::stopLogging()
{
    char buf[1024];
    QtTest::qt_snprintf(buf, sizeof(buf),
                         "Totals: %d passed, %d failed, %d skipped\n"
                         "********* Finished testing of %s *********\n",
                         QtTestResult::passCount(), QtTestResult::failCount(),
                         QtTestResult::skipCount(), QtTestResult::currentTestObjectName());
    QtTest::outputMessage(buf);

    QAbstractTestLogger::stopLogging();
}


void QPlainTestLogger::enterTestFunction(const char * /*function*/)
{
    if (QtTestLog::verboseLevel() >= 1)
        QtTest::printMessage(QtTest::messageType2String(Info), "entering");
}

void QPlainTestLogger::leaveTestFunction()
{
}

void QPlainTestLogger::addIncident(IncidentTypes type, const char *description,
                                   const char *file, int line)
{
    QtTest::printMessage(QtTest::incidentType2String(type), description, file, line);
}


void QPlainTestLogger::addMessage(MessageTypes type, const char *message,
                                  const char *file, int line)
{
    QtTest::printMessage(QtTest::messageType2String(type), message, file, line);
}



