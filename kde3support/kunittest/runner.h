/**
 * kunittest.h
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef KUNITTEST_RUNNER_H
#define KUNITTEST_RUNNER_H

#include <iostream>
using namespace std;

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QString>

#include "kunittest_export.h"
#include "tester.h"


namespace KUnitTest
{
    /*! @def KUNITTEST_SUITE(suite)
     *
     * This macro must be used if you are not making a test-module. The macro
     * defines the name of the test suite.
     */
    #define KUNITTEST_SUITE(suite)\
    static const QString s_kunittest_suite  = suite;

    /*! @def KUNITTEST_REGISTER_TESTER( tester )
     * @brief Automatic registration of Tester classes.
     *
     * This macro can be used to register the Tester into the global registry. Use
     * this macro in the implementation file of your Tester class. If you keep the
     * Tester classes in a shared or convenience library then you should not use this
     * macro as this macro relies on the static initialization of a TesterAutoregister class.
     * You can always use the static Runner::registerTester(const char *name, Tester *test) method.
    */
    #define KUNITTEST_REGISTER_TESTER( tester )\
    static TesterAutoregister tester##Autoregister( QString(s_kunittest_suite + QString("::") + QString::fromLocal8Bit(#tester)).local8Bit() , new tester ())

    #define KUNITTEST_REGISTER_NAMEDTESTER( name, tester )\
    static TesterAutoregister tester##Autoregister( QString(s_kunittest_suite + QString("::") + QString::fromLocal8Bit(name)).local8Bit() , new tester ())

    /*! The type of the registry. */
    typedef QHash<QByteArray, Tester*> Registry;

    /*! The Runner class holds a list of registered Tester classes and is able
     * to run those test cases. The Runner class follows the singleton design
     * pattern, which means that you can only have one Runner instance. This
     * instance can be retrieved using the Runner::self() method.
     *
     * The registry is an object of type Registry, it is able to map the name
     * of a test to a pointer to a Tester object. The registry is also a singleton
     * and can be accessed via Runner::registry(). Since there is only one registry,
     * which can be accessed at all times, test cases can be added without having to
     * worry if a Runner instance is present or not. This allows for a design in which
     * the KUnitTest library can be kept separate from the test case sources. Test cases
     * (classes inheriting from Tester) can be added using the static
     * registerTester(const char *name, Tester *test) method. Allthough most users
     * will want to use the KUNITTEST_REGISTER_TESTER macro.
     *
     * @see KUNITTEST_REGISTER_TESTER
     */
    class KUNITTEST_EXPORT Runner : public QObject
    {
        Q_OBJECT

    public:
        /*! Registers a test case. A registry will be automatically created if necessary.
         * @param name The name of the test case.
         * @param test A pointer to a Tester object.
         */
        static void registerTester(const char *name, Tester *test);

        /*! @returns The registry holding all the Tester objects.
         */
        Registry &registry();

        /*! @returns The global Runner instance. If necessary an instance will be created.
         */
        static Runner *self();

        /*! @returns The number of registered test cases.
         */
        int numberOfTestCases();

        /*! Load all modules found in the folder.
         * @param folder The folder where to look for modules.
         * @param query A regular expression. Only modules which match the query will be run.
         */
        static void loadModules(const QString &folder, const QString &query);

        /*! The runner can spit out special debug messages needed by the Perl script: kunittest_debughelper.
         * This script can attach the debug output of each suite to the results in the KUnitTest GUI.
         * Not very useful for console minded developers, so this static method can be used to disable
         * those debug messages.
         * @param enabled If true the debug messages are enabled (default), otherwise they are disabled.
         */
        static void setDebugCapturingEnabled(bool enabled);
            
    private:
        Registry             m_registry;
        static Runner       *s_self;
        static bool          s_debugCapturingEnabled;
    
    protected:
        Runner();

    public:
        /*! @returns The number of finished tests. */
        int numberOfTests() const;

        /*! @returns The number of passed tests. */
        int numberOfPassedTests() const;

        /*! @returns The number of failed tests, this includes the number of expected failures. */
        int numberOfFailedTests() const;

        /*! @returns The number of failed tests which were expected. */
        int numberOfExpectedFailures() const;

        /*! @returns The number of skipped tests. */
        int numberOfSkippedTests() const;

    public Q_SLOTS:
        /*! Call this slot to run all the registered tests.
         * @returns The number of finished tests.
         */
        int runTests();

        /*! Call this slot to run a single test.
         * @param name The name of the test case. This name has to correspond to the name
         * that was used to register the test. If the KUNITTEST_REGISTER_TESTER macro was
         * used to register the test case then this name is the class name.
         */
        void runTest(const char *name);

        /*! Call this slot to run tests with names starting with prefix.
         * @param prefix Only run tests starting with the string prefix.
         */
        void runMatchingTests(const QString &prefix);

        /*! Reset the Runner in order to prepare it to run one or more tests again.
         */
        void reset();

    Q_SIGNALS:
        /*! Emitted after a test is finished.
         * @param name The name of the test.
         * @param test A pointer to the Tester object.
         */
        void finished(const char *name, Tester *test);
        void invoke();

    private:
        void registerTests();

    private:
        int globalSteps;
        int globalTests;
        int globalPasses;
        int globalFails;
        int globalXFails;
        int globalXPasses;
        int globalSkipped;
    };

    /*! The TesterAutoregister is a helper class to allow the automatic registration
     * of Tester classes.
     */
    class TesterAutoregister
    {
    public:
        /*! @param name A unique name that identifies the Tester class.
         * @param test A pointer to a Tester object.
         */
        TesterAutoregister(const char *name, Tester *test)
        {
            if ( test->objectName().isNull())
                test->setObjectName(QLatin1String(name));
            Runner::registerTester(name, test);
        }
    };

}

#endif
