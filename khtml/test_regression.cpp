/**
 * This file is part of the KDE project
 *
 * Copyright (C) 2001,2003 Peter Kelly (pmk@post.com)
 * Copyright (C) 2003 Stephan Kulow (coolo@kde.org)
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
 */

#include <stdlib.h>
#include <kapplication.h>
#include <qfile.h>
// to be able to delete a static protected member pointer in kbrowser...
// just for memory debugging
#define protected public
#undef protected

#include "test_regression.h"
#include <unistd.h>
#include <stdio.h>

#include "css/cssstyleselector.h"
#include <dom_string.h>
#include "rendering/render_style.h"
#include "rendering/render_layer.h"
#include "khtmldefaults.h"

#include <kaction.h>
#include <kcmdlineargs.h>
#include "khtml_factory.h"
#include <kio/job.h>
#include <kmainwindow.h>
#include <ksimpleconfig.h>
#include <kglobalsettings.h>

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
#include <qtimer.h>

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
#include "ecma/kjs_binding.h"
#include "ecma/kjs_dom.h"
#include "ecma/kjs_window.h"
#include "ecma/kjs_binding.h"
#include "ecma/kjs_proxy.h"

using namespace khtml;
using namespace DOM;
using namespace KJS;

bool visual = false;

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

void PartMonitor::timeout()
{
    kapp->exit_loop();
}

void PartMonitor::partCompleted()
{
    if (!m_inLoop)
        m_completed = true;
    else {
	disconnect(m_part,SIGNAL(completed()),this,SLOT(partCompleted()));
        if ( visual )
            QTimer::singleShot( 100, this, SLOT( timeout() ) );
        else
            timeout();
    }
}

// -------------------------------------------------------------------------

RegTestObject::RegTestObject(ExecState *exec, RegressionTest *_regTest)
{
    m_regTest = _regTest;
    putDirect("print",new RegTestFunction(exec,m_regTest,RegTestFunction::Print,1), DontEnum);
    putDirect("reportResult",new RegTestFunction(exec,m_regTest,RegTestFunction::ReportResult,3), DontEnum);
    putDirect("checkOutput",new RegTestFunction(exec,m_regTest,RegTestFunction::CheckOutput,1), DontEnum);
    // add "quit" for compatibility with the mozilla js shell
    putDirect("quit", new RegTestFunction(exec,m_regTest,RegTestFunction::Quit,1), DontEnum );
}

RegTestFunction::RegTestFunction(ExecState */*exec*/, RegressionTest *_regTest, int _id, int length)
{
    m_regTest = _regTest;
    id = _id;
    putDirect("length",length);
}

bool RegTestFunction::implementsCall() const
{
    return true;
}

Value RegTestFunction::call(ExecState *exec, Object &/*thisObj*/, const List &args)
{
    Value result = Undefined();
    if ( m_regTest->ignore_errors )
        return result;

    switch (id) {
	case Print: {
	    UString str = args[0].toString(exec);
            if ( str.qstring().lower().find( "failed!" ) >= 0 )
                m_regTest->saw_failure = true;
	    fprintf(stderr, "%s\n",str.qstring().latin1());
	    break;
	}
	case ReportResult: {
            bool passed = args[0].toBoolean(exec);
            QString description = args[1].toString(exec).qstring();
            if (args[1].isA(UndefinedType) || args[1].isA(NullType))
                description = QString::null;
            m_regTest->reportResult(passed,description);
            break;
        }
	case CheckOutput: {
            DOM::DocumentImpl* docimpl = static_cast<DOM::DocumentImpl*>( m_regTest->m_part->document().handle() );
            if ( docimpl && docimpl->view() && docimpl->renderer() )
            {
                docimpl->updateRendering();
                docimpl->view()->layout();
            }
            QString filename = args[0].toString(exec).qstring();
	    filename = RegressionTest::curr->m_currentCategory+"/"+filename;
            if ( m_regTest->m_genOutput ) {
                m_regTest->reportResult( m_regTest->checkOutput(filename+"-dom"),
                                         "Script-generated " + filename + "-dom");
                m_regTest->reportResult( m_regTest->checkOutput(filename+"-render"),
                                         "Script-generated " + filename + "-render");
            } else {
                // compare with output file
                if ( m_regTest->reportResult( m_regTest->checkOutput(filename+"-dom") ) )
                    m_regTest->reportResult( m_regTest->checkOutput(filename+"-render"));
            }
            break;
        }
        case Quit:
            m_regTest->reportResult(true,
				    "Called quit" );
            if ( !m_regTest->saw_failure )
                m_regTest->ignore_errors = true;
            break;
    }

    return result;
}

// -------------------------------------------------------------------------

KHTMLPartObject::KHTMLPartObject(ExecState *exec, KHTMLPart *_part)
{
    m_part = _part;
    putDirect("openPage", new KHTMLPartFunction(exec,m_part,KHTMLPartFunction::OpenPage,1), DontEnum);
    putDirect("openPageAsUrl", new KHTMLPartFunction(exec,m_part,KHTMLPartFunction::OpenPageAsUrl,1), DontEnum);
    putDirect("begin",     new KHTMLPartFunction(exec,m_part,KHTMLPartFunction::Begin,1), DontEnum);
    putDirect("write",    new KHTMLPartFunction(exec,m_part,KHTMLPartFunction::Write,1), DontEnum);
    putDirect("end",    new KHTMLPartFunction(exec,m_part,KHTMLPartFunction::End,0), DontEnum);
    putDirect("executeScript", new KHTMLPartFunction(exec,m_part,KHTMLPartFunction::ExecuteScript,0), DontEnum);
    putDirect("processEvents", new KHTMLPartFunction(exec,m_part,KHTMLPartFunction::ProcessEvents,0), DontEnum);
}

Value KHTMLPartObject::get(ExecState *exec, const Identifier &propertyName) const
{
    if (propertyName == "document")
        return getDOMNode(exec,m_part->document());
    else if (propertyName == "window")
        return Object(KJS::Window::retrieveWindow(m_part));
    else
        return ObjectImp::get(exec,propertyName);
}

KHTMLPartFunction::KHTMLPartFunction(ExecState */*exec*/, KHTMLPart *_part, int _id, int length)
{
    m_part = _part;
    id = _id;
    putDirect("length",length);
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
	    kapp->processEvents(60000);
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
	    kapp->processEvents(60000);
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
	    kapp->processEvents(60000);
            break;
        }
	case ExecuteScript: {
	    QString code = args[0].toString(exec).qstring();
	    Completion comp;
	    KJSProxy *proxy = m_part->jScript();
	    proxy->evaluate("",0,code,0,&comp);
	    if (comp.complType() == Throw)
		exec->setException(comp.value());
	    kapp->processEvents(60000);
	    break;
	}
	case ProcessEvents: {
	    kapp->processEvents(60000);
	    break;
	}
    }

    return result;
}

// -------------------------------------------------------------------------

static KCmdLineOptions options[] =
{
  { "g", 0, 0 } ,
  { "genoutput", "Regenerate baseline (instead of checking)", 0 } ,
  { "s", 0, 0 } ,
  { "show", "Show the window while running tests", 0 } ,
  { "t", 0, 0 } ,
  { "test <filename>", "Run only a single test", 0 } ,
  { "+base_dir", "Directory containing tests,basedir and output directories", 0 } ,
  KCmdLineLastOption
};

int main(int argc, char *argv[])
{
    // forget about any settings
    setenv( "KDEHOME", "/var/tmp/non_existant", 1 );
    setenv( "LC_ALL", "C", 1 );
    setenv( "LANG", "C", 1 );

    KCmdLineArgs::init(argc, argv, "test_regression", "TestRegression",
                       "Regression tester for khtml", "1.0");
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication a;
    a.setStyle( "windows" );
    KSimpleConfig sc1( "cryptodefaults" );
    sc1.setGroup( "Warnings" );
    sc1.writeEntry( "OnUnencrypted",  false );
    a.config()->setGroup( "Notification Messages" );
    a.config()->writeEntry( "kjscupguard_alarmhandler", true );
    a.config()->setGroup("HTML Settings");
    a.config()->writeEntry("ReportJSErrors", false);
    KConfig cfg( "khtmlrc" );
    cfg.setGroup("HTML Settings");
    cfg.writeEntry( "StandardFont", KGlobalSettings::generalFont().family() );
    cfg.writeEntry( "FixedFont", KGlobalSettings::fixedFont().family() );
    cfg.writeEntry( "SerifFont", HTML_DEFAULT_VIEW_SERIF_FONT );
    cfg.writeEntry( "SansSerifFont", HTML_DEFAULT_VIEW_SANSSERIF_FONT );
    cfg.writeEntry( "CursiveFont", HTML_DEFAULT_VIEW_CURSIVE_FONT );
    cfg.writeEntry( "FantasyFont", HTML_DEFAULT_VIEW_FANTASY_FONT );
    cfg.writeEntry( "MinimumFontSize", HTML_DEFAULT_MIN_FONT_SIZE );
    cfg.writeEntry( "MediumFontSize", 10 );
    cfg.writeEntry( "Fonts", QStringList() );
    cfg.writeEntry( "DefaultEncoding", "" );
    cfg.sync();

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs( );
    int rv = 1;

    if ( args->count() < 1 ) {
	KCmdLineArgs::usage();
	::exit( 1 );
    }

    const char *subdirs[] = {"tests", "baseline", "output"};
    for ( int i = 0; i < 3; i++ ) {
        QFileInfo sourceDir(QFile::encodeName( args->arg(0) ) + "/" + subdirs[i]);
        if ( !sourceDir.exists() || !sourceDir.isDir() ) {
            fprintf(stderr,"ERROR: Source directory \"%s/%s\": no such directory.\n",args->arg(0), subdirs[i]);
            exit(1);
        }
    }


    // create widgets
    KHTMLFactory *fac = new KHTMLFactory();
    KMainWindow *toplevel = new KMainWindow();
    KHTMLPart *part = new KHTMLPart( toplevel, 0, toplevel, 0, KHTMLPart::BrowserViewGUI );

    toplevel->setCentralWidget( part->widget() );
    part->setJScriptEnabled(true);

    part->executeScript(DOM::Node(), ""); // force the part to create an interpreter
//    part->setJavaEnabled(true);

    if (args->isSet("show"))
	visual = true;

    a.setTopWidget(part->widget());
    a.setMainWidget( toplevel );
    if ( visual )
        toplevel->show();

    // run the tests
    RegressionTest *regressionTest = new RegressionTest(part,
                                                        args->arg(0),
                                                        args->isSet("genoutput"));
    QObject::connect(part->browserExtension(), SIGNAL(openURLRequest(const KURL &, const KParts::URLArgs &)),
		     regressionTest, SLOT(slotOpenURL(const KURL&, const KParts::URLArgs &)));

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
            printf("Total:    %d\n",regressionTest->m_passes+regressionTest->m_failures+regressionTest->m_errors);
	    printf("Passes:   %d\n",regressionTest->m_passes);
	    printf("Failures: %d\n",regressionTest->m_failures);
	    printf("Errors:   %d\n",regressionTest->m_errors);
	}
    }

    // Only return a 0 exit code if all tests were successful
    if (regressionTest->m_failures == 0 && regressionTest->m_errors == 0)
	rv = 0;

    // cleanup
    delete regressionTest;
    delete part;
    delete toplevel;
    delete fac;

    khtml::Cache::clear();
    khtml::CSSStyleSelector::clear();
    khtml::RenderStyle::cleanup();

    return rv;
}

// -------------------------------------------------------------------------

RegressionTest *RegressionTest::curr = 0;

RegressionTest::RegressionTest(KHTMLPart *part, const QString &baseDir,
			       bool _genOutput)
  : QObject(part)
{
    m_part = part;
    m_baseDir = baseDir;
    m_baseDir = m_baseDir.replace( "//", "/" );
    if ( m_baseDir.endsWith( "/" ) )
        m_baseDir = m_baseDir.left( m_baseDir.length() - 1 );
    m_genOutput = _genOutput;
    m_passes = 0;
    m_failures = 0;
    m_errors = 0;

    curr = this;
}

#include <qobjectlist.h>

bool RegressionTest::runTests(QString relPath, bool mustExist)
{
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

	// Read ignore file for this directory
	QString ignoreFilename = m_baseDir + "/tests/"+relPath+"/ignore";
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

	// Run each test in this directory, recusively
	QDir sourceDir(m_baseDir + "/tests/"+relPath);
	for (uint fileno = 0; fileno < sourceDir.count(); fileno++) {
	    QString filename = sourceDir[fileno];
	    QString relFilename = relPath.isEmpty() ? filename : relPath+"/"+filename;
	    if (filename != "." && filename != ".." && !ignoreFiles.contains(filename))
		runTests(relFilename,false);
	}
    }
    else if (info.isFile()) {
	QString relativeDir = QFileInfo(relPath).dirPath();
	QString filename = info.fileName();
	m_currentBase = m_baseDir + "/tests/"+relativeDir;
	m_currentCategory = relativeDir;
	m_currentTest = filename;
	if (filename.endsWith(".html") || filename.endsWith( ".htm" )) {
	    testStaticFile(relPath);
	}
	else if (filename.endsWith(".js")) {
	    testJSFile(relPath);
	}
	else if (mustExist) {
	    fprintf(stderr,"%s: Not a valid test file (must be .htm(l) or .js)\n",relPath.latin1());
	    return false;
	}

#if 0
	PartMonitor pm(m_part);
        m_part->closeURL();
        kapp->processEvents(60000);
        qApp->mainWidget()->resize( 800, 600); // restore size
        m_part->begin(KURL());
        m_part->write("<html><body></body></html>");
        m_part->end();
        pm.waitForCompletion();
        RenderObject *r = static_cast<DocumentImpl*>( m_part->document().handle() )->renderer();

        QObjectList *l = qApp->mainWidget()->queryList( "QWidget" );
        QObjectListIt it( *l ); // iterate over the buttons
        QObject *obj;

        while ( (obj = it.current()) != 0 ) {
            // for each found object...
            ++it;
            kdDebug() << ( QWidget* )*it << endl;
        }
        delete l; // delete the list, not the objects

        if ( r->contentHeight() != 594 || r->contentWidth() != 796 )
            printf( "ERROR renderer size %d %d\n", r->contentHeight(), r->contentWidth() );
#endif
    }

    else {
	if (mustExist) {
	    fprintf(stderr,"%s: Not a regular file\n",relPath.latin1());
	    return false;
	}
    }

    return true;
}

void RegressionTest::getPartDOMOutput( QTextStream &outputStream )
{
    Node node = m_part->document();
    uint indent = 0;
    while (!node.isNull()) {
	// process

	for (uint i = 0; i < indent; i++)
	    outputStream << "  ";
	outputStream << node.nodeName().string();

	switch (node.nodeType()) {
	    case Node::ELEMENT_NODE: {
		// Sort strings to ensure consistent output
		QStringList attrNames;
		NamedNodeMap attrs = node.attributes();
		for (uint a = 0; a < attrs.length(); a++)
		    attrNames.append(attrs.item(a).nodeName().string());
		attrNames.sort();

		QStringList::iterator it;
		Element elem(node);
		for (it = attrNames.begin(); it != attrNames.end(); ++it) {
		    QString name = *it;
		    QString value = elem.getAttribute(*it).string();
		    outputStream << " " << name << "=\"" << value << "\"";
		}
		break;
	    }
	    case Node::ATTRIBUTE_NODE:
		// Should not be present in tree
		assert(false);
		break;
            case Node::TEXT_NODE:
		outputStream << " \"" << Text(node).data().string() << "\"";
		break;
            case Node::CDATA_SECTION_NODE:
		outputStream << " \"" << CDATASection(node).data().string() << "\"";
		break;
            case Node::ENTITY_REFERENCE_NODE:
		break;
            case Node::ENTITY_NODE:
		break;
            case Node::PROCESSING_INSTRUCTION_NODE:
		break;
            case Node::COMMENT_NODE:
		outputStream << " \"" << Comment(node).data().string() << "\"";
		break;
            case Node::DOCUMENT_NODE:
		break;
            case Node::DOCUMENT_TYPE_NODE:
		break;
            case Node::DOCUMENT_FRAGMENT_NODE:
		// Should not be present in tree
		assert(false);
		break;
            case Node::NOTATION_NODE:
		break;
            default:
		assert(false);
		break;
	}

	outputStream << endl;

	if (!node.firstChild().isNull()) {
	    node = node.firstChild();
	    indent++;
	}
	else if (!node.nextSibling().isNull()) {
	    node = node.nextSibling();
	}
	else {
	    while (!node.isNull() && node.nextSibling().isNull()) {
		node = node.parentNode();
		indent--;
	    }
	    if (!node.isNull())
		node = node.nextSibling();
	}
    }
}

QString RegressionTest::getPartOutput( OutputType type)
{
    // dump out the contents of the rendering & DOM trees
    QString dump;
    QTextStream outputStream(dump,IO_WriteOnly);

    if ( type == RenderTree ) {
        static_cast<DocumentImpl*>( m_part->document().handle() )->renderer()->layer()->dump( outputStream );
    } else {
        assert( type == DOMTree );
        getPartDOMOutput( outputStream );
    }

    dump.replace( m_baseDir + "/tests", QString::fromLatin1( "REGRESSION_SRCDIR" ) );
    return dump;
}

void RegressionTest::testStaticFile(const QString & filename)
{
    qApp->mainWidget()->resize( 800, 600); // restore size

    // load page
    KURL url;
    url.setProtocol("file");
    url.setPath(QFileInfo(m_baseDir + "/tests/"+filename).absFilePath());
    PartMonitor pm(m_part);
    m_part->openURL(url);
    pm.waitForCompletion();
    m_part->closeURL();

    if ( m_genOutput ) {
        reportResult( checkOutput(filename+"-dom") );
        reportResult( checkOutput(filename+"-render") );
    } else {
        // compare with output file
        if ( reportResult( checkOutput(filename+"-dom") ) )
            reportResult(checkOutput(filename+"-render"));
    }
}

void RegressionTest::evalJS( ScriptInterpreter &interp, const QString &filename, bool report_result )
{
    qDebug("eval %s", filename.latin1());
    QString fullSourceName = filename;
    QFile sourceFile(fullSourceName);

    if (!sourceFile.open(IO_ReadOnly)) {
        fprintf(stderr,"Error reading file %s\n",fullSourceName.latin1());
        exit(1);
    }

    QTextStream stream ( &sourceFile );
    stream.setEncoding( QTextStream::UnicodeUTF8 );
    QString code = stream.read();
    sourceFile.close();

    saw_failure = false;
    ignore_errors = false;
    Completion c = interp.evaluate(UString( code ) );

    if ( report_result && !ignore_errors) {
        if (c.complType() == Throw) {
            QString errmsg = c.value().toString(interp.globalExec()).qstring();
            if ( !filename.endsWith( "-n.js" ) ) {
                printf( "ERROR: %s (%s)\n",filename.latin1(),errmsg.latin1());
                m_errors++;
            } else {
                reportResult( true, QString( "Expected Failure: %1" ).arg( errmsg ) );
            }
        } else if ( saw_failure ) {
            reportResult( filename.endsWith( "-n.js" ), "saw 'failed!'" );
        } else {
            reportResult( !filename.endsWith( "-n.js" ), "passed" );
        }
    }
}

class GlobalImp : public ObjectImp {
public:
  virtual UString className() const { return "global"; }
};

void RegressionTest::testJSFile(const QString & filename)
{
    qApp->mainWidget()->resize( 800, 600); // restore size

    // create interpreter
    // note: this is different from the interpreter used by the part,
    // it contains regression test-specific objects & functions
    Object global(new GlobalImp());
    ScriptInterpreter interp(global,m_part);
    ExecState *exec = interp.globalExec();

    global.put(exec, "part", Object(new KHTMLPartObject(exec,m_part)));
    global.put(exec, "regtest", Object(new RegTestObject(exec,this)));
    global.put(exec, "debug", Object(new RegTestFunction(exec,this,RegTestFunction::Print,1) ) );
    global.put(exec, "print", Object(new RegTestFunction(exec,this,RegTestFunction::Print,1) ) );

    QStringList dirs = QStringList::split( '/', filename );
    // NOTE: the basename is of little interest here, but the last basedir change
    // isn't taken in account
    QString basedir =  m_baseDir + "/tests/";
    for ( QStringList::ConstIterator it = dirs.begin(); it != dirs.end(); ++it )
    {
        if ( ! ::access( QFile::encodeName( basedir + "shell.js" ), R_OK ) )
            evalJS( interp, basedir + "shell.js", false );
        basedir += *it + "/";
    }
    evalJS( interp, m_baseDir + "/tests/"+ filename, true );
}

bool RegressionTest::checkOutput(const QString &againstFilename)
{
    QString absFilename = QFileInfo(m_baseDir + "/baseline/" + againstFilename).absFilePath();
    if ( cvsIgnored( absFilename ) )
        return true;

    QString data = getPartOutput( againstFilename.endsWith( "-dom" ) ? DOMTree : RenderTree );

    bool result = true;

    // compare result to existing file

    QString outputFilename = QFileInfo(m_baseDir + "/output/" + againstFilename).absFilePath();

    if ( m_genOutput )
        outputFilename = absFilename;

    QFile file(absFilename);
    if (file.open(IO_ReadOnly)) {
        QTextStream stream ( &file );
        stream.setEncoding( QTextStream::UnicodeUTF8 );

        QString fileData = stream.read();

        result = ( fileData == data );
        if ( !m_genOutput && result ) {
            ::unlink( QFile::encodeName( outputFilename ) );
            return true;
        }
    }

    // generate result file

    QFileInfo info(outputFilename);
    createMissingDirs(info.dirPath());
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

bool RegressionTest::reportResult(bool passed, const QString & description)
{
    if (m_genOutput)
	return true;

    if (passed) {
	printf("PASS: ");
	m_passes++;
    }
    else {
	printf("FAIL: ");
	m_failures++;
    }

    if (!m_currentCategory.isEmpty())
	printf("%s ",m_currentCategory.latin1());

    printf("[%s]",m_currentTest.latin1());

    if (!description.isEmpty()) {
        QString desc = description;
        desc.replace( '\n', ' ' );
	printf(" %s", desc.latin1());
    }

    printf("\n");
    return passed;
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

void RegressionTest::slotOpenURL(const KURL &url, const KParts::URLArgs &args)
{
    m_part->browserExtension()->setURLArgs( args );

    PartMonitor pm(m_part);
    m_part->openURL(url);
    pm.waitForCompletion();
}

bool RegressionTest::cvsIgnored( const QString &filename )
{
    QFileInfo fi( filename );
    QString ignoreFilename = fi.dirPath() + "/.cvsignore";
    QFile ignoreFile(ignoreFilename);
    if (!ignoreFile.open(IO_ReadOnly))
        return false;

    QTextStream ignoreStream(&ignoreFile);
    QString line;
    while (!(line = ignoreStream.readLine()).isNull()) {
        if ( line == fi.fileName() ) {
            kdDebug() << filename << " is ignored\n";
            return true;
        }
    }
    ignoreFile.close();
    return false;
}

#include "test_regression.moc"
