/**
 * This file is part of the KDE project
 *
 * (C) 2001,2003 Peter Kelly (pmk@post.com)
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */

#include <stdlib.h>
#include <kapplication.h>
#include <qfile.h>
// to be able to delete a static protected member pointer in kbrowser...
// just for memory debugging
#define protected public
#undef protected

#include "test_regression.h"

#include <stdio.h>

#include "css/cssstyleselector.h"
#include <dom_string.h>
#include "rendering/render_style.h"

#include <kaction.h>
#include <kcmdlineargs.h>
#include <khtml_factory.h>
#include <kio/job.h>
#include <kmainwindow.h>

#include <qcolor.h>
#include <qcursor.h>
#include <qdir.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qscrollview.h>
#include <qstring.h>
#include <qtextstream.h>
#include <qvaluelist.h>
#include <qwidget.h>
#include <qfileinfo.h>

#include "decoder.h"
#include "dom/dom2_range.h"
#include "dom/dom_exception.h"
#include "dom/html_document.h"
#include "html_document.h"
#include "htmltokenizer.h"
#include "khtml_part.h"
#include "khtmlview.h"
#include "rendering/render_object.h"
#include "xml/dom_docimpl.h"
#include "dom/dom_doc.h"
#include "misc/loader.h"
#include "ecma/kjs_dom.h"
#include "ecma/kjs_window.h"
#include "ecma/kjs_binding.h"

using namespace khtml;
using namespace DOM;
using namespace KJS;

// -------------------------------------------------------------------------

void PageLoader::loadPage(KHTMLPart *part, KURL url)
{
    PageLoader *pl = new PageLoader();

    pl->m_started = false;
    pl->m_completed = false;

    connect(part,SIGNAL(completed()),pl,SLOT(partCompleted()));
    part->openURL(url);
    if (!pl->m_completed) {
	pl->m_started = true;
	kapp->enter_loop();
    }
}

void PageLoader::partCompleted()
{
    if (!m_started)
	m_completed = true;
    else {
	kapp->exit_loop();
	delete this;
    }
}

// -------------------------------------------------------------------------

RegTestObject::RegTestObject(ExecState *exec, RegressionTest *_regTest)
{
    m_regTest = _regTest;
    put(exec, "reportResult", Object(new RegTestFunction(exec,m_regTest,RegTestFunction::ReportResult,3)), DontEnum);
}

RegTestFunction::RegTestFunction(ExecState *exec, RegressionTest *_regTest, int _id, int length)
{
    m_regTest = _regTest;
    id = _id;
    put(exec,"length",Number(length));
}

bool RegTestFunction::implementsCall() const
{
    return true;
}

Value RegTestFunction::call(ExecState *exec, Object &/*thisObj*/, const List &args)
{
    Value result = Undefined();
    switch (id) {
	case ReportResult: {
            bool passed = args[0].toBoolean(exec);
            QString testname = args[1].toString(exec).qstring();
            if (args[1].isA(UndefinedType) || args[1].isA(NullType))
                testname = "";
            QString description = args[2].toString(exec).qstring();
            if (args[2].isA(UndefinedType) || args[2].isA(NullType))
                description = "";
            m_regTest->reportResult(passed,testname,description);
            break;
        }
	case CheckOutput: {
            QByteArray dumpData = m_regTest->getPartOutput();
            QString filename = args[0].toString(exec).qstring();
            result = Boolean(m_regTest->checkOutput(filename+".dump",dumpData));
            break;
        }
    }

    return result;
}

// -------------------------------------------------------------------------

KHTMLPartObject::KHTMLPartObject(ExecState *exec, KHTMLPart *_part)
{
    m_part = _part;
    put(exec, "openPage", Object(new KHTMLPartFunction(exec,m_part,KHTMLPartFunction::OpenPage,1)), DontEnum);
    put(exec, "open",     Object(new KHTMLPartFunction(exec,m_part,KHTMLPartFunction::Open,1)), DontEnum);
    put(exec, "write",    Object(new KHTMLPartFunction(exec,m_part,KHTMLPartFunction::Write,1)), DontEnum);
    put(exec, "close",    Object(new KHTMLPartFunction(exec,m_part,KHTMLPartFunction::Close,0)), DontEnum);
}

Value KHTMLPartObject::get(ExecState *exec, const UString &propertyName) const
{
    if (propertyName == "document")
        return getDOMNode(exec,m_part->document());
    else if (propertyName == "window")
        return Object(Window::retrieveWindow(m_part));
    else
        return ObjectImp::get(exec,propertyName);
}

KHTMLPartFunction::KHTMLPartFunction(ExecState *exec, KHTMLPart *_part, int _id, int length)
{
    m_part = _part;
    id = _id;
    put(exec,"length",Number(length));
}

bool KHTMLPartFunction::implementsCall() const
{
    return true;
}

Value KHTMLPartFunction::call(ExecState *exec, Object &/*thisObj*/, const List &args)
{
    Value result = Undefined();

    switch (id) {
        case OpenPage: {
            QString filename = args[0].toString(exec).qstring();
            QString fullFilename = QFileInfo(RegressionTest::curr->m_currentBase+"/"+filename).absFilePath();
            KURL url;
            url.setProtocol("file");
            url.setPath(fullFilename);
            PageLoader::loadPage(m_part,url);
        }
            break;
        case Open:
            break;
        case Write:
            break;
        case Close:
            break;
    }

    return result;
}

// -------------------------------------------------------------------------

static KCmdLineOptions options[] = {
				     { "genoutput", "Generate output (instead of checking)", 0 } ,
				     { "+source_dir", "Directory containing html files to prcoess", 0 } ,
				     { "+output_dir", "Directory for comparison/storage of KHTMLPart internal structure dump", 0 } ,
				     {0, 0, 0}, };

int main(int argc, char *argv[])
{
    KCmdLineArgs::init(argc, argv, "test_regression", "Regression tester for khtml", "1.0");
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication a;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs( );
    if ( args->count() < 2 ) {
	KCmdLineArgs::usage();
	::exit( 1 );
    }

    QFileInfo sourceDir(args->arg(0));
    if (!sourceDir.exists() || !sourceDir.isDir() || !sourceDir.isDir()) {
	fprintf(stderr,"ERROR: Source directory \"%s\": no such directory.\n",args->arg(0));
	exit(1);
    }

    QFileInfo outputDir(args->arg(0));
    if (!outputDir.exists() || !outputDir.isDir() || !outputDir.isDir()) {
	fprintf(stderr,"ERROR: Output directory \"%s\": no such directory.\n",args->arg(1));
	exit(1);
    }

    // create widgets
    KHTMLFactory *fac = new KHTMLFactory();
    KMainWindow *toplevel = new KMainWindow();
    KHTMLPart *part = new KHTMLPart( toplevel, 0, toplevel, 0, KHTMLPart::BrowserViewGUI );

    toplevel->setCentralWidget( part->widget() );
    toplevel->resize( 800, 600);
    part->setJScriptEnabled(true);
    part->executeScript(""); // force the part to create an interpreter
//    part->setJavaEnabled(true);

    a.setTopWidget(part->widget());

    // run the tests
    RegressionTest *regressionTest = new RegressionTest(part,args->arg(0),
							args->arg(1),args->isSet("genoutput"));
    regressionTest->runTests();

    if (args->isSet("genoutput")) {
	printf("\nOutput generation completed.\n");
    }
    else {
	printf("\nTests completed.\n");
	printf("Passed:   %d\n",regressionTest->m_totalPassed);
	printf("Failed:   %d\n",regressionTest->m_totalFailed);
	printf("Total:    %d\n",regressionTest->m_totalPassed+regressionTest->m_totalFailed);
    }

    // cleanup
    delete regressionTest;
    delete part;
    delete toplevel;
    delete fac;

    khtml::Cache::clear();
    khtml::CSSStyleSelector::clear();
    khtml::RenderStyle::cleanup();

    return 0;
}

// -------------------------------------------------------------------------

RegressionTest *RegressionTest::curr = 0;

RegressionTest::RegressionTest(KHTMLPart *part, QString _sourceFilesDir,
			       QString _outputFilesDir, bool _genOutput)
  : QObject(part)
{
    m_part = part;
    m_sourceFilesDir = _sourceFilesDir;
    m_outputFilesDir = _outputFilesDir;
    m_genOutput = _genOutput;
    m_currentBase = "";
    m_totalPassed = 0;
    m_totalFailed = 0;
    m_currentCategory = "";
    m_currentTest = "";

    curr = this;
};

void RegressionTest::runTests(QString relDir)
{
    QDir sourceDir(m_sourceFilesDir+"/"+relDir);
    for (uint fileno = 0; fileno < sourceDir.count(); fileno++) {
	QString filename = sourceDir[fileno];

	QString relFilename;
	if (relDir == "")
	    relFilename = filename;
	else
	    relFilename = relDir+"/"+filename;

	m_currentBase = m_sourceFilesDir+"/"+relDir;
	m_currentCategory = relDir;
	m_currentTest = filename;
	if (filename != "." && filename != ".." &&
            QFileInfo(m_sourceFilesDir+"/"+relFilename).isDir()) {
	    runTests(relFilename);
	}
	if (filename.endsWith(".html") && !filename.endsWith("-js.html")) {
	    testStaticFile(relFilename);
	}
	else if (filename.endsWith(".js")) {
	    testJSFile(relFilename);
	}
    }
}

QByteArray RegressionTest::getPartOutput()
{
    // dump out the contents of the rendering & DOM trees
    QByteArray dumpData;
    QTextStream outputStream(dumpData,IO_WriteOnly);

    outputStream << "Rendering Tree" << endl;
    outputStream << "--------------" << endl;
    m_part->document().handle()->renderer()->dump(&outputStream);
    outputStream << "DOM Tree" << endl;
    outputStream << "--------" << endl;
    m_part->document().handle()->dump(&outputStream);

    return dumpData;
}

void RegressionTest::testStaticFile(QString filename)
{
    // load page
    KURL url;
    url.setProtocol("file");
    url.setPath(QFileInfo(m_sourceFilesDir+"/"+filename).absFilePath());
    PageLoader::loadPage(m_part,url);

    // compare with (or generate) output file
    QByteArray dumpData = getPartOutput();
    reportResult(checkOutput(filename+".dump",dumpData));
}

void RegressionTest::testJSFile(QString filename)
{
    // create interpreter
    // note: this is different from the interpreter used by the part,
    // it contains regression test-specific objects & functions
    Object global(new ObjectImp());
    ScriptInterpreter interp(global,m_part);
    ExecState *exec = interp.globalExec();

    global.put(exec, "part", Object(new KHTMLPartObject(exec,m_part)));
    global.put(exec, "regtest", Object(new RegTestObject(exec,this)));

    QString fullSourceName = m_sourceFilesDir+"/"+filename;
    QFile sourceFile(fullSourceName);

    if (!sourceFile.open(IO_ReadOnly)) {
        fprintf(stderr,"Error reading file %s\n",fullSourceName.latin1());
        exit(-1);
    }

    QByteArray fileData;
    QDataStream stream(fileData,IO_WriteOnly);

    char buf[1024];
    int bytesread;

    while (!sourceFile.atEnd()) {
	bytesread = sourceFile.readBlock(buf,1024);
	stream.writeRawBytes(buf,bytesread);
    }

    sourceFile.close();
    QString code(fileData);

    interp.evaluate(code.latin1());
}

bool RegressionTest::checkOutput(QString againstFilename, QByteArray data)
{
    QString absFilename = QFileInfo(m_outputFilesDir+"/"+againstFilename).absFilePath();
    if (!m_genOutput) {
	// compare result to existing file

	QFile file(absFilename);
	if (!file.open(IO_ReadOnly)) {
	    fprintf(stderr,"Error reading file %s\n",absFilename.latin1());
	    exit(-1);
	}

	QByteArray fileData;
	QTextStream stream(fileData,IO_WriteOnly);

	char buf[1024];
	int bytesread;
	while (!file.atEnd()) {
	    bytesread = file.readBlock(buf,1024);
	    stream.writeRawBytes(buf,bytesread);
	}

	file.close();

	return (fileData == data);
    }
    else {
	// generate result file

	QDir dir(m_outputFilesDir);
	QString absParentDir = absFilename.mid(0,absFilename.findRev('/'));
	dir.mkdir(absParentDir,true);

	QFile file(absFilename);
	if (!file.open(IO_WriteOnly)) {
	    fprintf(stderr,"Error writing to file %s\n",absFilename.latin1());
	    exit(-1);
	}

	QDataStream fileOut(&file);
	fileOut.writeRawBytes(data.data(),data.size());
	file.close();

	return true;
    }
}

void RegressionTest::reportResult(bool passed, QString testname, QString description)
{
    if (m_genOutput)
	return;

    if (passed) {
	printf("PASS: ");
	m_totalPassed++;
    }
    else {
	printf("FAIL: ");
	m_totalFailed++;
    }

    if (m_currentCategory != "")
	printf("%s ",m_currentCategory.latin1());

    printf("[%s]",m_currentTest.latin1());

    if (testname != "")
	printf(" %s",testname.latin1());

    if (description != "")
	printf(" (%s)",description.latin1());

    printf("\n");
}

#include "test_regression.moc"
