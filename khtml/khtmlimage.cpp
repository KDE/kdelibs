/* This file is part of the KDE project
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "khtmlimage.h"
#include "khtmlview.h"
#include "khtml_ext.h"
#include "xml/dom_docimpl.h"
#include "html/html_documentimpl.h"
#include "html/html_elementimpl.h"
#include "rendering/render_image.h"
#include "misc/loader.h"

#include <qvbox.h>
#include <qtimer.h>

#include <kio/job.h>
#include <kinstance.h>
#include <kmimetype.h>
#include <klocale.h>

K_EXPORT_COMPONENT_FACTORY( khtmlimagefactory /*NOT the part name, see Makefile.am*/, KHTMLImageFactory )

KInstance *KHTMLImageFactory::s_instance = 0;

KHTMLImageFactory::KHTMLImageFactory()
{
    s_instance = new KInstance( "khtmlimage" );
}

KHTMLImageFactory::~KHTMLImageFactory()
{
    delete s_instance;
}

KParts::Part *KHTMLImageFactory::createPartObject( QWidget *parentWidget, const char *widgetName,
                                                   QObject *parent, const char *name,
                                                   const char *, const QStringList & )
{
    return new KHTMLImage( parentWidget, widgetName, parent, name );
}

KHTMLImage::KHTMLImage( QWidget *parentWidget, const char *widgetName,
                        QObject *parent, const char *name )
    : KParts::ReadOnlyPart( parent, name ), m_image( 0 )
{
    setInstance( KHTMLImageFactory::instance() );

    QVBox *box = new QVBox( parentWidget, widgetName );

    m_khtml = new KHTMLPart( box, widgetName, this, "htmlimagepart", KHTMLPart::BrowserViewGUI );
    m_khtml->setAutoloadImages( true );
    m_khtml->widget()->installEventFilter(this);

    setWidget( box );

    // VBox can't take focus, so pass it on to sub-widget
    box->setFocusProxy( m_khtml->widget() );

    m_ext = new KHTMLImageBrowserExtension( this, "be" );

    // Remove unnecessary actions.
    KAction *encodingAction = actionCollection()->action( "setEncoding" );
    if ( encodingAction )
    {
        encodingAction->unplugAll();
        delete encodingAction;      
    }
    KAction *viewSourceAction= actionCollection()->action( "viewDocumentSource" );
    if ( viewSourceAction )
    {
        viewSourceAction->unplugAll();
        delete viewSourceAction;
    }

    KAction *selectAllAction= actionCollection()->action( "selectAll" );
    if ( selectAllAction )
    {
        selectAllAction->unplugAll();
        delete selectAllAction;
    }

    // forward important signals from the khtml part
    
    // forward opening requests to parent frame (if existing)
    KHTMLPart *p = ::qt_cast<KHTMLPart *>(parent);
    KParts::BrowserExtension *be = p ? p->browserExtension() : m_ext;
    connect(m_khtml->browserExtension(), SIGNAL(openURLRequestDelayed(const KURL &, const KParts::URLArgs &)),
    		be, SIGNAL(openURLRequestDelayed(const KURL &, const KParts::URLArgs &)));

    connect( m_khtml->browserExtension(), SIGNAL( popupMenu( KXMLGUIClient *, const QPoint &, const KURL &,
             const KParts::URLArgs &, KParts::BrowserExtension::PopupFlags, mode_t) ), m_ext, SIGNAL( popupMenu( KXMLGUIClient *, const QPoint &, const KURL &,
             const KParts::URLArgs &, KParts::BrowserExtension::PopupFlags, mode_t) ) );

    connect( m_khtml->browserExtension(), SIGNAL( enableAction( const char *, bool ) ),
             m_ext, SIGNAL( enableAction( const char *, bool ) ) );

    m_ext->setURLDropHandlingEnabled( true );
}

KHTMLImage::~KHTMLImage()
{
    disposeImage();

    // important: delete the html part before the part or qobject destructor runs.
    // we now delete the htmlpart which deletes the part's widget which makes
    // _OUR_ m_widget 0 which in turn avoids our part destructor to delete the
    // widget ;-)
    // ### additional note: it _can_ be that the part has been deleted before:
    // when we're in a html frameset and the view dies first, then it will also
    // kill the htmlpart
    if ( m_khtml )
        delete static_cast<KHTMLPart *>( m_khtml );
}

bool KHTMLImage::openURL( const KURL &url )
{
    static const QString &html = KGlobal::staticQString( "<html><body><img src=\"%1\"></body></html>" );

    disposeImage();

    m_url = url;

    emit started( 0 );

    KParts::URLArgs args = m_ext->urlArgs();
    m_mimeType = args.serviceType;

    emit setWindowCaption( url.prettyURL() );

    m_khtml->begin( m_url, args.xOffset, args.yOffset );
    m_khtml->setAutoloadImages( true );

    DOM::DocumentImpl *impl = dynamic_cast<DOM::DocumentImpl *>( m_khtml->document().handle() ); // ### hack ;-)
    if ( impl && m_ext->urlArgs().reload )
        impl->docLoader()->setCachePolicy( KIO::CC_Reload );

    khtml::DocLoader *dl = impl ? impl->docLoader() : 0;
    m_image = dl->requestImage( m_url.url() );
    if ( m_image )
        m_image->ref( this );

    m_khtml->write( html.arg( m_url.url() ) );
    m_khtml->end();

    /*
    connect( khtml::Cache::loader(), SIGNAL( requestDone( khtml::DocLoader*, khtml::CachedObject *) ),
            this, SLOT( updateWindowCaption() ) );
            */
    return true;
}

bool KHTMLImage::closeURL()
{
    disposeImage();
    return m_khtml->closeURL();
}

void KHTMLImage::notifyFinished( khtml::CachedObject *o )
{
    if ( !m_image || o != m_image )
        return;

    const QPixmap &pix = m_image->pixmap();
    QString caption;

    KMimeType::Ptr mimeType;
    if ( !m_mimeType.isEmpty() )
        mimeType = KMimeType::mimeType( m_mimeType );

    if ( mimeType )
        caption = i18n( "%1 - %2x%3 Pixels" ).arg( mimeType->comment() )
                  .arg( pix.width() ).arg( pix.height() );
    else
        caption = i18n( "Image - %1x%2 Pixels" ).arg( pix.width() ).arg( pix.height() );

    emit setWindowCaption( caption );
    emit completed();
    emit setStatusBarText(i18n("Done."));
}

void KHTMLImage::guiActivateEvent( KParts::GUIActivateEvent *e )
{
    // prevent the base implementation from emitting setWindowCaption with
    // our url. It destroys our pretty, previously caption. Konq saves/restores
    // the caption for us anyway.
    if ( e->activated() )
        return;
    KParts::ReadOnlyPart::guiActivateEvent(e);
}

/*
void KHTMLImage::slotImageJobFinished( KIO::Job *job )
{
    if ( job->error() )
    {
        job->showErrorDialog();
        emit canceled( job->errorString() );
    }
    else
    {
        if ( m_khtml->view()->contentsY() == 0 )
        {
            KParts::URLArgs args = m_ext->urlArgs();
            m_khtml->view()->setContentsPos( args.xOffset, args.yOffset );
        }

        emit completed();

        QTimer::singleShot( 0, this, SLOT( updateWindowCaption() ) );
    }
}

void KHTMLImage::updateWindowCaption()
{
    if ( !m_khtml )
        return;

    DOM::HTMLDocumentImpl *impl = dynamic_cast<DOM::HTMLDocumentImpl *>( m_khtml->document().handle() );
    if ( !impl )
        return;

    DOM::HTMLElementImpl *body = impl->body();
    if ( !body )
        return;

    DOM::NodeImpl *image = body->firstChild();
    if ( !image )
        return;

    khtml::RenderImage *renderImage = dynamic_cast<khtml::RenderImage *>( image->renderer() );
    if ( !renderImage )
        return;

    QPixmap pix = renderImage->pixmap();

    QString caption;

    KMimeType::Ptr mimeType;
    if ( !m_mimeType.isEmpty() )
        mimeType = KMimeType::mimeType( m_mimeType );

    if ( mimeType )
        caption = i18n( "%1 - %2x%3 Pixels" ).arg( mimeType->comment() )
                  .arg( pix.width() ).arg( pix.height() );
    else
        caption = i18n( "Image - %1x%2 Pixels" ).arg( pix.width() ).arg( pix.height() );

    emit setWindowCaption( caption );
    emit completed();
    emit setStatusBarText(i18n("Done."));
}
*/

void KHTMLImage::disposeImage()
{
    if ( !m_image )
        return;

    m_image->deref( this );
    m_image = 0;
}

bool KHTMLImage::eventFilter(QObject *, QEvent *e) {
    switch (e->type()) {
      case QEvent::DragEnter:
      case QEvent::DragMove:
      case QEvent::DragLeave:
      case QEvent::Drop: {
        // find out if this part is embedded in a frame, and send the
	// event to its outside widget
	KHTMLPart *p = ::qt_cast<KHTMLPart *>(parent());
	if (p)
	    return QApplication::sendEvent(p->widget(), e);
        // otherwise simply forward all dnd events to the part widget,
	// konqueror will handle them properly there
        return QApplication::sendEvent(widget(), e);
      }
      default: ;
    }
    return false;
}

KHTMLImageBrowserExtension::KHTMLImageBrowserExtension( KHTMLImage *parent, const char *name )
    : KParts::BrowserExtension( parent, name )
{
    m_imgPart = parent;
}

int KHTMLImageBrowserExtension::xOffset()
{
    return m_imgPart->doc()->view()->contentsX();
}

int KHTMLImageBrowserExtension::yOffset()
{
    return m_imgPart->doc()->view()->contentsY();
}

void KHTMLImageBrowserExtension::print()
{
    static_cast<KHTMLPartBrowserExtension *>( m_imgPart->doc()->browserExtension() )->print();
}

void KHTMLImageBrowserExtension::reparseConfiguration()
{
    static_cast<KHTMLPartBrowserExtension *>( m_imgPart->doc()->browserExtension() )->reparseConfiguration();
    m_imgPart->doc()->setAutoloadImages( true );
}

using namespace KParts;

/* vim: et sw=4 ts=4
 */

#include "khtmlimage.moc"
