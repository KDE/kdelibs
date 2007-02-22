/**
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

#ifndef KUNITTEST_MODULE_H
#define KUNITTEST_MODULE_H

#include <QtCore/QString>

#include <klibloader.h>
#include <kunittest/runner.h>

namespace KUnitTest
{
    /*! @def KUNITTEST_MODULE(library,suite)
    * Use this macro if you are creating a KUnitTest module named library.
    * This macro creates a module-class named a factory class. The module
    * will appear under the name suite in the test runner.
    * There is no need in calling the K_EXPORT_COMPONENT_FACTORY macro,
    * this is taken care of automatically.
    *
    * @code KUNITTEST_MODULE(kunittest_samplemodule,"TestSuite") @endcode
    */
    #define KUNITTEST_MODULE(library,suite)                                                 \
    static const QString s_kunittest_suite  = QLatin1String(suite);                   \
    class library##Module : public QObject                                                  \
    {                                                                                       \
    public:                                                                                 \
        library##Module()                                                                   \
        {                                                                                   \
            KUnitTest::Registry::const_iterator it = s_registry.constBegin();               \
            for( ; it != s_registry.constEnd(); ++it )                                      \
                KUnitTest::Runner::registerTester(it.key(), it.value());                    \
        }                                                                                   \
                                                                                            \
        static KUnitTest::Registry s_registry;                                              \
    };                                                                                      \
                                                                                            \
    KUnitTest::Registry library##Module::s_registry;                                        \
                                                                                            \
    void kunittest_registerModuleTester(const char *name, KUnitTest::Tester *test)          \
    {                                                                                       \
        library##Module::s_registry.insert(name, test);                                     \
    }                                                                                       \
                                                                                            \
    class module##Factory : public KLibFactory                                              \
    {                                                                                       \
    public:                                                                                 \
        QObject *createObject (QObject *, const char *, const char *, const QStringList &)  \
        {                                                                                   \
            return new library##Module();                                                   \
        };                                                                                  \
    };                                                                                      \
                                                                                            \
    K_EXPORT_COMPONENT_FACTORY( library, module##Factory )

    /*! @def KUNITTEST_MODULE_REGISTER_TESTER(tester)
    * Use this macro to add a tester class to your module. The name of the tester will
    * be identical to the class name.
    *
    * @code KUNITTEST_MODULE_REGISTER_TESTER(SimpleSampleTester) @endcode
    */
    #define KUNITTEST_MODULE_REGISTER_TESTER( tester)                                           \
    static class tester##ModuleAutoregister                                                     \
    {                                                                                           \
    public:                                                                                     \
        tester##ModuleAutoregister()                                                            \
        {                                                                                       \
            KUnitTest::Tester *test = new tester();                                             \
            QString name = s_kunittest_suite + QLatin1String("::") + QString::fromLocal8Bit(#tester); \
            test->setName(name.local8Bit());                                                    \
            kunittest_registerModuleTester(name.local8Bit(), test );                            \
        }                                                                                       \
    } tester##ModuleAutoregisterInstance;

    /*! @def KUNITTEST_MODULE_REGISTER_NAMEDTESTER(name,tester)
    * Use this macro to add a tester class, with specified name, to your module..
    *
    * @code KUNITTEST_MODULE_REGISTER_TESTER("SubSuite::PrettyName",SimpleSampleTester) @endcode
    */
    #define KUNITTEST_MODULE_REGISTER_NAMEDTESTER( name , tester)                             \
    static class tester##ModuleAutoregister                                                   \
    {                                                                                         \
    public:                                                                                   \
        tester##ModuleAutoregister()                                                          \
        {                                                                                     \
            QString fullName = s_kunittest_suite + QString("::") + QString::fromLocal8Bit(name); \
            KUnitTest::Tester *test = new tester(fullName.local8Bit());                       \
            kunittest_registerModuleTester(fullName.local8Bit(), test);                       \
        }                                                                                     \
    } tester##ModuleAutoregisterInstance;
}

#endif
