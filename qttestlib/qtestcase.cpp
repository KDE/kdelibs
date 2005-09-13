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
#include "QtTest/qttestcase.h"
#include "QtTest/qttestassert.h"

#include <QtCore/qobject.h>
#include <QtCore/qmetaobject.h>

#include "QtTest/private/qtestlog_p.h"
#include "QtTest/qttesttable.h"
#include "QtTest/qttestdata.h"
#include "QtTest/private/qtestresult_p.h"
#include "QtTest/private/qsignaldumper_p.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef Q_OS_WIN32
#include <windows.h> // for Sleep
#endif
#ifdef Q_OS_UNIX
#include <time.h>
#endif

namespace QtTest
{
    static bool skipCurrentTest = false;
    static QObject *currentTestObject = 0;

    struct TestFunction {
        TestFunction() { function = 0; data = 0; }
        ~TestFunction() { if(data) free(data); }
        int function;
        char *data;
    } testFuncs[512];
    static int lastTestFuncIdx = -1;

    static int keyDelay = -1;
    static int mouseDelay = -1;
    static int eventDelay = -1;
    static int keyVerbose = -1;

int qt_snprintf(char *str, int size, const char *format, ...)
{
    va_list ap;
    int res = 0;

    va_start(ap, format);
    ::qvsnprintf(str, size, format, ap);
    va_end(ap);
    str[size - 1] = '\0';

    char *idx = str;
    while (*idx) {
        if (((*idx < 0x20 && *idx != '\n' && *idx != '\t') || *idx > 0x7e))
            *idx = '?';
        ++idx;
    }
    return res;
}

bool Q_TESTLIB_EXPORT defaultKeyVerbose()
{
    if (keyVerbose == -1) {
        keyVerbose = ::getenv("QTEST_KEYEVENT_VERBOSE") ? 1 : 0;
    }
    return keyVerbose == 1;
}

int Q_TESTLIB_EXPORT defaultEventDelay()
{
    if (eventDelay == -1) {
        if (::getenv("QTEST_EVENT_DELAY"))
            eventDelay = atoi(::getenv("QTEST_EVENT_DELAY"));
        else
            eventDelay = 0;
    }
    return eventDelay;
}

int Q_TESTLIB_EXPORT defaultMouseDelay()
{
    if (mouseDelay == -1) {
        if (::getenv("QTEST_MOUSEEVENT_DELAY"))
            mouseDelay = atoi((::getenv("QTEST_MOUSEEVENT_DELAY")));
        else
            mouseDelay = defaultEventDelay();
    }
    return mouseDelay;
}

int Q_TESTLIB_EXPORT defaultKeyDelay()
{
    if (keyDelay == -1) {
        if (::getenv("QTEST_KEYEVENT_DELAY"))
            keyDelay = atoi(::getenv("QTEST_KEYEVENT_DELAY"));
        else
            keyDelay = defaultEventDelay();
    }
    return keyDelay;
}

static bool isValidSlot(const QMetaMethod &sl)
{
    if (sl.access() != QMetaMethod::Private || !sl.parameterTypes().isEmpty()
        || qstrlen(sl.typeName()) || sl.methodType() != QMetaMethod::Slot)
        return false;
    const char *sig = sl.signature();
    int len = qstrlen(sig);
    if (len < 2)
        return false;
    if (sig[len - 2] != '(' || sig[len - 1] != ')')
        return false;
    if (strcmp(sig, "initTestCase()") == 0 || strcmp(sig, "cleanupTestCase()") == 0
        || strcmp(sig, "cleanup()") == 0 || strcmp(sig, "init()") == 0)
        return false;
    return true;
}

static void qPrintTestSlots()
{
    for (int i = 0; i < QtTest::currentTestObject->metaObject()->methodCount(); ++i) {
        QMetaMethod sl = QtTest::currentTestObject->metaObject()->method(i);
        if (isValidSlot(sl))
            printf("%s\n", sl.signature());
    }
}

static void qParseArgs(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-help") == 0) {
            const char *eHelp = "";
            printf(" Usage: %s [options] [testfunctions[:testdata]]...\n"
                   "    By default, all testfunction will be run.\n\n"
                   " options:\n"
                   " -functions : Returns a list of current testfunctions\n"
                   " -xml       : Outputs results as XML document\n"
                   " -lightxml  : Outputs results as stream of XML tags\n"
                   " -o filename: Writes all output into a file\n"
                   " -v1        : Print enter messages for each testfunction\n"
                   " -v2        : Also print out each VERIFY/COMPARE/TEST\n"
                   " -vs        : Print every signal emitted\n"
                   " -eventdelay ms    : Set default delay for mouse and keyboard simulation to ms milliseconds\n"
                   " -keydelay ms      : Set default delay for keyboard simulation to ms milliseconds\n"
                   " -mousedelay ms    : Set default delay for mouse simulation to ms milliseconds\n"
                   " -keyevent-verbose : Turn on verbose messages for keyboard simulation\n"
                   "%s"
                   " -help      : This help\n", argv[0], eHelp);
            exit(0);
        } else if (strcmp(argv[i], "-functions") == 0) {
            qPrintTestSlots();
            exit(0);
        } else if (strcmp(argv[i], "-xml") == 0) {
            QtTestLog::setLogMode(QtTestLog::XML);
        } else if (strcmp(argv[i], "-lightxml") == 0) {
            QtTestLog::setLogMode(QtTestLog::LightXML);
        } else if (strcmp(argv[i], "-v1") == 0) {
            QtTestLog::setVerboseLevel(1);
        } else if (strcmp(argv[i], "-v2") == 0) {
            QtTestLog::setVerboseLevel(2);
        } else if (strcmp(argv[i], "-vs") == 0) {
            QSignalDumper::startDump();
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                printf("-o needs an extra parameter specifying the filename\n");
                exit(1);
            } else {
                QtTestLog::redirectOutput(argv[++i]);
            }
        } else if (strcmp(argv[i], "-eventdelay") == 0) {
            if (i + 1 >= argc) {
                printf("-eventdelay needs an extra parameter to indicate the delay(ms)\n");
                exit(1);
            } else {
                QtTest::eventDelay = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-keydelay") == 0) {
            if (i + 1 >= argc) {
                printf("-keydelay needs an extra parameter to indicate the delay(ms)\n");
                exit(1);
            } else {
                QtTest::keyDelay = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-mousedelay") == 0) {
            if (i + 1 >= argc) {
                printf("-mousedelay needs an extra parameter to indicate the delay(ms)\n");
                exit(1);
            } else {
                QtTest::mouseDelay = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-keyevent-verbose") == 0) {
            QtTest::keyVerbose = 1;
        } else if (strcmp(argv[i], "-qws") == 0) {
            // do nothing
        } else if (argv[i][0] == '-') {
            printf("Unknown option: '%s'\n", argv[i]);
            exit(1);
        } else {
            int colon = -1;
            char buf[512], *data=0;
            for(int off = 0; *(argv[i]+off); ++off) {
                if (*(argv[i]+off) == ':') {
                    colon = off;
                    break;
                }
            }
            if(colon != -1) {
                *(argv[i]+colon) = '\0';
                data = qstrdup(argv[i]+colon+1);
            }
            QtTest::qt_snprintf(buf, 512, "%s()", argv[i]);
            int idx = QtTest::currentTestObject->metaObject()->indexOfMethod(buf);
            if (idx < 0 || !isValidSlot(QtTest::currentTestObject->metaObject()->method(idx))) {
                printf("Unknown testfunction: '%s'\n", buf);
                printf("Available testfunctions:\n");
                qPrintTestSlots();
                exit(1);
            }
            ++QtTest::lastTestFuncIdx;
            QtTest::testFuncs[QtTest::lastTestFuncIdx].function = idx;
            QtTest::testFuncs[QtTest::lastTestFuncIdx].data = data;
            QTEST_ASSERT(QtTest::lastTestFuncIdx < 512);
        }
    }
}


static bool qInvokeTestMethod(const char *slotName, const char *data=0)
{
    QTEST_ASSERT(slotName);

    char cur[512];
    QtTestTable table;

    char *sl = qstrdup(slotName);
    sl[strlen(sl) - 2] = '\0';
    QtTestResult::setCurrentTestFunction(sl);

    const QtTestTable *gTable = QtTestTable::globalTestTable();
    const int globalDataCount = gTable->dataCount();
    int curGlobalDataIndex = 0;
    do {
        if (!gTable->isEmpty())
            QtTestResult::setCurrentGlobalTestData(gTable->testData(curGlobalDataIndex));

        QtTestResult::setCurrentTestLocation(QtTestResult::DataFunc);
        QtTest::qt_snprintf(cur, 512, "%s_data", sl);
        QMetaObject::invokeMethod(QtTest::currentTestObject, cur, Qt::DirectConnection,
                QGenericArgument("QtTestTable&", &table));

        bool foundFunction = false;
        if (!QtTest::skipCurrentTest) {
            int curDataIndex = 0;
            const int dataCount = table.dataCount();
            do {
                if (!data || !qstrcmp(data, table.testData(curDataIndex)->dataTag())) {
                    foundFunction = true;
                    if (!table.isEmpty())
                        QtTestResult::setCurrentTestData(table.testData(curDataIndex));
                    QtTestResult::setCurrentTestLocation(QtTestResult::InitFunc);
                    QMetaObject::invokeMethod(QtTest::currentTestObject, "init");
                    if (QtTest::skipCurrentTest)
                        break;

                    QtTestResult::setCurrentTestLocation(QtTestResult::Func);
                    if (!QMetaObject::invokeMethod(QtTest::currentTestObject, sl,
                                                  Qt::DirectConnection)) {
                        QtTestResult::addFailure("Unable to execute slot", __FILE__, __LINE__);
                        break;
                    }

                    QtTestResult::setCurrentTestLocation(QtTestResult::CleanupFunc);
                    QMetaObject::invokeMethod(QtTest::currentTestObject, "cleanup");
                    QtTestResult::setCurrentTestLocation(QtTestResult::NoWhere);
                    QtTestResult::setCurrentTestData(0);

                    if (QtTest::skipCurrentTest)
                        // check whether SkipAll was requested
                        break;
                    if (data)
                        break;
                }
                ++curDataIndex;
            } while (curDataIndex < dataCount);
        }
        QtTest::skipCurrentTest = false;

        if (data && !foundFunction) {
            printf("Unknown testdata for function %s: '%s'\n", slotName, data);
            printf("Available testdata:\n");
            for(int i = 0; i < table.dataCount(); ++i)
                printf("%s\n", table.testData(i)->dataTag());
            return false;
        }

        QtTestResult::setCurrentGlobalTestData(0);
        ++curGlobalDataIndex;
    } while (curGlobalDataIndex < globalDataCount);

    QtTestResult::finishedCurrentTestFunction();
    delete[] sl;

    return true;
}

void *fetchData(QtTestData *data, const char *tagName, const char *typeName)
{
    QTEST_ASSERT(typeName);
    QTEST_ASSERT_X(data, "QtTest::fetchData()", "Test data requested, but no testdata available .");
    QTEST_ASSERT(data->parent());

    int idx = data->parent()->indexOf(tagName);

    if (qstrcmp(typeName, data->parent()->elementType(idx)) != 0) {
        qFatal("Requested type '%s' does not match available type '%s'.", typeName,
               data->parent()->elementType(idx));
    }

    return data->data(idx);
}

} // namespace

int QtTest::exec(QObject *testObject, int argc, char **argv)
{
#ifndef QT_NO_EXCEPTIONS
    try {
#endif

#if defined(Q_CC_MSVC)
    SetErrorMode(SetErrorMode(0) | SEM_NOGPFAULTERRORBOX);
#endif

    QTEST_ASSERT(testObject);
    QTEST_ASSERT(!currentTestObject);
    currentTestObject = testObject;

    const QMetaObject *mo = testObject->metaObject();
    QTEST_ASSERT(mo);

    QtTestResult::setCurrentTestObject(mo->className());
    qParseArgs(argc, argv);

    QtTestLog::startLogging();

    QtTestResult::setCurrentTestFunction("initTestCase");
    QtTestResult::setCurrentTestLocation(QtTestResult::DataFunc);
    QtTestTable *gTable = QtTestTable::globalTestTable();
    QMetaObject::invokeMethod(testObject, "initTestCase_data", Qt::DirectConnection,
                              QGenericArgument("QtTestTable&", gTable));

    QtTestResult::setCurrentTestLocation(QtTestResult::Func);
    QMetaObject::invokeMethod(testObject, "initTestCase");
    QtTestResult::finishedCurrentTestFunction();

    if (lastTestFuncIdx >= 0) {
        for (int i = 0; i <= lastTestFuncIdx; ++i) {
            qInvokeTestMethod(mo->method(testFuncs[i].function).signature(), testFuncs[i].data);
        }
    } else {
        int sc = mo->methodCount();
        for (int i = 0; i < sc; ++i) {
            QMetaMethod sl = mo->method(i);
            if (!isValidSlot(sl))
                continue;
            qInvokeTestMethod(sl.signature());
        }
    }

    QtTestResult::setCurrentTestFunction("cleanupTestCase");
    QMetaObject::invokeMethod(testObject, "cleanupTestCase");
    QtTestResult::finishedCurrentTestFunction();
    QtTestResult::setCurrentTestFunction(0);
    delete gTable; gTable = 0;

#ifndef QT_NO_EXCEPTIONS
    } catch (...) {
        QtTestResult::addFailure("Caught unhandled exception", __FILE__, __LINE__);
        QtTestLog::stopLogging();
        return -1;
    }
#endif

    QtTestLog::stopLogging();
    currentTestObject = 0;
#ifdef QTEST_NOEXITCODE
    return 0;
#else
    return QtTestResult::failCount();
#endif
}

void QtTest::fail(const char *statementStr, const char *file, int line)
{
    QtTestResult::addFailure(statementStr, file, line);
}

bool QtTest::verify(bool statement, const char *statementStr, const char *description,
                   const char *file, int line)
{
    return QtTestResult::verify(statement, statementStr, description, file, line);
}

void QtTest::skip(const char *message, QtTest::SkipMode mode,
                 const char *file, int line)
{
    QtTestResult::addSkip(message, mode, file, line);
    if (mode == QtTest::SkipAll)
        skipCurrentTest = true;
}

bool QtTest::expectFail(const char *dataIndex, const char *comment,
                       QtTest::TestFailMode mode, const char *file, int line)
{
    return QtTestResult::expectFail(dataIndex, comment, mode, file, line);
}

void QtTest::warn(const char *message)
{
    QtTestLog::warn(message);
}

void QtTest::ignoreMessage(QtMsgType type, const char *message)
{
    QtTestResult::ignoreMessage(type, message);
}

void *QtTest::data(const char *tagName, const char *typeName)
{
    return fetchData(QtTestResult::currentTestData(), tagName, typeName);
}

void *QtTest::globalData(const char *tagName, const char *typeName)
{
    return fetchData(QtTestResult::currentGlobalTestData(), tagName, typeName);
}

void *QtTest::elementData(const char *tagName, int metaTypeId)
{
    QTEST_ASSERT(tagName);
    QtTestData *data = QtTestResult::currentTestData();
    QTEST_ASSERT(data);
    QTEST_ASSERT(data->parent());

    int idx = data->parent()->indexOf(tagName);
    QTEST_ASSERT(idx != -1);
    QTEST_ASSERT(data->parent()->elementTypeId(idx) == metaTypeId);

    return data->data(data->parent()->indexOf(tagName));
}

const char *QtTest::currentTestFunction()
{
    return QtTestResult::currentTestFunction();
}

const char *QtTest::currentDataTag()
{
    return QtTestResult::currentDataTag();
}

bool QtTest::currentTestFailed()
{
    return QtTestResult::currentTestFailed();
}

void QtTest::sleep(int ms)
{
#ifdef Q_OS_WIN32
    Sleep(uint(ms));
#else
    struct timespec ts = { 0, 0 };
    // a nanosecond is 1/1000 of a microsecond, a microsecond is 1/1000 of a millisecond
    ts.tv_nsec = ms * 1000 * 1000;
    nanosleep(&ts, NULL);
#endif
}

QObject *QtTest::testObject()
{
    return currentTestObject;
}

namespace QtTest
{

#define COMPARE_IMPL2(klass, format)\
template<> \
bool compare(klass const &t1, klass const &t2, const char *file, int line)\
{\
    char msg[1024];\
    msg[0] = '\0';\
    bool isOk = true;\
    if (!(t1 == t2)) {\
        qt_snprintf(msg, 1024, "Compared values of type "#klass" are not the same.\n"\
                            "   Actual  : '"#format"'\n"\
                            "   Expected: '"#format"'", t1, t2);\
        isOk = false;\
    } else {\
        qt_snprintf(msg, 1024, "COMPARE('"#format"', type "#klass")'", t1);\
    } \
    return QtTestResult::compare(isOk, msg, file, line);\
}

COMPARE_IMPL2(short, %hd)
COMPARE_IMPL2(ushort, %hu)
COMPARE_IMPL2(int, %d)
COMPARE_IMPL2(uint, %u)
COMPARE_IMPL2(long, %ld)
COMPARE_IMPL2(ulong, %lu)
COMPARE_IMPL2(qint64, %lld)
COMPARE_IMPL2(quint64, %llu)
COMPARE_IMPL2(bool, %d)
COMPARE_IMPL2(char, %c)

bool compare_helper(bool success, const char *msg, const char *file, int line)
{
    return QtTestResult::compare(success, msg, file, line);
}

bool compare_helper(bool success, const char *msg, char *val1, char *val2,
             const char *file, int line)
{
    return QtTestResult::compare(success, msg, val1, val2, file, line);
}

bool compare_string_helper(const char *t1, const char *t2, const char *file, int line)
{
    char msg[1024];
    msg[0] = '\0';
    bool isOk = true;
    if (t1 == t2)
        return QtTestResult::compare(true, msg, file, line);
    if (!t1 || !t2 || strcmp(t1, t2)) {
        qt_snprintf(msg, 1024, "Compared values of type char * are not the same.\n"
                "   Actual  : '%s'\n"
                "   Expected: '%s'",
                t1 == 0 ? "<nul>" : t1,
                t2 == 0 ? "<nul>" : t2);
        isOk = false;
    } else {
        qt_snprintf(msg, 1024, "COMPARE('%s', type char *)", t1);
    }
    return QtTestResult::compare(isOk, msg, file, line);
}

bool compare_ptr_helper(const void *t1, const void *t2, const char *file, int line)
{
    char msg[1024];
    msg[0] = '\0';
    bool isOk = true;
    if (!(t1 == t2)) {
        qt_snprintf(msg, 1024, "Compared pointers are not the same.\n"
                            "   Actual  : '%p'\n"
                            "   Expected: '%p'", t1, t2);
        isOk = false;
    } else {
        qt_snprintf(msg, 1024, "COMPARE('%p', type pointer)'", t1);
    }
    return QtTestResult::compare(isOk, msg, file, line);
}

template<>
bool compare(float const &t1, float const &t2, const char *file, int line)
{
    char msg[1024];
    msg[0] = '\0';
    bool isOk = true;
    if (qAbs(t1 - t2) > 0.00001f) {
        qt_snprintf(msg, 1024, "Compared values of type float are not the same (fuzzy compare).\n"
                            "   Actual  : %f\n"
                            "   Expected: %f", t1, t2);
        isOk = false;
    } else {
        qt_snprintf(msg, 1024, "COMPARE('%f', type float)", t1);
    }
    return QtTestResult::compare(isOk, msg, file, line);
}

template<>
bool compare(double const &t1, double const &t2, const char *file, int line)
{
    char msg[1024];
    msg[0] = '\0';
    bool isOk = true;
    if (qAbs(t1 - t2) > 0.000000000001) {
        qt_snprintf(msg, 1024, "Compared values of type double are not the same (fuzzy compare).\n"
                            "   Actual  : %lf\n"
                            "   Expected: %lf", t1, t2);
        isOk = false;
    } else {
        qt_snprintf(msg, 1024, "COMPARE('%lf', double)", t1);
    }
    return QtTestResult::compare(isOk, msg, file, line);
}

} // namespace QtTest
