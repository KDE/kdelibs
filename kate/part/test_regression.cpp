/**
 * This file is part of the KDE project
 *
 * Copyright (C) 2001,2003 Peter Kelly (pmk@post.com)
 * Copyright (C) 2003,2004 Stephan Kulow (coolo@kde.org)
 * Copyright (C) 2004 Dirk Mueller ( mueller@kde.org )
 * Copyright 2006 Leo Savernik (l.savernik@aon.at)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>

#include <kapplication.h>
#include <kstandarddirs.h>
#include <qimage.h>
#include <qfile.h>
#include "test_regression.h"
#include <unistd.h>
#include <stdio.h>

#include <kaction.h>
#include <kcmdlineargs.h>
#include "katefactory.h"
#include <kio/job.h>
#include <kmainwindow.h>
#include <ksimpleconfig.h>
#include <kglobalsettings.h>

#include <qcolor.h>
#include <qcursor.h>
#include <qdir.h>
#include <qevent.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qscrollview.h>
#include <qstring.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qvaluelist.h>
#include <qwidget.h>
#include <qfileinfo.h>
#include <qtimer.h>
#include <kstatusbar.h>
#include <qfileinfo.h>

#include "katedocument.h"
#include "kateview.h"
#include <kparts/browserextension.h>
#include "katejscript.h"
#include "katedocumenthelpers.h"
#include "kateconfig.h"
#include "../interfaces/katecmd.h"

using namespace KJS;

#define BASE_DIR_CONFIG "/.testkateregression-3.5"

//BEGIN TestJScriptEnv

TestJScriptEnv::TestJScriptEnv(KateDocument *part) {
  ExecState *exec = m_interpreter->globalExec();

  KJS::ObjectImp *wd = wrapDocument(m_interpreter->globalExec(), part);
  KateView *v = static_cast<KateView *>(part->widget());
  KJS::ObjectImp *wv = new KateViewObject(exec, v, wrapView(m_interpreter->globalExec(), v));

  *m_view = KJS::Object(wv);
  *m_document = KJS::Object(wd);
  m_output = new OutputObject(exec, part, v);
  m_output->ref();

  // recreate properties
  m_interpreter->globalObject().put(exec, "document", *m_document);
  m_interpreter->globalObject().put(exec, "view", *m_view);
  // create new properties
  m_interpreter->globalObject().put(exec, "output", KJS::Object(m_output));
  // add convenience shortcuts
  m_interpreter->globalObject().put(exec, "d", *m_document);
  m_interpreter->globalObject().put(exec, "v", *m_view);
  m_interpreter->globalObject().put(exec, "out", KJS::Object(m_output));
  m_interpreter->globalObject().put(exec, "o", KJS::Object(m_output));
}

TestJScriptEnv::~TestJScriptEnv() {
    m_output->deref();
}

//END TestJScriptEnv

//BEGIN KateViewObject

KateViewObject::KateViewObject(ExecState *exec, KateView *v, ObjectImp *fallback)
  : view(v), fallback(fallback)
{
// put a function
#define PUT_FUNC(name, enumval) \
    putDirect(#name, new KateViewFunction(exec,v,KateViewFunction::enumval,1), DontEnum)
    fallback->ref();

    PUT_FUNC(keyReturn, KeyReturn);
    PUT_FUNC(enter, KeyReturn);
    PUT_FUNC(type, Type);
    PUT_FUNC(keyDelete, KeyDelete);
    PUT_FUNC(deleteWordRight, DeleteWordRight);
    PUT_FUNC(transpose, Transpose);
    PUT_FUNC(cursorLeft, CursorLeft);
    PUT_FUNC(cursorPrev, CursorLeft);
    PUT_FUNC(left, CursorLeft);
    PUT_FUNC(prev, CursorLeft);
    PUT_FUNC(shiftCursorLeft, ShiftCursorLeft);
    PUT_FUNC(shiftCursorPrev, ShiftCursorLeft);
    PUT_FUNC(shiftLeft, ShiftCursorLeft);
    PUT_FUNC(shiftPrev, ShiftCursorLeft);
    PUT_FUNC(cursorRight, CursorRight);
    PUT_FUNC(cursorNext, CursorRight);
    PUT_FUNC(right, CursorRight);
    PUT_FUNC(next, CursorRight);
    PUT_FUNC(shiftCursorRight, ShiftCursorRight);
    PUT_FUNC(shiftCursorNext, ShiftCursorRight);
    PUT_FUNC(shiftRight, ShiftCursorRight);
    PUT_FUNC(shiftNext, ShiftCursorRight);
    PUT_FUNC(wordLeft, WordLeft);
    PUT_FUNC(wordPrev, WordLeft);
    PUT_FUNC(shiftWordLeft, ShiftWordLeft);
    PUT_FUNC(shiftWordPrev, ShiftWordLeft);
    PUT_FUNC(wordRight, WordRight);
    PUT_FUNC(wordNext, WordRight);
    PUT_FUNC(shiftWordRight, ShiftWordRight);
    PUT_FUNC(shiftWordNext, ShiftWordRight);
    PUT_FUNC(home, Home);
    PUT_FUNC(shiftHome, ShiftHome);
    PUT_FUNC(end, End);
    PUT_FUNC(shiftEnd, ShiftEnd);
    PUT_FUNC(up, Up);
    PUT_FUNC(shiftUp, ShiftUp);
    PUT_FUNC(down, Down);
    PUT_FUNC(shiftDown, ShiftDown);
    PUT_FUNC(scrollUp, ScrollUp);
    PUT_FUNC(scrollDown, ScrollDown);
    PUT_FUNC(topOfView, TopOfView);
    PUT_FUNC(shiftTopOfView, ShiftTopOfView);
    PUT_FUNC(bottomOfView, BottomOfView);
    PUT_FUNC(shiftBottomOfView, ShiftBottomOfView);
    PUT_FUNC(pageUp, PageUp);
    PUT_FUNC(shiftPageUp, ShiftPageUp);
    PUT_FUNC(pageDown, PageDown);
    PUT_FUNC(shiftPageDown, ShiftPageDown);
    PUT_FUNC(top, Top);
    PUT_FUNC(shiftTop, ShiftTop);
    PUT_FUNC(bottom, Bottom);
    PUT_FUNC(shiftBottom, ShiftBottom);
    PUT_FUNC(toMatchingBracket, ToMatchingBracket);
    PUT_FUNC(shiftToMatchingBracket, ShiftToMatchingBracket);
#undef PUT_FUNC
}

KateViewObject::~KateViewObject()
{
    fallback->deref();
}

const ClassInfo *KateViewObject::classInfo() const {
    // evil hack II: disguise as fallback, otherwise we can't fall back
    return fallback->classInfo();
}

Value KateViewObject::get(ExecState *exec, const Identifier &propertyName) const
{
    ValueImp *val = getDirect(propertyName);
    if (val)
        return Value(val);

    return fallback->get(exec, propertyName);
}

//END KateViewObject

//BEGIN KateViewFunction

KateViewFunction::KateViewFunction(ExecState */*exec*/, KateView *v, int _id, int length)
{
    m_view = v;
    id = _id;
    putDirect("length",length);
}

bool KateViewFunction::implementsCall() const
{
    return true;
}

Value KateViewFunction::call(ExecState *exec, Object &/*thisObj*/, const List &args)
{
    // calls a function repeatedly as specified by its first parameter (once
    // if not specified).
#define REP_CALL(enumval, func) \
        case enumval: {\
            int cnt = 1;\
            if (args.size() > 0) cnt = args[0].toInt32(exec);\
            while (cnt-- > 0) { m_view->func(); }\
            return Undefined();\
        }
    switch (id) {
        REP_CALL(KeyReturn, keyReturn);
        REP_CALL(KeyDelete, keyDelete);
        REP_CALL(DeleteWordRight, deleteWordRight);
        REP_CALL(Transpose, transpose);
        REP_CALL(CursorLeft, cursorLeft);
        REP_CALL(ShiftCursorLeft, shiftCursorLeft);
        REP_CALL(CursorRight, cursorRight);
        REP_CALL(ShiftCursorRight, shiftCursorRight);
        REP_CALL(WordLeft, wordLeft);
        REP_CALL(ShiftWordLeft, shiftWordLeft);
        REP_CALL(WordRight, wordRight);
        REP_CALL(ShiftWordRight, shiftWordRight);
        REP_CALL(Home, home);
        REP_CALL(ShiftHome, shiftHome);
        REP_CALL(End, end);
        REP_CALL(ShiftEnd, shiftEnd);
        REP_CALL(Up, up);
        REP_CALL(ShiftUp, shiftUp);
        REP_CALL(Down, down);
        REP_CALL(ShiftDown, shiftDown);
        REP_CALL(ScrollUp, scrollUp);
        REP_CALL(ScrollDown, scrollDown);
        REP_CALL(TopOfView, topOfView);
        REP_CALL(ShiftTopOfView, shiftTopOfView);
        REP_CALL(BottomOfView, bottomOfView);
        REP_CALL(ShiftBottomOfView, shiftBottomOfView);
        REP_CALL(PageUp, pageUp);
        REP_CALL(ShiftPageUp, shiftPageUp);
        REP_CALL(PageDown, pageDown);
        REP_CALL(ShiftPageDown, shiftPageDown);
        REP_CALL(Top, top);
        REP_CALL(ShiftTop, shiftTop);
        REP_CALL(Bottom, bottom);
        REP_CALL(ShiftBottom, shiftBottom);
        REP_CALL(ToMatchingBracket, toMatchingBracket);
        REP_CALL(ShiftToMatchingBracket, shiftToMatchingBracket);
        case Type: {
            UString str = args[0].toString(exec);
            QString res = str.qstring();
            return Boolean(m_view->doc()->typeChars(m_view, res));
        }
    }

    return Undefined();
#undef REP_CALL
}

//END KateViewFunction

//BEGIN OutputObject

OutputObject::OutputObject(KJS::ExecState *exec, KateDocument *d, KateView *v) : doc(d), view(v), changed(0), outstr(0) {
    putDirect("write", new OutputFunction(exec,this,OutputFunction::Write,-1), DontEnum);
    putDirect("print", new OutputFunction(exec,this,OutputFunction::Write,-1), DontEnum);
    putDirect("writeln", new OutputFunction(exec,this,OutputFunction::Writeln,-1), DontEnum);
    putDirect("println", new OutputFunction(exec,this,OutputFunction::Writeln,-1), DontEnum);
    putDirect("writeLn", new OutputFunction(exec,this,OutputFunction::Writeln,-1), DontEnum);
    putDirect("printLn", new OutputFunction(exec,this,OutputFunction::Writeln,-1), DontEnum);

    putDirect("writeCursorPosition", new OutputFunction(exec,this,OutputFunction::WriteCursorPosition,-1), DontEnum);
    putDirect("cursorPosition", new OutputFunction(exec,this,OutputFunction::WriteCursorPosition,-1), DontEnum);
    putDirect("pos", new OutputFunction(exec,this,OutputFunction::WriteCursorPosition,-1), DontEnum);
    putDirect("writeCursorPositionln", new OutputFunction(exec,this,OutputFunction::WriteCursorPositionln,-1), DontEnum);
    putDirect("cursorPositionln", new OutputFunction(exec,this,OutputFunction::WriteCursorPositionln,-1), DontEnum);
    putDirect("posln", new OutputFunction(exec,this,OutputFunction::WriteCursorPositionln,-1), DontEnum);

}

OutputObject::~OutputObject() {
}

KJS::UString OutputObject::className() const {
    return UString("OutputObject");
}

//END OutputObject

//BEGIN OutputFunction

OutputFunction::OutputFunction(KJS::ExecState *exec, OutputObject *output, int _id, int length)
    : o(output)
{
    id = _id;
    if (length >= 0)
        putDirect("length",length);
}

bool OutputFunction::implementsCall() const
{
    return true;
}

KJS::Value OutputFunction::call(KJS::ExecState *exec, KJS::Object &thisObj, const KJS::List &args)
{
    if (!*o->changed) *o->outstr = QString();

    switch (id) {
        case Write:
        case Writeln: {
            // Gather all parameters and concatenate to string
            QString res;
            for (int i = 0; i < args.size(); i++) {
                res += args[i].toString(exec).qstring();
            }

            if (id == Writeln)
                res += "\n";

            *o->outstr += res;
            break;
        }
        case WriteCursorPositionln:
        case WriteCursorPosition: {
            // Gather all parameters and concatenate to string
            QString res;
            for (int i = 0; i < args.size(); i++) {
                res += args[i].toString(exec).qstring();
            }

            // Append cursor position
            uint l, c;
            o->view->cursorPosition(&l, &c);
            res += "(" + QString::number(l) + "," + QString::number(c) + ")";

            if (id == WriteCursorPositionln)
                res += "\n";

            *o->outstr += res;
            break;
        }
    }

    *o->changed = true;
    return Undefined();
}

//END OutputFunction

// -------------------------------------------------------------------------

const char failureSnapshotPrefix[] = "testkateregressionrc-FS.";

static QString findMostRecentFailureSnapshot() {
    QDir dir(kapp->dirs()->saveLocation("config"),
             QString(failureSnapshotPrefix)+"*",
             QDir::Time, QDir::Files);
    return dir[0].mid(sizeof failureSnapshotPrefix - 1);
}

static KCmdLineOptions options[] =
{
    { "b", 0, 0 },
    { "base <base_dir>", "Directory containing tests, basedir and output directories.", 0},
    { "cmp-failures <snapshot>", "Compare failures of this testrun against snapshot <snapshot>. Defaults to the most recently captured failure snapshot or none if none exists.", 0 },
    { "d", 0, 0 },
    { "debug", "Do not supress debug output", 0},
    { "g", 0, 0 } ,
    { "genoutput", "Regenerate baseline (instead of checking)", 0 } ,
    { "keep-output", "Keep output files even on success", 0 },
    { "save-failures <snapshot>", "Save failures of this testrun as failure snapshot <snapshot>", 0 },
    { "s", 0, 0 } ,
    { "show", "Show the window while running tests", 0 } ,
    { "t", 0, 0 } ,
    { "test <filename>", "Only run a single test. Multiple options allowed.", 0 } ,
    { "o", 0, 0 },
    { "output <directory>", "Put output in <directory> instead of <base_dir>/output", 0 } ,
    { "+[base_dir]", "Directory containing tests,basedir and output directories. Only regarded if -b is not specified.", 0 } ,
    { "+[testcases]", "Relative path to testcase, or directory of testcases to be run (equivalent to -t).", 0 } ,
    KCmdLineLastOption
};

int main(int argc, char *argv[])
{
    // forget about any settings
    passwd* pw = getpwuid( getuid() );
    if (!pw) {
        fprintf(stderr, "dang, I don't even know who I am.\n");
        exit(1);
    }

    QString kh("/var/tmp/%1_kate_non_existent");
    kh = kh.arg( pw->pw_name );
    setenv( "KDEHOME", kh.latin1(), 1 );
    setenv( "LC_ALL", "C", 1 );
    setenv( "LANG", "C", 1 );

//     signal( SIGALRM, signal_handler );

    KCmdLineArgs::init(argc, argv, "testregression", "TestRegression",
                       "Regression tester for kate", "1.0");
    KCmdLineArgs::addCmdLineOptions(options);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs( );

    QCString baseDir = args->getOption("base");
    QCString baseDirConfigFile(::getenv("HOME") + QCString(BASE_DIR_CONFIG));
    {
        QFile baseDirConfig(baseDirConfigFile);
        if (baseDirConfig.open(IO_ReadOnly)) {
            QTextStream bds(&baseDirConfig);
            baseDir = bds.readLine().latin1();
        }
    }

    if ( args->count() < 1 && baseDir.isEmpty() ) {
        printf("For regression testing, make sure to have checked out the kate regression\n"
               "testsuite from svn:\n"
               "\tsvn co \"https://<user>@svn.kde.org:/home/kde/trunk/tests/katetests/regression\"\n"
               "Remember the root path into which you checked out the testsuite.\n"
               "\n");
	printf("%s needs the root path of the kate regression\n"
               "testsuite to function properly\n"
               "By default, the root path is looked up in the file\n"
               "\t%s\n"
               "If it doesn't exist yet, create it by invoking\n"
               "\techo \"<root-path>\" > %s\n"
               "You may override the location by specifying the root explicitly on the\n"
               "command line with option -b\n"
               "", KCmdLineArgs::appName(),
               (const char *)baseDirConfigFile,
               (const char *)baseDirConfigFile);
	::exit( 1 );
    }

    int testcase_index = 0;
    if (baseDir.isEmpty()) baseDir = args->arg(testcase_index++);

    QFileInfo bdInfo(baseDir);
    baseDir = QFile::encodeName(bdInfo.absFilePath());

    const char *subdirs[] = {"tests", "baseline", "output", "resources"};
    for ( int i = 0; i < 2; i++ ) {
        QFileInfo sourceDir(QFile::encodeName( baseDir ) + "/" + subdirs[i]);
        if ( !sourceDir.exists() || !sourceDir.isDir() ) {
            fprintf(stderr,"ERROR: Source directory \"%s/%s\": no such directory.\n", (const char *)baseDir, subdirs[i]);
            exit(1);
        }
    }

    KApplication a;
    a.disableAutoDcopRegistration();
    a.setStyle("windows");
    KSimpleConfig cfg( "testkateregressionrc" );
    cfg.setGroup("Kate Document Defaults");
    cfg.writeEntry("Basic Config Flags",
      KateDocumentConfig::cfBackspaceIndents
//       | KateDocumentConfig::cfWordWrap
//       | KateDocumentConfig::cfRemoveSpaces
      | KateDocumentConfig::cfWrapCursor
//       | KateDocumentConfig::cfAutoBrackets
//       | KateDocumentConfig::cfTabIndentsMode
//       | KateDocumentConfig::cfOvr
      | KateDocumentConfig::cfKeepIndentProfile
      | KateDocumentConfig::cfKeepExtraSpaces
      | KateDocumentConfig::cfTabIndents
      | KateDocumentConfig::cfShowTabs
      | KateDocumentConfig::cfSpaceIndent
      | KateDocumentConfig::cfSmartHome
      | KateDocumentConfig::cfTabInsertsTab
//       | KateDocumentConfig::cfReplaceTabsDyn
//       | KateDocumentConfig::cfRemoveTrailingDyn
      | KateDocumentConfig::cfDoxygenAutoTyping
//       | KateDocumentConfig::cfMixedIndent
      | KateDocumentConfig::cfIndentPastedText
    );
    cfg.sync();

    int rv = 1;

    {
        KSimpleConfig dc( "kdebugrc" );
        // FIXME adapt to kate
        static int areas[] = { 1000, 13000, 13001, 13002, 13010,
                               13020, 13025, 13030, 13033, 13035,
                               13040, 13050, 13051, 7000, 7006, 170,
                               171, 7101, 7002, 7019, 7027, 7014,
                               7001, 7011, 6070, 6080, 6090, 0};
        int channel = args->isSet( "debug" ) ? 2 : 4;
        for ( int i = 0; areas[i]; ++i ) {
            dc.setGroup( QString::number( areas[i] ) );
            dc.writeEntry( "InfoOutput", channel );
        }
        dc.sync();

        kdClearDebugConfig();
    }

    // create widgets
    KateFactory *fac = KateFactory::self();
    KMainWindow *toplevel = new KMainWindow();
    KateDocument *part = new KateDocument(/*bSingleViewMode*/true,
                                          /*bBrowserView*/false,
                                          /*bReadOnly*/false,
                                          /*parentWidget*/toplevel,
                                          /*widgetName*/"testkate");
    part->readConfig(&cfg);

    toplevel->setCentralWidget( part->widget() );

    Q_ASSERT(part->config()->configFlags() & KateDocumentConfig::cfDoxygenAutoTyping);

    bool visual = false;
    if (args->isSet("show"))
	visual = true;

    a.setTopWidget(part->widget());
    a.setMainWidget( toplevel );
    if ( visual )
        toplevel->show();

    // we're not interested
    toplevel->statusBar()->hide();

    if (!getenv("KDE_DEBUG")) {
        // set ulimits
        rlimit vmem_limit = { 256*1024*1024, RLIM_INFINITY };	// 256Mb Memory should suffice
        setrlimit(RLIMIT_AS, &vmem_limit);
        rlimit stack_limit = { 8*1024*1024, RLIM_INFINITY };	// 8Mb Memory should suffice
        setrlimit(RLIMIT_STACK, &stack_limit);
    }

    // run the tests
    RegressionTest *regressionTest = new RegressionTest(part,
                                                        &cfg,
                                                        baseDir,
                                                        args->getOption("output"),
                                                        args->isSet("genoutput"));
    QObject::connect(part->browserExtension(), SIGNAL(openURLRequest(const KURL &, const KParts::URLArgs &)),
		     regressionTest, SLOT(slotOpenURL(const KURL&, const KParts::URLArgs &)));
    QObject::connect(part->browserExtension(), SIGNAL(resizeTopLevelWidget( int, int )),
		     regressionTest, SLOT(resizeTopLevelWidget( int, int )));

    regressionTest->m_keepOutput = args->isSet("keep-output");
    regressionTest->m_showGui = args->isSet("show");

    {
        QString failureSnapshot = args->getOption("cmp-failures");
        if (failureSnapshot.isEmpty())
            failureSnapshot = findMostRecentFailureSnapshot();
        if (!failureSnapshot.isEmpty())
            regressionTest->setFailureSnapshotConfig(
                    new KSimpleConfig(failureSnapshotPrefix + failureSnapshot, true),
                    failureSnapshot);
    }

    if (args->isSet("save-failures")) {
        QString failureSaver = args->getOption("save-failures");
        regressionTest->setFailureSnapshotSaver(
                new KSimpleConfig(failureSnapshotPrefix + failureSaver, false),
                failureSaver);
    }

    bool result = false;
    QCStringList tests = args->getOptionList("test");
    // merge testcases specified on command line
    for (; testcase_index < args->count(); testcase_index++)
        tests << args->arg(testcase_index);
    if (tests.count() > 0)
        for (QValueListConstIterator<QCString> it = tests.begin(); it != tests.end(); ++it) {
	    result = regressionTest->runTests(*it,true);
            if (!result) break;
        }
    else
	result = regressionTest->runTests();

    if (result) {
	if (args->isSet("genoutput")) {
	    printf("\nOutput generation completed.\n");
	}
	else {
	    printf("\nTests completed.\n");
            printf("Total:    %d\n",
                   regressionTest->m_passes_work+
                   regressionTest->m_passes_fail+
                   regressionTest->m_failures_work+
                   regressionTest->m_failures_fail+
                   regressionTest->m_errors);
	    printf("Passes:   %d",regressionTest->m_passes_work);
            if ( regressionTest->m_passes_fail )
                printf( " (%d unexpected passes)", regressionTest->m_passes_fail );
            if (regressionTest->m_passes_new)
                printf(" (%d new since %s)", regressionTest->m_passes_new, regressionTest->m_failureComp->group().latin1());
            printf( "\n" );
	    printf("Failures: %d",regressionTest->m_failures_work);
            if ( regressionTest->m_failures_fail )
                printf( " (%d expected failures)", regressionTest->m_failures_fail );
            if ( regressionTest->m_failures_new )
                printf(" (%d new since %s)", regressionTest->m_failures_new, regressionTest->m_failureComp->group().latin1());
            printf( "\n" );
            if ( regressionTest->m_errors )
                printf("Errors:   %d\n",regressionTest->m_errors);

            QFile list( regressionTest->m_outputDir + "/links.html" );
            list.open( IO_WriteOnly|IO_Append );
            QString link, cl;
            link = QString( "<hr>%1 failures. (%2 expected failures)" )
                   .arg(regressionTest->m_failures_work )
                   .arg( regressionTest->m_failures_fail );
            if (regressionTest->m_failures_new)
                link += QString(" <span style=\"color:red;font-weight:bold\">(%1 new failures since %2)</span>")
                        .arg(regressionTest->m_failures_new)
                        .arg(regressionTest->m_failureComp->group());
            if (regressionTest->m_passes_new)
                link += QString(" <p style=\"color:green;font-weight:bold\">%1 new passes since %2</p>")
                        .arg(regressionTest->m_passes_new)
                        .arg(regressionTest->m_failureComp->group());
            list.writeBlock( link.latin1(), link.length() );
            list.close();
	}
    }

    // Only return a 0 exit code if all tests were successful
    if (regressionTest->m_failures_work == 0 && regressionTest->m_errors == 0)
	rv = 0;

    // cleanup
    delete regressionTest;
    delete part;
    delete toplevel;
//     delete fac;

    return rv;
}

// -------------------------------------------------------------------------

RegressionTest *RegressionTest::curr = 0;

RegressionTest::RegressionTest(KateDocument *part, KConfig *baseConfig,
                               const QString &baseDir,
                               const QString &outputDir, bool _genOutput)
  : QObject(part)
{
    m_part = part;
    m_view = static_cast<KateView *>(m_part->widget());
    m_baseConfig = baseConfig;
    m_baseDir = baseDir;
    m_baseDir = m_baseDir.replace( "//", "/" );
    if ( m_baseDir.endsWith( "/" ) )
        m_baseDir = m_baseDir.left( m_baseDir.length() - 1 );
    if (outputDir.isEmpty())
        m_outputDir = m_baseDir + "/output";
    else
        m_outputDir = outputDir;
    createMissingDirs(m_outputDir + "/");
    m_keepOutput = false;
    m_genOutput = _genOutput;
    m_failureComp = 0;
    m_failureSave = 0;
    m_showGui = false;
    m_passes_work = m_passes_fail = m_passes_new = 0;
    m_failures_work = m_failures_fail = m_failures_new = 0;
    m_errors = 0;

    ::unlink( QFile::encodeName( m_outputDir + "/links.html" ) );
    QFile f( m_outputDir + "/empty.html" );
    QString s;
    f.open( IO_WriteOnly | IO_Truncate );
    s = "<html><body>Follow the white rabbit";
    f.writeBlock( s.latin1(), s.length() );
    f.close();
    f.setName( m_outputDir + "/index.html" );
    f.open( IO_WriteOnly | IO_Truncate );
    s = "<html><frameset cols=150,*><frame src=links.html><frame name=content src=empty.html>";
    f.writeBlock( s.latin1(), s.length() );
    f.close();

    curr = this;
}

#include <qobjectlist.h>

static QStringList readListFile( const QString &filename )
{
    // Read ignore file for this directory
    QString ignoreFilename = filename;
    QFileInfo ignoreInfo(ignoreFilename);
    QStringList ignoreFiles;
    if (ignoreInfo.exists()) {
        QFile ignoreFile(ignoreFilename);
        if (!ignoreFile.open(IO_ReadOnly)) {
            fprintf(stderr,"Can't open %s\n",ignoreFilename.latin1());
            exit(1);
        }
        QTextStream ignoreStream(&ignoreFile);
        QString line;
        while (!(line = ignoreStream.readLine()).isNull())
            ignoreFiles.append(line);
        ignoreFile.close();
    }
    return ignoreFiles;
}

RegressionTest::~RegressionTest()
{
    // Important! Delete comparison config *first* as saver config
    // might point to the same physical file.
    delete m_failureComp;
    delete m_failureSave;
}

void RegressionTest::setFailureSnapshotConfig(KConfig *cfg, const QString &sname)
{
    Q_ASSERT(cfg);
    m_failureComp = cfg;
    m_failureComp->setGroup(sname);
}

void RegressionTest::setFailureSnapshotSaver(KConfig *cfg, const QString &sname)
{
    Q_ASSERT(cfg);
    m_failureSave = cfg;
    m_failureSave->setGroup(sname);
}

QStringList RegressionTest::concatListFiles(const QString &relPath, const QString &filename)
{
    QStringList cmds;
    int pos = relPath.findRev('/');
    if (pos >= 0)
        cmds += concatListFiles(relPath.left(pos), filename);
    cmds += readListFile(m_baseDir + "/tests/" + relPath + "/" + filename);
    return cmds;
}

bool RegressionTest::runTests(QString relPath, bool mustExist, int known_failure)
{
    m_currentOutput = QString::null;

    if (!QFile(m_baseDir + "/tests/"+relPath).exists()) {
	fprintf(stderr,"%s: No such file or directory\n",relPath.latin1());
	return false;
    }

    QString fullPath = m_baseDir + "/tests/"+relPath;
    QFileInfo info(fullPath);

    if (!info.exists() && mustExist) {
	fprintf(stderr,"%s: No such file or directory\n",relPath.latin1());
	return false;
    }

    if (!info.isReadable() && mustExist) {
	fprintf(stderr,"%s: Access denied\n",relPath.latin1());
	return false;
    }

    if (info.isDir()) {
        QStringList ignoreFiles = readListFile(  m_baseDir + "/tests/"+relPath+"/ignore" );
        QStringList failureFiles = readListFile(  m_baseDir + "/tests/"+relPath+"/KNOWN_FAILURES" );

	// Run each test in this directory, recusively
	QDir sourceDir(m_baseDir + "/tests/"+relPath);
	for (uint fileno = 0; fileno < sourceDir.count(); fileno++) {
	    QString filename = sourceDir[fileno];
	    QString relFilename = relPath.isEmpty() ? filename : relPath+"/"+filename;

            if (filename.startsWith(".") || ignoreFiles.contains(filename) )
                continue;
            int failure_type = NoFailure;
            if ( failureFiles.contains( filename ) )
                failure_type |= AllFailure;
            if ( failureFiles.contains ( filename + "-result" ) )
                failure_type |= ResultFailure;
            runTests(relFilename, false, failure_type);
	}
    }
    else if (info.isFile()) {

	QString relativeDir = QFileInfo(relPath).dirPath();
	QString filename = info.fileName();
	m_currentBase = m_baseDir + "/tests/"+relativeDir;
	m_currentCategory = relativeDir;
	m_currentTest = filename;
        m_known_failures = known_failure;
        m_outputCustomised = false;
        // gather commands
        // directory-specific commands
        QStringList commands = concatListFiles(relPath, ".kateconfig-commands");
        // testcase-specific commands
        commands += readListFile(m_currentBase + "/" + filename + "-commands");

        rereadConfig(); // reset options to default
	if ( filename.endsWith(".txt") ) {
#if 0
            if ( relPath.startsWith( "domts/" ) && !m_runJS )
                return true;
	    if ( relPath.startsWith( "ecma/" ) && !m_runJS )
	        return true;
#endif
//             if ( m_runHTML )
                testStaticFile(relPath, commands);
	}
	else if (mustExist) {
	    fprintf(stderr,"%s: Not a valid test file (must be .txt)\n",relPath.latin1());
	    return false;
	}
    } else if (mustExist) {
        fprintf(stderr,"%s: Not a regular file\n",relPath.latin1());
        return false;
    }

    return true;
}

void RegressionTest::createLink( const QString& test, int failures )
{
    createMissingDirs( m_outputDir + "/" + test + "-compare.html" );

    QFile list( m_outputDir + "/links.html" );
    list.open( IO_WriteOnly|IO_Append );
    QString link;
    link = QString( "<a href=\"%1\" target=\"content\" title=\"%2\">" )
           .arg( test + "-compare.html" )
           .arg( test );
    link += m_currentTest;
    link += "</a> ";
    if (failures & NewFailure)
        link += "<span style=\"font-weight:bold;color:red\">";
    link += "[";
    if ( failures & ResultFailure )
        link += "R";
    link += "]";
    if (failures & NewFailure)
        link += "</span>";
    link += "<br>\n";
    list.writeBlock( link.latin1(), link.length() );
    list.close();
}

/** returns the path in a way that is relatively reachable from base.
 * @param base base directory (must not include trailing slash)
 * @param path directory/file to be relatively reached by base
 * @return path with all elements replaced by .. and concerning path elements
 *	to be relatively reachable from base.
 */
static QString makeRelativePath(const QString &base, const QString &path)
{
    QString absBase = QFileInfo(base).absFilePath();
    QString absPath = QFileInfo(path).absFilePath();
//     kdDebug() << "absPath: \"" << absPath << "\"" << endl;
//     kdDebug() << "absBase: \"" << absBase << "\"" << endl;

    // walk up to common ancestor directory
    int pos = 0;
    do {
        pos++;
        int newpos = absBase.find('/', pos);
        if (newpos == -1) newpos = absBase.length();
        QConstString cmpPathComp(absPath.unicode() + pos, newpos - pos);
        QConstString cmpBaseComp(absBase.unicode() + pos, newpos - pos);
//         kdDebug() << "cmpPathComp: \"" << cmpPathComp.string() << "\"" << endl;
//         kdDebug() << "cmpBaseComp: \"" << cmpBaseComp.string() << "\"" << endl;
//         kdDebug() << "pos: " << pos << " newpos: " << newpos << endl;
        if (cmpPathComp.string() != cmpBaseComp.string()) { pos--; break; }
        pos = newpos;
    } while (pos < (int)absBase.length() && pos < (int)absPath.length());
    int basepos = pos < (int)absBase.length() ? pos + 1 : pos;
    int pathpos = pos < (int)absPath.length() ? pos + 1 : pos;

//     kdDebug() << "basepos " << basepos << " pathpos " << pathpos << endl;

    QString rel;
    {
        QConstString relBase(absBase.unicode() + basepos, absBase.length() - basepos);
        QConstString relPath(absPath.unicode() + pathpos, absPath.length() - pathpos);
        // generate as many .. as there are path elements in relBase
        if (relBase.string().length() > 0) {
            for (int i = relBase.string().contains('/'); i > 0; --i)
                rel += "../";
            rel += "..";
            if (relPath.string().length() > 0) rel += "/";
        }
        rel += relPath.string();
    }
    return rel;
}

/** processes events for at least \c msec milliseconds */
static void pause(int msec)
{
    QTime t;
    t.start();
    do {
        kapp->processEvents();
    } while (t.elapsed() < msec);
}

void RegressionTest::doFailureReport( const QString& test, int failures )
{
    if ( failures == NoFailure ) {
        ::unlink( QFile::encodeName( m_outputDir + "/" + test + "-compare.html" ) );
        return;
    }

    createLink( test, failures );

    QFile compare( m_outputDir + "/" + test + "-compare.html" );

    QString testFile = QFileInfo(test).fileName();

    QString renderDiff;
    QString domDiff;

    QString relOutputDir = makeRelativePath(m_baseDir, m_outputDir);

    // are blocking reads possible with KProcess?
    char pwd[PATH_MAX];
    (void) getcwd( pwd, PATH_MAX );
    chdir( QFile::encodeName( m_baseDir ) );

    if ( failures & ResultFailure ) {
        domDiff += "<pre>";
        FILE *pipe = popen( QString::fromLatin1( "diff -u baseline/%1-result %3/%2-result" )
                            .arg ( test, test, relOutputDir ).latin1(), "r" );
        QTextIStream *is = new QTextIStream( pipe );
        for ( int line = 0; line < 100 && !is->eof(); ++line ) {
            QString line = is->readLine();
            line = line.replace( '<', "&lt;" );
            line = line.replace( '>', "&gt;" );
            domDiff += line  + "\n";
        }
        delete is;
        pclose( pipe );
        domDiff += "</pre>";
    }

    chdir( pwd );

    // create a relative path so that it works via web as well. ugly
    QString relpath = makeRelativePath(m_outputDir + "/"
        + QFileInfo(test).dirPath(), m_baseDir);

    compare.open( IO_WriteOnly|IO_Truncate );
    QString cl;
    cl = QString( "<html><head><title>%1</title>" ).arg( test );
    cl += QString( "<script>\n"
                  "var pics = new Array();\n"
                  "pics[0]=new Image();\n"
                  "pics[0].src = '%1';\n"
                  "pics[1]=new Image();\n"
                  "pics[1].src = '%2';\n"
                  "var doflicker = 1;\n"
                  "var t = 1;\n"
                  "var lastb=0;\n" )
          .arg( relpath+"/baseline/"+test+"-dump.png" )
          .arg( testFile+"-dump.png" );
    cl += QString( "function toggleVisible(visible) {\n"
                  "     document.getElementById('render').style.visibility= visible == 'render' ? 'visible' : 'hidden';\n"
                  "     document.getElementById('image').style.visibility= visible == 'image' ? 'visible' : 'hidden';\n"
                  "     document.getElementById('dom').style.visibility= visible == 'dom' ? 'visible' : 'hidden';\n"
                  "}\n"
                  "function show() { document.getElementById('image').src = pics[t].src; "
                  "document.getElementById('image').style.borderColor = t && !doflicker ? 'red' : 'gray';\n"
                  "toggleVisible('image');\n"
                   "}" );
    cl += QString ( "function runSlideShow(){\n"
                  "   document.getElementById('image').src = pics[t].src;\n"
                  "   if (doflicker)\n"
                  "       t = 1 - t;\n"
                  "   setTimeout('runSlideShow()', 200);\n"
                  "}\n"
                  "function m(b) { if (b == lastb) return; document.getElementById('b'+b).className='buttondown';\n"
                  "                var e = document.getElementById('b'+lastb);\n"
                  "                 if(e) e.className='button';\n"
                  "                 lastb = b;\n"
                  "}\n"
                  "function showRender() { doflicker=0;toggleVisible('render')\n"
                  "}\n"
                  "function showDom() { doflicker=0;toggleVisible('dom')\n"
                  "}\n"
                   "</script>\n");

    cl += QString ("<style>\n"
                   ".buttondown { cursor: pointer; padding: 0px 20px; color: white; background-color: blue; border: inset blue 2px;}\n"
                   ".button { cursor: pointer; padding: 0px 20px; color: black; background-color: white; border: outset blue 2px;}\n"
                   ".diff { position: absolute; left: 10px; top: 100px; visibility: hidden; border: 1px black solid; background-color: white; color: black; /* width: 800; height: 600; overflow: scroll; */ }\n"
                   "</style>\n" );

    cl += QString( "<body onload=\"m(5); toggleVisible('dom');\"" );
    cl += QString(" text=black bgcolor=gray>\n<h1>%3</h1>\n" ).arg( test );
    if ( renderDiff.length() )
        cl += "<span id='b4' class='button' onclick='showRender();m(4)'>R-DIFF</span>&nbsp;\n";
    if ( domDiff.length() )
        cl += "<span id='b5' class='button' onclick='showDom();m(5);'>D-DIFF</span>&nbsp;\n";
    // The test file always exists - except for checkOutput called from *.js files
    if ( QFile::exists( m_baseDir + "/tests/"+ test ) )
        cl += QString( "<a class=button href=\"%1\">HTML</a>&nbsp;" )
              .arg( relpath+"/tests/"+test );

    cl += QString( "<hr>"
                   "<img style='border: solid 5px gray' src=\"%1\" id='image'>" )
          .arg( relpath+"/baseline/"+test+"-dump.png" );

    cl += "<div id='render' class='diff'>" + renderDiff + "</div>";
    cl += "<div id='dom' class='diff'>" + domDiff + "</div>";

    cl += "</body></html>";
    compare.writeBlock( cl.latin1(), cl.length() );
    compare.close();
}

void RegressionTest::testStaticFile(const QString & filename, const QStringList &commands)
{
    qApp->mainWidget()->resize( 800, 600); // restore size

    // Set arguments
    KParts::URLArgs args;
    if (filename.endsWith(".txt")) args.serviceType = "text/plain";
    m_part->browserExtension()->setURLArgs(args);
    // load page
    KURL url;
    url.setProtocol("file");
    url.setPath(QFileInfo(m_baseDir + "/tests/"+filename).absFilePath());
    m_part->openURL(url);

    // inject commands
    for (QStringList::ConstIterator cit = commands.begin(); cit != commands.end(); ++cit) {
        QString str = (*cit).stripWhiteSpace();
        if (str.isEmpty() || str.startsWith("#")) continue;
        Kate::Command *cmd = KateCmd::self()->queryCommand(str);
        if (cmd) {
            QString msg;
            if (!cmd->exec(m_view, str, msg))
                fprintf(stderr, "ERROR executing command '%s': %s\n", str.latin1(), msg.latin1());
        }
    }

    pause(200);

    Q_ASSERT(m_part->config()->configFlags() & KateDocumentConfig::cfDoxygenAutoTyping);

    bool script_error = false;
    {
        // Execute script
        TestJScriptEnv jsenv(m_part);
        jsenv.output()->setChangedFlag(&m_outputCustomised);
        jsenv.output()->setOutputString(&m_outputString);
        script_error = evalJS(jsenv.interpreter(), m_baseDir + "/tests/"+QFileInfo(filename).dirPath()+"/.kateconfig-script", true)
            && evalJS(jsenv.interpreter(), m_baseDir + "/tests/"+filename+"-script");
    }

    int back_known_failures = m_known_failures;

    if (!script_error) goto bail_out;

    if (m_showGui) kapp->processEvents();

    if ( m_genOutput ) {
        reportResult(checkOutput(filename+"-result"), "result");
    } else {
        int failures = NoFailure;

        // compare with output file
        if ( m_known_failures & ResultFailure)
            m_known_failures = AllFailure;
        bool newfail;
        if ( !reportResult( checkOutput(filename+"-result"), "result", &newfail ) )
            failures |= ResultFailure;
        if (newfail)
            failures |= NewFailure;

        doFailureReport(filename, failures );
    }

bail_out:
    m_known_failures = back_known_failures;
    m_part->setModified(false);
    m_part->closeURL();
}

bool RegressionTest::evalJS(Interpreter &interp, const QString &filename, bool ignore_nonexistent)
{
    QString fullSourceName = filename;
    QFile sourceFile(fullSourceName);

    if (!sourceFile.open(IO_ReadOnly)) {
        if (!ignore_nonexistent) {
            fprintf(stderr,"ERROR reading file %s\n",fullSourceName.latin1());
            m_errors++;
        }
        return ignore_nonexistent;
    }

    QTextStream stream ( &sourceFile );
    stream.setEncoding( QTextStream::UnicodeUTF8 );
    QString code = stream.read();
    sourceFile.close();

    saw_failure = false;
    ignore_errors = false;
    Completion c = interp.evaluate(UString( code ) );

    if ( /*report_result &&*/ !ignore_errors) {
        if (c.complType() == Throw) {
            QString errmsg = c.value().toString(interp.globalExec()).qstring();
            printf( "ERROR: %s (%s)\n",filename.latin1(), errmsg.latin1());
            m_errors++;
            return false;
        }
    }
    return true;
}

class GlobalImp : public ObjectImp {
public:
  virtual UString className() const { return "global"; }
};

RegressionTest::CheckResult RegressionTest::checkOutput(const QString &againstFilename)
{
    QString absFilename = QFileInfo(m_baseDir + "/baseline/" + againstFilename).absFilePath();
    if ( svnIgnored( absFilename ) ) {
        m_known_failures = NoFailure;
        return Ignored;
    }

    CheckResult result = Success;

    // compare result to existing file
    QString outputFilename = QFileInfo(m_outputDir + "/" + againstFilename).absFilePath();
    bool kf = false;
    if ( m_known_failures & AllFailure )
        kf = true;
    if ( kf )
        outputFilename += "-KF";

    if ( m_genOutput )
        outputFilename = absFilename;

    // get existing content
    QString data;
    if (m_outputCustomised) {
        data = m_outputString;
    } else {
        data = m_part->text();
    }

    QFile file(absFilename);
    if (file.open(IO_ReadOnly)) {
        QTextStream stream ( &file );
        stream.setEncoding( QTextStream::UnicodeUTF8 );

        QString fileData = stream.read();

        result = ( fileData == data ) ? Success : Failure;
        if ( !m_genOutput && result == Success && !m_keepOutput ) {
            ::unlink( QFile::encodeName( outputFilename ) );
            return Success;
        }
    } else if (!m_genOutput) {
        fprintf(stderr, "Error reading file %s\n", absFilename.latin1());
        result = Failure;
    }

    // generate result file
    createMissingDirs( outputFilename );
    QFile file2(outputFilename);
    if (!file2.open(IO_WriteOnly)) {
        fprintf(stderr,"Error writing to file %s\n",outputFilename.latin1());
        exit(1);
    }

    QTextStream stream2(&file2);
    stream2.setEncoding( QTextStream::UnicodeUTF8 );
    stream2 << data;
    if ( m_genOutput )
        printf("Generated %s\n", outputFilename.latin1());

    return result;
}

void RegressionTest::rereadConfig()
{
    m_baseConfig->setGroup("Kate Document Defaults");
    m_part->config()->readConfig(m_baseConfig);
    m_baseConfig->setGroup("Kate View Defaults");
    m_view->config()->readConfig(m_baseConfig);
}

bool RegressionTest::reportResult(CheckResult result, const QString & description, bool *newfail)
{
    if ( result == Ignored ) {
        //printf("IGNORED: ");
        //printDescription( description );
        return true; // no error
    } else
        return reportResult( result == Success, description, newfail );
}

bool RegressionTest::reportResult(bool passed, const QString & description, bool *newfail)
{
    if (newfail) *newfail = false;

    if (m_genOutput)
	return true;

    QString filename(m_currentTest + "-" + description);
    if (!m_currentCategory.isEmpty())
        filename = m_currentCategory + "/" + filename;

    const bool oldfailed = m_failureComp && m_failureComp->readNumEntry(filename);
    if (passed) {
        if ( m_known_failures & AllFailure ) {
            printf("PASS (unexpected!)");
            m_passes_fail++;
        } else {
            printf("PASS");
            m_passes_work++;
        }
        if (oldfailed) {
            printf(" (new)");
            m_passes_new++;
        }
        if (m_failureSave)
            m_failureSave->deleteEntry(filename);
    }
    else {
        if ( m_known_failures & AllFailure ) {
            printf("FAIL (known)");
            m_failures_fail++;
            passed = true; // we knew about it
        } else {
            printf("FAIL");
            m_failures_work++;
        }
        if (!oldfailed && m_failureComp) {
            printf(" (new)");
            m_failures_new++;
            if (newfail) *newfail = true;
        }
        if (m_failureSave)
            m_failureSave->writeEntry(filename, 1);
    }
    printf(": ");

    printDescription( description );
    return passed;
}

void RegressionTest::printDescription(const QString& description)
{
    if (!m_currentCategory.isEmpty())
	printf("%s/", m_currentCategory.latin1());

    printf("%s", m_currentTest.latin1());

    if (!description.isEmpty()) {
        QString desc = description;
        desc.replace( '\n', ' ' );
	printf(" [%s]", desc.latin1());
    }

    printf("\n");
    fflush(stdout);
}

void RegressionTest::createMissingDirs(const QString & filename)
{
    QFileInfo dif(filename);
    QFileInfo dirInfo( dif.dirPath() );
    if (dirInfo.exists())
	return;

    QStringList pathComponents;
    QFileInfo parentDir = dirInfo;
    pathComponents.prepend(parentDir.absFilePath());
    while (!parentDir.exists()) {
	QString parentPath = parentDir.absFilePath();
	int slashPos = parentPath.findRev('/');
	if (slashPos < 0)
	    break;
	parentPath = parentPath.left(slashPos);
	pathComponents.prepend(parentPath);
	parentDir = QFileInfo(parentPath);
    }
    for (uint pathno = 1; pathno < pathComponents.count(); pathno++) {
	if (!QFileInfo(pathComponents[pathno]).exists() &&
	    !QDir(pathComponents[pathno-1]).mkdir(pathComponents[pathno])) {
	    fprintf(stderr,"Error creating directory %s\n",pathComponents[pathno].latin1());
	    exit(1);
	}
    }
}

void RegressionTest::slotOpenURL(const KURL &url, const KParts::URLArgs &args)
{
    m_part->browserExtension()->setURLArgs( args );

    m_part->openURL(url);
}

bool RegressionTest::svnIgnored( const QString &filename )
{
    QFileInfo fi( filename );
    QString ignoreFilename = fi.dirPath() + "/svnignore";
    QFile ignoreFile(ignoreFilename);
    if (!ignoreFile.open(IO_ReadOnly))
        return false;

    QTextStream ignoreStream(&ignoreFile);
    QString line;
    while (!(line = ignoreStream.readLine()).isNull()) {
        if ( line == fi.fileName() )
            return true;
    }
    ignoreFile.close();
    return false;
}

void RegressionTest::resizeTopLevelWidget( int w, int h )
{
    qApp->mainWidget()->resize( w, h );
    // Since we're not visible, this doesn't have an immediate effect, QWidget posts the event
    QApplication::sendPostedEvents( 0, QEvent::Resize );
}

#include "test_regression.moc"

// kate: indent-width 4
