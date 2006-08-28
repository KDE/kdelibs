// program to test the new khtml implementation

#include <QCursor>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QToolButton>
#include <QComboBox>
#include <QLineEdit>
#include <QMovie>
#include <QLabel>
#include <QImage>

#include <kapplication.h>
#include <kxmlguifactory.h>
#include <kmainwindow.h>
#include <kcmdlineargs.h>
#include <ktoggleaction.h>
#include <kcombobox.h>
#include <kpushbutton.h>
#include <kicon.h>
#include <kstdaction.h>

#include "css/cssstyleselector.h"

#include "dom/dom_string.h"
#include "dom/dom2_range.h"
#include "dom/html_document.h"
#include "dom/dom_exception.h"

#include "html/htmltokenizer.h"
#include "html/html_imageimpl.h"

#include "misc/decoder.h"
#include "misc/loader.h"

#include "rendering/render_style.h"

#include "khtml_part.h"
#include "domtreeview.h"

// to be able to delete a static protected member pointer in kbrowser...
// just for memory debugging
#define protected public
#undef protected

#define protected public
#include "khtml_factory.h"
#undef protected

#include "testkhtmlnew.h"
#include "testkhtmlnew.moc"

TestKHTML::TestKHTML()
    : KMainWindow()
{
#ifndef __KDE_HAVE_GCC_VISIBILITY
    m_factory = new KHTMLFactory(true);
#endif

    m_movie = new QMovie(":/pics/progress.gif");
    m_indicator = new QLabel;
    m_indicator->setFixedSize(QSize(24, 24));
    m_indicator->setMovie(m_movie);

    m_part = new KHTMLPart( this, this, KHTMLPart::BrowserViewGUI );
    connect( m_part->browserExtension(), SIGNAL(openURLRequest( const KUrl &, const KParts::URLArgs & )),
             this, SLOT(openUrl( const KUrl&, const KParts::URLArgs & )));

    m_combo = new KComboBox;
    m_combo->setEditable(true);
    connect(m_combo, SIGNAL(returnPressed(const QString&)), this, SLOT(openUrl(const QString&)));

    m_goButton = new QToolButton;
    m_goButton->setText("Go");
    connect(m_goButton, SIGNAL(clicked()), this, SLOT(openUrl()));

    m_reloadButton = new QToolButton;
    m_reloadButton->setIcon(KIcon("reload"));
    connect(m_reloadButton, SIGNAL(clicked()), this, SLOT(reload()));

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(m_reloadButton);
    topLayout->addWidget(m_combo);
    topLayout->addWidget(m_goButton);
    topLayout->addWidget(m_indicator);
    m_indicator->show();

    QFrame *mainWidget = new QFrame;
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_part->widget());

    setCentralWidget(mainWidget);
    resize(800, 600);
    setupActions();

    m_part->setJScriptEnabled(true);
    m_part->setJavaEnabled(true);
    m_part->setPluginsEnabled(true);
    m_part->setURLCursor(QCursor(Qt::PointingHandCursor));

    connect(m_part, SIGNAL(setWindowCaption(const QString &)),
            m_part->widget()->topLevelWidget(), SLOT(setCaption(const QString &)));

    connect(m_part, SIGNAL(started(KIO::Job *)), this, SLOT(startLoading()));
    connect(m_part, SIGNAL(completed()), this, SLOT(finishedLoading()));
}

void TestKHTML::startLoading()
{
    m_movie->start();
}

void TestKHTML::finishedLoading()
{
    m_movie->stop();
}

TestKHTML::~TestKHTML()
{
#ifndef __KDE_HAVE_GCC_VISIBILITY
    m_factory->deref();
#endif
}

void TestKHTML::setupActions()
{
    QDomDocument document = m_part->domDocument();
    QDomElement fileMenu = document.documentElement().firstChild().childNodes().item( 0 ).toElement();

    QDomElement quitElement = document.createElement("action");
    quitElement.setAttribute("name", "quit");
    fileMenu.appendChild(quitElement);

    QDomElement viewMenu = document.documentElement().firstChild().childNodes().item( 2 ).toElement();

    QDomElement element = document.createElement("action");
    element.setAttribute("name", "debugRenderTree");
    viewMenu.appendChild(element);

    element = document.createElement("action");
    element.setAttribute("name", "debugDOMTree");
    viewMenu.appendChild(element);

    QDomElement toolBar = document.documentElement().firstChild().nextSibling().toElement();
    element = document.createElement("action");
    element.setAttribute("name", "editable");
    toolBar.insertBefore(element, toolBar.firstChild());

    element = document.createElement("action");
    element.setAttribute("name", "navigable");
    toolBar.insertBefore(element, toolBar.firstChild());

    element = document.createElement("action");
    element.setAttribute("name", "reload");
    toolBar.insertBefore(element, toolBar.firstChild());

    element = document.createElement("action");
    element.setAttribute("name", "print");
    toolBar.insertBefore(element, toolBar.firstChild());


    KAction *quitAction = new KAction( i18n( "Quit" ), m_part->actionCollection(), "quit" );
    connect( quitAction, SIGNAL( triggered( bool ) ), kapp, SLOT( quit() ) );

    KAction *action = new KAction(KIcon("reload"),  "Reload", m_part->actionCollection(), "reload" );
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reload()));
    action->setShortcut(Qt::Key_F5);

    KAction *kprint = new KAction(KIcon("print"),  "Print", m_part->actionCollection(), "print" );
    connect(kprint, SIGNAL(triggered(bool)), m_part->browserExtension(), SLOT(print()));
    kprint->setEnabled(true);

    KToggleAction *ta = new KToggleAction("Navigable", "editclear", 0, actionCollection(), "navigable" );
    ta->setChecked(m_part->isCaretMode());
    connect(ta, SIGNAL(toggled(bool)), this, SLOT( toggleNavigable(bool) ));

    ta = new KToggleAction( "Editable", "edit", 0, actionCollection(), "editable" );
    ta->setChecked(m_part->isEditable());
    connect(ta, SIGNAL(toggled(bool)), this, SLOT(toggleEditable(bool)));

    KStdAction::quit( this, SLOT(kpp->quit()), m_part->actionCollection() );

    guiFactory()->addClient(m_part);
}

KHTMLPart *TestKHTML::doc() const
{
    if (m_part)
        return m_part;

    return 0;
}

void TestKHTML::openUrl( const KUrl &url, const KParts::URLArgs &args )
{
    m_part->browserExtension()->setUrlArgs( args );
    m_part->openURL( url );
}

void TestKHTML::openUrl(const KUrl &url)
{
    m_part->openURL(url);
}

void TestKHTML::openUrl(const QString &url)
{
    kDebug() << "url: " << url << endl;
    KUrl parsedUrl(url);
    m_part->openURL(parsedUrl);
}

void TestKHTML::openUrl()
{
    QLineEdit *edit = m_combo->lineEdit();
    if (!edit)
        return;

    QString url = edit->text();
    KUrl parsedUrl(url);
    m_part->openURL(parsedUrl);
}

void TestKHTML::reload()
{
    KParts::URLArgs args; args.reload = true;
    m_part->browserExtension()->setUrlArgs( args );
    m_part->openURL( m_part->url() );
}

void TestKHTML::toggleNavigable(bool s)
{
    m_part->setCaretMode(s);
}

void TestKHTML::toggleEditable(bool s)
{
    kDebug() << "editable: " << s << endl;
    m_part->setEditable(s);
}

static KCmdLineOptions options[] = { { "+file", "url to open", 0 } , KCmdLineLastOption };

int main(int argc, char *argv[])
{
    KCmdLineArgs::init(argc, argv, "testkhtml", "Testkhtml",
            "a basic web browser using the KHTML library", "1.0");
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->count() == 0)
    {
        KCmdLineArgs::usage();
        ::exit( 1 );
    }

    TestKHTML *test = new TestKHTML;
    if (args->url(0).url().right(4).toLower() == ".xml")
    {
        KParts::URLArgs ags(test->doc()->browserExtension()->urlArgs());
        ags.serviceType = "text/xml";
        test->doc()->browserExtension()->setUrlArgs(ags);
    }

    test->openUrl(args->url(0));
    test->show();

//     DOMTreeView * dtv = new DOMTreeView(0, doc, "DomTreeView");
//     dtv->show();
//     dtv->resize(toplevel->width()/2, toplevel->height());

    return app.exec();
}











