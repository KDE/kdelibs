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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
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


#include <QtCore/QTimer>

#include <kjobuidelegate.h>
#include <kio/job.h>
#include <kcomponentdata.h>
#include <kmimetype.h>
#include <klocale.h>
#include <kvbox.h>
#include <kactioncollection.h>

// Export init_khtmlimagefactory. This way we don't have to export the whole class
// just for khtmlimagepart. See khtmlimage_init.cpp
K_EXPORT_COMPONENT_FACTORY( khtmlimagefactory, KHTMLImageFactory )

KComponentData *KHTMLImageFactory::s_componentData = 0;

KHTMLImageFactory::KHTMLImageFactory()
{
    s_componentData = new KComponentData( "khtmlimage" );
}

KHTMLImageFactory::~KHTMLImageFactory()
{
    delete s_componentData;
}

KParts::Part *KHTMLImageFactory::createPartObject( QWidget *parentWidget,
                                                   QObject *parent,
                                                   const char *className, const QStringList &args )
{
  KHTMLPart::GUIProfile prof = KHTMLPart::DefaultGUI;
    if ( strcmp( className, "Browser/View" ) == 0 ) // old hack
        prof = KHTMLPart::BrowserViewGUI;
    if (args.contains("Browser/View"))
        prof = KHTMLPart::BrowserViewGUI;
  return new KHTMLImage( parentWidget, parent, prof );
}

KHTMLImage::KHTMLImage( QWidget *parentWidget,
                        QObject *parent, KHTMLPart::GUIProfile prof )
    : KParts::ReadOnlyPart( parent ), m_image( 0 )
{
    KHTMLPart* parentPart = qobject_cast<KHTMLPart*>( parent );
    setComponentData( KHTMLImageFactory::componentData(), prof == KHTMLPart::BrowserViewGUI && !parentPart );

    KVBox *box = new KVBox( parentWidget );
    box->setAcceptDrops( true );

    m_khtml = new KHTMLPart( box, this, prof );
    m_khtml->setAutoloadImages( true );

    // We do not want our subpart to be destroyed when its widget is,
    // since that may cause all KHTMLParts to die when we're dealing
    // with 
    m_khtml->setAutoDeletePart( false );

    connect( m_khtml->view(), SIGNAL( finishedLayout() ), this, SLOT( restoreScrollPosition() ) );

    setWidget( box );

    // VBox can't take focus, so pass it on to sub-widget
    box->setFocusProxy( m_khtml->widget() );

    m_ext = new KHTMLImageBrowserExtension( this );
    m_ext->setObjectName( "be" );

    // Remove unnecessary actions.
    delete actionCollection()->action( "setEncoding" );
    delete actionCollection()->action( "viewDocumentSource" );
    delete actionCollection()->action( "selectAll" );

    // forward important signals from the khtml part

    // forward opening requests to parent frame (if existing)
    KHTMLPart *p = qobject_cast<KHTMLPart*>(parent);
    KParts::BrowserExtension *be = p ? p->browserExtension() : m_ext;
    connect(m_khtml->browserExtension(), SIGNAL(openUrlRequestDelayed(const KUrl &, const KParts::OpenUrlArguments&, const KParts::BrowserArguments &)),
    		be, SIGNAL(openUrlRequestDelayed(const KUrl &, const KParts::OpenUrlArguments&, const KParts::BrowserArguments &)));

    connect(m_khtml->browserExtension(), SIGNAL(popupMenu(QPoint,KUrl,mode_t,KParts::OpenUrlArguments,KParts::BrowserArguments,KParts::BrowserExtension::PopupFlags,KParts::BrowserExtension::ActionGroupMap)),
            this, SLOT(slotPopupMenu(QPoint,KUrl,mode_t,KParts::OpenUrlArguments,KParts::BrowserArguments,KParts::BrowserExtension::PopupFlags,KParts::BrowserExtension::ActionGroupMap)));

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

bool KHTMLImage::openUrl( const KUrl &url )
{
    static const QString &html = KGlobal::staticQString( "<html><body><img src=\"%1\"></body></html>" );

    disposeImage();

    setUrl(url);

    emit started( 0 );

    KParts::OpenUrlArguments args = arguments();
    m_mimeType = args.mimeType();

    emit setWindowCaption( url.prettyUrl() );

    // Need to keep a copy of the offsets since they are cleared when emitting completed
    m_xOffset = args.xOffset();
    m_yOffset = args.yOffset();

    m_khtml->begin( this->url() );
    m_khtml->setAutoloadImages( true );

    DOM::DocumentImpl *impl = dynamic_cast<DOM::DocumentImpl *>( m_khtml->document().handle() ); // ### hack ;-)
    if (!impl) return false;

    if ( arguments().reload() )
        impl->docLoader()->setCachePolicy( KIO::CC_Reload );

    khtml::DocLoader *dl = impl->docLoader();
    m_image = dl->requestImage( this->url().url() );
    if ( m_image )
        m_image->ref( this );

    m_khtml->write( html.arg( this->url().url() ) );
    m_khtml->end();

    /*
    connect( khtml::Cache::loader(), SIGNAL( requestDone( khtml::DocLoader*, khtml::CachedObject *) ),
            this, SLOT( updateWindowCaption() ) );
            */
    return true;
}

bool KHTMLImage::closeUrl()
{
    disposeImage();
    return m_khtml->closeUrl();
}

// This can happen after openUrl returns, or directly from m_image->ref()
void KHTMLImage::notifyFinished( khtml::CachedObject *o )
{
    if ( !m_image || o != m_image )
        return;

    //const QPixmap &pix = m_image->pixmap();
    QString caption;

    KMimeType::Ptr mimeType;
    if ( !m_mimeType.isEmpty() )
        mimeType = KMimeType::mimeType(m_mimeType, KMimeType::ResolveAliases);

    if ( mimeType ) {
        if (m_image && !m_image->suggestedTitle().isEmpty()) {
            caption = i18n( "%1 (%2 - %3x%4 Pixels)", m_image->suggestedTitle(), mimeType->comment(), m_image->pixmap_size().width(), m_image->pixmap_size().height() );
        } else {
            caption = i18n( "%1 - %2x%3 Pixels" ,  mimeType->comment() ,
                  m_image->pixmap_size().width() ,  m_image->pixmap_size().height() );
        }
    } else {
        if (m_image && !m_image->suggestedTitle().isEmpty()) {
            caption = i18n( "%1 (%2x%3 Pixels)" , m_image->suggestedTitle(),  m_image->pixmap_size().width() ,  m_image->pixmap_size().height() );
        } else {
            caption = i18n( "Image - %1x%2 Pixels" ,  m_image->pixmap_size().width() ,  m_image->pixmap_size().height() );
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
        job->uiDelegate()->showErrorMessage();
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
        mimeType = KMimeType::mimeType( m_mimeType, KMimeType::ResolveAliases );

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

void KHTMLImage::slotPopupMenu( const QPoint &global, const KUrl &url, mode_t mode,
                                const KParts::OpenUrlArguments &origArgs,
                                const KParts::BrowserArguments &browserArgs,
                                KParts::BrowserExtension::PopupFlags flags,
                                const KParts::BrowserExtension::ActionGroupMap& actionGroups )
{
    KParts::OpenUrlArguments args = origArgs;
    args.setMimeType(m_mimeType);
    m_ext->popupMenu(global, url, mode, args, browserArgs, flags, actionGroups);
}

#include "khtmlimage.moc"
