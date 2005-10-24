/*
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
 * Copyright (C)  2005  Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>
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

#include <stdio.h>
#include <iostream>
using namespace std;

#include <qregexp.h>
#include <qdir.h>
#include <qmetaobject.h>

#include <kdebug.h>
#include <klibloader.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include "runner.h"
#include "tester.h"

namespace KUnitTest
{
    Runner *Runner::s_self = 0L;
    bool    Runner::s_debugCapturingEnabled = false;

    void Runner::registerTester(const char *name, Tester *test)
    {
        Runner::self()->m_registry.insert(name, test);
    }

    void Runner::loadModules(const QString &folder, const QString &query)
    {
        QRegExp reQuery(query);
        QDir dir(folder, "kunittest_*.la");

        // Add the folder to the "module" resource such that the KLibLoader can
        // find the modules in this folder.
        KGlobal::dirs()->addResourceDir("module", folder);
        kdDebug() << "Looking in folder: " << dir.absPath() << endl;

        // Get a list of all modules.
        QStringList modules = dir.entryList();

        for ( uint i = 0; i < modules.count(); ++i )
        {
            QString module = modules[i];
            kdDebug() << "Module: " << dir.absPath() + "/" + module << endl;

            if ( reQuery.search(module) != -1 )
            {
                // strip the .la extension
                module.truncate(module.length()-3);
                KLibFactory *factory = KLibLoader::self()->factory(module.local8Bit());
                if ( factory )
                    factory->create();
                else {
                    kdWarning() << "\tError loading " << module << " : " << KLibLoader::self()->lastErrorMessage() << endl;
                    ::exit( 1 );
                }
            }
            else
                kdDebug() << "\tModule doesn't match." << endl;
        }
    }

    void Runner::setDebugCapturingEnabled(bool enabled)
    {
      s_debugCapturingEnabled = enabled;
    }

    RegistryType &Runner::registry()
    {
        return m_registry;
    }

    int Runner::numberOfTestCases()
    {
        return m_registry.count();
    }

    Runner *Runner::self()
    {
        if ( s_self == 0L ) s_self = new Runner();

        return s_self;
    }

    Runner::Runner()
    {
        reset();
    }

    int Runner::numberOfTests() const
    {
        return globalSteps;
    }

    int Runner::numberOfPassedTests() const
    {
        return globalPasses;
    }

    int Runner::numberOfFailedTests() const
    {
        return globalFails;
    }

    int Runner::numberOfExpectedFailures() const
    {
        return globalXFails;
    }

    int Runner::numberOfSkippedTests() const
    {
        return globalSkipped;
    }

    void Runner::reset()
    {
        globalSteps = 0;
        globalPasses = 0;
        globalFails = 0;
        globalXFails = 0;
        globalXPasses = 0;
        globalSkipped = 0;
    }

    int Runner::runTests()
    {
        globalSteps = 0;
        globalPasses = 0;
        globalFails = 0;
        globalXFails = 0;
        globalXPasses = 0;
        globalSkipped = 0;

        cout << "# Running normal tests... #" << endl << endl;
        RegistryIteratorType it(m_registry);

        for( ; it.current(); ++it )
            runTest(it.currentKey());

#if 0 // very thorough, but not very readable
        cout << "# Done with normal tests:" << endl;
        cout << "  Total test cases: " << m_registry.count() << endl;
        cout << "  Total test steps                                 : " << globalSteps << endl;
        cout << "    Total passed test steps (including unexpected) : " << globalPasses << endl;
        cout << "      Total unexpected passed test steps           :  " << globalXPasses << endl;
        cout << "    Total failed test steps (including expected)   :  " << globalFails << endl;
        cout << "      Total expected failed test steps             :  " << globalXFails << endl;
        cout << "    Total skipped test steps                       :  " << globalSkipped << endl;
#else
        unsigned int numTests = m_registry.count(); // should this be globalSteps instead?
        QString str;
        if ( globalFails == 0 )
            if ( globalXFails == 0 )
                str = QString( "All %1 tests passed" ).arg( numTests );
            else
                str = QString( "All %1 tests behaved as expected (%2 expected failures)" ).arg( numTests ).arg( globalXFails );
        else
            if ( globalXPasses == 0 )
                str = QString( "%1 of %2 tests failed" ).arg( globalFails ).arg( numTests );
            else
                str = QString( "%1 of %2 tests did not behave as expected (%1 unexpected passes)" ).arg( globalFails ).arg( numTests ).arg( globalXPasses );
        if ( globalSkipped )
            str += QString( " (%1 tests skipped)" ).arg( globalSkipped );
        cout << str.local8Bit() << endl;
#endif

        return m_registry.count();
    }

    void Runner::runMatchingTests(const QString &prefix)
    {
        RegistryIteratorType it(m_registry);
        for( ; it.current(); ++it )
            if ( QString(it.currentKey()).startsWith(prefix) )
                runTest(it.currentKey());
    }

    void Runner::runTest(const char *name)
    {
        Tester *test = m_registry.find(name);
        if ( test == 0L ) return;

        if ( s_debugCapturingEnabled )
        {
          cout << "KUnitTest_Debug_Start[" << name << "]" << endl;
        }

        test->results()->clear();
        test->allTests();

        if ( s_debugCapturingEnabled )
        {
          cout << "KUnitTest_Debug_End[" << name << "]" << endl << endl << flush;
        }

        int numPass = 0;
        int numFail = 0;
        int numXFail = 0;
        int numXPass = 0;
        int numSkip = 0;

        if ( test->inherits("KUnitTest::SlotTester") )
        {
            SlotTester *sltest = static_cast<SlotTester*>(test);
            TestResultsListIteratorType it(sltest->resultsList());
            for ( ; it.current(); ++it)
            {
                numPass += it.current()->passed() + it.current()->xpasses();
                numFail += it.current()->errors() + it.current()->xfails();
                numXFail += it.current()->xfails();
                numXPass += it.current()->xpasses();
                numSkip += it.current()->skipped();
                globalSteps += it.current()->testsFinished();
            }
        }
        else
        {
            numPass= test->results()->passed() + test->results()->xpasses();
            numFail= test->results()->errors() + test->results()->xfails();
            numXFail = test->results()->xfails();
            numXPass = test->results()->xpasses();
            numSkip= test->results()->skipped();
            globalSteps += test->results()->testsFinished();
        }


        globalPasses += numPass;
        globalFails += numFail;
        globalXFails += numXFail;
        globalXPasses += numXPass;
        globalSkipped += numSkip;

        cout << name << " - ";
        cout << numPass << " test" << ( ( 1 == numPass )?"":"s") << " passed";
        if ( 0 < test->results()->xpassList().count() ) {
            cout << " (" << numXPass << " unexpected pass" << ( ( 1 == numXPass )?"":"es") << ")";
        }
        cout << ", " << numFail << " test" << ( ( 1 == numFail )?"":"s") << " failed";
        if ( 0 < numXFail  ) {
            cout << " (" << numXFail << " expected failure" << ( ( 1 == numXFail )?"":"s") << ")";
        }
        if ( 0 < numSkip ) {
            cout << "; also " << numSkip << " skipped";
        }
        cout  << endl;

        if ( 0 < numXPass  ) {
        cout << "    Unexpected pass" << ( ( 1 == numXPass )?"":"es") << ":" << endl;
        QStringList list = test->results()->xpassList();
        for ( QStringList::Iterator itr = list.begin(); itr != list.end(); ++itr ) {
            cout << "\t" << (*itr).latin1() << endl;
        }
        }
        if ( 0 < (numFail - numXFail) ) {
        cout << "    Unexpected failure" << ( ( 1 == numFail )?"":"s") << ":" << endl;
        QStringList list = test->results()->errorList();
        for ( QStringList::Iterator itr = list.begin(); itr != list.end(); ++itr ) {
            cout << "\t" << (*itr).latin1() << endl;
        }
        }
        if ( 0 < numXFail ) {
        cout << "    Expected failure" << ( ( 1 == numXFail)?"":"s") << ":" << endl;
        QStringList list = test->results()->xfailList();
        for ( QStringList::Iterator itr = list.begin(); itr != list.end(); ++itr ) {
            cout << "\t" << (*itr).latin1() << endl;
        }
        }
        if ( 0 < numSkip ) {
            cout << "    Skipped test" << ( ( 1 == numSkip )?"":"s") << ":" << endl;
            QStringList list = test->results()->skipList();
            for ( QStringList::Iterator itr = list.begin(); itr != list.end(); ++itr ) {
            cout << "\t" << (*itr).latin1() << endl;
            }
        }
        cout << endl;

        emit finished(name, test);
    }
}

#include "runner.moc"

