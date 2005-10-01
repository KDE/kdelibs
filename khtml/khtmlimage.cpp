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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "khtmlimage.h"
#include "khtmlview.h"
#include "khtml_ext.h"
#include "xml/dom_docimpl.h"
#include "html/html_documentimpl.h"
#include "html/html_elementimpl.h"
#include "rendering/render_image.h"
#include "misc/loader.h"


#include <qtimer.h>

#include <kio/job.h>
#include <kinstance.h>
#include <kmimetype.h>
#include <klocale.h>
#include <kvbox.h>

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
                                                   const char *className, const QStringList & )
{
  KHTMLPart::GUIProfile prof = KHTMLPart::DefaultGUI;
  if ( strcmp( className, "Browser/View" ) == 0 )
    prof = KHTMLPart::BrowserViewGUI;
  return new KHTMLImage( parentWidget, widgetName, parent, name, prof );
}

KHTMLImage::KHTMLImage( QWidget *parentWidget, const char *widgetName,
                        QObject *parent, const char *name, KHTMLPart::GUIProfile prof )
    : KParts::ReadOnlyPart( parent ), m_image( 0 )
{
    KHTMLPart* parentPart = qobject_cast<KHTMLPart*>( parent );
    setInstance( KHTMLImageFactory::instance(), prof == KHTMLPart::BrowserViewGUI && !parentPart );

    KVBox *box = new KVBox( parentWidget/*, widgetName*/ );

    m_khtml = new KHTMLPart( box, widgetName, this, "htmlimagepart", prof );
    m_khtml->setAutoloadImages( true );
    m_khtml->widget()->installEventFilter(this);
    connect( m_khtml->view(), SIGNAL( finishedLayout() ), this, SLOT( restoreScrollPosition() ) );

    setWidget( box );

    // VBox can't take focus, so pass it on to sub-widget
    box->setFocusProxy( m_khtml->widget() );

    m_ext = new KHTMLImageBrowserExtension( this );
    m_ext->setObjectName( "be" );

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
    KHTMLPart *p = qobject_cast<KHTMLPart*>(parent);
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

    // Need to keep a copy of the offsets since they are cleared when emitting completed
    m_xOffset = args.xOffset;
    m_yOffset = args.yOffset;

    m_khtml->begin( m_url );
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

// This can happen after openURL returns, or directly from m_image->ref()
void KHTMLImage::notifyFinished( khtml::CachedObject *o )
{
    if ( !m_image || o != m_image )
        return;

    //const QPixmap &pix = m_image->pixmap();
    QString caption;

    KMimeType::Ptr mimeType;
    if ( !m_mimeType.isEmpty() )
        mimeType = KMimeType::mimeType( m_mimeType );

    if ( mimeType ) {
        if (m_image && !m_image->suggestedTitle().isEmpty()) {
            caption = i18n( "%1 (%2 - %3x%4 Pixels)" ).arg( m_image->suggestedTitle(), mimeType->comment() ).arg( m_image->pixmap_size().width() ).arg( m_image->pixmap_size().height() );
        } else {
            caption = i18n( "%1 - %2x%3 Pixels" ).arg( mimeType->comment() )
                .arg( m_image->pixmap_size().width() ).arg( m_image->pixmap_size().height() );
        }
    } else {
        if (m_image && !m_image->suggestedTitle().isEmpty()) {
            caption = i18n( "%1 (%2x%3 Pixels)" ).arg(m_image->suggestedTitle()).arg( m_image->pixmap_size().width() ).arg( m_image->pixmap_size().height() );
        } else {
            caption = i18n( "Image - %1x%2 Pixels" ).arg( m_image->pixmap_size().width() ).arg( m_image->pixmap_size().height() );
        }
    }

    emit setWindowCaption( caption );
    emit completed();
    emit setStatusBarText(i18n("Done."));
}

void KHTMLImage::restoreScrollPosition()
{
    if ( m_khtml->view()->contentsY() == 0 ) {
        m_khtml->view()->setContentsPos( m_xOffset, m_yOffset );
    }
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
	KHTMLPart *p = qobject_cast<KHTMLPart*>(parent());
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

KHTMLImageBrowserExtension::KHTMLImageBrowserExtension( KHTMLImage *parent )
    : KParts::BrowserExtension( parent )
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


void KHTMLImageBrowserExtension::disableScrolling()
{
    static_cast<KHTMLPartBrowserExtension *>( m_imgPart->doc()->browserExtension() )->disableScrolling();
}

using namespace KParts;

/* vim: et sw=4 ts=4
 */

#include "khtmlimage.moc"
