// program to test the new khtml implementation

#include "testkhtml.h"
#include "testkhtml.moc"

#include <stdlib.h>
#include "kapplication.h"
#include "khtmlview.h"
#include "html_document.h"
#include "htmltokenizer.h"
// to be able to delete a static protected member pointer in kbrowser...
// just for memory debugging
#define protected public
#include "khtml_part.h"
#undef protected
#include "misc/loader.h"
#include <QtGui/QCursor>
#include <QDomDocument>
#include <dom_string.h>
#include "dom/dom2_range.h"
#include "dom/html_document.h"
#include "dom/dom_exception.h"
#include <stdio.h>
#include "css/cssstyleselector.h"
#include "html/html_imageimpl.h"
#include "rendering/render_style.h"
#include "khtml_global.h"
#include <kxmlguiwindow.h>
#include <kcmdlineargs.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <kicon.h>
#include "kxmlguifactory.h"

int main(int argc, char *argv[])
{
    KCmdLineOptions options;
    options.add("+file", ki18n("URL to open"));

    KCmdLineArgs::init(argc, argv, "testkhtml", 0, ki18n("Testkhtml"),
            "1.0", ki18n("a basic web browser using the KHTML library"));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication a;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs( );
    if ( args->count() == 0 ) {
	KCmdLineArgs::usage();
	::exit( 1 );
    }

    KHTMLGlobal *fac = new KHTMLGlobal;

    KXmlGuiWindow *toplevel = new KXmlGuiWindow();
    KHTMLPart *doc = new KHTMLPart( toplevel, toplevel, KHTMLPart::BrowserViewGUI );

    Dummy *dummy = new Dummy( doc );
    QObject::connect( doc->browserExtension(), SIGNAL( openUrlRequest(const KUrl&, const KParts::OpenUrlArguments&, const KParts::BrowserArguments &) ),
		      dummy, SLOT( slotOpenURL( const KUrl&, const KParts::OpenUrlArguments&, const KParts::BrowserArguments& ) ) );

    if (args->url(0).url().right(4).toLower() == ".xml") {
        KParts::OpenUrlArguments args(doc->arguments());
        args.setMimeType("text/xml");
        doc->setArguments(args);
    }

    doc->openUrl( args->url(0) );

    toplevel->setCentralWidget( doc->widget() );
    toplevel->resize( 800, 600);

    QDomDocument d = doc->domDocument();
    QDomElement viewMenu = d.documentElement().firstChild().childNodes().item( 2 ).toElement();
    QDomElement e = d.createElement( "action" );
    e.setAttribute( "name", "debugRenderTree" );
    viewMenu.appendChild( e );
    e = d.createElement( "action" );
    e.setAttribute( "name", "debugDOMTree" );
    viewMenu.appendChild( e );
    QDomElement toolBar = d.documentElement().firstChild().nextSibling().toElement();
    e = d.createElement( "action" );
    e.setAttribute( "name", "editable" );
    toolBar.insertBefore( e, toolBar.firstChild() );
    e = d.createElement( "action" );
    e.setAttribute( "name", "navigable" );
    toolBar.insertBefore( e, toolBar.firstChild() );
    e = d.createElement( "action" );
    e.setAttribute( "name", "reload" );
    toolBar.insertBefore( e, toolBar.firstChild() );
    e = d.createElement( "action" );
    e.setAttribute( "name", "print" );
    toolBar.insertBefore( e, toolBar.firstChild() );

    KAction *action = new KAction(KIcon("view-refresh"),  "Reload", doc );
    doc->actionCollection()->addAction( "reload", action );
    QObject::connect(action, SIGNAL(triggered(bool)), dummy, SLOT( reload() ));
    action->setShortcut(Qt::Key_F5);
    KAction *kprint = new KAction(KIcon("document-print"),  "Print", doc );
    doc->actionCollection()->addAction( "print", kprint );
    QObject::connect(kprint, SIGNAL(triggered(bool)), doc->browserExtension(), SLOT( print() ));
    kprint->setEnabled(true);
    KToggleAction *ta = new KToggleAction( KIcon("edit-rename"), "Navigable", doc );
    doc->actionCollection()->addAction( "navigable", ta );
    ta->setShortcuts( KShortcut() );
    ta->setChecked(doc->isCaretMode());
    QWidget::connect(ta, SIGNAL(toggled(bool)), dummy, SLOT( toggleNavigable(bool) ));
    ta = new KToggleAction( KIcon("document-properties"), "Editable", doc );
    doc->actionCollection()->addAction( "editable", ta );
    ta->setShortcuts( KShortcut() );
    ta->setChecked(doc->isEditable());
    QWidget::connect(ta, SIGNAL(toggled(bool)), dummy, SLOT( toggleEditable(bool) ));
    toplevel->guiFactory()->addClient( doc );

    doc->setJScriptEnabled(true);
    doc->setJavaEnabled(true);
    doc->setPluginsEnabled( true );
    doc->setURLCursor(QCursor(Qt::PointingHandCursor));
    a.setTopWidget(doc->widget());
    QWidget::connect(doc, SIGNAL(setWindowCaption(const QString &)),
		     doc->widget()->topLevelWidget(), SLOT(setCaption(const QString &)));
    doc->widget()->show();
    toplevel->show();
    doc->view()->viewport()->show();
    doc->view()->widget()->show();


    int ret = a.exec();
    return ret;
}

