/**
 * This file is part of the KDE project
 *
 * Copyright (C) 2001,2003 Peter Kelly (pmk@post.com)
 * Copyright (C) 2003,2004 Stephan Kulow (coolo@kde.org)
 * Copyright (C) 2004 Dirk Mueller ( mueller@kde.org )
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
#include <pwd.h>
#include <signal.h>
#include <unistd.h>

#include <kapplication.h>
#include <kacceleratormanager.h>
#include <qimage.h>
#include <qfile.h>
#include "test_regression.h"
#include <unistd.h>
#include <stdio.h>

#include "css/cssstyleselector.h"
#include <dom_string.h>
#include "rendering/render_style.h"
#include "rendering/render_layer.h"
#include "khtmldefaults.h"
#include <QWindowsStyle>
#include <QStyleOptionSlider>

//We don't use the default fonts, though, but traditional testregression ones
#undef HTML_DEFAULT_VIEW_FONT
#undef HTML_DEFAULT_VIEW_FIXED_FONT
#undef HTML_DEFAULT_VIEW_SERIF_FONT
#undef HTML_DEFAULT_VIEW_SANSSERIF_FONT
#undef HTML_DEFAULT_VIEW_CURSIVE_FONT
#undef HTML_DEFAULT_VIEW_FANTASY_FONT
#define HTML_DEFAULT_VIEW_FONT "helvetica"
#define HTML_DEFAULT_VIEW_FIXED_FONT "courier"
#define HTML_DEFAULT_VIEW_SERIF_FONT "times"
#define HTML_DEFAULT_VIEW_SANSSERIF_FONT "helvetica"
#define HTML_DEFAULT_VIEW_CURSIVE_FONT "helvetica"
#define HTML_DEFAULT_VIEW_FANTASY_FONT "helvetica"

#ifdef __GNUC__
#warning "Kill this at some point"
#endif

struct PalInfo
{
    QPalette::ColorRole role;
    quint32            color;
};

PalInfo palInfo[] =
{
    {QPalette::Foreground, 0xff000000},
    {QPalette::Button, 0xffc0c0c0},
    {QPalette::Light, 0xffffffff},
    {QPalette::Midlight, 0xffdfdfdf},
    {QPalette::Dark, 0xff808080},
    {QPalette::Mid, 0xffa0a0a4},
    {QPalette::Text, 0xff000000},
    {QPalette::BrightText, 0xffffffff},
    {QPalette::ButtonText, 0xff000000},
    {QPalette::Base, 0xffffffff},
    {QPalette::Background, 0xffc0c0c0},
    {QPalette::Shadow, 0xff000000},
    {QPalette::Highlight, 0xff000080},
    {QPalette::HighlightedText, 0xffffffff},
    {QPalette::Link, 0xff0000ff},
    {QPalette::LinkVisited, 0xffff00ff},
    {QPalette::LinkVisited, 0}
};

PalInfo disPalInfo[] =
{
    {QColorGroup::Foreground, 0xff808080},
    {QColorGroup::Button, 0xffc0c0c0},
    {QColorGroup::Light, 0xffffffff},
    {QColorGroup::Midlight, 0xffdfdfdf},
    {QColorGroup::Dark, 0xff808080},
    {QColorGroup::Mid, 0xffa0a0a4},
    {QColorGroup::Text, 0xff808080},
    {QColorGroup::BrightText, 0xffffffff},
    {QColorGroup::ButtonText, 0xff808080},
    {QColorGroup::Base, 0xffc0c0c0},
    {QColorGroup::Background, 0xffc0c0c0},
    {QColorGroup::Shadow, 0xff000000},
    {QColorGroup::Highlight, 0xff000080},
    {QColorGroup::HighlightedText, 0xffffffff},
    {QColorGroup::Link, 0xff0000ff},
    {QColorGroup::LinkVisited, 0xffff00ff},
    {QPalette::LinkVisited, 0}
};



class TestStyle: public QWindowsStyle
{
public:
    TestStyle()
    {}

    virtual void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
    {
        switch (element)
        {
        case CE_ScrollBarSubLine:
        case CE_ScrollBarAddLine:
        case CE_ScrollBarSubPage:
        case CE_ScrollBarAddPage:
        case CE_ScrollBarFirst:
        case CE_ScrollBarLast:
        case CE_ScrollBarSlider:
        {
            const QStyleOptionSlider* sbOpt = qstyleoption_cast<const QStyleOptionSlider*>(option);

            if (sbOpt->minimum == sbOpt->maximum)
            {
                const_cast<QStyleOptionSlider*>(sbOpt)->state ^= QStyle::State_Enabled;
                if (element == CE_ScrollBarSlider)
                    element = CE_ScrollBarAddPage;
            }

            if (element == CE_ScrollBarAddPage || element == CE_ScrollBarSubPage)
            {
                //Fun. in Qt4, the brush offset seems to be sensitive to window position??
                painter->setBrushOrigin(0,1);
            }
            break;
        }
        default: //shaddup
            break;
        }

        QWindowsStyle::drawControl(element, option, painter, widget);
    }

    virtual QRect subControlRect(ComplexControl control, const QStyleOptionComplex* option,
                                 SubControl subControl, const QWidget* widget) const
    {
        QRect rect = QWindowsStyle::subControlRect(control, option, subControl, widget);

        switch (control)
        {
        case CC_ComboBox:
            if (subControl == SC_ComboBoxEditField)
                return rect.translated(2,-1);
            else
                return rect;
        default:
            return rect;
        }
    }

    virtual QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& contentsSize, const QWidget* widget) const
    {
        QSize size = QWindowsStyle::sizeFromContents(type, option, contentsSize, widget);

        switch (type)
        {
        case CT_LineEdit:
            return QSize(size.width() + 2, size.height() + 2);
        case CT_ComboBox:
        {
            const QStyleOptionComboBox* cbOpt = qstyleoption_cast<const QStyleOptionComboBox*>(option);
            return QSize(qMax(43, size.width() + 6), size.height());
        }
        default:
            return size;
        }

    }

    virtual int pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
    {
        if (metric == PM_ButtonMargin)
            return 7;
        return QWindowsStyle::pixelMetric(metric, option, widget);
    }

    virtual QRect subElementRect(SubElement element, const QStyleOption* option, const QWidget* widget) const
    {
        QRect rect = QWindowsStyle::subElementRect(element, option, widget);
        if (element == SE_PushButtonContents)
        {
            const QStyleOptionButton* butOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (butOpt->icon.isNull())
                return rect.translated(0, -1);
        }
        return rect;
    }
};

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
#include <q3scrollview.h>
#include <qstring.h>
#include <qtextstream.h>
#include <q3valuelist.h>
#include <qwidget.h>
#include <qfileinfo.h>
#include <qtimer.h>
#include <kstatusbar.h>
#include <qfileinfo.h>

#include "misc/decoder.h"
#include "dom/dom2_range.h"
#include "dom/dom_exception.h"
#include "dom/html_document.h"
#include "html/htmltokenizer.h"
#include "khtml_part.h"
#include "khtmlpart_p.h"
#include <kparts/browserextension.h>

#include "khtmlview.h"
#include "rendering/render_replaced.h"
#include "xml/dom_docimpl.h"
#include "html/html_baseimpl.h"
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
pid_t xvfb;

// -------------------------------------------------------------------------

PartMonitor *PartMonitor::sm_highestMonitor = NULL;

PartMonitor::PartMonitor(KHTMLPart *_part)
{
    m_part = _part;
    m_completed = false;
    connect(m_part,SIGNAL(completed()),this,SLOT(partCompleted()));
    m_timer_waits = 200;
    m_timeout_timer = new QTimer(this);
}

PartMonitor::~PartMonitor()
{
   if (this == sm_highestMonitor)
	sm_highestMonitor = 0;
}


void PartMonitor::waitForCompletion()
{
    if (!m_completed) {

        if (sm_highestMonitor)
		return;

	sm_highestMonitor = this;

        qApp->enter_loop();

        //connect(m_timeout_timer, SIGNAL(timeout()), this, SLOT( timeout() ) );
        //m_timeout_timer->stop();
	//m_timeout_timer->start( visual ? 100 : 2, true );
    }

    QTimer::singleShot( 0, this, SLOT( finishTimers() ) );
    qApp->enter_loop();
}

void PartMonitor::timeout()
{
    qApp->exit_loop();
}

void PartMonitor::finishTimers()
{
    KJS::Window *w = KJS::Window::retrieveWindow( m_part );
    --m_timer_waits;
    if ( m_timer_waits && (w && w->winq->hasTimers()) || m_part->inProgress()) {
        // wait a bit
        QTimer::singleShot( 10, this, SLOT(finishTimers() ) );
        return;
    }
    qApp->exit_loop();
}

void PartMonitor::partCompleted()
{
    m_completed = true;
    RenderWidget::flushWidgetResizes();
    m_timeout_timer->stop();
    connect(m_timeout_timer, SIGNAL(timeout()),this, SLOT( timeout() ) );
    m_timeout_timer->start( /*visual ? 100 :*/ 2, true );
    disconnect(m_part,SIGNAL(completed()),this,SLOT(partCompleted()));
}

void signal_handler( int )
{
    printf( "timeout\n" );
    abort();
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

RegTestFunction::RegTestFunction(ExecState* /*exec*/, RegressionTest *_regTest, int _id, int length)
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
            if ( str.qstring().toLower().find( "failed!" ) >= 0 )
                m_regTest->saw_failure = true;
            QString res = str.qstring().replace('\007', "");
            m_regTest->m_currentOutput += res + "\n";
	    break;
	}
	case ReportResult: {
            bool passed = args[0].toBoolean(exec);
            QString description = args[1].toString(exec).qstring();
            if (args[1].isA(UndefinedType) || args[1].isA(NullType))
                description = QString::null;
            m_regTest->reportResult(passed,description);
            if ( !passed )
                m_regTest->saw_failure = true;
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
            int failures = RegressionTest::NoFailure;
            if ( m_regTest->m_genOutput ) {
                if ( !m_regTest->reportResult( m_regTest->checkOutput(filename+"-dom"),
                                               "Script-generated " + filename + "-dom") )
                    failures |= RegressionTest::DomFailure;
                if ( !m_regTest->reportResult( m_regTest->checkOutput(filename+"-render"),
                                         "Script-generated " + filename + "-render") )
                    failures |= RegressionTest::RenderFailure;
            } else {
                // compare with output file
                if ( !m_regTest->reportResult( m_regTest->checkOutput(filename+"-dom"), "DOM") )
                    failures |= RegressionTest::DomFailure;
                if ( !m_regTest->reportResult( m_regTest->checkOutput(filename+"-render"), "RENDER") )
                    failures |= RegressionTest::RenderFailure;
            }
            RegressionTest::curr->doFailureReport( filename, failures );
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
            QString fullFilename = QFileInfo(RegressionTest::curr->m_currentBase+"/"+filename).absoluteFilePath();
            KURL url;
            url.setProtocol("file");
            url.setPath(fullFilename);
            PartMonitor pm(m_part);
            m_part->openURL(url);
            pm.waitForCompletion();
	    kapp->processEvents(QEventLoop::AllEvents, 60000);
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
	    if (!file.open(QIODevice::ReadOnly)) {
		exec->setException(Error::create(exec, GeneralError,
						 QString("Error reading " + filename).latin1()));
	    }
	    else {
		QByteArray fileData;
		QDataStream stream(&fileData,QIODevice::WriteOnly);
		char buf[1024];
		int bytesread;
		while (!file.atEnd()) {
		    bytesread = file.readBlock(buf,1024);
		    stream.writeRawBytes(buf,bytesread);
		}
		file.close();
		QString contents(fileData);
		PartMonitor pm(m_part);
		m_part->begin(KURL( url ));
		m_part->write(contents);
		m_part->end();
		pm.waitForCompletion();
	    }
	    kapp->processEvents(QEventLoop::AllEvents, 60000);
	    break;
	}
	case Begin: {
            QString url = args[0].toString(exec).qstring();
            m_part->begin(KURL( url ));
            break;
        }
        case Write: {
            QString str = args[0].toString(exec).qstring();
            m_part->write(str);
            break;
        }
        case End: {
            m_part->end();
	    kapp->processEvents(QEventLoop::AllEvents, 60000);
            break;
        }
	case ExecuteScript: {
	    QString code = args[0].toString(exec).qstring();
	    Completion comp;
	    KJSProxy *proxy = m_part->jScript();
	    proxy->evaluate("",0,code,0,&comp);
	    if (comp.complType() == Throw)
		exec->setException(comp.value());
	    kapp->processEvents(QEventLoop::AllEvents, 60000);
	    break;
	}
	case ProcessEvents: {
	    kapp->processEvents(QEventLoop::AllEvents, 60000);
	    break;
	}
    }

    return result;
}

// -------------------------------------------------------------------------

static KCmdLineOptions options[] =
{
    { "b", 0, 0 },
    { "base <base_dir>", "Directory containing tests, basedir and output directories.", 0},
    { "d", 0, 0 },
    { "debug", "Do not supress debug output", 0},
    { "g", 0, 0 } ,
    { "genoutput", "Regenerate baseline (instead of checking)", 0 } ,
    { "s", 0, 0 } ,
    { "noshow", "Do not show the window while running tests", 0 } ,
    { "t", 0, 0 } ,
    { "test <filename>", "Only run a single test. Multiple options allowed.", 0 } ,
    { "js",  "Only run .js tests", 0 },
    { "html", "Only run .html tests", 0},
    { "noxvfb", "Do not use Xvfb", 0},
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

    QString kh("/var/tmp/%1_non_existant");
    kh = kh.arg( pw->pw_name );
    setenv( "KDEHOME", kh.latin1(), 1 );
    setenv( "LC_ALL", "C", 1 );
    setenv( "LANG", "C", 1 );

    signal( SIGALRM, signal_handler );

    KCmdLineArgs::init(argc, argv, "testregression", "TestRegression",
                       "Regression tester for khtml", "1.0");
    KCmdLineArgs::addCmdLineOptions(options);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs( );

    QByteArray baseDir = args->getOption("base");

    if ( args->count() < 1 && baseDir.isEmpty() ) {
	KCmdLineArgs::usage();
	::exit( 1 );
    }

    int testcase_index = 0;
    if (baseDir.isEmpty()) baseDir = args->arg(testcase_index++);

    QFileInfo bdInfo(baseDir);
    baseDir = QFile::encodeName(bdInfo.absoluteFilePath());

    const char *subdirs[] = {"tests", "baseline", "output", "resources"};
    for ( int i = 0; i < 3; i++ ) {
        QFileInfo sourceDir(QFile::encodeName( baseDir ) + "/" + subdirs[i]);
        if ( !sourceDir.exists() || !sourceDir.isDir() ) {
            fprintf(stderr,"ERROR: Source directory \"%s/%s\": no such directory.\n", (const char *)baseDir, subdirs[i]);
            exit(1);
        }
    }

    if ( ::access( "/usr/X11R6/bin/Xvfb", X_OK ) ) {
        fprintf( stderr, "ERROR: We need /usr/X11R6/bin/Xvfb to be installed for reliable results\n" );
        exit( 1 );
    }


    if (args->isSet("xvfb"))
    {
        xvfb = fork();
        if ( !xvfb ) {
            char buffer[1000];
            sprintf( buffer, "%s/resources,/usr/X11R6/lib/X11/fonts/75dpi:unscaled,/usr/X11R6/lib/X11/fonts/misc:unscaled,/usr/X11R6/lib/X11/fonts/Type1", (const char *)baseDir );
            execl( "/usr/X11R6/bin/Xvfb", "/usr/X11R6/bin/Xvfb", "-once", "-dpi", "100", "-screen", "0", "1024x768x16", "-ac", "-fp", buffer, ":47", (char*)NULL );
        }

        setenv( "DISPLAY", ":47", 1 );
    }

    KApplication a;
    a.disableAutoDcopRegistration();
    a.setStyle( new TestStyle );
    KSimpleConfig sc1( "cryptodefaults" );
    sc1.setGroup( "Warnings" );
    sc1.writeEntry( "OnUnencrypted",  false );
    KGlobal::instance()->config()->setGroup( "Notification Messages" );
    KGlobal::instance()->config()->writeEntry( "kjscupguard_alarmhandler", true );
    KGlobal::instance()->config()->setGroup("HTML Settings");
    KGlobal::instance()->config()->writeEntry("ReportJSErrors", false);
    KConfig cfg( "khtmlrc" );
    cfg.setGroup("HTML Settings");
    cfg.writeEntry( "StandardFont", HTML_DEFAULT_VIEW_SANSSERIF_FONT );
    cfg.writeEntry( "FixedFont", HTML_DEFAULT_VIEW_FIXED_FONT );
    cfg.writeEntry( "SerifFont", HTML_DEFAULT_VIEW_SERIF_FONT );
    cfg.writeEntry( "SansSerifFont", HTML_DEFAULT_VIEW_SANSSERIF_FONT );
    cfg.writeEntry( "CursiveFont", HTML_DEFAULT_VIEW_CURSIVE_FONT );
    cfg.writeEntry( "FantasyFont", HTML_DEFAULT_VIEW_FANTASY_FONT );
    cfg.writeEntry( "MinimumFontSize", HTML_DEFAULT_MIN_FONT_SIZE );
    cfg.writeEntry( "MediumFontSize", 10 );
    cfg.writeEntry( "Fonts", QStringList() );
    cfg.writeEntry( "DefaultEncoding", "" );
    cfg.setGroup("Java/JavaScript Settings");
    cfg.writeEntry( "WindowOpenPolicy", KHTMLSettings::KJSWindowOpenAllow);

    cfg.sync();

    QPalette pal = a.palette();
    for (int c = 0; palInfo[c].color; ++c)
    {
        pal.setColor(QPalette::Active,   palInfo[c].role, QColor(palInfo[c].color));
        pal.setColor(QPalette::Inactive, palInfo[c].role, QColor(palInfo[c].color));
        pal.setColor(QPalette::Disabled, palInfo[c].role, QColor(disPalInfo[c].color));
    }
    a.setPalette(pal);

    int rv = 1;

    if ( !args->isSet( "debug" ) ) {
        KSimpleConfig dc( "kdebugrc" );
        static int areas[] = { 1000, 6000, 6005, 6010, 6020, 6030,
                               6031, 6035, 6036, 6040, 6041, 6045,
                               6050, 6060, 6061, 7000, 7006, 170,
                               171, 7101, 7002, 7019, 7027, 7014,
                               7011, 6070, 6080, 6090, 0};
        for ( int i = 0; areas[i]; ++i ) {
            dc.setGroup( QString::number( areas[i] ) );
            dc.writeEntry( "InfoOutput", 4 );
        }
        dc.sync();

        kdClearDebugConfig();
    }

    // create widgets
    KHTMLFactory *fac = new KHTMLFactory();
    KMainWindow *toplevel = new KMainWindow();
    KHTMLPart *part = new KHTMLPart( toplevel, 0, toplevel, 0, KHTMLPart::BrowserViewGUI );

    toplevel->setCentralWidget( part->widget() );
    KAcceleratorManager::setNoAccel ( part->widget() );
    part->setJScriptEnabled(true);

    part->executeScript(DOM::Node(), ""); // force the part to create an interpreter
//    part->setJavaEnabled(true);

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
                                                        baseDir,
                                                        args->getOption("output"),
                                                        args->isSet("genoutput"),
                                                        !args->isSet( "html" ),
                                                        !args->isSet( "js" ));
    QObject::connect(part->browserExtension(), SIGNAL(openURLRequest(const KURL &, const KParts::URLArgs &)),
		     regressionTest, SLOT(slotOpenURL(const KURL&, const KParts::URLArgs &)));
    QObject::connect(part->browserExtension(), SIGNAL(resizeTopLevelWidget( int, int )),
		     regressionTest, SLOT(resizeTopLevelWidget( int, int )));

    bool result = false;
    QByteArrayList tests = args->getOptionList("test");
    // merge testcases specified on command line
    for (; testcase_index < args->count(); testcase_index++)
        tests << args->arg(testcase_index);
    if (tests.count() > 0)
        foreach (QByteArray test, tests) {
	    result = regressionTest->runTests(test,true);
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
                printf( " (%d unexpected passes)\n", regressionTest->m_passes_fail );
            else
                printf( "\n" );
	    printf("Failures: %d",regressionTest->m_failures_work);
            if ( regressionTest->m_failures_fail )
                printf( " (%d expected failures)\n", regressionTest->m_failures_fail );
            else
                printf( "\n" );
            if ( regressionTest->m_errors )
                printf("Errors:   %d\n",regressionTest->m_errors);

            QFile list( regressionTest->m_outputDir + "/links.html" );
            list.open( QIODevice::WriteOnly|QIODevice::Append );
            QString link, cl;
            link = QString( "<hr>%1 failures. (%2 expected failures)" )
                   .arg(regressionTest->m_failures_work )
                   .arg( regressionTest->m_failures_fail );
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
    delete fac;

    khtml::Cache::clear();
    khtml::CSSStyleSelector::clear();
    khtml::RenderStyle::cleanup();

    kill( xvfb, SIGINT );

    return rv;
}

// -------------------------------------------------------------------------

RegressionTest *RegressionTest::curr = 0;

RegressionTest::RegressionTest(KHTMLPart *part, const QString &baseDir, const QString &outputDir,
			       bool _genOutput, bool runJS, bool runHTML)
  : QObject(part)
{
    m_part = part;
    m_baseDir = baseDir;
    m_baseDir = m_baseDir.replace( "//", "/" );
    if ( m_baseDir.endsWith( "/" ) )
        m_baseDir = m_baseDir.left( m_baseDir.length() - 1 );
    if (outputDir.isEmpty())
        m_outputDir = m_baseDir + "/output";
    else {
        createMissingDirs(outputDir + "/");
        m_outputDir = outputDir;
    }
    m_genOutput = _genOutput;
    m_runJS = runJS;
    m_runHTML =  runHTML;
    m_passes_work = m_passes_fail = 0;
    m_failures_work = m_failures_fail = 0;
    m_errors = 0;

    ::unlink( QFile::encodeName( m_outputDir + "/links.html" ) );
    QFile f( m_outputDir + "/empty.html" );
    QString s;
    f.open( QIODevice::WriteOnly | QIODevice::Truncate );
    s = "<html><body>Follow the white rabbit";
    f.writeBlock( s.latin1(), s.length() );
    f.close();
    f.setName( m_outputDir + "/index.html" );
    f.open( QIODevice::WriteOnly | QIODevice::Truncate );
    s = "<html><frameset cols=150,*><frame src=links.html><frame name=content src=empty.html>";
    f.writeBlock( s.latin1(), s.length() );
    f.close();

    m_paintBuffer = 0;

    curr = this;
}

#include <qobject.h>

static QStringList readListFile( const QString &filename )
{
    // Read ignore file for this directory
    QString ignoreFilename = filename;
    QFileInfo ignoreInfo(ignoreFilename);
    QStringList ignoreFiles;
    if (ignoreInfo.exists()) {
        QFile ignoreFile(ignoreFilename);
        if (!ignoreFile.open(QIODevice::ReadOnly)) {
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
    delete m_paintBuffer;
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

	    if (filename == "." || filename == ".." ||  ignoreFiles.contains(filename) )
                continue;
            int failure_type = NoFailure;
            if ( failureFiles.contains( filename ) )
                failure_type |= AllFailure;
            if ( failureFiles.contains ( filename + "-render" ) )
                failure_type |= RenderFailure;
            if ( failureFiles.contains ( filename + "-dump.png" ) )
                failure_type |= PaintFailure;
            if ( failureFiles.contains ( filename + "-dom" ) )
                failure_type |= DomFailure;
            runTests(relFilename, false, failure_type );
	}
    }
    else if (info.isFile()) {

        alarm( 400 );

        khtml::Cache::init();

	QString relativeDir = QFileInfo(relPath).path();
	QString filename = info.fileName();
	m_currentBase = m_baseDir + "/tests/"+relativeDir;
	m_currentCategory = relativeDir;
	m_currentTest = filename;
        m_known_failures = known_failure;
	if ( filename.endsWith(".html") || filename.endsWith( ".htm" ) || filename.endsWith( ".xhtml" ) || filename.endsWith( ".xml" ) ) {
            if ( relPath.startsWith( "domts/" ) && !m_runJS )
                return true;
	    if ( relPath.startsWith( "ecma/" ) && !m_runJS )
	        return true;
            if ( m_runHTML )
                testStaticFile(relPath);
	}
	else if (filename.endsWith(".js")) {
            if ( m_runJS )
                testJSFile(relPath);
	}
	else if (mustExist) {
	    fprintf(stderr,"%s: Not a valid test file (must be .htm(l) or .js)\n",relPath.latin1());
	    return false;
	}
    } else if (mustExist) {
        fprintf(stderr,"%s: Not a regular file\n",relPath.latin1());
        return false;
    }

    return true;
}

void RegressionTest::getPartDOMOutput( QTextStream &outputStream, KHTMLPart* part, uint indent )
{
    Node node = part->document();
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
		if ( node.handle()->id() == ID_FRAME ) {
			outputStream << endl;
			QString frameName = static_cast<DOM::HTMLFrameElementImpl *>( node.handle() )->name.string();
			KHTMLPart* frame = part->findFrame( frameName );
			Q_ASSERT( frame );
			if ( frame )
			    getPartDOMOutput( outputStream, frame, indent );
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

void RegressionTest::dumpRenderTree( QTextStream &outputStream, KHTMLPart* part )
{
    DOM::DocumentImpl* doc = static_cast<DocumentImpl*>( part->document().handle() );
    if ( !doc || !doc->renderer() )
        return;
    doc->renderer()->layer()->dump( outputStream );

    // Dump frames if any
    // Get list of names instead of frames() to sort the list alphabetically
    QStringList names = part->frameNames();
    names.sort();
    for ( QStringList::iterator it = names.begin(); it != names.end(); ++it ) {
        outputStream << "FRAME: " << (*it) << "\n";
	KHTMLPart* frame = part->findFrame( (*it) );
//	Q_ASSERT( frame );
	if ( frame )
            dumpRenderTree( outputStream, frame );
    }
}

QString RegressionTest::getPartOutput( OutputType type)
{
    // dump out the contents of the rendering & DOM trees
    QString dump;
    QTextStream outputStream(&dump, QIODevice::WriteOnly);

    if ( type == RenderTree ) {
        dumpRenderTree( outputStream, m_part );
    } else {
        assert( type == DOMTree );
        getPartDOMOutput( outputStream, m_part, 0 );
    }

    dump.replace( m_baseDir + "/tests", QLatin1String( "REGRESSION_SRCDIR" ) );
    return dump;
}

QImage RegressionTest::renderToImage()
{
    int ew = m_part->view()->contentsWidth();
    int eh = m_part->view()->contentsHeight();

    if (ew * eh > 4000 * 4000) // don't DoS us
        return QImage();

    QImage img( ew, eh, 32 );
    img.fill( 0xff0000 );
    if (!m_paintBuffer )
        m_paintBuffer = new QPixmap( 512, 128 );

    for ( int py = 0; py < eh; py += 128 ) {
        for ( int px = 0; px < ew; px += 512 ) {
            QPainter* tp = new QPainter;
            tp->begin( m_paintBuffer );
            tp->translate( -px, -py );
            tp->fillRect(px, py, 512, 128, Qt::magenta);
            m_part->document().handle()->renderer()->layer()->paint( tp, QRect( px, py, 512, 128 ) );
            tp->end();
            delete tp;

            // now fill the chunk into our image
            QImage chunk = m_paintBuffer->convertToImage();
            assert( chunk.depth() == 32 );
            for ( int y = 0; y < 128 && py + y < eh; ++y )
                memcpy( img.scanLine( py+y ) + px*4, chunk.scanLine( y ), qMin( 512, ew-px )*4 );
        }
    }

    assert( img.depth() == 32 );
    return img;
}

bool RegressionTest::imageEqual( const QImage &lhsi, const QImage &rhsi )
{
    if ( lhsi.width() != rhsi.width() || lhsi.height() != rhsi.height() ) {
        kdDebug() << "dimensions different " << lhsi.size() << " " << rhsi.size() << endl;
        return false;
    }
    int w = lhsi.width();
    int h = lhsi.height();
    int bytes = lhsi.bytesPerLine();

    const unsigned char* origLs = lhsi.bits();
    const unsigned char* origRs = rhsi.bits();

    for ( int y = 0; y < h; ++y )
    {
        const QRgb* ls = (const QRgb*)(origLs + y * bytes);
        const QRgb* rs = (const QRgb*)(origRs + y * bytes);
        if ( memcmp( ls, rs, bytes ) ) {
            for ( int x = 0; x < w; ++x ) {
                QRgb l = ls[x];
                QRgb r = rs[x];
                if ( ( abs( qRed( l ) - qRed(r ) ) < 20 ) &&
                     ( abs( qGreen( l ) - qGreen(r ) ) < 20 ) &&
                     ( abs( qBlue( l ) - qBlue(r ) ) < 20 ) )
                    continue;
                 kdDebug() << "pixel (" << x << ", " << y << ") is different " << QColor(  lhsi.pixel (  x, y ) ) << " " << QColor(  rhsi.pixel (  x, y ) ) << endl;
                return false;
            }
        }
    }

    return true;
}

void RegressionTest::createLink( const QString& test, int failures )
{
    createMissingDirs( m_outputDir + "/" + test + "-compare.html" );

    QFile list( m_outputDir + "/links.html" );
    list.open( QIODevice::WriteOnly|QIODevice::Append );
    QString link;
    link = QString( "<a href=\"%1\" target=\"content\" title=\"%2\">" )
           .arg( test + "-compare.html" )
           .arg( test );
    link += m_currentTest;
    link += "</a> [";
    if ( failures & DomFailure )
        link += "D";
    if ( failures & RenderFailure )
        link += "R";
    if ( failures & PaintFailure )
        link += "P";
    link += "]<br>\n";
    list.writeBlock( link.latin1(), link.length() );
    list.close();
}

void RegressionTest::doJavascriptReport( const QString &test )
{
    QFile compare( m_outputDir + "/" + test + "-compare.html" );
    if ( !compare.open( QIODevice::WriteOnly|QIODevice::Truncate ) )
        kdDebug() << "failed to open " << m_outputDir + "/" + test + "-compare.html" << endl;
    QString cl;
    cl = QString( "<html><head><title>%1</title>" ).arg( test );
    cl += "<body><tt>";
    QString text = "\n" + m_currentOutput;
    text.replace( '<', "&lt;" );
    text.replace( '>', "&gt;" );
    text.replace( QRegExp( "\nFAILED" ), "\n<span style='color: red'>FAILED</span>" );
    text.replace( QRegExp( "\nFAIL" ), "\n<span style='color: red'>FAIL</span>" );
    text.replace( QRegExp( "\nPASSED" ), "\n<span style='color: green'>PASSED</span>" );
    text.replace( QRegExp( "\nPASS" ), "\n<span style='color: green'>PASS</span>" );
    if ( text.at( 0 ) == '\n' )
        text = text.mid( 1, text.length() );
    text.replace( '\n', "<br>\n" );
    cl += text;
    cl += "</tt></body></html>";
    compare.writeBlock( cl.latin1(), cl.length() );
    compare.close();
}

/** returns the path in a way that is relatively reachable from base.
 * @param base base directory (must not include trailing slash)
 * @param path directory/file to be relatively reached by base
 * @return path with all elements replaced by .. and concerning path elements
 *	to be relatively reachable from base.
 */
static QString makeRelativePath(const QString &base, const QString &path)
{
    QString absBase = QFileInfo(base).absoluteFilePath();
    QString absPath = QFileInfo(path).absoluteFilePath();
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
            for (int i = relBase.string().count('/'); i > 0; --i)
                rel += "../";
            rel += "..";
            if (relPath.string().length() > 0) rel += "/";
        }
        rel += relPath.string();
    }
    return rel;
}

void RegressionTest::doFailureReport( const QString& test, int failures )
{
    if ( failures == NoFailure ) {
        ::unlink( QFile::encodeName( m_outputDir + "/" + test + "-compare.html" ) );
        return;
    }

    createLink( test, failures );

    if ( failures & JSFailure ) {
        doJavascriptReport( test );
        return; // no support for both kind
    }

    QFile compare( m_outputDir + "/" + test + "-compare.html" );

    QString testFile = QFileInfo(test).fileName();

    QString renderDiff;
    QString domDiff;

    QString relOutputDir = makeRelativePath(m_baseDir, m_outputDir);

    // are blocking reads possible with KProcess?
    QString pwd = QDir::currentPath();
    chdir( QFile::encodeName( m_baseDir ) );

    if ( failures & RenderFailure ) {
        renderDiff += "<pre>";
        FILE *pipe = popen( QString::fromLatin1( "diff -u baseline/%1-render %3/%2-render" )
                            .arg ( test, test, relOutputDir ).latin1(), "r" );
        QTextIStream *is = new QTextIStream( pipe );
        for ( int line = 0; line < 100 && !is->atEnd(); ++line ) {
            QString line = is->readLine();
            line = line.replace( '<', "&lt;" );
            line = line.replace( '>', "&gt;" );
            renderDiff += line + "\n";
        }
        delete is;
        pclose( pipe );
        renderDiff += "</pre>";
    }

    if ( failures & DomFailure ) {
        domDiff += "<pre>";
        FILE *pipe = popen( QString::fromLatin1( "diff -u baseline/%1-dom %3/%2-dom" )
                            .arg ( test, test, relOutputDir ).latin1(), "r" );
        QTextIStream *is = new QTextIStream( pipe );
        for ( int line = 0; line < 100 && !is->atEnd(); ++line ) {
            QString line = is->readLine();
            line = line.replace( '<', "&lt;" );
            line = line.replace( '>', "&gt;" );
            domDiff += line  + "\n";
        }
        delete is;
        pclose( pipe );
        domDiff += "</pre>";
    }

    chdir( QFile::encodeName( pwd ) );

    // create a relative path so that it works via web as well. ugly
    QString relpath = makeRelativePath(m_outputDir + "/"
        + QFileInfo(test).path(), m_baseDir);

    compare.open( QIODevice::WriteOnly|QIODevice::Truncate );
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

    if ( failures & PaintFailure )
        cl += QString( "<body onload=\"m(1); show(); runSlideShow();\"" );
    else if ( failures & RenderFailure )
        cl += QString( "<body onload=\"m(4); toggleVisible('render');\"" );
    else
        cl += QString( "<body onload=\"m(5); toggleVisible('dom');\"" );
    cl += QString(" text=black bgcolor=gray>\n<h1>%3</h1>\n" ).arg( test );
    if ( failures & PaintFailure )
        cl += QString ( "<span id='b1' class='buttondown' onclick=\"doflicker=1;show();m(1)\">FLICKER</span>&nbsp;\n"
                        "<span id='b2' class='button' onclick=\"doflicker=0;t=0;show();m(2)\">BASE</span>&nbsp;\n"
                        "<span id='b3' class='button' onclick=\"doflicker=0;t=1;show();m(3)\">OUT</span>&nbsp;\n" );
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

void RegressionTest::testStaticFile(const QString & filename)
{
    qApp->mainWidget()->resize( 800, 598 ); // restore size

    // Set arguments
    KParts::URLArgs args;
    if (filename.endsWith(".html") || filename.endsWith(".htm")) args.serviceType = "text/html";
    else if (filename.endsWith(".xhtml")) args.serviceType = "application/xhtml+xml";
    else if (filename.endsWith(".xml")) args.serviceType = "text/xml";
    m_part->browserExtension()->setURLArgs(args);
    // load page
    KURL url;
    url.setProtocol("file");
    url.setPath(QFileInfo(m_baseDir + "/tests/"+filename).absoluteFilePath());
    PartMonitor pm(m_part);
    m_part->openURL(url);
    pm.waitForCompletion();
    m_part->closeURL();

    if ( filename.startsWith( "domts/" ) ) {
        QString functionname;

        KJS::Completion comp = m_part->jScriptInterpreter()->evaluate("exposeTestFunctionNames();");
        /*
         *  Error handling
         */
        KJS::ExecState *exec = m_part->jScriptInterpreter()->globalExec();
        if ( comp.complType() == ReturnValue || comp.complType() == Normal )
        {
            if (comp.value().isValid() && comp.value().isA(ObjectType) &&
               (Object::dynamicCast(comp.value()).className() == "Array" ) )
            {
                Object argArrayObj = Object::dynamicCast(comp.value());
                unsigned int length = argArrayObj.
                                      get(exec,lengthPropertyName).
                                      toUInt32(exec);
                if ( length == 1 )
                    functionname = argArrayObj.get(exec, 0).toString(exec).qstring();
            }
        }
        if ( functionname.isNull() ) {
            kdDebug() << "DOM " << filename << " doesn't expose 1 function name - ignoring" << endl;
            return;
        }

        KJS::Completion comp2 = m_part->jScriptInterpreter()->evaluate("setUpPage(); " + functionname + "();" );
        bool success = ( comp2.complType() == ReturnValue || comp2.complType() == Normal );
        QString description = "DOMTS";
        if ( comp2.complType() == Throw ) {
            KJS::Value val = comp2.value();
            KJS::Object obj = Object::dynamicCast(val);
            if ( obj.isValid() && obj.hasProperty( exec, "jsUnitMessage" ) )
                description = obj.get( exec, "jsUnitMessage" ).toString( exec ).qstring();
            else
                description = comp2.value().toString( exec ).qstring();
        }
        reportResult( success,  description );

        if (!success && !m_known_failures)
            doFailureReport( filename, JSFailure );
        return;
    }

    int back_known_failures = m_known_failures;

    if ( m_genOutput ) {
        if ( m_known_failures & DomFailure)
            m_known_failures = AllFailure;
        reportResult( checkOutput(filename+"-dom"), "DOM" );
        if ( m_known_failures & RenderFailure )
            m_known_failures = AllFailure;
        reportResult( checkOutput(filename+"-render"), "RENDER" );
        if ( m_known_failures & PaintFailure )
            m_known_failures = AllFailure;
        renderToImage().save(m_baseDir + "/baseline/" + filename + "-dump.png","PNG", 60);
        printf("Generated %s\n", QString( m_baseDir + "/baseline/" + filename + "-dump.png" ).latin1() );
        reportResult( true, "PAINT" );
    } else {
        int failures = NoFailure;

        // compare with output file
        if ( m_known_failures & DomFailure)
            m_known_failures = AllFailure;
        if ( !reportResult( checkOutput(filename+"-dom"), "DOM" ) )
            failures |= DomFailure;

        if ( m_known_failures & RenderFailure )
            m_known_failures = AllFailure;
        if ( !reportResult( checkOutput(filename+"-render"), "RENDER" ) )
            failures |= RenderFailure;

        if ( m_known_failures & PaintFailure )
            m_known_failures = AllFailure;
        CheckResult dumped = checkPaintdump(filename);
        reportResult( dumped, "PAINT");
        if (dumped == Failure)
            failures |= PaintFailure;

        doFailureReport(filename, failures );
    }

    m_known_failures = back_known_failures;
}

void RegressionTest::evalJS( ScriptInterpreter &interp, const QString &filename, bool report_result )
{
    QString fullSourceName = filename;
    QFile sourceFile(fullSourceName);

    if (!sourceFile.open(QIODevice::ReadOnly)) {
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
        bool expected_failure = filename.endsWith( "-n.js" );
        if (c.complType() == Throw) {
            QString errmsg = c.value().toString(interp.globalExec()).qstring();
            if ( !expected_failure ) {
                printf( "ERROR: %s (%s)\n",filename.latin1(), errmsg.latin1());
                m_errors++;
            } else {
                reportResult( true, QString( "Expected Failure: %1" ).arg( errmsg ) );
            }
        } else if ( saw_failure ) {
            if ( !expected_failure )
                doFailureReport( m_currentCategory + "/" + m_currentTest, JSFailure );
            reportResult( expected_failure, "saw 'failed!'" );
        } else {
            reportResult( !expected_failure, "passed" );
        }
    }
}

class GlobalImp : public ObjectImp {
public:
  virtual UString className() const { return "global"; }
};

void RegressionTest::testJSFile(const QString & filename )
{
    qApp->mainWidget()->resize( 800, 598 ); // restore size

    // create interpreter
    // note: this is different from the interpreter used by the part,
    // it contains regression test-specific objects & functions
    Object global(new GlobalImp());
    khtml::ChildFrame frame;
    frame.m_part = m_part;
    ScriptInterpreter interp(global,&frame);
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

RegressionTest::CheckResult RegressionTest::checkPaintdump(const QString &filename)
{
    QString againstFilename( filename + "-dump.png" );
    QString absFilename = QFileInfo(m_baseDir + "/baseline/" + againstFilename).absoluteFilePath();
    if ( svnIgnored( absFilename ) ) {
        m_known_failures = NoFailure;
        return Ignored;
    }
    CheckResult result = Failure;

    QImage baseline;
    baseline.load( absFilename, "PNG");
    QImage output = renderToImage();
    if ( !imageEqual( baseline, output ) ) {
        QString outputFilename = m_outputDir + "/" + againstFilename;
        createMissingDirs(outputFilename );
        output.save(outputFilename, "PNG", 60);
    }
    else {
        ::unlink( QFile::encodeName( m_outputDir + "/" + againstFilename ) );
        result = Success;
    }
    return result;
}

RegressionTest::CheckResult RegressionTest::checkOutput(const QString &againstFilename)
{
    QString absFilename = QFileInfo(m_baseDir + "/baseline/" + againstFilename).absoluteFilePath();
    if ( svnIgnored( absFilename ) ) {
        m_known_failures = NoFailure;
        return Ignored;
    }

    bool domOut = againstFilename.endsWith( "-dom" );
    QString data = getPartOutput( domOut ? DOMTree : RenderTree );
    data.remove( char( 13 ) );

    CheckResult result = Success;

    // compare result to existing file
    QString outputFilename = QFileInfo(m_outputDir + "/" + againstFilename).absoluteFilePath();
    bool kf = false;
    if ( m_known_failures & AllFailure )
        kf = true;
    else if ( domOut && ( m_known_failures & DomFailure ) )
        kf = true;
    else if ( !domOut && ( m_known_failures & RenderFailure ) )
        kf = true;
    if ( kf )
        outputFilename += "-KF";

    if ( m_genOutput )
        outputFilename = absFilename;

    QFile file(absFilename);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream ( &file );
        stream.setEncoding( QTextStream::UnicodeUTF8 );

        QString fileData = stream.read();

        result = ( fileData == data ) ? Success : Failure;
        if ( !m_genOutput && result == Success ) {
            ::unlink( QFile::encodeName( outputFilename ) );
            return Success;
        }
    }

    // generate result file
    createMissingDirs( outputFilename );
    QFile file2(outputFilename);
    if (!file2.open(QIODevice::WriteOnly)) {
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

bool RegressionTest::reportResult(CheckResult result, const QString & description)
{
    if ( result == Ignored ) {
        //printf("IGNORED: ");
        //printDescription( description );
        return true; // no error
    } else
        return reportResult( result == Success, description );
}

bool RegressionTest::reportResult(bool passed, const QString & description)
{
    if (m_genOutput)
	return true;

   if (passed) {
        if ( m_known_failures & AllFailure ) {
            printf("PASS (unexpected!): ");
            m_passes_fail++;
        } else {
            printf("PASS: ");
            m_passes_work++;
        }
    }
    else {
        if ( m_known_failures & AllFailure ) {
            printf("FAIL (known): ");
            m_failures_fail++;
            passed = true; // we knew about it
        } else {
            printf("FAIL: ");
            m_failures_work++;
        }
    }

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
    QFileInfo dirInfo( dif.path() );
    if (dirInfo.exists())
	return;

    QStringList pathComponents;
    QFileInfo parentDir = dirInfo;
    pathComponents.prepend(parentDir.absoluteFilePath());
    while (!parentDir.exists()) {
	QString parentPath = parentDir.absoluteFilePath();
	int slashPos = parentPath.lastIndexOf('/');
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

bool RegressionTest::svnIgnored( const QString &filename )
{
    QFileInfo fi( filename );
    QString ignoreFilename = fi.path() + "/svnignore";
    QFile ignoreFile(ignoreFilename);
    if (!ignoreFile.open(QIODevice::ReadOnly))
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
