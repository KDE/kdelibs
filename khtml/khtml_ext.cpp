
#include <assert.h>
#include "khtml_ext.h"
#include "khtmlview.h"
#include "khtml_pagecache.h"
#include "rendering/render_form.h"
#include "dom/html_form.h"
#include "dom/html_image.h"
#include <qclipboard.h>
#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qmetaobject.h>
#include <private/qucomextra_p.h>

#include <kdebug.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kio/job.h>
#include <ktoolbarbutton.h>
#include <ktoolbar.h>
#include <ksavefile.h>
#include <kurldrag.h>
#include <kstringhandler.h>
#include <kapplication.h>
#include <kmessagebox.h>   
#include <kstandarddirs.h> 
#include <krun.h>          

#include "dom/dom_element.h"
#include "misc/htmltags.h"

KHTMLPartBrowserExtension::KHTMLPartBrowserExtension( KHTMLPart *parent, const char *name )
: KParts::BrowserExtension( parent, name )
{
    m_part = parent;
    setURLDropHandlingEnabled( true );

    enableAction( "cut", false );
    enableAction( "copy", false );
    enableAction( "paste", false );

    m_connectedToClipboard = false;
}

int KHTMLPartBrowserExtension::xOffset()
{
    return m_part->view()->contentsX();
}

int KHTMLPartBrowserExtension::yOffset()
{
  return m_part->view()->contentsY();
}

void KHTMLPartBrowserExtension::saveState( QDataStream &stream )
{
  kdDebug( 6050 ) << "saveState!" << endl;
  m_part->saveState( stream );
}

void KHTMLPartBrowserExtension::restoreState( QDataStream &stream )
{
  kdDebug( 6050 ) << "restoreState!" << endl;
  m_part->restoreState( stream );
}

void KHTMLPartBrowserExtension::editableWidgetFocused( QWidget *widget )
{
    m_editableFormWidget = widget;
    updateEditActions();

    if ( !m_connectedToClipboard && m_editableFormWidget )
    {
        connect( QApplication::clipboard(), SIGNAL( dataChanged() ),
                 this, SLOT( updateEditActions() ) );

        if ( m_editableFormWidget->inherits( "QLineEdit" ) )
            connect( m_editableFormWidget, SIGNAL( textChanged( const QString & ) ),
                     this, SLOT( updateEditActions() ) );
        else if ( m_editableFormWidget->inherits( "QMultiLineEdit" ) )
            connect( m_editableFormWidget, SIGNAL( textChanged() ),
                     this, SLOT( updateEditActions() ) );

        m_connectedToClipboard = true;
    }
}

void KHTMLPartBrowserExtension::editableWidgetBlurred( QWidget *widget )
{
    QWidget *oldWidget = m_editableFormWidget;

    m_editableFormWidget = widget;
    enableAction( "cut", false );
    enableAction( "paste", false );
    m_part->emitSelectionChanged();

    if ( m_connectedToClipboard )
    {
        disconnect( QApplication::clipboard(), SIGNAL( dataChanged() ),
                    this, SLOT( updateEditActions() ) );

        if ( oldWidget )
        {
            if ( oldWidget->inherits( "QLineEdit" ) )
                disconnect( oldWidget, SIGNAL( textChanged( const QString & ) ),
                            this, SLOT( updateEditActions() ) );
            else if ( oldWidget->inherits( "QMultiLineEdit" ) )
                disconnect( oldWidget, SIGNAL( textChanged() ),
                            this, SLOT( updateEditActions() ) );
        }

        m_connectedToClipboard = false;
    }
}

void KHTMLPartBrowserExtension::setExtensionProxy( KParts::BrowserExtension *proxy )
{
    if ( m_extensionProxy )
        disconnect( m_extensionProxy, SIGNAL( enableAction( const char *, bool ) ),
                    this, SLOT( extensionProxyActionEnabled( const char *, bool ) ) );

    m_extensionProxy = proxy;

    if ( m_extensionProxy )
    {
        connect( m_extensionProxy, SIGNAL( enableAction( const char *, bool ) ),
                 this, SLOT( extensionProxyActionEnabled( const char *, bool ) ) );

        enableAction( "cut", m_extensionProxy->isActionEnabled( "cut" ) );
        enableAction( "copy", m_extensionProxy->isActionEnabled( "copy" ) );
        enableAction( "paste", m_extensionProxy->isActionEnabled( "paste" ) );
    }
    else
    {
        updateEditActions();
        enableAction( "copy", false ); // ### re-check this
    }
}

void KHTMLPartBrowserExtension::cut()
{
    if ( m_extensionProxy )
    {
        callExtensionProxyMethod( "cut()" );
        return;
    }

    assert( m_editableFormWidget );
    if ( !m_editableFormWidget )
        return; // shouldn't happen

    if ( m_editableFormWidget->inherits( "QLineEdit" ) )
        static_cast<QLineEdit *>( &(*m_editableFormWidget) )->cut();
    else if ( m_editableFormWidget->inherits( "QTextEdit" ) )
        static_cast<QTextEdit *>( &(*m_editableFormWidget) )->cut();
}

void KHTMLPartBrowserExtension::copy()
{
    if ( m_extensionProxy )
    {
        callExtensionProxyMethod( "copy()" );
        return;
    }

    kdDebug( 6050 ) << "************! KHTMLPartBrowserExtension::copy()" << endl;
    if ( !m_editableFormWidget )
    {
        // get selected text and paste to the clipboard
        QString text = m_part->selectedText();
        QClipboard *cb = QApplication::clipboard();
        disconnect( cb, SIGNAL( selectionChanged() ), m_part, SLOT( slotClearSelection() ) );
        cb->setText(text);
        connect( cb, SIGNAL( selectionChanged() ), m_part, SLOT( slotClearSelection() ) );
    }
    else
    {
        if ( m_editableFormWidget->inherits( "QLineEdit" ) )
            static_cast<QLineEdit *>( &(*m_editableFormWidget) )->copy();
        else if ( m_editableFormWidget->inherits( "QTextEdit" ) )
            static_cast<QTextEdit *>( &(*m_editableFormWidget) )->copy();
    }
}

void KHTMLPartBrowserExtension::paste()
{
    if ( m_extensionProxy )
    {
        callExtensionProxyMethod( "paste()" );
        return;
    }

    assert( m_editableFormWidget );
    if ( !m_editableFormWidget )
        return; // shouldn't happen

    if ( m_editableFormWidget->inherits( "QLineEdit" ) )
        static_cast<QLineEdit *>( &(*m_editableFormWidget) )->paste();
    else if ( m_editableFormWidget->inherits( "QTextEdit" ) )
        static_cast<QTextEdit *>( &(*m_editableFormWidget) )->paste();
}

void KHTMLPartBrowserExtension::callExtensionProxyMethod( const char *method )
{
    if ( !m_extensionProxy )
        return;

    int slot = m_extensionProxy->metaObject()->findSlot( method );
    if ( slot == -1 )
        return;

    QUObject o[ 1 ];
    m_extensionProxy->qt_invoke( slot, o );
}

void KHTMLPartBrowserExtension::updateEditActions()
{
    if ( !m_editableFormWidget )
    {
        enableAction( "cut", false );
        enableAction( "paste", false );
        return;
    }

    // ### duplicated from KonqMainWindow::slotClipboardDataChanged
#ifndef QT_NO_MIMECLIPBOARD // Handle minimalized versions of Qt Embedded
    QMimeSource *data = QApplication::clipboard()->data();
    enableAction( "paste", data->provides( "text/plain" ) );
#else
    QString data=QApplication::clipboard()->text();
    enableAction( "paste", data.contains("://"));
#endif

    bool hasSelection = false;

    if ( m_editableFormWidget->inherits( "QLineEdit" ) )
        hasSelection = static_cast<QLineEdit *>( &(*m_editableFormWidget) )->hasSelectedText();
    else if ( m_editableFormWidget->inherits( "khtml::TextAreaWidget" ) )
        hasSelection = static_cast<khtml::TextAreaWidget *>( &(*m_editableFormWidget) )->hasSelectedText();

    enableAction( "copy", hasSelection );
    enableAction( "cut", hasSelection );
}

void KHTMLPartBrowserExtension::extensionProxyActionEnabled( const char *action, bool enable )
{
    // only forward enableAction calls for actions we actually do foward
    if ( strcmp( action, "cut" ) == 0 ||
         strcmp( action, "copy" ) == 0 ||
         strcmp( action, "paste" ) == 0 )
        enableAction( action, enable );
}

void KHTMLPartBrowserExtension::reparseConfiguration()
{
  m_part->reparseConfiguration();
}

void KHTMLPartBrowserExtension::print()
{
  m_part->view()->print();
}

class KHTMLPopupGUIClient::KHTMLPopupGUIClientPrivate
{
public:
  KHTMLPart *m_khtml;
  KURL m_url;
  KURL m_imageURL;
  KAction *m_paPrintFrame;
  KAction *m_paSaveLinkAs;
  KAction *m_paSaveImageAs;
  KAction *m_paCopyLinkLocation;
  KAction *m_paStopAnimations;
  KAction *m_paCopyImageLocation;
  KAction *m_paViewImage;
  KAction *m_paReloadFrame;
  KAction *m_paViewFrameSource;
  KAction *m_paViewFrameInfo;
};


KHTMLPopupGUIClient::KHTMLPopupGUIClient( KHTMLPart *khtml, const QString &doc, const KURL &url )
{
  d = new KHTMLPopupGUIClientPrivate;
  d->m_khtml = khtml;
  d->m_url = url;

  setInstance( khtml->instance() );

  actionCollection()->insert( khtml->actionCollection()->action( "selectAll" ) );
  actionCollection()->insert( khtml->actionCollection()->action( "security" ) );
  actionCollection()->insert( khtml->actionCollection()->action( "viewDocumentSource" ) );

  // frameset? -> add "Reload Frame" etc.
  if ( khtml->parentPart() )
  {
    d->m_paReloadFrame = new KAction( i18n( "Reload Frame" ), 0, this, SLOT( slotReloadFrame() ),
                                      actionCollection(), "reloadframe" );
    d->m_paViewFrameSource = new KAction( i18n( "View Frame Source" ), 0, d->m_khtml, SLOT( slotViewDocumentSource() ),
                                          actionCollection(), "viewFrameSource" );
    d->m_paViewFrameInfo = new KAction( i18n( "View Frame Information" ), 0, d->m_khtml, SLOT( slotViewPageInfo() ), actionCollection(), "viewPageInfo" );
    // This one isn't in khtml_popupmenu.rc anymore, because Print isn't either,
    // and because print frame is already in the toolbar and the menu.
    // But leave this here, so that it's easy to readd it.
    d->m_paPrintFrame = new KAction( i18n( "Print Frame..." ), "fileprint", 0, d->m_khtml->browserExtension(), SLOT( print() ), actionCollection(), "printFrame" );
  } else {
    actionCollection()->insert( khtml->actionCollection()->action( "viewPageInfo" ) );
  }

  actionCollection()->insert( khtml->actionCollection()->action( "setEncoding" ) );

  if ( !url.isEmpty() )
  {
    d->m_paSaveLinkAs = new KAction( i18n( "&Save Link As..." ), 0, this, SLOT( slotSaveLinkAs() ),
                                     actionCollection(), "savelinkas" );
    d->m_paCopyLinkLocation = new KAction( i18n( "Copy Link Location" ), 0, this, SLOT( slotCopyLinkLocation() ),
                                           actionCollection(), "copylinklocation" );
  }

  d->m_paStopAnimations = new KAction( i18n( "Stop Animations" ), 0, this, SLOT( slotStopAnimations() ),
                                       actionCollection(), "stopanimations" );

  DOM::Element e;
  e = khtml->nodeUnderMouse();

  if ( !e.isNull() && (e.elementId() == ID_IMG ||
                       (e.elementId() == ID_INPUT && !static_cast<DOM::HTMLInputElement>(e).src().isEmpty())))
  {
      if ( e.elementId() == ID_IMG )
          d->m_imageURL = KURL( static_cast<DOM::HTMLImageElement>( e ).src().string() );
      else
          d->m_imageURL = KURL( static_cast<DOM::HTMLInputElement>( e ).src().string() );
    d->m_paSaveImageAs = new KAction( i18n( "Save Image As..." ), 0, this, SLOT( slotSaveImageAs() ),
                                      actionCollection(), "saveimageas" );
    d->m_paCopyImageLocation = new KAction( i18n( "Copy Image Location" ), 0, this, SLOT( slotCopyImageLocation() ),
                                            actionCollection(), "copyimagelocation" );
    QString name = KStringHandler::csqueeze(d->m_imageURL.fileName()+d->m_imageURL.query(), 25);
    d->m_paViewImage = new KAction( i18n( "View Image (%1)" ).arg(name.replace(QRegExp("&"), "&&")), 0, this, SLOT( slotViewImage() ),
                                            actionCollection(), "viewimage" );
  }

  setXML( doc );
  setDOMDocument( QDomDocument(), true ); // ### HACK

  QDomElement menu = domDocument().documentElement().namedItem( "Menu" ).toElement();

  if ( actionCollection()->count() > 0 )
    menu.insertBefore( domDocument().createElement( "separator" ), menu.firstChild() );
}

KHTMLPopupGUIClient::~KHTMLPopupGUIClient()
{
  delete d;
}

void KHTMLPopupGUIClient::slotSaveLinkAs()
{
  KIO::MetaData metaData;
  metaData["referrer"] = d->m_khtml->referrer();
  saveURL( d->m_khtml->widget(), i18n( "Save Link As" ), d->m_url, metaData );
}

void KHTMLPopupGUIClient::slotSaveImageAs()
{
  KIO::MetaData metaData;
  metaData["referrer"] = d->m_khtml->referrer();
  saveURL( d->m_khtml->widget(), i18n( "Save Image As" ), d->m_imageURL, metaData );
}

void KHTMLPopupGUIClient::slotCopyLinkLocation()
{
  KURL::List lst;
  lst.append( d->m_url );
#ifndef QT_NO_MIMECLIPBOARD
  // Set it in both the mouse selection and in the clipboard
  QApplication::clipboard()->setSelectionMode(true);
  QApplication::clipboard()->setData( KURLDrag::newDrag( lst ) );
  QApplication::clipboard()->setSelectionMode(false);
  QApplication::clipboard()->setData( KURLDrag::newDrag( lst ) );
#else
  QApplication::clipboard()->setText( d->m_url.url() ); //FIXME(E): Handle multiple entries
#endif
}

void KHTMLPopupGUIClient::slotStopAnimations()
{
  d->m_khtml->stopAnimations();
}

void KHTMLPopupGUIClient::slotCopyImageLocation()
{
  KURL::List lst;
  lst.append( d->m_imageURL );
#ifndef QT_NO_MIMECLIPBOARD
  // Set it in both the mouse selection and in the clipboard
  QApplication::clipboard()->setSelectionMode(true);
  QApplication::clipboard()->setData( KURLDrag::newDrag( lst ) );
  QApplication::clipboard()->setSelectionMode(false);
  QApplication::clipboard()->setData( KURLDrag::newDrag( lst ) );
#else
  QApplication::clipboard()->setText(d->m_imageURL.url()); //FIXME(E): Handle multiple entries
#endif
}

void KHTMLPopupGUIClient::slotViewImage()
{
  d->m_khtml->browserExtension()->createNewWindow(d->m_imageURL.url());
}

void KHTMLPopupGUIClient::slotReloadFrame()
{
  KParts::URLArgs args( d->m_khtml->browserExtension()->urlArgs() );
  args.reload = true;
  // reload document
  d->m_khtml->closeURL();
  d->m_khtml->browserExtension()->setURLArgs( args );
  d->m_khtml->openURL( d->m_khtml->url() );
}

void KHTMLPopupGUIClient::saveURL( QWidget *parent, const QString &caption,
                                   const KURL &url,
                                   const QMap<QString, QString> &metadata,
                                   const QString &filter, long cacheId,
                                   const QString & suggestedFilename )
{
  QString name = QString::fromLatin1( "index.html" );
  if ( !suggestedFilename.isEmpty() )
    name = suggestedFilename;
  else if ( !url.fileName().isEmpty() )
    name = url.fileName();
  
  KURL destURL;
  int query;
  do {
    query = KMessageBox::Yes;
    destURL = KFileDialog::getSaveURL( name, filter, parent, caption );
      if( destURL.isLocalFile() )
      {
        QFileInfo info( destURL.path() );
        if( info.exists() )
          query = KMessageBox::warningContinueCancel( parent, i18n( "A file named \"%1\" already exists. " "Are you sure you want to overwrite it?" ).arg( info.fileName() ), i18n( "Overwrite File?" ), i18n( "Overwrite" ) );
       }
   } while ( query == KMessageBox::Cancel );
  
  if ( !destURL.isMalformed() )
    saveURL(url, destURL, metadata, cacheId);
}

void KHTMLPopupGUIClient::saveURL( const KURL &url, const KURL &destURL,
                                   const QMap<QString, QString> &metadata,
                                   long cacheId )
{
    if ( !destURL.isMalformed() )
    {
        bool saved = false;
        if (KHTMLPageCache::self()->isValid(cacheId))
        {
            if (destURL.isLocalFile())
            {
                KSaveFile destFile(destURL.path());
                if (destFile.status() == 0)
                {
                    KHTMLPageCache::self()->saveData(cacheId, destFile.dataStream());
                    saved = true;
                }
            }
            else
            {
                // save to temp file, then move to final destination.
                KTempFile destFile;
                if (destFile.status() == 0)
                {
                    KHTMLPageCache::self()->saveData(cacheId, destFile.dataStream());
                    destFile.close();
                    KURL url2 = KURL();
                    url2.setPath(destFile.name());
                    KIO::move(url2, destURL);
                    saved = true;
                }
            }
        }
        if(!saved)
        {
            // DownloadManager <-> konqueror integration
            // find if the integration is enabled 
            // the empty key  means no integration
            KConfig *cfg = new KConfig("konquerorrc", false, false);
            cfg->setGroup("HTML Settings");
            QString downloadManger=cfg->readEntry("DownloadManager");
            if (!downloadManger.isEmpty())
            {
                // then find the download manager location
                kdDebug(1000) << "Using: "<<downloadManger <<" as Download Manager" <<endl;
                QString cmd=KStandardDirs::findExe(downloadManger);
                if (cmd.isEmpty())
                {
                    QString errMsg=i18n("The Download Manager (%1) could not be found in your $PATH ").arg(downloadManger);
                    QString errMsgEx= i18n("Try to reinstall it  \n\nThe integration with Konqueror will be disabled!");
                    KMessageBox::detailedSorry(0,errMsg,errMsgEx);
                    cfg->writeEntry("DownloadManager",QString::null);
                    cfg->sync ();
                }
                else
                {
                    cmd+=" " + url.url()+" "+destURL.url();
                    kdDebug(1000) << "Calling command  "<<cmd<<endl;
                    KRun::runCommand(cmd);
                }
            }
            else
            {
                KIO::Job *job = KIO::copy( url, destURL );
                job->setMetaData(metadata);
                job->addMetaData("MaxCacheSize", "0"); // Don't store in http cache.
                job->setAutoErrorHandlingEnabled( true );
            } 
            delete cfg;
        } //end if(!saved)
    }
}

KHTMLPartBrowserHostExtension::KHTMLPartBrowserHostExtension( KHTMLPart *part )
: KParts::BrowserHostExtension( part )
{
  m_part = part;
}

KHTMLPartBrowserHostExtension::~KHTMLPartBrowserHostExtension()
{
}

QStringList KHTMLPartBrowserHostExtension::frameNames() const
{
  return m_part->frameNames();
}

const QPtrList<KParts::ReadOnlyPart> KHTMLPartBrowserHostExtension::frames() const
{
  return m_part->frames();
}

bool KHTMLPartBrowserHostExtension::openURLInFrame( const KURL &url, const KParts::URLArgs &urlArgs )
{
  return m_part->openURLInFrame( url, urlArgs );
}

KHTMLZoomFactorAction::KHTMLZoomFactorAction( KHTMLPart *part, bool direction, const QString &text, const QString &icon, const QObject *receiver, const char *slot, QObject *parent, const char *name )
    : KAction( text, icon, 0, receiver, slot, parent, name )
{
    m_direction = direction;
    m_part = part;

    m_popup = new QPopupMenu;
    m_popup->insertItem( i18n( "Default Font Size" ) );

    int m = m_direction ? 1 : -1;

    for ( int i = 1; i < 5; ++i )
    {
        int num = i * m;
        QString numStr = QString::number( num );
        if ( num > 0 ) numStr.prepend( '+' );

        m_popup->insertItem( i18n( "Font Size %1" ).arg( numStr ) );
    }

    connect( m_popup, SIGNAL( activated( int ) ), this, SLOT( slotActivated( int ) ) );
}

KHTMLZoomFactorAction::~KHTMLZoomFactorAction()
{
    delete m_popup;
}

int KHTMLZoomFactorAction::plug( QWidget *w, int index )
{
    int containerId = KAction::plug( w, index );
    if ( containerId == -1 || !w->inherits( "KToolBar" ) )
        return containerId;

    KToolBarButton *button = static_cast<KToolBar *>( w )->getButton( menuId( containerId ) );
    if ( !button )
        return containerId;

    button->setDelayedPopup( m_popup );
    return containerId;
}

void KHTMLZoomFactorAction::slotActivated( int id )
{
    int idx = m_popup->indexOf( id );

    if (idx == 0)
        m_part->setZoomFactor(100);
    else
        m_part->setZoomFactor(m_part->zoomFactor() + (m_direction ? 10 : -10) * idx);
}

#include "khtml_ext.moc"

