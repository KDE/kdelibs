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
#include "ecma/kjs_proxy.h"

using namespace khtml;
using namespace DOM;
using namespace KJS;

// -------------------------------------------------------------------------

PartMonitor::PartMonitor(KHTMLPart *_part)
{
    m_part = _part;
    m_inLoop = false;
    m_completed = false;
    connect(m_part,SIGNAL(completed()),this,SLOT(partCompleted()));
}

void PartMonitor::waitForCompletion()
{
    if (!m_completed) {
        m_inLoop = true;
        kapp->enter_loop();
    }
}

void PartMonitor::partCompleted()
{
    if (!m_inLoop)
        m_completed = true;
    else {
        kapp->exit_loop();
	disconnect(m_part,SIGNAL(completed()),this,SLOT(partCompleted()));
    }
}

// -------------------------------------------------------------------------

RegTestObject::RegTestObject(ExecState *exec, RegressionTest *_regTest)
{
    m_regTest = _regTest;
    put(exec,"print",Object(new RegTestFunction(exec,m_regTest,RegTestFunction::Print,1)), DontEnum);
    put(exec,"reportResult",Object(new RegTestFunction(exec,m_regTest,RegTestFunction::ReportResult,3)), DontEnum);
    put(exec,"checkOutput",Object(new RegTestFunction(exec,m_regTest,RegTestFunction::CheckOutput,1)), DontEnum);
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
	case Print: {
	    UString str = args[0].toString(exec);
	    printf("%s\n",str.ascii());
	    break;
	}
	case ReportResult: {
            bool passed = args[0].toBoolean(exec);
            QString description = args[1].toString(exec).qstring();
            if (args[1].isA(UndefinedType) || args[1].isA(NullType))
                description = "";
            m_regTest->reportResult(passed,description);
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
    put(exec, "openPageAsUrl", Object(new KHTMLPartFunction(exec,m_part,KHTMLPartFunction::OpenPageAsUrl,1)), DontEnum);
    put(exec, "begin",     Object(new KHTMLPartFunction(exec,m_part,KHTMLPartFunction::Begin,1)), DontEnum);
    put(exec, "write",    Object(new KHTMLPartFunction(exec,m_part,KHTMLPartFunction::Write,1)), DontEnum);
    put(exec, "end",    Object(new KHTMLPartFunction(exec,m_part,KHTMLPartFunction::End,0)), DontEnum);
    put(exec, "executeScript", Object(new KHTMLPartFunction(exec,m_part,KHTMLPartFunction::ExecuteScript,0)), DontEnum);
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
	    if (args[0].type() == NullType || args[0].type() == NullType) {
		exec->setException(Error::create(exec, GeneralError,"No filename specified"));
		return Undefined();
	    }

            QString filename = args[0].toString(exec).qstring();
            QString fullFilename = QFileInfo(RegressionTest::curr->m_currentBase+"/"+filename).absFilePath();
            KURL url;
            url.setProtocol("file");
            url.setPath(fullFilename);
            PartMonitor pm(m_part);
            m_part->openURL(url);
            pm.waitForCompletion();
            break;
        }
	case OpenPageAsUrl: {
	    if (args[0].type() == NullType || args[0].type() == UndefinedType) {
		exec->setException(Error::create(exec, GeneralError,"No filename specified"));
		return Undefined();
	    }
	    if (args[1].type() == NullType || args[1].type() == UndefinedType) {
		exec->setException(Error::create(exec, GeneralError,"No url specified"));
		return Undefined();
	    }

            QString filename = args[0].toString(exec).qstring();
            QString url = args[1].toString(exec).qstring();
            QFile file(RegressionTest::curr->m_currentBase+"/"+filename);
	    if (!file.open(IO_ReadOnly)) {
		exec->setException(Error::create(exec, GeneralError,
						 QString("Error reading " + filename).latin1()));
	    }
	    else {
		QByteArray fileData;
		QDataStream stream(fileData,IO_WriteOnly);
		char buf[1024];
		int bytesread;
		while (!file.atEnd()) {
		    bytesread = file.readBlock(buf,1024);
		    stream.writeRawBytes(buf,bytesread);
		}
		file.close();
		QString contents(fileData);
		PartMonitor pm(m_part);
		m_part->begin(url);
		m_part->write(contents);
		m_part->end();
		pm.waitForCompletion();
	    }
	    break;
	}
	case Begin: {
            QString url = args[0].toString(exec).qstring();
            m_part->begin(url);
            break;
        }
        case Write: {
            QString str = args[0].toString(exec).qstring();
            m_part->write(str);
            break;
        }
        case End: {
            m_part->end();
            break;
        }
	case ExecuteScript: {
	    QString code = args[0].toString(exec).qstring();
	    Completion comp;
	    KJSProxy *proxy = m_part->jScript();
	    proxy->evaluate("",0,code,0,&comp);
	    if (comp.complType() == Throw)
		exec->setException(comp.value());
	    break;
	}
    }

    return result;
}

// -------------------------------------------------------------------------

static KCmdLineOptions options[] =
{
  { "g", 0, 0 } ,
  { "genoutput", "Generate output (instead of checking)", 0 } ,
  { "s", 0, 0 } ,
  { "show", "Show the window while running tests", 0 } ,
  { "t", 0, 0 } ,
  { "test <filename>", "Run only a single test", 0 } ,
  { "+source_dir", "Directory containing html files to prcoess", 0 } ,
  { "+output_dir", "Directory for comparison/storage of KHTMLPart internal structure dump", 0 } ,
  {0, 0, 0}
};

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

    QFileInfo outputDir(args->arg(1));
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

    if (args->isSet("show"))
	toplevel->show();

    a.setTopWidget(part->widget());

    // run the tests
    RegressionTest *regressionTest = new RegressionTest(part,args->arg(0),
							args->arg(1),args->isSet("genoutput"));

    bool result;
    if (!args->getOption("test").isNull())
	result = regressionTest->runTests(args->getOption("test"),true);
    else
	result = regressionTest->runTests();
    if (result) {
    if (args->isSet("genoutput")) {
	printf("\nOutput generation completed.\n");
    }
    else {
	printf("\nTests completed.\n");
	printf("Passes:   %d\n",regressionTest->m_passes);
	printf("Failures: %d\n",regressionTest->m_failures);
	printf("Errors:   %d\n",regressionTest->m_errors);
	printf("Total:    %d\n",regressionTest->m_passes+regressionTest->m_failures+regressionTest->m_errors);
    }
    }

    // cleanup
    delete regressionTest;
    delete part;
    delete toplevel;
    delete fac;

    khtml::Cache::clear();
    khtml::CSSStyleSelector::clear();
    khtml::RenderStyle::cleanup();

    // Only return a 0 exit code if all tests were successful
    if (regressionTest->m_failures == 0 && regressionTest->m_errors == 0)
      return 0;
    else
      return 1;
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
    m_passes = 0;
    m_failures = 0;
    m_errors = 0;
    m_currentCategory = "";
    m_currentTest = "";

    curr = this;
};

bool RegressionTest::runTests(QString relPath, bool mustExist)
{
    if (!QFile(m_sourceFilesDir+"/"+relPath).exists()) {
	fprintf(stderr,"%s: No such file or directory\n",relPath.latin1());
	return false;
    }

    QString fullPath = m_sourceFilesDir+"/"+relPath;
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

	QDir sourceDir(m_sourceFilesDir+"/"+relPath);
	for (uint fileno = 0; fileno < sourceDir.count(); fileno++) {
	    QString filename = sourceDir[fileno];
	    QString relFilename = relPath == "" ? filename : relPath+"/"+filename;
	    if (filename != "." && filename != "..")
		runTests(relFilename,false);
	}

    }
    else if (info.isFile()) {
	QString relativeDir = QFileInfo(relPath).dirPath();
	QString filename = info.fileName();
	m_currentBase = m_sourceFilesDir+"/"+relativeDir;
	m_currentCategory = relativeDir;
	m_currentTest = filename;
	if (filename.endsWith(".html") && !filename.endsWith("-js.html")){
	    testStaticFile(relPath);
	}
	else if (filename.endsWith(".js")) {
	    testJSFile(relPath);
	}
	else if (mustExist) {
	    fprintf(stderr,"%s: Not a valid test file (must be .html or .js)\n",relPath.latin1());
	    return false;
	}

    }
    else {
	if (mustExist) {
	    fprintf(stderr,"%s: Not a regular file\n",relPath.latin1());
	    return false;
	}
    }

    return true;
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
    PartMonitor pm(m_part);
    m_part->openURL(url);
    pm.waitForCompletion();

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
        exit(1);
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

    Completion c = interp.evaluate(code.latin1());

    if (c.complType() == Throw) {
	QString errmsg = c.value().toString(interp.globalExec()).qstring();
	printf("ERROR: %s (%s)\n",filename.latin1(),errmsg.latin1());
	m_errors++;
    }
}

bool RegressionTest::checkOutput(QString againstFilename, QByteArray data)
{
    QString absFilename = QFileInfo(m_outputFilesDir+"/"+againstFilename).absFilePath();
    if (!m_genOutput) {
	// compare result to existing file

	QFile file(absFilename);
	if (!file.open(IO_ReadOnly)) {
	    fprintf(stderr,"Error reading file %s\n",absFilename.latin1());
	    exit(1);
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
	QFileInfo info(absFilename);
	createMissingDirs(info.dirPath());
	QFile file(absFilename);
	if (!file.open(IO_WriteOnly)) {
	    fprintf(stderr,"Error writing to file %s\n",absFilename.latin1());
	    exit(1);
	}

	QDataStream fileOut(&file);
	fileOut.writeRawBytes(data.data(),data.size());
	file.close();

	printf("Generated %s\n",againstFilename.latin1());

	return true;
    }
}

void RegressionTest::reportResult(bool passed, QString description)
{
    if (m_genOutput)
	return;

    if (passed) {
	printf("PASS: ");
	m_passes++;
    }
    else {
	printf("FAIL: ");
	m_failures++;
    }

    if (m_currentCategory != "")
	printf("%s ",m_currentCategory.latin1());

    printf("[%s]",m_currentTest.latin1());

    if (description != "")
	printf(" %s",description.latin1());

    printf("\n");
}

void RegressionTest::createMissingDirs(QString path)
{
    QFileInfo dirInfo(path);
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

#include "test_regression.moc"
