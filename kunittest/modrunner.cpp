/*
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

#include <kdebug.h>
#include <kglobal.h>
#include <kinstance.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "runner.h"

static const char description[] =
    I18N_NOOP("A command-line application that can be used to run KUnitTest modules.");

static const char version[] = "0.1";

static KCmdLineOptions options[] =
{
    {"query [regexp]", I18N_NOOP("Only run modules whose filenames match the regexp."), "^kunittest_.*\\.la$"},
    {"folder [folder]", I18N_NOOP("Only run tests modules which are found in the folder. Use the query option to select modules."), "."},
    { "enable-dbgcap", I18N_NOOP("Disables debug capturing. You typically use this option when you use the GUI."), 0},
    KCmdLineLastOption
};


int main( int argc, char **argv )
{    
    KInstance instance("modrunner");

    KAboutData about("KUnitTest Module Runner", I18N_NOOP("KUnitTest ModRunner"), version, description,
                     KAboutData::License_BSD, "(C) 2005 Jeroen Wijnhout", 0, 0,
                     "Jeroen.Wijnhout@kdemail.net");

    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KUnitTest::Runner::loadModules(args->getOption("folder"), args->getOption("query"));
    KUnitTest::Runner::setDebugCapturingEnabled(args->isSet("enable-dbgcap"));

    KUnitTest::Runner::self()->runTests();

    return KUnitTest::Runner::self()->numberOfFailedTests() - KUnitTest::Runner::self()->numberOfExpectedFailures();
}
