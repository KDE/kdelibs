// programm to test the new khtml implementation

#include <stdlib.h>
#include "decoder.h"
#include "kapplication.h"
#include "html_document.h"
#include "htmltokenizer.h"
// to be able to delete a static protected member pointer in kbrowser...
// just for memory debugging
#define protected public
#include "khtml_part.h"
#undef protected
#include "testkhtml.h"
#include "testkhtml.moc"
#include "misc/loader.h"
#include <qcursor.h>
#include <dom_string.h>
#include "dom/dom2_range.h"
#include "dom/html_document.h"
#include "dom/dom_exception.h"
#include <stdio.h>
#include <khtml_factory.h>
#include <css/cssstyleselector.h>
#include <html/html_imageimpl.h>
#include <rendering/render_style.h>
#include <kmainwindow.h>
#include <kcmdlineargs.h>
#include <kaction.h>
#include "domtreeview.h"

static KCmdLineOptions options[] = { { "+file", "url to open", 0 } , {0, 0, 0} };

int main(int argc, char *argv[])
{

    KCmdLineArgs::init(argc, argv, "Testkhtml", "a basic web browser using the KHTML library", "1.0");
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication a;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs( );
    if ( args->count() == 0 ) {
	KCmdLineArgs::usage();
	::exit( 1 );
    }

    KHTMLFactory *fac = new KHTMLFactory();

    KMainWindow *toplevel = new KMainWindow();
    KHTMLPart *doc = new KHTMLPart( toplevel, 0, toplevel, 0, KHTMLPart::BrowserViewGUI );

    Dummy *dummy = new Dummy( doc );
    QObject::connect( doc->browserExtension(), SIGNAL( openURLRequest( const KURL &, const KParts::URLArgs & ) ),
		      dummy, SLOT( slotOpenURL( const KURL&, const KParts::URLArgs & ) ) );

    if (args->url(0).url().right(4).find(".xml", 0, false) == 0) {
        KParts::URLArgs ags(doc->browserExtension()->urlArgs());
        ags.serviceType = "text/xml";
        doc->browserExtension()->setURLArgs(ags);
    }

    doc->openURL( args->url(0) );

//     DOMTreeView * dtv = new DOMTreeView(0, doc, "DomTreeView");
//     dtv->show();

    toplevel->setCentralWidget( doc->widget() );
    toplevel->resize( 640, 800);

//     dtv->resize(toplevel->width()/2, toplevel->height());

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
    e.setAttribute( "name", "reload" );
    toolBar.insertBefore( e, toolBar.firstChild() );
    e = d.createElement( "action" );
    e.setAttribute( "name", "print" );
    toolBar.insertBefore( e, toolBar.firstChild() );

    (void)new KAction( "Reload", "reload", Qt::Key_F5, dummy, SLOT( reload() ), doc->actionCollection(), "reload" );
    KAction* kprint = new KAction( "Print", "print", 0, doc->browserExtension(), SLOT( print() ), doc->actionCollection(), "print" );
    kprint->setEnabled(true);

    toplevel->guiFactory()->addClient( doc );

    doc->setJScriptEnabled(true);
    doc->setJavaEnabled(true);
    doc->setURLCursor(QCursor(Qt::PointingHandCursor));
    a.setTopWidget(doc->widget());
    QWidget::connect(doc, SIGNAL(setWindowCaption(const QString &)),
		     doc->widget(), SLOT(setCaption(const QString &)));
    doc->widget()->show();
    toplevel->show();
    ((QScrollView *)doc->widget())->viewport()->show();


    int ret = a.exec();
    
    //delete doc;
    //delete dtv;

    khtml::Cache::clear();
    khtml::CSSStyleSelector::clear();
    khtml::RenderStyle::cleanup();

    delete fac;
    return ret;
}

