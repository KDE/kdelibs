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
#include "QtTest/qttestassert.h"

#include "QtTest/private/qtestlog_p.h"
#include "QtTest/private/qtestresult_p.h"
#include "QtTest/private/qabstracttestlogger_p.h"
#include "QtTest/private/qplaintestlogger_p.h"
#include "QtTest/private/qxmltestlogger_p.h"

#include "qatomic.h"

#include <stdlib.h>
#include <string.h>

namespace QtTest {

    struct IgnoreResultList
    {
        inline IgnoreResultList(QtMsgType tp, const char *message)
            : type(tp), next(0)
        { msg = strdup(message); }
        inline ~IgnoreResultList()
        { free(msg); }

        static inline void clearList(IgnoreResultList *&list)
        {
            while (list) {
                IgnoreResultList *current = list;
                list = list->next;
                delete current;
            }
        }

        QtMsgType type;
        char *msg;
        IgnoreResultList *next;
    };

    static IgnoreResultList *ignoreResultList = 0;

    static QtTestLog::LogMode logMode = QtTestLog::Plain;
    static int verbosity = 0;

    static QAbstractTestLogger *testLogger = 0;
    static const char *outFile = 0;

    static QtMsgHandler oldMessageHandler;

    static bool handleIgnoredMessage(QtMsgType type, const char *msg)
    {
        IgnoreResultList *last = 0;
        IgnoreResultList *list = ignoreResultList;
        while (list) {
            if (list->type == type && strcmp(msg, list->msg) == 0) {
                // remove the item from the list
                if (last)
                    last->next = list->next;
                else if (list->next)
                    ignoreResultList = list->next;
                else
                    ignoreResultList = 0;

                delete list;
                return true;
            }

            last = list;
            list = list->next;
        }
        return false;
    }

    static void messageHandler(QtMsgType type, const char *msg)
    {
        static QBasicAtomic counter = Q_ATOMIC_INIT(2002);

        if (!msg || !QtTest::testLogger) {
            // if this goes wrong, something is seriously broken.
            qInstallMsgHandler(oldMessageHandler);
            QTEST_ASSERT(msg);
            QTEST_ASSERT(QtTest::testLogger);
        }

        if (handleIgnoredMessage(type, msg))
            // the message is expected, so just swallow it.
            return;

        if (type != QtFatalMsg) {
            if (counter <= 0)
                return;

            if (!counter.deref()) {
                QtTest::testLogger->addMessage(QAbstractTestLogger::QSystem,
                        "Maximum amount of warnings exceeded.");
                return;
            }
        }

        switch (type) {
        case QtDebugMsg:
            QtTest::testLogger->addMessage(QAbstractTestLogger::QDebug, msg);
            break;
        case QtCriticalMsg:
            QtTest::testLogger->addMessage(QAbstractTestLogger::QSystem, msg);
            break;
        case QtWarningMsg:
            QtTest::testLogger->addMessage(QAbstractTestLogger::QWarning, msg);
            break;
        case QtFatalMsg:
            QtTest::testLogger->addMessage(QAbstractTestLogger::QFatal, msg);
            break;
        }
    }


}

QtTestLog::QtTestLog()
{
}

QtTestLog::~QtTestLog()
{
}

void QtTestLog::enterTestFunction(const char* function)
{
    QTEST_ASSERT(QtTest::testLogger);
    QTEST_ASSERT(function);

    QtTest::testLogger->enterTestFunction(function);
}

int QtTestLog::unhandledIgnoreMessages()
{
    int i = 0;
    QtTest::IgnoreResultList *list = QtTest::ignoreResultList;
    while (list) {
        ++i;
        list = list->next;
    }
    return i;
}

void QtTestLog::leaveTestFunction()
{
    QTEST_ASSERT(QtTest::testLogger);

    QtTest::IgnoreResultList::clearList(QtTest::ignoreResultList);
    QtTest::testLogger->leaveTestFunction();
}

void QtTestLog::printUnhandledIgnoreMessages()
{
    QTEST_ASSERT(QtTest::testLogger);

    char msg[1024];
    QtTest::IgnoreResultList *list = QtTest::ignoreResultList;
    while (list) {
        QtTest::qt_snprintf(msg, 1024, "Did not receive message: \"%s\"", list->msg);
        QtTest::testLogger->addMessage(QAbstractTestLogger::Info, msg);

        list = list->next;
    }
}

void QtTestLog::addPass(const char *msg)
{
    QTEST_ASSERT(QtTest::testLogger);
    QTEST_ASSERT(msg);

    QtTest::testLogger->addIncident(QAbstractTestLogger::Pass, msg);
}

void QtTestLog::addFail(const char *msg, const char *file, int line)
{
    QTEST_ASSERT(QtTest::testLogger);

    QtTest::testLogger->addIncident(QAbstractTestLogger::Fail, msg, file, line);
}

void QtTestLog::addXFail(const char *msg, const char *file, int line)
{
    QTEST_ASSERT(QtTest::testLogger);
    QTEST_ASSERT(msg);
    QTEST_ASSERT(file);

    QtTest::testLogger->addIncident(QAbstractTestLogger::XFail, msg, file, line);
}

void QtTestLog::addXPass(const char *msg, const char *file, int line)
{
    QTEST_ASSERT(QtTest::testLogger);
    QTEST_ASSERT(msg);
    QTEST_ASSERT(file);

    QtTest::testLogger->addIncident(QAbstractTestLogger::XPass, msg, file, line);
}

void QtTestLog::addSkip(const char *msg, QtTest::SkipMode /*mode*/,
                       const char *file, int line)
{
    QTEST_ASSERT(QtTest::testLogger);
    QTEST_ASSERT(msg);
    QTEST_ASSERT(file);

    QtTest::testLogger->addMessage(QAbstractTestLogger::Skip, msg, file, line);
}

void QtTestLog::startLogging()
{
    QTEST_ASSERT(!QtTest::testLogger);

    switch (QtTest::logMode) {
    case QtTestLog::Plain:
        QtTest::testLogger = new QPlainTestLogger();
        break;
    case QtTestLog::XML:
        QtTest::testLogger = new QXmlTestLogger();
        break;
    }

    QtTest::testLogger->startLogging();

    QtTest::oldMessageHandler = qInstallMsgHandler(QtTest::messageHandler);
}

void QtTestLog::stopLogging()
{
    qInstallMsgHandler(QtTest::oldMessageHandler);

    QTEST_ASSERT(QtTest::testLogger);
    QtTest::testLogger->stopLogging();
    delete QtTest::testLogger;
    QtTest::testLogger = 0;
}

void QtTestLog::warn(const char *msg)
{
    QTEST_ASSERT(msg);

    QtTest::testLogger->addMessage(QAbstractTestLogger::Warn, msg);
}

void QtTestLog::info(const char *msg, const char *file, int line)
{
    QTEST_ASSERT(msg);

    if (QtTest::testLogger)
    QtTest::testLogger->addMessage(QAbstractTestLogger::Info, msg, file, line);
}

void QtTestLog::setLogMode(LogMode mode)
{
    QtTest::logMode = mode;
}

QtTestLog::LogMode QtTestLog::logMode()
{
    return QtTest::logMode;
}

void QtTestLog::setVerboseLevel(int level)
{
    QtTest::verbosity = level;
}

int QtTestLog::verboseLevel()
{
    return QtTest::verbosity;
}

void QtTestLog::addIgnoreMessage(QtMsgType type, const char *msg)
{
    QtTest::IgnoreResultList *item = new QtTest::IgnoreResultList(type, msg);

    QtTest::IgnoreResultList *list = QtTest::ignoreResultList;
    if (!list) {
        QtTest::ignoreResultList = item;
        return;
    }
    while (list->next)
        list = list->next;
    list->next = item;
}

void QtTestLog::redirectOutput(const char *fileName)
{
    QTEST_ASSERT(fileName);

    QtTest::outFile = fileName;
}

const char *QtTestLog::outputFileName()
{
    return QtTest::outFile;
}

