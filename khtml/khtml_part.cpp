/* This file is part of the KDE project
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999 Lars Knoll <knoll@kde.org>
 *                     1999 Antti Koivisto <koivisto@kde.org>
 *                     2000 Simon Hausmann <hausmann@kde.org>
 *                     2000 Stefan Schimanski <1Stein@gmx.de>
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
 */
#include "khtml_part.h"

#include "khtml_factory.h"
#include "khtml_run.h"
#include "khtml_events.h"
#include "khtml_find.h"
#include "khtml_ext.h"
#include "khtml_pagecache.h"

#include "dom/dom_string.h"
#include "dom/dom_element.h"
#include "html/html_documentimpl.h"
#include "html/html_miscimpl.h"
#include "html/html_inlineimpl.h"
#include "rendering/render_text.h"
#include "rendering/render_frames.h"
#include "rendering/render_image.h"
#include "misc/htmlhashes.h"
#include "misc/loader.h"
#include "xml/dom_textimpl.h"
#include "java/kjavaappletcontext.h"
using namespace DOM;

#include "khtmlview.h"
#include "decoder.h"
#include "ecma/kjs_proxy.h"
#include "khtml_settings.h"

#include <sys/types.h>
#include <assert.h>
#include <unistd.h>

#include <kglobal.h>
#include <kstddirs.h>
#include <kio/job.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <ktrader.h>
#include <kparts/partmanager.h>
#include <kxmlgui.h>
#include <kcursor.h>
#include <kiconeffect.h>
#include <kdatastream.h>
#include <ktempfile.h>
#include <kglobalsettings.h>

#include <kssl.h>
#include <ksslinfodlg.h>

#include <qtextcodec.h>

#include <qstring.h>
#include <qfile.h>
#include <qclipboard.h>
#include <qapplication.h>
#include <qdragobject.h>
#include <qmetaobject.h>

namespace khtml
{
  struct ChildFrame
  {
    ChildFrame() { m_bCompleted = false; m_frame = 0L; m_bPreloaded = false; m_bFrame = true; }

    RenderPart *m_frame;
    QGuardedPtr<KParts::ReadOnlyPart> m_part;
    QGuardedPtr<KParts::BrowserExtension> m_extension;
    QString m_serviceName;
    QString m_serviceType;
    QStringList m_services;
    bool m_bCompleted;
    QString m_name;
    KParts::URLArgs m_args;
    QGuardedPtr<KHTMLRun> m_run;
    bool m_bPreloaded;
    KURL m_workingURL;
    bool m_bFrame;
    QStringList m_params;
  };

};

int kjs_lib_count = 0;

typedef QMap<QString,khtml::ChildFrame>::ConstIterator ConstFrameIt;
typedef QMap<QString,khtml::ChildFrame>::Iterator FrameIt;

class KHTMLPartPrivate
{
public:
  KHTMLPartPrivate()
  {
    m_doc = 0L;
    m_decoder = 0L;
    m_jscript = 0L;
    m_kjs_lib = 0;
    m_job = 0L;
    m_bComplete = true;
    m_bParsing = false;
    m_bReloading = false;
    m_manager = 0L;
    m_settings = new KHTMLSettings(*KHTMLFactory::defaultHTMLSettings());
    m_bClearing = false;
    m_bCleared = false;
    m_userSheet = QString::null;
    m_fontBase = 0;
    m_bDnd = true;
    m_startOffset = m_endOffset = 0;
    m_startBeforeEnd = true;
    m_linkCursor = KCursor::handCursor();
    m_loadedImages = 0;
    m_totalImageCount = 0;
    m_userHeaders = QString::null;
    m_haveEncoding = false;
    m_activeFrame = 0L;
    keepCharset = false;
    m_findDialog = 0;
    m_ssl_in_use = false;
    m_javaContext = 0;
    m_bJScriptForce = false;
    m_bJScriptOverride = false;
    m_bJavaForce = false;
    m_bJavaOverride = false;
  }
  ~KHTMLPartPrivate()
  {
    //no need to delete m_view here! kparts does it for us (: (Simon)
    if ( m_extension )
      delete m_extension;
    delete m_settings;
    delete m_jscript;
    if ( m_kjs_lib && !--kjs_lib_count )
      delete m_kjs_lib;
    delete m_javaContext;
  }

  QMap<QString,khtml::ChildFrame> m_frames;
  QValueList<khtml::ChildFrame> m_objects;

  QGuardedPtr<KHTMLView> m_view;
  KHTMLPartBrowserExtension *m_extension;
  KHTMLPartBrowserHostExtension *m_hostExtension;
  DOM::HTMLDocumentImpl *m_doc;
  khtml::Decoder *m_decoder;
  QString m_encoding;
  long m_cacheId;
  QString scheduledScript;

  KJSProxy *m_jscript;
  KLibrary *m_kjs_lib;
  bool m_bJScriptEnabled;
  bool m_bJavaEnabled;
  bool m_bJScriptForce;
  bool m_bJScriptOverride;
  bool m_bJavaForce;
  bool m_bJavaOverride;
  KJavaAppletContext *m_javaContext;

  bool keepCharset;

  KHTMLSettings *m_settings;

  KIO::TransferJob * m_job;

  // QStrings for SSL metadata
  bool m_ssl_in_use;
  QString m_ssl_peer_cert_subject,
          m_ssl_peer_cert_issuer,
          m_ssl_peer_ip,
          m_ssl_cipher,
          m_ssl_cipher_desc,
          m_ssl_cipher_version,
          m_ssl_cipher_used_bits,
          m_ssl_cipher_bits,
          m_ssl_cert_state,
          m_ssl_good_from,
          m_ssl_good_until;

  bool m_bComplete;
  bool m_bParsing;
  bool m_bReloading;
  bool m_haveEncoding;

  KURL m_workingURL;
  KURL m_baseURL;
  QString m_baseTarget;

  QTimer m_redirectionTimer;
  int m_delayRedirect;
  QString m_redirectURL;

  KAction *m_paViewDocument;
  KAction *m_paViewFrame;
  KAction *m_paSaveBackground;
  KAction *m_paSaveDocument;
  KAction *m_paSaveFrame;
  KAction *m_paSecurity;
  KSelectAction *m_paSetEncoding;
  KHTMLFontSizeAction *m_paIncFontSizes;
  KHTMLFontSizeAction *m_paDecFontSizes;
  KAction *m_paLoadImages;
  KAction *m_paFind;
  KAction *m_paPrintFrame;
  KAction *m_paSelectAll;

  KParts::PartManager *m_manager;

  bool m_bClearing;
  bool m_bCleared;

  DOM::DOMString m_userSheet;
  DOM::DOMString m_userSheetUrl;

  QString m_popupMenuXML;

  int m_fontBase;

  int m_findPos;
  DOM::NodeImpl *m_findNode;

  QString m_strSelectedURL;

  bool m_bMousePressed;

  DOM::Node m_selectionStart;
  long m_startOffset;
  DOM::Node m_selectionEnd;
  long m_endOffset;
  bool m_startBeforeEnd;
  QString m_overURL;
  bool m_bDnd;

  QPoint m_dragStartPos;

  QCursor m_linkCursor;
  QTimer m_scrollTimer;

  unsigned long m_loadedImages;
  unsigned long m_totalImageCount;

  QString m_userHeaders;

  KHTMLFind *m_findDialog;

  struct findState
  {
    findState()
    { caseSensitive = false; direction = false; }
    QString text;
    bool caseSensitive;
    bool direction;
  };

  findState m_lastFindState;

  //QGuardedPtr<KParts::Part> m_activeFrame;
  KParts::Part * m_activeFrame;
};

namespace khtml {
    class PartStyleSheetLoader : public CachedObjectClient
    {
    public:
        PartStyleSheetLoader(KHTMLPartPrivate *part, DOM::DOMString url, DocLoader *docLoader)
        {
            m_part = part;
            if (docLoader)
                docLoader->requestStyleSheet(url, DOMString());
            else
                Cache::requestStyleSheet(url, DOMString(),false);
        }

        virtual void setStyleSheet(const DOM::DOMString &url, const DOM::DOMString &sheet)
        {
            m_part->m_userSheet = sheet;
            m_part->m_userSheetUrl = url;
            delete this;
        }
        KHTMLPartPrivate *m_part;
    };
};

static QString splitUrlTarget(const QString &url, QString *target=0)
{
   QString result = url;
   if(url.left(7) == "target:")
   {
      KURL u(url);
      result = u.ref();
      if (target)
         *target = u.host();
   }
   return result;
}

KHTMLPart::KHTMLPart( QWidget *parentWidget, const char *widgetname, QObject *parent, const char *name,
                      GUIProfile prof )
: KParts::ReadOnlyPart( parent ? parent : parentWidget, name ? name : widgetname )
{
  KHTMLFactory::registerPart( this );
  setInstance( KHTMLFactory::instance() ); // doesn't work inside init() for derived classes
                                           // Why?? :-} (Simon)
  init( new KHTMLView( this, parentWidget, widgetname ), prof );
}

KHTMLPart::KHTMLPart( KHTMLView *view, QObject *parent, const char *name, GUIProfile prof )
: KParts::ReadOnlyPart( parent, name )
{
  KHTMLFactory::registerPart( this );
  setInstance( KHTMLFactory::instance() );
  assert( view );
  init( view, prof );
}

void KHTMLPart::init( KHTMLView *view, GUIProfile prof )
{
  khtml::Cache::ref();

  if ( prof == DefaultGUI )
    setXMLFile( "khtml.rc" );
  else if ( prof == BrowserViewGUI )
    setXMLFile( "khtml_browser.rc" );

  d = new KHTMLPartPrivate;
  kdDebug() << "KHTMLPart::init this=" << this << " d=" << d << endl;

  d->m_view = view;
  setWidget( d->m_view );

  d->m_extension = new KHTMLPartBrowserExtension( this );
  d->m_hostExtension = new KHTMLPartBrowserHostExtension( this );

  d->m_paLoadImages = 0;

  autoloadImages( KHTMLFactory::defaultHTMLSettings()->autoLoadImages() );

  d->m_paViewDocument = new KAction( i18n( "View Document Source" ), 0, this, SLOT( slotViewDocumentSource() ), actionCollection(), "viewDocumentSource" );
  d->m_paViewFrame = new KAction( i18n( "View Frame Source" ), 0, this, SLOT( slotViewFrameSource() ), actionCollection(), "viewFrameSource" );
  d->m_paSaveBackground = new KAction( i18n( "Save &Background Image As.." ), 0, this, SLOT( slotSaveBackground() ), actionCollection(), "saveBackground" );
  d->m_paSaveDocument = new KAction( i18n( "&Save As.." ), CTRL+Key_S, this, SLOT( slotSaveDocument() ), actionCollection(), "saveDocument" );
  d->m_paSaveFrame = new KAction( i18n( "Save &Frame As.." ), 0, this, SLOT( slotSaveFrame() ), actionCollection(), "saveFrame" );
  d->m_paSecurity = new KAction( i18n( "Security..." ), "unlock", 0, this, SLOT( slotSecurity() ), actionCollection(), "security" );

  d->m_paSetEncoding = new KSelectAction( i18n( "Set &Encoding.." ), 0, this, SLOT( slotSetEncoding() ), actionCollection(), "setEncoding" );
  QStringList encodings = KGlobal::charsets()->availableEncodingNames();
  encodings.prepend( i18n( "Auto" ) );
  d->m_paSetEncoding->setItems( encodings );
  d->m_paSetEncoding->setCurrentItem(0);

  d->m_paIncFontSizes = new KHTMLFontSizeAction( this, true, i18n( "Increase Font Sizes" ), "viewmag+", this, SLOT( slotIncFontSizes() ), actionCollection(), "incFontSizes" );
  d->m_paDecFontSizes = new KHTMLFontSizeAction( this, false, i18n( "Decrease Font Sizes" ), "viewmag-", this, SLOT( slotDecFontSizes() ), actionCollection(), "decFontSizes" );
  d->m_paDecFontSizes->setEnabled( false );

  d->m_paFind = KStdAction::find( this, SLOT( slotFind() ), actionCollection(), "find" );

  d->m_paPrintFrame = new KAction( i18n( "Print Frame" ), "fileprint", 0, this, SLOT( slotPrintFrame() ), actionCollection(), "printFrame" );

  d->m_paSelectAll = KStdAction::selectAll( this, SLOT( slotSelectAll() ), actionCollection(), "selectAll" );

  // set the default java(script) flags according to the current host.
  d->m_bJScriptEnabled = KHTMLFactory::defaultHTMLSettings()->isJavaScriptEnabled();
  d->m_bJavaEnabled = KHTMLFactory::defaultHTMLSettings()->isJavaEnabled();

  connect( this, SIGNAL( completed() ),
           this, SLOT( updateActions() ) );
  connect( this, SIGNAL( started( KIO::Job * ) ),
           this, SLOT( updateActions() ) );

  d->m_popupMenuXML = KXMLGUIFactory::readConfigFile( locate( "data", "khtml/khtml_popupmenu.rc", KHTMLFactory::instance() ) );

  connect( khtml::Cache::loader(), SIGNAL( requestDone( const DOM::DOMString &, khtml::CachedObject *) ),
           this, SLOT( slotLoaderRequestDone( const DOM::DOMString &, khtml::CachedObject *) ) );

  findTextBegin(); //reset find variables

  connect( &d->m_redirectionTimer, SIGNAL( timeout() ),
           this, SLOT( slotRedirect() ) );
}

KHTMLPart::~KHTMLPart()
{
  if ( d->m_findDialog )
      disconnect( d->m_findDialog, SIGNAL( destroyed() ),
                  this, SLOT( slotFindDialogDestroyed() ) );

  if ( d->m_manager )
  {
    d->m_manager->setActivePart( 0 );
    // Shouldn't we delete d->m_manager here ? (David)
    // No need to, I would say. We specify "this" as parent qobject
    // in ::partManager() (Simon)
  }

  stopAutoScroll();
  d->m_redirectionTimer.stop();

  if ( d->m_job )
    d->m_job->kill();

  khtml::Cache::loader()->cancelRequests( m_url.url() );

  disconnect( khtml::Cache::loader(), SIGNAL( requestDone( const DOM::DOMString &, khtml::CachedObject * ) ),
              this, SLOT( slotLoaderRequestDone( const DOM::DOMString &, khtml::CachedObject * ) ) );

  clear();

  if ( d->m_view )
  {
    d->m_view->hide();
    d->m_view->viewport()->hide();
    d->m_view->m_part = 0;
  }

  delete d;
  khtml::Cache::deref();
  KHTMLFactory::deregisterPart( this );
}

bool KHTMLPart::restoreURL( const KURL &url )
{
  QString referrerUrl = m_url.url();

  kdDebug( 6050 ) << "KHTMLPart::restoreURL " << url.url() << endl;

  d->m_redirectionTimer.stop();

  kdDebug( 6050 ) << "closing old URL" << endl;
  if ( !closeURL() )
    return false;

  d->m_bComplete = false;
  d->m_workingURL = url;

  // set the java(script) flags according to the current host.
  d->m_bJScriptEnabled = KHTMLFactory::defaultHTMLSettings()->isJavaScriptEnabled(url.host());
  d->m_bJavaEnabled = KHTMLFactory::defaultHTMLSettings()->isJavaEnabled(url.host());

  KHTMLPageCache::self()->fetchData( d->m_cacheId, this, SLOT(slotRestoreData(const QByteArray &)));

  emit started( 0L );

  return true;
}


bool KHTMLPart::openURL( const KURL &url )
{
  QString referrerUrl = m_url.url();

  KURL::List *vlinks = KHTMLFactory::vLinks();
  if ( !vlinks->contains( url ) )
      vlinks->append( url );

  kdDebug( 6050 ) << "KHTMLPart::openURL " << url.url() << endl;

  d->m_redirectionTimer.stop();

  KParts::URLArgs args( d->m_extension->urlArgs() );
  if ( d->m_frames.count() == 0 && urlcmp( url.url(), m_url.url(), true, true ) && args.postData.size() == 0 && !args.reload )
  {
    kdDebug( 6050 ) << "KHTMLPart::openURL now m_url = " << url.url() << endl;
    m_url = url;
    emit started( 0L );

    if ( !url.htmlRef().isEmpty() )
      gotoAnchor( url.htmlRef() );
    else
      d->m_view->setContentsPos( 0, 0 );

    d->m_bComplete = true;
    d->m_bParsing = false;

    kdDebug( 6050 ) << "completed..." << endl;
    emit completed();
    return true;
  }

  kdDebug( 6050 ) << "closing old URL" << endl;
  if ( !closeURL() )
    return false;

  d->m_bReloading = args.reload;
  if ( (args.postData.size() > 0) && (url.protocol().startsWith("http")) )
  {
      d->m_job = KIO::http_post( url, args.postData, false );
      // DA: Send the content-type as a meta-data instead...
//    d->m_job = KIO::http_post( url, args.postData, d->m_userHeaders, false );
      d->m_job->addMetaData("content-type", d->m_userHeaders );
  }
  else
      d->m_job = KIO::get( url, args.reload, false );

  d->m_job->addMetaData("referrer", referrerUrl);

  connect( d->m_job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotFinished( KIO::Job * ) ) );
  connect( d->m_job, SIGNAL( data( KIO::Job*, const QByteArray &)),
           SLOT( slotData( KIO::Job*, const QByteArray &)));

  connect( d->m_job, SIGNAL(redirection(KIO::Job*, const KURL&) ),
           SLOT( slotRedirection(KIO::Job*,const KURL&) ) );

  d->m_bComplete = false;

  // Tell the slave where we come from (SSL or not)
  d->m_job->addMetaData( "ssl_was_in_use", d->m_ssl_in_use ? "TRUE" : "FALSE" );
  // GS - 9/15/00 - This seems to be wrong - don't know why it was here.
  // d->m_ssl_in_use = false;
  // Tell the slave that this is about loading the main page, so activate warnings
  d->m_job->addMetaData( "ssl_activate_warnings", "TRUE" );
  kdDebug() << "ACTIVATING SSL WARNINGS" << endl;

  d->m_workingURL = url;

  // set the javascript flags according to the current url
  d->m_bJScriptEnabled = KHTMLFactory::defaultHTMLSettings()->isJavaScriptEnabled(url.host());
  d->m_bJavaEnabled = KHTMLFactory::defaultHTMLSettings()->isJavaEnabled(url.host());

  // initializing m_url to the new url breaks relative links when opening such a link after this call and _before_ begin() is called (when the first
  // data arrives) (Simon)
  //  m_url = url;
  kdDebug( 6050 ) << "KHTMLPart::openURL now (before started) m_url = " << m_url.url() << endl;

  emit started( d->m_job );

  return true;
}

bool KHTMLPart::closeURL()
{
  if ( d->m_job )
  {
    KHTMLPageCache::self()->cancelEntry(d->m_cacheId);
    d->m_job->kill();
    d->m_job = 0;
  }

  d->m_bComplete = true; // to avoid emitting completed() in slotFinishedParsing() (David)
  d->m_bReloading = false;

  KHTMLPageCache::self()->cancelFetch(this);
  if ( d->m_bParsing )
  {
    kdDebug( 6050 ) << " was still parsing... calling end " << endl;
    slotFinishedParsing();
    d->m_bParsing = false;
  }

  if ( !d->m_workingURL.isEmpty() )
  {
    // Aborted before starting to render
    kdDebug( 6050 ) << "Aborted before starting to render, reverting location bar to " << m_url.prettyURL() << endl;
    emit d->m_extension->setLocationBarURL( m_url.prettyURL() );
  }

  d->m_workingURL = KURL();

  khtml::Cache::loader()->cancelRequests( m_url.url() );

  return true;
}

DOM::HTMLDocument KHTMLPart::htmlDocument() const
{
  return d->m_doc;
}

KParts::BrowserExtension *KHTMLPart::browserExtension() const
{
  return d->m_extension;
}

KHTMLView *KHTMLPart::view() const
{
  return d->m_view;
}

void KHTMLPart::enableJScript( bool enable )
{
  d->m_bJScriptForce = enable;
  d->m_bJScriptOverride = true;
}

bool KHTMLPart::jScriptEnabled() const
{
  if ( d->m_bJScriptOverride )
      return d->m_bJScriptForce;
  return d->m_bJScriptEnabled;
}

KJSProxy *KHTMLPart::jScript()
{
  if ( d->m_bJScriptOverride && !d->m_bJScriptForce)
      return 0;

  if ( !d->m_bJScriptEnabled && !d->m_bJScriptOverride )
      return 0;

  if ( !d->m_jscript )
  {
    KLibrary *lib = KLibLoader::self()->library("kjs_html");
    if ( !lib )
      return 0;
    // look for plain C init function
    void *sym = lib->symbol("kjs_html_init");
    if ( !sym ) {
      delete lib;
      return 0;
    }
    typedef KJSProxy* (*initFunction)(KHTMLPart *);
    initFunction initSym = (initFunction) sym;
    d->m_jscript = (*initSym)(this);
    d->m_kjs_lib = lib;
    kjs_lib_count++;
  }

  return d->m_jscript;
}

bool KHTMLPart::executeScript( const QString &script )
{
  return executeScript( DOM::Node(), script );
}

bool KHTMLPart::executeScript( const DOM::Node &n, const QString &script )
{
  KJSProxy *proxy = jScript();

  if (!proxy)
    return false;

   bool ret = proxy->evaluate( script.unicode(), script.length(), n );
  d->m_doc->updateRendering();
  return ret;
}

bool KHTMLPart::scheduleScript( const QString &script )
{
    //kdDebug() << "KHTMLPart::scheduleScript "<< script << endl;
    d->scheduledScript = script;
    return true;
}

bool KHTMLPart::executeScheduledScript()
{
    if(d->scheduledScript.isEmpty() || d->scheduledScript.isNull() )
        return false;

    KJSProxy *proxy = jScript();

  if (!proxy)
    return false;

  //kdDebug() << "executing delayed " << d->scheduledScript << endl;

  bool ret = proxy->evaluate( d->scheduledScript.unicode(), d->scheduledScript.length(), Node() );
  d->scheduledScript = QString();
  d->m_doc->updateRendering();
  return ret;
}


void KHTMLPart::enableJava( bool enable )
{
  d->m_bJavaForce = enable;
  d->m_bJavaOverride = true;
}

bool KHTMLPart::javaEnabled() const
{
  if( d->m_bJavaOverride )
      return d->m_bJavaForce;
  return d->m_bJavaEnabled;
}

KJavaAppletContext *KHTMLPart::javaContext()
{
  return d->m_javaContext;
}

KJavaAppletContext *KHTMLPart::createJavaContext()
{
  if ( !d->m_javaContext ) {
      d->m_javaContext = new KJavaAppletContext();
      connect( d->m_javaContext, SIGNAL(showStatus(const QString&)),
               this, SIGNAL(setStatusBarText(const QString&)) );
      connect( d->m_javaContext, SIGNAL(showDocument(const QString&, const QString&)),
               this, SLOT(slotShowDocument(const QString&, const QString&)) );
  }

  return d->m_javaContext;
}

void KHTMLPart::slotShowDocument( const QString &url, const QString &target )
{
  // this is mostly copied from KHTMLPart::slotChildURLRequest. The better approach
  // would be to put those functions into a single one.
  khtml::ChildFrame *child = 0;
  KParts::URLArgs args;
  args.frameName = target;

  QString frameName = args.frameName.lower();
  if ( !frameName.isEmpty() )
  {
    if ( frameName == QString::fromLatin1( "_top" ) )
    {
      emit d->m_extension->openURLRequest( url, args );
      return;
    }
    else if ( frameName == QString::fromLatin1( "_blank" ) )
    {
      emit d->m_extension->createNewWindow( url, args );
      return;
    }
    else if ( frameName == QString::fromLatin1( "_parent" ) )
    {
      KParts::URLArgs newArgs( args );
      newArgs.frameName = QString::null;

      emit d->m_extension->openURLRequest( url, newArgs );
      return;
    }
    else if ( frameName != QString::fromLatin1( "_self" ) )
    {
      khtml::ChildFrame *_frame = recursiveFrameRequest( url, args );

      if ( !_frame )
      {
        emit d->m_extension->openURLRequest( url, args );
        return;
      }

      child = _frame;
    }
  }

  // TODO: handle child target correctly! currently the script are always executed fur the parent
  if ( url.find( QString::fromLatin1( "javascript:" ), 0, false ) == 0 ) {
      executeScript( url.right( url.length() - 11) );
      return;
  }

  if ( child ) {
      requestObject( child, KURL(url), args );
  }  else if ( frameName==QString::fromLatin1("_self") ) // this is for embedded objects (via <object>) which want to replace the current document
  {
      KParts::URLArgs newArgs( args );
      newArgs.frameName = QString::null;
      emit d->m_extension->openURLRequest( KURL(url), newArgs );
  }
}

void KHTMLPart::autoloadImages( bool enable )
{
  if ( enable == khtml::Cache::autoloadImages() )
    return;

  khtml::Cache::autoloadImages( enable );

  unplugActionList( "loadImages" );

  if ( enable )
  {
    if ( d->m_paLoadImages )
      delete d->m_paLoadImages;
    d->m_paLoadImages = 0;
  }
  else if ( !d->m_paLoadImages )
    d->m_paLoadImages = new KAction( i18n( "Display Images on Page" ), "mime-image", 0, this, SLOT( slotLoadImages() ), actionCollection(), "loadImages" );

  if ( d->m_paLoadImages )
  {
      QList<KAction> lst;
      lst.append( d->m_paLoadImages );
      plugActionList( "loadImages", lst );
  }

}

bool KHTMLPart::autoloadImages() const
{
  return khtml::Cache::autoloadImages();
}

void KHTMLPart::clear()
{
    kdDebug( 6090 ) << "KHTMLPart::clear() this = " << this << endl;
  if ( d->m_bCleared )
    return;
  d->m_bCleared = true;

  d->m_bClearing = true;

  {
    ConstFrameIt it = d->m_frames.begin();
    ConstFrameIt end = d->m_frames.end();
    for(; it != end; ++it )
    {
      // Stop HTMLRun jobs.
      if ( it.data().m_run )
        delete it.data().m_run;
    }
  }

  findTextBegin(); // resets d->m_findNode and d->m_findPos

  if ( d->m_jscript )
    d->m_jscript->clear();

  if ( d->m_doc )
  {
    kdDebug( 6090 ) << "KHTMLPart::clear(): dereferencing the document" << endl;
    d->m_doc->detach();
    d->m_doc->deref();
  }
  d->m_doc = 0;

  if ( d->m_decoder )
    delete d->m_decoder;

  d->m_decoder = 0;
  d->m_haveEncoding = false;

  if ( d->m_view )
    d->m_view->clear();

  {
    ConstFrameIt it = d->m_frames.begin();
    ConstFrameIt end = d->m_frames.end();
    for(; it != end; ++it )
    {
      if ( it.data().m_part )
      {
        partManager()->removePart( it.data().m_part );
        delete (KParts::ReadOnlyPart *)it.data().m_part;
      }
    }
  }

  d->m_frames.clear();
  d->m_objects.clear();

  d->m_baseURL = KURL();
  d->m_baseTarget = QString::null;
  d->m_delayRedirect = 0;
  d->m_redirectURL = QString::null;
  d->m_bClearing = false;

  d->m_bMousePressed = false;

  d->m_selectionStart = DOM::Node();
  d->m_selectionEnd = DOM::Node();
  d->m_startOffset = 0;
  d->m_endOffset = 0;

  d->m_totalImageCount = 0;
  d->m_loadedImages = 0;
}

bool KHTMLPart::openFile()
{
  return true;
}

DOM::HTMLDocumentImpl *KHTMLPart::docImpl() const
{
  return d->m_doc;
}

/*bool KHTMLPart::isSSLInUse() const
{
  return d->m_ssl_in_use;
}*/

void KHTMLPart::slotData( KIO::Job*, const QByteArray &data )
{
  kdDebug( 6050 ) << "slotData: " << data.size() << endl;
  // The first data ?
  if ( !d->m_workingURL.isEmpty() )
  {
    kdDebug( 6050 ) << "begin!" << endl;
    d->m_bParsing = true;

    begin( d->m_workingURL, d->m_extension->urlArgs().xOffset, d->m_extension->urlArgs().yOffset );

    if (d->m_bReloading)
        d->m_doc->setReloading();

    d->m_workingURL = KURL();

    d->m_cacheId = KHTMLPageCache::self()->createCacheEntry();

    // When the first data arrives, the metadata has just been made available

    kdDebug( 6050 ) << "First data is arriving. Reading SSL metadata." << endl;
    d->m_ssl_in_use = (d->m_job->queryMetaData("ssl_in_use") == "TRUE");
    kdDebug() << "SSL in use ? " << d->m_ssl_in_use << endl;
    d->m_paSecurity->setIcon( d->m_ssl_in_use ? "lock" : "unlock" );
    kdDebug() << "setIcon " << ( d->m_ssl_in_use ? "lock" : "unlock" ) << " done." << endl;

    // Shouldn't all of this be done only if ssl_in_use == true ? (DF)

    d->m_ssl_peer_cert_subject = d->m_job->queryMetaData("ssl_peer_cert_subject");
    d->m_ssl_peer_cert_issuer = d->m_job->queryMetaData("ssl_peer_cert_issuer");
    d->m_ssl_peer_ip = d->m_job->queryMetaData("ssl_peer_ip");
    d->m_ssl_cipher = d->m_job->queryMetaData("ssl_cipher");
    d->m_ssl_cipher_desc = d->m_job->queryMetaData("ssl_cipher_desc");
    d->m_ssl_cipher_version = d->m_job->queryMetaData("ssl_cipher_version");
    d->m_ssl_cipher_used_bits = d->m_job->queryMetaData("ssl_cipher_used_bits");
    d->m_ssl_cipher_bits = d->m_job->queryMetaData("ssl_cipher_bits");
    d->m_ssl_good_from = d->m_job->queryMetaData("ssl_good_from");
    d->m_ssl_good_until = d->m_job->queryMetaData("ssl_good_until");
    d->m_ssl_cert_state = d->m_job->queryMetaData("ssl_cert_state");
    //
    QString charset = d->m_job->queryMetaData("charset");
    if ( !charset.isEmpty() )
        setCharset( charset, true );
  }

  KHTMLPageCache::self()->addData(d->m_cacheId, data);

  write( data.data(), data.size() );
}

void KHTMLPart::slotRestoreData(const QByteArray &data )
{
  // The first data ?
  if ( !d->m_workingURL.isEmpty() )
  {
     long saveCacheId = d->m_cacheId;
     begin( d->m_workingURL, d->m_extension->urlArgs().xOffset, d->m_extension->urlArgs().yOffset );
     d->m_cacheId = saveCacheId;
     d->m_workingURL = KURL();
  }

  kdDebug( 6050 ) << "slotRestoreData: " << data.size() << endl;
  write( data.data(), data.size() );

  if (data.size() == 0)
  {
     kdDebug( 6050 ) << "slotRestoreData: <<end of data>>" << endl;
     // End of data.
     if ( d->m_bParsing )
     {
        end(); //will emit completed()
     }
  }
}

void KHTMLPart::slotFinished( KIO::Job * job )
{
  if (job->error())
  {
    KHTMLPageCache::self()->cancelEntry(d->m_cacheId);
    job->showErrorDialog();
    d->m_job = 0L;
    emit canceled( job->errorString() );
    // TODO: what else ?
    return;
  }
  kdDebug( 6050 ) << "slotFinished" << endl;

  KHTMLPageCache::self()->endData(d->m_cacheId);

  d->m_workingURL = KURL();

  d->m_job = 0L;

  if ( d->m_bParsing )
  {
    end(); //will emit completed()
  }
}

void KHTMLPart::begin( const KURL &url, int xOffset, int yOffset )
{
  clear();
  d->m_bCleared = false;
  d->m_cacheId = 0;

  // ###
  //stopParser();

  KParts::URLArgs args( d->m_extension->urlArgs() );
  args.xOffset = xOffset;
  args.yOffset = yOffset;
  d->m_extension->setURLArgs( args );

  m_url = url;
  if ( !m_url.isEmpty() )
  {
    KURL::List lst = KURL::split( m_url );
    KURL baseurl;
    if ( !lst.isEmpty() )
      baseurl = *lst.begin();

    KURL title( baseurl );
    title.setRef( QString::null );
    title.setQuery( QString::null );
    emit setWindowCaption( title.url() );
  }
  else
    emit setWindowCaption( i18n( "* Unknown *" ) );

  d->m_doc = new HTMLDocumentImpl( d->m_view );
  d->m_doc->ref();
  d->m_doc->attach( d->m_view );
  d->m_doc->setURL( m_url.url() );
  d->m_doc->setRestoreState(args.docState);
  d->m_doc->open();
  // clear widget
  d->m_view->resizeContents( 0, 0 );
  connect(d->m_doc,SIGNAL(finishedParsing()),this,SLOT(slotFinishedParsing()));

  emit d->m_extension->enableAction( "print", true );

  d->m_bParsing = true;
}

void KHTMLPart::write( const char *str, int len )
{
    if ( !d->m_decoder ) {
        d->m_decoder = new khtml::Decoder();
        if(d->m_encoding != QString::null)
            d->m_decoder->setEncoding(d->m_encoding.latin1(), d->m_haveEncoding);
        else
            d->m_decoder->setEncoding(settings()->encoding().latin1(), d->m_haveEncoding);
    }
  if ( len == 0 )
    return;

  if ( len == -1 )
    len = strlen( str );

  QString decoded = d->m_decoder->decode( str, len );

  if(decoded.isEmpty()) return;

  //kdDebug() << "KHTMLPart::write haveEnc = " << d->m_haveEncoding << endl;
  if(!d->m_haveEncoding) {
      // ### this is still quite hacky, but should work a lot better than the old solution
      if(d->m_decoder->visuallyOrdered()) d->m_doc->setVisuallyOrdered();
      const QTextCodec *c = d->m_decoder->codec();
      if( !d->keepCharset ) {
	  //kdDebug(6005) << "setting up charset to " << (int) KGlobal::charsets()->charsetForEncoding(c->name()) << endl;
          d->m_settings->setCharset( KGlobal::charsets()->charsetForEncoding(c->name()) );
          //kdDebug(6005) << "charset is " << (int)d->m_settings->charset() << endl;
      }
      d->m_doc->applyChanges(true, true);
      d->m_haveEncoding = true;
  }

  d->m_doc->write( decoded );
}

void KHTMLPart::write( const QString &str )
{
  if ( str.isNull() )
    return;

  d->m_doc->write( str );
}

void KHTMLPart::end()
{
    // make sure nothing's left in there...
    write(d->m_decoder->flush());
    d->m_doc->finishParsing();
}

void KHTMLPart::slotFinishedParsing()
{
  d->m_bParsing = false;
  d->m_doc->close();
  disconnect(d->m_doc,SIGNAL(finishedParsing()),this,SLOT(slotFinishedParsing()));

  if (!d->m_view)
    return; // We are probably being destructed.

  // check if the scrollbars are really needed for the content
  // if not, remove them, relayout, and repaint
  int ow = d->m_view->visibleWidth();
  if (d->m_view->vScrollBarMode()==QScrollView::AlwaysOn)
    d->m_view->setVScrollBarMode(QScrollView::Auto);
  if (d->m_view->visibleWidth() != ow)
  {
    d->m_view->layout();
    d->m_view->updateContents(d->m_view->contentsX(),d->m_view->contentsY(),
            d->m_view->visibleWidth(),d->m_view->visibleHeight());
  }

  if ( !m_url.htmlRef().isEmpty() )
    gotoAnchor( m_url.htmlRef() );
  else if (d->m_view->contentsY()==0) // check that the view has not been moved by the use
  {
    d->m_view->setContentsPos( d->m_extension->urlArgs().xOffset, d->m_extension->urlArgs().yOffset );
  }

  HTMLCollectionImpl imgColl( d->m_doc, HTMLCollectionImpl::DOC_IMAGES );

  d->m_totalImageCount = 0;
  KURL::List imageURLs;
  unsigned long i = 0;
  unsigned long len = imgColl.length();
  for (; i < len; i++ )
  {
    NodeImpl *node = imgColl.item( i );
    if ( node->id() != ID_IMG )
      continue;

    QString imgURL = static_cast<DOM::ElementImpl *>( node )->getAttribute( ATTR_SRC ).string();
    KURL url;

    if ( KURL::isRelativeURL( imgURL ) )
      url = completeURL( imgURL );
    else
      url = KURL( imgURL );

    if ( !imageURLs.contains( url ) )
    {
      d->m_totalImageCount++;
      imageURLs.append( url );
    }
  }

  checkCompleted();
}

void KHTMLPart::slotLoaderRequestDone( const DOM::DOMString &baseURL, khtml::CachedObject *obj )
{
  if ( baseURL != m_url.url() )
    return;

  if ( obj && obj->type() == khtml::CachedObject::Image && !d->m_bParsing )
  {
    d->m_loadedImages++;

    // in case we have more images than we originally found, then they are most likely loaded by some
    // javascript code. as we can't find out the exact number anyway we skip displaying any further image
    // loading info message :P
    if ( d->m_loadedImages <= d->m_totalImageCount )
      emit d->m_extension->infoMessage( i18n( "%1 of %2 Images loaded" ).arg( d->m_loadedImages ).arg( d->m_totalImageCount ) );
  }

  checkCompleted();
}

void KHTMLPart::checkCompleted()
{
  //kdDebug( 6050 ) << "KHTMLPart::checkCompleted() parsing: " << d->m_bParsing
  //          << " complete: " << d->m_bComplete << endl;
  int requests = 0;

  ConstFrameIt it = d->m_frames.begin();
  ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it )
    if ( !it.data().m_bCompleted )
      return;

  requests = khtml::Cache::loader()->numRequests( m_url.url() );
  kdDebug( 6060 ) << "number of loader requests: " << requests << endl;
  if ( requests > 0 )
    return;

  if ( d->m_bParsing || d->m_bComplete )
    return;

  d->m_bComplete = true;

  if (!parentPart())
    emit setStatusBarText(i18n("Done."));

  if ( !d->m_redirectURL.isEmpty() )
  {
  //    QTimer::singleShot( 1000 * d->m_delayRedirect, this, SLOT( slotRedirect() ) );
    d->m_redirectionTimer.start( 1000 * d->m_delayRedirect, true );
    return;
  }


  emit completed();
}

const KHTMLSettings *KHTMLPart::settings() const
{
  return d->m_settings;
}

void KHTMLPart::setBaseURL( const KURL &url )
{
  d->m_baseURL = url;
}

KURL KHTMLPart::baseURL() const
{
  return d->m_baseURL;
}

void KHTMLPart::setBaseTarget( const QString &target )
{
  d->m_baseTarget = target;
}

QString KHTMLPart::baseTarget() const
{
  return d->m_baseTarget;
}

KURL KHTMLPart::completeURL( const QString &url, const QString &/*target*/ )
{
  // WABA: The following check is necassery to fix forms which don't set
  // an action URL in the believe that it default to the same URL as
  // the current page which contains the form.
  if (url.isEmpty())
  {
    return m_url;
  }

  if (d->m_baseURL.isEmpty())
  {
     KURL u ( m_url, url );
     return u;
  }
  else
  {
     KURL u ( d->m_baseURL, url );
     return u;
  }
}

void KHTMLPart::scheduleRedirection( int delay, const QString &url )
{
  d->m_delayRedirect = delay;
  d->m_redirectURL = url;
  if(!d->m_bComplete)
      d->m_redirectionTimer.start( 1000 * d->m_delayRedirect, true );
}

void KHTMLPart::slotRedirect()
{
  kdDebug( 6050 ) << "KHTMLPart::slotRedirect()" << endl;

  QString u = d->m_redirectURL;
  d->m_delayRedirect = 0;
  d->m_redirectURL = QString::null;
  urlSelected( u );
}

void KHTMLPart::slotRedirection(KIO::Job*, const KURL& url)
{
  // the slave told us that we got redirected
  kdDebug( 6050 ) << "redirection by KIO to " << url.url() << endl;

  emit d->m_extension->setLocationBarURL( url.prettyURL() );

  d->m_workingURL = url;
}

// ####
bool KHTMLPart::setCharset( const QString &name, bool override )
{
  QFont f(settings()->stdFontName());
  KGlobal::charsets()->setQFont(f, KGlobal::charsets()->charsetForEncoding(name) );

  //kdDebug(6005) << "setting to charset " << (int)QFontInfo(f).charSet() <<" " << override << " should be " << name << endl;

  d->m_settings->setCharset( f.charSet() );
  d->keepCharset = override;
  return true;
}

bool KHTMLPart::setEncoding( const QString &name, bool override )
{
    d->m_encoding = name;
    d->m_haveEncoding = override;

    // ### hack!!!!
    if(!d->m_settings->charset() == QFont::Unicode)
        d->m_settings->setCharset( KGlobal::charsets()->charsetForEncoding(name) );

    if( !m_url.isEmpty() ) {
        // reload document
        closeURL();
        KURL url = m_url;
        m_url = 0;
        openURL(url);
    }

    return true;
}

void KHTMLPart::setUserStyleSheet(const KURL &url)
{
    d->m_userSheetUrl = DOMString();
    d->m_userSheet = DOMString();
    new khtml::PartStyleSheetLoader(d, url.url(), d->m_doc ? d->m_doc->docLoader() : 0);
}

void KHTMLPart::setUserStyleSheet(const QString &styleSheet)
{
    d->m_userSheet = styleSheet;
    d->m_userSheetUrl = DOMString();
}


bool KHTMLPart::gotoAnchor( const QString &name )
{
  HTMLCollectionImpl *anchors =
      new HTMLCollectionImpl( d->m_doc, HTMLCollectionImpl::DOC_ANCHORS);
  anchors->ref();
  NodeImpl *n = anchors->namedItem(name);
  anchors->deref();

  if(!n)
  {
    n = d->m_doc->getElementById(name);
  }

  if(!n) return false;

  int x = 0, y = 0;
  HTMLAnchorElementImpl *a = static_cast<HTMLAnchorElementImpl *>(n);
  a->getAnchorPosition(x, y);
  d->m_view->setContentsPos(x-50, y-50);
  return true;
}

void KHTMLPart::setFontSizes( const QValueList<int> &newFontSizes )
{
  d->m_settings->setFontSizes( newFontSizes );
}

QValueList<int> KHTMLPart::fontSizes() const
{
  return d->m_settings->fontSizes();
}

void KHTMLPart::resetFontSizes()
{
  d->m_settings->resetFontSizes();
}

void KHTMLPart::setStandardFont( const QString &name )
{
    d->m_settings->setStdFontName(name);
}

void KHTMLPart::setFixedFont( const QString &name )
{
    d->m_settings->setFixedFontName(name);
}

void KHTMLPart::setURLCursor( const QCursor &c )
{
  d->m_linkCursor = c;
}

const QCursor &KHTMLPart::urlCursor() const
{
  return d->m_linkCursor;
}

void KHTMLPart::findTextBegin()
{
  d->m_findPos = -1;
  d->m_findNode = 0;
}

bool KHTMLPart::findTextNext( const QRegExp &exp, bool forward )
{
    if(!d->m_findNode) d->m_findNode = d->m_doc->body();

    if ( !d->m_findNode ||
         d->m_findNode->id() == ID_FRAMESET )
      return false;

    while(1)
    {
        if(d->m_findNode->id() == ID_TEXT)
        {
            DOMStringImpl *t = (static_cast<TextImpl *>(d->m_findNode))->string();
            QConstString s(t->s, t->l);
            d->m_findPos = s.string().find(exp, d->m_findPos+1);
            if(d->m_findPos != -1)
            {
                int x = 0, y = 0;
        khtml::RenderText *text = static_cast<khtml::RenderText *>(d->m_findNode->renderer());
                text->posOfChar(d->m_findPos, x, y);
                d->m_view->setContentsPos(x-50, y-50);
                return true;
            }
        }
        d->m_findPos = -1;

        NodeImpl *next;

        if ( forward )
        {
          next = d->m_findNode->firstChild();

          if(!next) next = d->m_findNode->nextSibling();
          while(d->m_findNode && !next) {
              d->m_findNode = d->m_findNode->parentNode();
              if( d->m_findNode ) {
                  next = d->m_findNode->nextSibling();
              }
          }
        }
        else
        {
          next = d->m_findNode->lastChild();

          if (!next ) next = d->m_findNode->previousSibling();
          while ( d->m_findNode && !next )
          {
            d->m_findNode = d->m_findNode->parentNode();
            if( d->m_findNode )
            {
              next = d->m_findNode->previousSibling();
            }
          }
        }

        d->m_findNode = next;
        if(!d->m_findNode) return false;
    }
}

bool KHTMLPart::findTextNext( const QString &str, bool forward, bool caseSensitive )
{
    if(!d->m_findNode) d->m_findNode = d->m_doc->body();

    if ( !d->m_findNode ||
         d->m_findNode->id() == ID_FRAMESET )
      return false;

    while(1)
    {
        if(d->m_findNode->id() == ID_TEXT)
        {
            DOMStringImpl *t = (static_cast<TextImpl *>(d->m_findNode))->string();
            QConstString s(t->s, t->l);
            d->m_findPos = s.string().find(str, d->m_findPos+1, caseSensitive);
            if(d->m_findPos != -1)
            {
                int x = 0, y = 0;
        static_cast<khtml::RenderText *>(d->m_findNode->renderer())
            ->posOfChar(d->m_findPos, x, y);
                d->m_view->setContentsPos(x-50, y-50);

                d->m_selectionStart = d->m_findNode;
                d->m_startOffset = d->m_findPos;
                d->m_selectionEnd = d->m_findNode;
                d->m_endOffset = d->m_findPos + str.length();
                d->m_startBeforeEnd = true;

                d->m_doc->setSelection( d->m_selectionStart.handle(), d->m_startOffset,
                                        d->m_selectionEnd.handle(), d->m_endOffset );
                emitSelectionChanged();
                return true;
            }
        }
        d->m_findPos = -1;

        NodeImpl *next;

        if ( forward )
        {
          next = d->m_findNode->firstChild();

          if(!next) next = d->m_findNode->nextSibling();
          while(d->m_findNode && !next) {
              d->m_findNode = d->m_findNode->parentNode();
              if( d->m_findNode ) {
                  next = d->m_findNode->nextSibling();
              }
          }
        }
        else
        {
          next = d->m_findNode->lastChild();

          if (!next ) next = d->m_findNode->previousSibling();
          while ( d->m_findNode && !next )
          {
            d->m_findNode = d->m_findNode->parentNode();
            if( d->m_findNode )
            {
              next = d->m_findNode->previousSibling();
            }
          }
        }

        d->m_findNode = next;
        if(!d->m_findNode) return false;
    }
}

QString KHTMLPart::selectedText() const
{
  QString text;
  DOM::Node n = d->m_selectionStart;
  while(!n.isNull()) {
      if(n.nodeType() == DOM::Node::TEXT_NODE) {
        QString str = static_cast<TextImpl *>(n.handle())->data().string();
        if(n == d->m_selectionStart && n == d->m_selectionEnd)
          text = str.mid(d->m_startOffset, d->m_endOffset - d->m_startOffset);
        else if(n == d->m_selectionStart)
          text = str.mid(d->m_startOffset);
        else if(n == d->m_selectionEnd)
          text += str.left(d->m_endOffset);
        else
         text += str;
      }
      else if(n.elementId() == ID_BR)
            text += "\n";
        else if(n.elementId() == ID_P || n.elementId() == ID_TD)
            text += "\n\n";
        if(n == d->m_selectionEnd) break;
        DOM::Node next = n.firstChild();
        if(next.isNull()) next = n.nextSibling();
        while( next.isNull() && !n.parentNode().isNull() ) {
            n = n.parentNode();
            next = n.nextSibling();
        }

        n = next;
    }
    return text;
}

bool KHTMLPart::hasSelection() const
{
  return ( !d->m_selectionStart.isNull() &&
           !d->m_selectionEnd.isNull() );
}

DOM::Range KHTMLPart::selection() const
{
    // ###
    return DOM::Range();
}


void KHTMLPart::overURL( const QString &url, const QString &target )
{
  emit onURL( url );

  if ( url.isEmpty() )
  {
    emit setStatusBarText( url );
    return;
  }

   if (url.find( QString::fromLatin1( "javascript:" ),0, false ) != -1 )
   {
     emit setStatusBarText(  url.mid(  url.find("javascript:",0,false)  )  );
     return;
   }


  KURL u = completeURL( url );
  QString com;

  if ( KHTMLFactory::vLinks()->contains( u ) )
      kdDebug( 6000 ) << "Ah, " << u.prettyURL() << " has been visited some time ago" << endl;

  KMimeType::Ptr typ = KMimeType::findByURL( u );

  if ( typ )
    com = typ->comment( u, false );

  if ( u.isMalformed() )
  {
    emit setStatusBarText( u.prettyURL() );
    return;
  }

  if ( u.isLocalFile() )
  {
    // TODO : use KIO::stat() and create a KFileItem out of its result,
   // to use KFileItem::statusBarText()
    QCString path = QFile::encodeName( u.path() );

    struct stat buff;
    stat( path.data(), &buff );

    struct stat lbuff;
    lstat( path.data(), &lbuff );

    QString text = u.url();
    QString text2 = text;

    if (S_ISLNK( lbuff.st_mode ) )
    {
      QString tmp;
      if ( com.isNull() )
        tmp = i18n( "Symbolic Link");
      else
        tmp = i18n("%1 (Link)").arg(com);
      char buff_two[1024];
      text += " -> ";
      int n = readlink ( path.data(), buff_two, 1022);
      if (n == -1)
      {
        text2 += "  ";
        text2 += tmp;
        emit setStatusBarText( text2 );
        return;
      }
      buff_two[n] = 0;

      text += buff_two;
      text += "  ";
      text += tmp;
    }
    else if ( S_ISREG( buff.st_mode ) )
    {
      if (buff.st_size < 1024)
        text = i18n("%1 (%2 bytes)").arg(text2).arg((long) buff.st_size);
      else
      {
        float d = (float) buff.st_size/1024.0;
        text = i18n("%1 (%2 K)").arg(text2).arg(KGlobal::locale()->formatNumber(d, 2)); // was %.2f
      }
      text += "  ";
      text += com;
    }
    else if ( S_ISDIR( buff.st_mode ) )
    {
      text += "  ";
      text += com;
    }
    else
    {
      text += "  ";
      text += com;
    }
    emit setStatusBarText( text );
  }
  else
  {
    QString extra;
    if (target == QString::fromLatin1("_blank"))
    {
       extra = i18n(" (In new window)");
    }
    else if (!target.isEmpty() &&
             (target != QString::fromLatin1("_top")) &&
             (target != QString::fromLatin1("_self")) &&
             (target != QString::fromLatin1("_parent")))
    {
       extra = i18n(" (In other frame)");
    }

    emit setStatusBarText( u.prettyURL()+extra );
  }

}

void KHTMLPart::urlSelected( const QString &url, int button, int state, const QString &_target )
{
  bool hasTarget = false;

  QString target = _target;
  if ( target.isEmpty() )
    target = d->m_baseTarget;
  if ( !target.isEmpty() )
      hasTarget = true;

  if ( url.find( QString::fromLatin1( "javascript:" ), 0, false ) == 0 )
  {
    executeScript( url.right( url.length() - 11) );
    return;
  }

  KURL cURL = completeURL( url );

  kdDebug( 6000 ) << "complete URL:" << cURL.url() << " target = " << target << endl;

  KURL::List *vlinks = KHTMLFactory::vLinks();
  if ( !vlinks->contains( cURL ) )
      vlinks->append( cURL );

  if ( button == LeftButton && ( state & ShiftButton ) && !cURL.isMalformed() )
  {
    KHTMLPopupGUIClient::saveURL( d->m_view, i18n( "Save As .." ), cURL );
    return;
  }

  if ( url.isEmpty() )
    return;

  if (!checkLinkSecurity(cURL))
    return;

  KParts::URLArgs args;
  args.frameName = target;

  if ( hasTarget )
  {
    // unknown frame names should open in a new window.
    khtml::ChildFrame *frame = recursiveFrameRequest( cURL, args, false );
    if ( frame )
    {
      requestObject( frame, cURL, args );
      return;
    }
  }

  if ( !d->m_bComplete && !hasTarget )
    closeURL();

  emit d->m_extension->openURLRequest( cURL, args );
}

void KHTMLPart::slotViewDocumentSource()
{
  KURL url(m_url);
  if (KHTMLPageCache::self()->isValid(d->m_cacheId))
  {
     QString extension = m_url.fileName(false);
     if (extension.isEmpty())
        extension = "index.html";
     KTempFile sourceFile(QString::null, extension);
     if (sourceFile.status() == 0)
     {
        KHTMLPageCache::self()->saveData(d->m_cacheId, sourceFile.dataStream());
        url = KURL();
        url.setPath(sourceFile.name());
     }
  }

  //  emit d->m_extension->openURLRequest( m_url, KParts::URLArgs( false, 0, 0, QString::fromLatin1( "text/plain" ) ) );
  (void) KRun::runURL( url, QString::fromLatin1("text/plain") );
}

void KHTMLPart::slotViewFrameSource()
{
  // ### frames
  //emit d->m_extension->openURLRequest( ((KParts::ReadOnlyPart *)partManager()->activePart())->url(), KParts::URLArgs( false, 0, 0, QString::fromLatin1( "text/plain" ) ) );
  (void) KRun::runURL( ((KParts::ReadOnlyPart *)partManager()->activePart())->url(), QString::fromLatin1("text/plain") );
}

void KHTMLPart::slotSaveBackground()
{
  QString relURL = d->m_doc->body()->getAttribute( ATTR_BACKGROUND ).string();

  KURL backgroundURL( m_url, relURL );

  KHTMLPopupGUIClient::saveURL( d->m_view, i18n("Save background image as"), backgroundURL );
}

void KHTMLPart::slotSaveDocument()
{
  KURL srcURL( m_url );

  if ( srcURL.fileName(false).isEmpty() )
    srcURL.setFileName( "index.html" );

  KHTMLPopupGUIClient::saveURL( d->m_view, i18n( "Save as" ), srcURL, i18n("HTML files|* *.html *.htm"), d->m_cacheId );
}

void KHTMLPart::slotSecurity()
{
  kdDebug( 6050 ) << "Meta Data:" << endl
                  << d->m_ssl_peer_cert_subject
                  << endl
                  << d->m_ssl_peer_cert_issuer
                  << endl
                  << d->m_ssl_cipher
                  << endl
                  << d->m_ssl_cipher_desc
                  << endl
                  << d->m_ssl_cipher_version
                  << endl
                  << d->m_ssl_good_from
                  << endl
                  << d->m_ssl_good_until
                  << endl
                  << d->m_ssl_cert_state
                  << endl;

  KSSLInfoDlg *kid = new KSSLInfoDlg(d->m_ssl_in_use, widget());
  if (d->m_ssl_in_use) {
    kid->setup(d->m_ssl_peer_cert_subject,
               d->m_ssl_peer_cert_issuer,
               d->m_ssl_peer_ip,
               m_url.url(),
               d->m_ssl_cipher,
               d->m_ssl_cipher_desc,
               d->m_ssl_cipher_version,
               d->m_ssl_cipher_used_bits.toInt(),
               d->m_ssl_cipher_bits.toInt(),
               (KSSLCertificate::KSSLValidation) d->m_ssl_cert_state.toInt(),
               d->m_ssl_good_from, d->m_ssl_good_until);
  }
  kid->show();
}

void KHTMLPart::slotSaveFrame()
{
    if ( !d->m_activeFrame )
        return; // should never be the case, but one never knows :-)

    KURL srcURL( static_cast<KParts::ReadOnlyPart *>( d->m_activeFrame )->url() );

    if ( srcURL.fileName(false).isEmpty() )
        srcURL.setFileName( "index.html" );

    KHTMLPopupGUIClient::saveURL( d->m_view, i18n( "Save as" ), srcURL, i18n("HTML files|* *.html *.htm") );
}

void KHTMLPart::slotSetEncoding()
{
    // first Item is always auto
    if(d->m_paSetEncoding->currentItem() == 0)
        setEncoding(QString::null, false);
    else
        setEncoding(d->m_paSetEncoding->currentText(), true);
}

void KHTMLPart::updateActions()
{
  bool frames = d->m_frames.count() > 0;
  d->m_paViewFrame->setEnabled( frames );
  d->m_paSaveFrame->setEnabled( frames );

  if ( frames )
    d->m_paFind->setText( i18n( "&Find in Frame" ) );
  else
    d->m_paFind->setText( i18n( "&Find" ) );

  KParts::Part *frame = 0;

  if ( frames )
    frame = partManager()->activePart();

  bool enableFindAndSelectAll = true;

  if ( frame )
    enableFindAndSelectAll = frame->inherits( "KHTMLPart" );

  d->m_paFind->setEnabled( enableFindAndSelectAll );
  d->m_paSelectAll->setEnabled( enableFindAndSelectAll );

  bool enablePrintFrame = false;

  if ( frame )
  {
    QObject *ext = frame->child( 0, "KParts::BrowserExtension" );
    if ( ext )
      enablePrintFrame = ext->metaObject()->slotNames().contains( "print()" );
  }

  d->m_paPrintFrame->setEnabled( enablePrintFrame );

  QString bgURL;

  // ### frames

  if ( d->m_doc && d->m_doc->body() && !d->m_bClearing )
    bgURL = d->m_doc->body()->getAttribute( ATTR_BACKGROUND ).string();

  d->m_paSaveBackground->setEnabled( !bgURL.isEmpty() );
}

bool KHTMLPart::requestFrame( khtml::RenderPart *frame, const QString &url, const QString &frameName,
                              const QStringList &params )
{
  kdDebug( 6050 ) << "childRequest( ..., " << url << ", " << frameName << " )" << endl;
  if (url.isEmpty())
    return false;
  FrameIt it = d->m_frames.find( frameName );

  if ( it == d->m_frames.end() )
  {
    khtml::ChildFrame child;
    kdDebug( 6050 ) << "inserting new frame into frame map" << endl;
    child.m_name = frameName;
    it = d->m_frames.insert( frameName, child );
  }

  it.data().m_frame = frame;
  it.data().m_params = params;

  return requestObject( &it.data(), completeURL( url ) );
}

bool KHTMLPart::requestObject( khtml::RenderPart *frame, const QString &url, const QString &serviceType,
                               const QStringList &params )
{
  if (url.isEmpty())
    return false;
  khtml::ChildFrame child;
  QValueList<khtml::ChildFrame>::Iterator it = d->m_objects.append( child );
  (*it).m_frame = frame;
  (*it).m_bFrame = false;
  (*it).m_params = params;

  KParts::URLArgs args;
  args.serviceType = serviceType;
  return requestObject( &(*it), completeURL( url ), args );
}

bool KHTMLPart::requestObject( khtml::ChildFrame *child, const KURL &url, const KParts::URLArgs &_args )
{
  if ( child->m_bPreloaded )
  {
      kdDebug() << "requestObject preload" << endl;
    if ( child->m_frame && child->m_part )
      child->m_frame->setWidget( child->m_part->widget() );

    child->m_bPreloaded = false;
    return true;
  }

   KParts::URLArgs args( _args );

  if ( child->m_run )
    delete (KHTMLRun *)child->m_run;

  if ( child->m_part && !args.reload && urlcmp( child->m_part->url().url(), url.url(), true, true ) )
    args.serviceType = child->m_serviceType;

  child->m_args = args;
  child->m_serviceName = QString::null;

  // Use a KHTMLRun if the service type is unknown, but only if this is not a POST
  // We can't put POSTs on hold, nor rely on HTTP HEAD, so we have to assume html for POSTs (DF).
  if ( args.serviceType.isEmpty() && args.postData.size() == 0 ) {
    child->m_run = new KHTMLRun( this, child, url );
    return false;
  } else {
    if (args.serviceType.isEmpty())
      args.serviceType = "text/html";
    return processObjectRequest( child, url, args.serviceType );
  }
}

bool KHTMLPart::processObjectRequest( khtml::ChildFrame *child, const KURL &_url, const QString &mimetype )
{
  kdDebug( 6050 ) << "trying to create part for " << mimetype << endl;

  // IMPORTANT: create a copy of the url here, because it is just a reference, which was likely to be given
  // by an emitting frame part (emit openURLRequest( blahurl, ... ) . A few lines below we delete the part
  // though -> the reference becomes invalid -> crash is likely
  KURL url( _url );

  if ( !child->m_services.contains( mimetype ) )
  {
    KParts::ReadOnlyPart *part = createPart( d->m_view->viewport(), child->m_name.ascii(), this, child->m_name.ascii(), mimetype, child->m_serviceName, child->m_services, child->m_params );

    if ( !part )
      return false;

    //CRITICAL STUFF
    if ( child->m_part )
    {
      partManager()->removePart( (KParts::ReadOnlyPart *)child->m_part );
      delete (KParts::ReadOnlyPart *)child->m_part;
    }

    child->m_serviceType = mimetype;
    if ( child->m_frame )
      child->m_frame->setWidget( part->widget() );

    if ( child->m_bFrame )
      partManager()->addPart( part );
    else
        kdDebug() << "AH! NO FRAME!!!!!" << endl;

    child->m_part = part;

    if ( child->m_bFrame )
    {
      connect( part, SIGNAL( started( KIO::Job *) ),
               this, SLOT( slotChildStarted( KIO::Job *) ) );
      connect( part, SIGNAL( completed() ),
               this, SLOT( slotChildCompleted() ) );
      connect( part, SIGNAL( setStatusBarText( const QString & ) ),
               this, SIGNAL( setStatusBarText( const QString & ) ) );
    }

    child->m_extension = static_cast<KParts::BrowserExtension *>( part->child( 0L, "KParts::BrowserExtension" ) );

    if ( child->m_extension )
    {
      connect( child->m_extension, SIGNAL( openURLNotify() ),
               d->m_extension, SIGNAL( openURLNotify() ) );

      connect( child->m_extension, SIGNAL( openURLRequest( const KURL &, const KParts::URLArgs & ) ),
               this, SLOT( slotChildURLRequest( const KURL &, const KParts::URLArgs & ) ) );

      connect( child->m_extension, SIGNAL( createNewWindow( const KURL &, const KParts::URLArgs & ) ),
               d->m_extension, SIGNAL( createNewWindow( const KURL &, const KParts::URLArgs & ) ) );

      connect( child->m_extension, SIGNAL( popupMenu( const QPoint &, const KFileItemList & ) ),
               d->m_extension, SIGNAL( popupMenu( const QPoint &, const KFileItemList & ) ) );
      connect( child->m_extension, SIGNAL( popupMenu( KXMLGUIClient *, const QPoint &, const KFileItemList & ) ),
               d->m_extension, SIGNAL( popupMenu( KXMLGUIClient *, const QPoint &, const KFileItemList & ) ) );
      connect( child->m_extension, SIGNAL( popupMenu( const QPoint &, const KURL &, const QString &, mode_t ) ),
               d->m_extension, SIGNAL( popupMenu( const QPoint &, const KURL &, const QString &, mode_t ) ) );
      connect( child->m_extension, SIGNAL( popupMenu( KXMLGUIClient *, const QPoint &, const KURL &, const QString &, mode_t ) ),
               d->m_extension, SIGNAL( popupMenu( KXMLGUIClient *, const QPoint &, const KURL &, const QString &, mode_t ) ) );

      connect( child->m_extension, SIGNAL( infoMessage( const QString & ) ),
               d->m_extension, SIGNAL( infoMessage( const QString & ) ) );
    }

  }

  if ( child->m_bPreloaded )
  {
    if ( child->m_frame && child->m_part )
      child->m_frame->setWidget( child->m_part->widget() );

    child->m_bPreloaded = false;
    return true;
  }

  child->m_args.reload = d->m_bReloading;

  child->m_bCompleted = false;
  if ( child->m_extension )
    child->m_extension->setURLArgs( child->m_args );

  kdDebug( 6050 ) << "opening " << url.url() << " in frame" << endl;
  return child->m_part->openURL( url );
}

KParts::ReadOnlyPart *KHTMLPart::createPart( QWidget *parentWidget, const char *widgetName,
                                             QObject *parent, const char *name, const QString &mimetype,
                                             QString &serviceName, QStringList &serviceTypes,
                                             const QStringList &params )
{
  QString constr = QString::fromLatin1( "('KParts/ReadOnlyPart' in ServiceTypes)" );

  if ( !serviceName.isEmpty() )
    constr.append( QString::fromLatin1( "and ( Name == '%1' )" ).arg( serviceName ) );

  KTrader::OfferList offers = KTrader::self()->query( mimetype, constr );

  //  assert( offers.count() >= 1 );
  if ( offers.count() == 0 )
    return 0L;

  KService::Ptr service = *offers.begin();

  KLibFactory *factory = KLibLoader::self()->factory( service->library().latin1() );

  if ( !factory )
    return 0L;

  KParts::ReadOnlyPart *res = 0L;

  const char *className = "KParts::ReadOnlyPart";
  if ( service->serviceTypes().contains( "Browser/View" ) )
    className = "Browser/View";

  if ( factory->inherits( "KParts::Factory" ) )
    res = static_cast<KParts::ReadOnlyPart *>(static_cast<KParts::Factory *>( factory )->createPart( parentWidget, widgetName, parent, name, className, params ));
  else
  res = static_cast<KParts::ReadOnlyPart *>(factory->create( parentWidget, widgetName, className ));

  if ( !res )
    return res;

  serviceTypes = service->serviceTypes();
  serviceName = service->name();

  return res;
}

KParts::PartManager *KHTMLPart::partManager()
{
  if ( !d->m_manager )
  {
    d->m_manager = new KParts::PartManager( d->m_view->topLevelWidget(), this, "khtml part manager" );
    d->m_manager->setAllowNestedParts( true );
    connect( d->m_manager, SIGNAL( activePartChanged( KParts::Part * ) ),
             this, SLOT( slotActiveFrameChanged( KParts::Part * ) ) );
    connect( d->m_manager, SIGNAL( partRemoved( KParts::Part * ) ),
             this, SLOT( slotPartRemoved( KParts::Part * ) ) );
  }

  return d->m_manager;
}

void KHTMLPart::submitForm( const char *action, const QString &url, const QByteArray &formData, const QString &_target, const QString& contentType, const QString& boundary )
{
  QString target = _target;
  if ( target.isEmpty() )
    target = d->m_baseTarget;

  KURL u = completeURL( url, target );

  if ( u.isMalformed() )
  {
    // ### ERROR HANDLING!
    return;
  }

  if (!checkLinkSecurity(u))
    return;

  KParts::URLArgs args;

  if ( strcmp( action, "get" ) == 0 )
  {
    u.setQuery( QString::fromLatin1( formData.data(), formData.size() ) );

    args.frameName = target;
  }
  else
  {
    args.postData = formData;
    args.frameName = target;

    // construct some user headers if necessary
    if (contentType.isNull() || contentType == "application/x-www-form-urlencoded")
      d->m_userHeaders = "Content-Type: application/x-www-form-urlencoded";
    else // contentType must be "multipart/form-data"
      d->m_userHeaders = "Content-Type: " + contentType + "; boundary=" + boundary;
  }

  // ### bail out if a submit() call from JS occured while parsing
  if ( d->m_bParsing ) {
    kdWarning( 6070 ) << "Ignoring submit() while parsing" << endl;
    return;
  }

  emit d->m_extension->openURLRequest( u, args );

}

void KHTMLPart::popupMenu( const QString &url )
{
  KURL u( m_url );
  if ( !url.isEmpty() )
    u = KURL( m_url, url );
  /*
  mode_t mode = 0;
  if ( !u.isLocalFile() )
  {
    QString cURL = u.url( 1 );
    int i = cURL.length();
    // A url ending with '/' is always a directory
    if ( i >= 1 && cURL[ i - 1 ] == '/' )
      mode = S_IFDIR;
  }
  */
  mode_t mode = S_IFDIR; // treat all html documents as "DIR" in order to have the back/fwd/reload
                         // buttons in the popupmenu

  KXMLGUIClient *client = new KHTMLPopupGUIClient( this, d->m_popupMenuXML, url.isEmpty() ? KURL() : u );

  emit d->m_extension->popupMenu( client, QCursor::pos(), u, QString::fromLatin1( "text/html" ), mode );

  delete client;

  emit popupMenu(url, QCursor::pos());
}

void KHTMLPart::slotChildStarted( KIO::Job *job )
{
  khtml::ChildFrame *child = frame( sender() );

  assert( child );

  child->m_bCompleted = false;

  if ( d->m_bComplete )
  {
    if ( !parentPart() ) // "toplevel" html document? if yes, then notify the hosting browser about the document (url) changes
      emit d->m_extension->openURLNotify();
    d->m_bComplete = false;
    emit started( job );
  }
}

void KHTMLPart::slotChildCompleted()
{
  khtml::ChildFrame *child = frame( sender() );

  assert( child );

  child->m_bCompleted = true;
  child->m_args = KParts::URLArgs();

  checkCompleted();
}

void KHTMLPart::slotChildURLRequest( const KURL &url, const KParts::URLArgs &args )
{
  khtml::ChildFrame *child = frame( sender()->parent() );

  QString frameName = args.frameName.lower();
  if ( !frameName.isEmpty() )
  {
    if ( frameName == QString::fromLatin1( "_top" ) )
    {
      emit d->m_extension->openURLRequest( url, args );
      return;
    }
    else if ( frameName == QString::fromLatin1( "_blank" ) )
    {
      emit d->m_extension->createNewWindow( url, args );
      return;
    }
    else if ( frameName == QString::fromLatin1( "_parent" ) )
    {
      KParts::URLArgs newArgs( args );
      newArgs.frameName = QString::null;

      emit d->m_extension->openURLRequest( url, newArgs );
      return;
    }
    else if ( frameName != QString::fromLatin1( "_self" ) )
    {
      khtml::ChildFrame *_frame = recursiveFrameRequest( url, args );

      if ( !_frame )
      {
        emit d->m_extension->openURLRequest( url, args );
        return;
      }

      child = _frame;
    }
  }

  // TODO: handle child target correctly! currently the script are always executed fur the parent
  QString urlStr = url.url();
  if ( urlStr.find( QString::fromLatin1( "javascript:" ), 0, false ) == 0 ) {
      executeScript( urlStr.right( urlStr.length() - 11) );
      return;
  }

  if ( child ) {
      requestObject( child, url, args );
  }  else if ( frameName==QString::fromLatin1("_self") ) // this is for embedded objects (via <object>) which want to replace the current document
  {
      KParts::URLArgs newArgs( args );
      newArgs.frameName = QString::null;
      emit d->m_extension->openURLRequest( url, newArgs );
  }
}

khtml::ChildFrame *KHTMLPart::frame( const QObject *obj )
{
    assert( obj->inherits( "KParts::ReadOnlyPart" ) );
    const KParts::ReadOnlyPart *part = static_cast<const KParts::ReadOnlyPart *>( obj );

    FrameIt it = d->m_frames.begin();
    FrameIt end = d->m_frames.end();
    for (; it != end; ++it )
      if ( (KParts::ReadOnlyPart *)it.data().m_part == part )
        return &it.data();

    return 0L;
}

KHTMLPart *KHTMLPart::findFrame( const QString &f )
{
  // ### http://www.w3.org/TR/html4/appendix/notes.html#notes-frames
  ConstFrameIt it = d->m_frames.find( f );
  if ( it == d->m_frames.end() )
    return 0L;
  else {
    KParts::ReadOnlyPart *p = it.data().m_part;
    if ( p && p->inherits( "KHTMLPart" ))
      return (KHTMLPart*)p;
    else
      return 0L;
  }
}

KHTMLPart *KHTMLPart::parentPart()
{
  if ( !parent() || !parent()->inherits( "KHTMLPart" ) )
    return 0L;

  return (KHTMLPart *)parent();
}

khtml::ChildFrame *KHTMLPart::recursiveFrameRequest( const KURL &url, const KParts::URLArgs &args,
                                                     bool callParent )
{
  FrameIt it = d->m_frames.find( args.frameName );

  if ( it != d->m_frames.end() )
    return &it.data();

  it = d->m_frames.begin();
  FrameIt end = d->m_frames.end();
  for (; it != end; ++it )
    if ( it.data().m_part && it.data().m_part->inherits( "KHTMLPart" ) )
    {
      KHTMLPart *childPart = (KHTMLPart *)(KParts::ReadOnlyPart *)it.data().m_part;

      khtml::ChildFrame *res = childPart->recursiveFrameRequest( url, args, false );
      if ( !res )
        continue;

      childPart->requestObject( res, url, args );
      return 0L;
    }

  if ( parentPart() && callParent )
  {
    khtml::ChildFrame *res = parentPart()->recursiveFrameRequest( url, args );

    if ( res )
      parentPart()->requestObject( res, url, args );

    return 0L;
  }

  return 0L;
}

void KHTMLPart::saveState( QDataStream &stream )
{
  kdDebug( 6050 ) << "KHTMLPart::saveState saving URL " << m_url.url() << endl;

  stream << m_url << (Q_INT32)d->m_view->contentsX() << (Q_INT32)d->m_view->contentsY();

  // Save the doc's cache id.
  stream << d->m_cacheId;

  // Save the state of the document (Most notably the state of any forms)
  QStringList docState;
  if (d->m_doc)
  {
     docState = d->m_doc->state();
  }
  stream << docState;

  // Save font data
  stream << fontSizes() << d->m_fontBase;

  // Save frame data
  stream << (Q_UINT32)d->m_frames.count();

  QStringList frameNameLst, frameServiceTypeLst, frameServiceNameLst;
  KURL::List frameURLLst;
  QValueList<QByteArray> frameStateBufferLst;

  ConstFrameIt it = d->m_frames.begin();
  ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it )
  {
    frameNameLst << (*it).m_name;
    frameServiceTypeLst << (*it).m_serviceType;
    frameServiceNameLst << (*it).m_serviceName;
    if ( (*it).m_part )
      frameURLLst << (*it).m_part->url();
    else
      frameURLLst << KURL();

    QByteArray state;
    QDataStream frameStream( state, IO_WriteOnly );

    if ( (*it).m_part && (*it).m_extension )
      (*it).m_extension->saveState( frameStream );

    frameStateBufferLst << state;
  }

  stream << frameNameLst << frameServiceTypeLst << frameServiceNameLst << frameURLLst << frameStateBufferLst;
  stream << *KHTMLFactory::vLinks();
}

void KHTMLPart::restoreState( QDataStream &stream )
{
  KURL u;
  Q_INT32 xOffset;
  Q_INT32 yOffset;
  Q_UINT32 frameCount;
  QStringList frameNames, frameServiceTypes, docState, frameServiceNames;
  KURL::List frameURLs;
  QValueList<QByteArray> frameStateBuffers;
  QValueList<int> fSizes;
  KURL::List visitedLinks;

  stream >> u >> xOffset >> yOffset;

  stream >> d->m_cacheId;

  stream >> docState;

  stream >> fSizes >> d->m_fontBase;
  setFontSizes( fSizes );

  stream >> frameCount >> frameNames >> frameServiceTypes >> frameServiceNames
         >> frameURLs >> frameStateBuffers;

  stream >> visitedLinks;

  if ( KHTMLFactory::partList()->count() == 1 ) // only us?
      *KHTMLFactory::vLinks() = visitedLinks;

  d->m_bComplete = false;

  kdDebug( 6050 ) << "restoreStakte() docState.count() = " << docState.count() << endl;
  kdDebug( 6050 ) << "m_url " << m_url.url() << " <-> " << u.url() << endl;
  kdDebug( 6050 ) << "m_frames.count() " << d->m_frames.count() << " <-> " << frameCount << endl;

  if ( u == m_url && frameCount >= 1 && frameCount == d->m_frames.count() )
  {
    kdDebug( 6050 ) << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!! partial restoring !!!!!!!!!!!!!!!!!!!!!" << endl;
//    emit started( 0L );

    FrameIt fIt = d->m_frames.begin();
    FrameIt fEnd = d->m_frames.end();

    for (; fIt != fEnd; ++fIt )
        (*fIt).m_bCompleted = false;

    fIt = d->m_frames.begin();

    QStringList::ConstIterator fNameIt = frameNames.begin();
    QStringList::ConstIterator fServiceTypeIt = frameServiceTypes.begin();
    QStringList::ConstIterator fServiceNameIt = frameServiceNames.begin();
    KURL::List::ConstIterator fURLIt = frameURLs.begin();
    QValueList<QByteArray>::ConstIterator fBufferIt = frameStateBuffers.begin();

    for (; fIt != fEnd; ++fIt, ++fNameIt, ++fServiceTypeIt, ++fServiceNameIt, ++fURLIt, ++fBufferIt )
    {
      khtml::ChildFrame *child = &(*fIt);

      kdDebug( 6050 ) <<  *fNameIt  << " ---- " <<  *fServiceTypeIt << endl;

      if ( child->m_name != *fNameIt || child->m_serviceType != *fServiceTypeIt )
      {
        child->m_bPreloaded = true;
        child->m_name = *fNameIt;
        child->m_serviceName = *fServiceNameIt;
        processObjectRequest( child, *fURLIt, *fServiceTypeIt );
      }

      if ( child->m_part )
      {
        child->m_bCompleted = false;
        if ( child->m_extension )
        {
          QDataStream frameStream( *fBufferIt, IO_ReadOnly );
          child->m_extension->restoreState( frameStream );
        }
        else
          child->m_part->openURL( *fURLIt );
      }
    }

    //    emit completed();
  }
  else
  {
    if ( !urlcmp( u.url(), m_url.url(), true, true ) )
      clear();

    QStringList::ConstIterator fNameIt = frameNames.begin();
    QStringList::ConstIterator fNameEnd = frameNames.end();

    QStringList::ConstIterator fServiceTypeIt = frameServiceTypes.begin();
    QStringList::ConstIterator fServiceNameIt = frameServiceNames.begin();
    KURL::List::ConstIterator fURLIt = frameURLs.begin();
    QValueList<QByteArray>::ConstIterator fBufferIt = frameStateBuffers.begin();

    for (; fNameIt != fNameEnd; ++fNameIt, ++fServiceTypeIt, ++fServiceNameIt, ++fURLIt, ++fBufferIt )
    {
      khtml::ChildFrame newChild;
      newChild.m_bPreloaded = true;
      newChild.m_name = *fNameIt;
      newChild.m_serviceName = *fServiceNameIt;

      kdDebug( 6050 ) << *fNameIt << " ---- " << *fServiceTypeIt << endl;

      FrameIt childFrame = d->m_frames.insert( *fNameIt, newChild );

      processObjectRequest( &childFrame.data(), *fURLIt, *fServiceTypeIt );

      childFrame.data().m_bPreloaded = true;

      if ( childFrame.data().m_part )
      {
        if ( childFrame.data().m_extension )
        {
          QDataStream frameStream( *fBufferIt, IO_ReadOnly );
          childFrame.data().m_extension->restoreState( frameStream );
        }
        else
          childFrame.data().m_part->openURL( *fURLIt );
      }
    }

    KParts::URLArgs args( d->m_extension->urlArgs() );
    args.xOffset = xOffset;
    args.yOffset = yOffset;
    args.docState = docState;
    d->m_extension->setURLArgs( args );
    kdDebug( 6050 ) << "in restoreState : calling openURL for " << u.url() << endl;
    if (!KHTMLPageCache::self()->isValid(d->m_cacheId))
       openURL( u );
    else
       restoreURL( u );
  }

}

void KHTMLPart::show()
{
  if ( d->m_view )
    d->m_view->show();
}

void KHTMLPart::hide()
{
  if ( d->m_view )
    d->m_view->hide();
}

DOM::Node KHTMLPart::nodeUnderMouse() const
{
    return d->m_view->nodeUnderMouse();
}

void KHTMLPart::emitSelectionChanged()
{
  emit d->m_extension->enableAction( "copy", hasSelection() );
  emit d->m_extension->selectionInfo( selectedText() );
  emit selectionChanged();
}

void KHTMLPart::slotIncFontSizes()
{
  updateFontSize( ++d->m_fontBase );
  if ( !d->m_paDecFontSizes->isEnabled() )
    d->m_paDecFontSizes->setEnabled( true );
}

void KHTMLPart::slotDecFontSizes()
{
  if ( d->m_fontBase >= 1 )
    updateFontSize( --d->m_fontBase );

  if ( d->m_fontBase == 0 )
    d->m_paDecFontSizes->setEnabled( false );
}

void KHTMLPart::setFontBaseInternal( int base, bool absolute )
{
    if ( absolute )
      d->m_fontBase = base;
    else
      d->m_fontBase += base;

    if ( d->m_fontBase < 0 )
        d->m_fontBase = 0;

   d->m_paDecFontSizes->setEnabled( d->m_fontBase > 0 );

    updateFontSize( d->m_fontBase );
}

void KHTMLPart::updateFontSize( int add )
{
  resetFontSizes();
  QValueList<int> sizes = fontSizes();

  QValueList<int>::Iterator it = sizes.begin();
  QValueList<int>::Iterator end = sizes.end();
  for (; it != end; ++it )
    (*it) += add;

  setFontSizes( sizes );

  QApplication::setOverrideCursor( waitCursor );
  if(d->m_doc) d->m_doc->applyChanges();
  QApplication::restoreOverrideCursor();
}

void KHTMLPart::slotLoadImages()
{
  khtml::Cache::autoloadImages( !khtml::Cache::autoloadImages() );
  khtml::Cache::autoloadImages( d->m_settings->autoLoadImages() );
}

void KHTMLPart::reparseConfiguration()
{
  KHTMLSettings *settings = KHTMLFactory::defaultHTMLSettings();
  settings->init();

  autoloadImages( settings->autoLoadImages() );

  // PENDING(lars) Pass hostname to the following two methods.
  d->m_bJScriptEnabled = settings->isJavaScriptEnabled();
  d->m_bJavaEnabled = settings->isJavaEnabled();
  delete d->m_settings;
  d->m_settings = new KHTMLSettings(*KHTMLFactory::defaultHTMLSettings());

  QApplication::setOverrideCursor( waitCursor );
  if(d->m_doc) d->m_doc->applyChanges();
  QApplication::restoreOverrideCursor();
}

QStringList KHTMLPart::frameNames() const
{
  QStringList res;

  ConstFrameIt it = d->m_frames.begin();
  ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it )
    res += it.key();

  return res;
}

const QList<KParts::ReadOnlyPart> KHTMLPart::frames() const
{
  QList<KParts::ReadOnlyPart> res;

  ConstFrameIt it = d->m_frames.begin();
  ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it )
     res.append( it.data().m_part );

  return res;
}

bool KHTMLPart::openURLInFrame( const KURL &url, const KParts::URLArgs &urlArgs )
{
  FrameIt it = d->m_frames.find( urlArgs.frameName );

  if ( it == d->m_frames.end() )
    return false;

  requestObject( &it.data(), url, urlArgs );

  return true;
}

void KHTMLPart::setDNDEnabled( bool b )
{
  d->m_bDnd = b;
}

bool KHTMLPart::dndEnabled() const
{
  return d->m_bDnd;
}

bool KHTMLPart::event( QEvent *event )
{
  if ( KParts::ReadOnlyPart::event( event ) )
   return true;

  if ( khtml::MousePressEvent::test( event ) )
  {
    khtmlMousePressEvent( static_cast<khtml::MousePressEvent *>( event ) );
    return true;
  }

  if ( khtml::MouseDoubleClickEvent::test( event ) )
  {
    khtmlMouseDoubleClickEvent( static_cast<khtml::MouseDoubleClickEvent *>( event ) );
    return true;
  }

  if ( khtml::MouseMoveEvent::test( event ) )
  {
    khtmlMouseMoveEvent( static_cast<khtml::MouseMoveEvent *>( event ) );
    return true;
  }

  if ( khtml::MouseReleaseEvent::test( event ) )
  {
    khtmlMouseReleaseEvent( static_cast<khtml::MouseReleaseEvent *>( event ) );
    return true;
  }

  if ( khtml::DrawContentsEvent::test( event ) )
  {
    khtmlDrawContentsEvent( static_cast<khtml::DrawContentsEvent *>( event ) );
    return true;
  }

  return false;
}

void KHTMLPart::khtmlMousePressEvent( khtml::MousePressEvent *event )
{
  DOM::DOMString url = event->url();
  QMouseEvent *_mouse = event->qmouseEvent();
  DOM::Node innerNode = event->innerNode();

   d->m_dragStartPos = _mouse->pos();

  if ( event->url() != 0 )
    d->m_strSelectedURL = event->url().string();
  else
    d->m_strSelectedURL = QString::null;

  if ( _mouse->button() == LeftButton ||
       _mouse->button() == MidButton )
  {
    d->m_bMousePressed = true;

    if ( _mouse->button() == LeftButton )
    {
      if ( !innerNode.isNull() )
      {
        d->m_selectionStart = innerNode;
        d->m_startOffset = event->offset();
        d->m_selectionEnd = innerNode;
        d->m_endOffset = d->m_startOffset;
        d->m_doc->clearSelection();
      }
      else
      {
        d->m_selectionStart = DOM::Node();
        d->m_selectionEnd = DOM::Node();
      }
      emitSelectionChanged();
      startAutoScroll();
    }
  }

  QString currentUrl = splitUrlTarget(d->m_strSelectedURL);
  if ( _mouse->button() == RightButton )
  {
    popupMenu( currentUrl );
    d->m_strSelectedURL = QString::null;
  }
}

void KHTMLPart::khtmlMouseDoubleClickEvent( khtml::MouseDoubleClickEvent * )
{
}

void KHTMLPart::khtmlMouseMoveEvent( khtml::MouseMoveEvent *event )
{
  QMouseEvent *_mouse = event->qmouseEvent();
  DOM::DOMString url = event->url();
  DOM::Node innerNode = event->innerNode();

  if( d->m_bMousePressed && (!d->m_strSelectedURL.isEmpty() || innerNode.elementId() == ID_IMG ) &&
      ( d->m_dragStartPos - _mouse->pos() ).manhattanLength() > KGlobalSettings::dndEventDelay() &&
      d->m_bDnd ) {

      QPixmap p;
      QDragObject *drag = 0;
      if( !d->m_strSelectedURL.isEmpty() ) {
          QStringList uris;
          KURL u( completeURL( splitUrlTarget(d->m_strSelectedURL)) );
          uris.append( u.url() );
          QUriDrag *udrag = new QUriDrag( d->m_view->viewport() );
          udrag->setUnicodeUris( uris );
          drag = udrag;
          p = KMimeType::pixmapForURL(u, 0, KIcon::SizeMedium);
      } else {
          HTMLImageElementImpl *i = static_cast<HTMLImageElementImpl *>(innerNode.handle());
          if( i ) {
              khtml::RenderImage *r = static_cast<khtml::RenderImage *>(i->renderer());
              if(r) {
                  drag = new QImageDrag( r->pixmap().convertToImage() , d->m_view->viewport() );
                  kdDebug(0) << " creating image drag " << endl;
                  p = KMimeType::mimeType("image/*")->pixmap(KIcon::Desktop);
              }
          }
      }

    if ( !p.isNull() )
      drag->setPixmap(p);
    else
      kdDebug( 6000 ) << "null pixmap" << endl;

    kdDebug(0) << "drag = " << drag;

    stopAutoScroll();
    if(drag)
        drag->drag();

    // when we finish our drag, we need to undo our mouse press
    d->m_bMousePressed = false;
    d->m_strSelectedURL = "";
    return;
  }

  if ( !d->m_bMousePressed && url.length() && innerNode.elementId() == ID_IMG )
  {
    HTMLImageElementImpl *i = static_cast<HTMLImageElementImpl *>(innerNode.handle());
    if ( i && i->isServerMap() )
    {
      khtml::RenderImage *r = static_cast<khtml::RenderImage *>(i->renderer());
      if(r)
      {
        int absx, absy, vx, vy;
        r->absolutePosition(absx, absy);
        view()->contentsToViewport( absx, absy, vx, vy );

        int x(_mouse->x() - vx), y(_mouse->y() - vy);

        QString target;
        QString surl = splitUrlTarget(url.string(), &target);
        d->m_overURL = surl + QString("?%1,%2").arg(x).arg(y);
        overURL( d->m_overURL, target );
        return;
      }
    }
  }

  if ( !d->m_bMousePressed && url.length() )
  {
    QString target;
    QString surl = splitUrlTarget(url.string(), &target);
    if ( d->m_overURL.isEmpty() )
    {
      d->m_overURL = surl;
      overURL( d->m_overURL, target );
    }
    else if ( d->m_overURL != surl )
    {
      d->m_overURL = surl;
      overURL( d->m_overURL, target );
    }
    return;
  }
  else if( d->m_overURL.length() && !url.length() )
  {
    overURL( QString::null, QString::null );
    d->m_overURL = "";
  }

  // selection stuff
  if( d->m_bMousePressed && !innerNode.isNull() && innerNode.nodeType() == DOM::Node::TEXT_NODE ) {
        d->m_selectionEnd = innerNode;
        d->m_endOffset = event->offset();
	//kdDebug( 6000 ) << "setting end of selection to " << innerNode.handle() << "/" << event->offset() << endl;

        // we have to get to know if end is before start or not...
        DOM::Node n = d->m_selectionStart;
        d->m_startBeforeEnd = false;
        while(!n.isNull()) {
            if(n == d->m_selectionEnd) {
                d->m_startBeforeEnd = true;
                break;
            }
            DOM::Node next = n.firstChild();
            if(next.isNull()) next = n.nextSibling();
            while( next.isNull() && !n.parentNode().isNull() ) {
                n = n.parentNode();
                next = n.nextSibling();
            }
            n = next;
            //d->m_view->viewport()->repaint(false);
        }

        if ( !d->m_selectionStart.isNull() && !d->m_selectionEnd.isNull() )
        {
          if (d->m_selectionEnd == d->m_selectionStart && d->m_endOffset < d->m_startOffset)
               d->m_doc
                  ->setSelection(d->m_selectionStart.handle(),d->m_endOffset,
                                 d->m_selectionEnd.handle(),d->m_startOffset);
          else if (d->m_startBeforeEnd)
              d->m_doc
                  ->setSelection(d->m_selectionStart.handle(),d->m_startOffset,
                                 d->m_selectionEnd.handle(),d->m_endOffset);
          else
              d->m_doc
                  ->setSelection(d->m_selectionEnd.handle(),d->m_endOffset,
                                 d->m_selectionStart.handle(),d->m_startOffset);
        }

    }
}


void KHTMLPart::khtmlMouseReleaseEvent( khtml::MouseReleaseEvent *event )
{
  QMouseEvent *_mouse = event->qmouseEvent();
  DOM::Node innerNode = event->innerNode();

  if ( d->m_bMousePressed )
  {
    // in case we started an autoscroll in MouseMove event
    // ###
    stopAutoScroll();
    //disconnect( this, SLOT( slotUpdateSelectText(int) ) );
  }

  // Used to prevent mouseMoveEvent from initiating a drag before
  // the mouse is pressed again.
  d->m_bMousePressed = false;

  if ( !d->m_strSelectedURL.isEmpty() && _mouse->button() != RightButton )
  {
     QString target;
     QString url;
     if( _mouse->button() == LeftButton ) {
       url = splitUrlTarget(d->m_strSelectedURL, &target);
     } else {
       url = splitUrlTarget(d->m_strSelectedURL);
       target = "_blank";
     }
     kdDebug( 6000 ) << "m_strSelectedURL='" << url << "' target=" << target << endl;

     // Visual action effect, over text links
     if ( !innerNode.isNull() && innerNode.nodeType() == DOM::Node::TEXT_NODE )
     {
       khtml::RenderText * renderText = static_cast<khtml::RenderText *>(innerNode.handle()->renderer());
       khtml::TextSlave * firstSlave = renderText->first();
       //QRect r ( firstSlave->x, firstSlave->y, firstSlave->m_width, firstSlave->m_height );
       int x, y;
       renderText->absolutePosition( x, y );
       //QRect r ( x, y, renderText->width(), renderText->height() );
       int vx, vy;
       view()->contentsToViewport( x, y, vx, vy );
       QRect r ( vx, vy, firstSlave->m_width, firstSlave->m_height );
       kdDebug( 6000 ) << " x=" << r.x() << " y=" << r.y() << " width=" << r.width() << " height=" << r.height() << endl;
       KIconEffect::visualActivate( view()->viewport(), r );
     }
     else if ( !innerNode.isNull() && innerNode.elementId() == ID_IMG ) {
       HTMLImageElementImpl *i = static_cast<HTMLImageElementImpl *>(innerNode.handle());
       if ( i && i->isServerMap() )
       {
         khtml::RenderImage *r = static_cast<khtml::RenderImage *>(i->renderer());
         if(r)
         {
           int absx, absy, vx, vy;
           r->absolutePosition(absx, absy);
           view()->contentsToViewport( absx, absy, vx, vy );

           int x(_mouse->x() - vx), y(_mouse->y() - vy);
           url += QString("?%1,%2").arg(x).arg(y);
         }
       }
     }

     urlSelected( url, _mouse->button(), _mouse->state(), target );
   }

  if(!innerNode.isNull() && innerNode.nodeType() == DOM::Node::TEXT_NODE) {
  //    kdDebug( 6000 ) << "final range of selection to " << d->selectionStart << "/" << d->startOffset << " --> " << innerNode << "/" << offset << endl;
        d->m_selectionEnd = innerNode;
        d->m_endOffset = event->offset();
    }

    // delete selection in case start and end position are at the same point
    if(d->m_selectionStart == d->m_selectionEnd && d->m_startOffset == d->m_endOffset) {
        d->m_selectionStart = 0;
        d->m_selectionEnd = 0;
        d->m_startOffset = 0;
        d->m_endOffset = 0;
        emitSelectionChanged();
    } else {
        // we have to get to know if end is before start or not...
        DOM::Node n = d->m_selectionStart;
        d->m_startBeforeEnd = false;
	if( d->m_selectionStart == d->m_selectionEnd ) {
	    if( d->m_startOffset < d->m_endOffset )
		d->m_startBeforeEnd = true;
	} else {
        while(!n.isNull()) {
            if(n == d->m_selectionEnd) {
                d->m_startBeforeEnd = true;
                break;
            }
            DOM::Node next = n.firstChild();
            if(next.isNull()) next = n.nextSibling();
            while( next.isNull() && !n.parentNode().isNull() ) {
                n = n.parentNode();
                next = n.nextSibling();
            }
            n = next;
        }
	}
        if(!d->m_startBeforeEnd)
        {
            DOM::Node tmpNode = d->m_selectionStart;
            int tmpOffset = d->m_startOffset;
            d->m_selectionStart = d->m_selectionEnd;
            d->m_startOffset = d->m_endOffset;
            d->m_selectionEnd = tmpNode;
            d->m_endOffset = tmpOffset;
            d->m_startBeforeEnd = true;
        }
        // get selected text and paste to the clipboard
        QString text = selectedText();
        text.replace(QRegExp(QChar(0xa0)), " ");
        QClipboard *cb = QApplication::clipboard();
        cb->setText(text);
        //kdDebug( 6000 ) << "selectedText = " << text << endl;
        emitSelectionChanged();
    }
}

void KHTMLPart::khtmlDrawContentsEvent( khtml::DrawContentsEvent * )
{
}

void KHTMLPart::guiActivateEvent( KParts::GUIActivateEvent *event )
{
  if ( event->activated() )
  {
    emitSelectionChanged();
    emit d->m_extension->enableAction( "print", d->m_doc != 0 );

    if ( !d->m_settings->autoLoadImages() && d->m_paLoadImages )
    {
        QList<KAction> lst;
        lst.append( d->m_paLoadImages );
        plugActionList( "loadImages", lst );
    }
  }
}

void KHTMLPart::slotFind()
{
  KHTMLPart *part = this;

  if ( d->m_frames.count() > 0 )
    part = static_cast<KHTMLPart *>( partManager()->activePart() );

  assert( part->inherits( "KHTMLPart" ) );
  assert( d->m_findDialog == 0 );

  // use the part's (possibly frame) widget as parent widget, so that it gets
  // properly destroyed when the (possible) frame dies
  KHTMLFind *findDlg = new KHTMLFind( part, part->widget(), "khtmlfind" );
  findDlg->setText( part->d->m_lastFindState.text );
  findDlg->setCaseSensitive( part->d->m_lastFindState.caseSensitive );
  findDlg->setDirection( part->d->m_lastFindState.direction );

  findDlg->show();
  connect( findDlg, SIGNAL( done() ),
           this, SLOT( slotFindDone() ) );
  connect( findDlg, SIGNAL( destroyed() ),
           this, SLOT( slotFindDialogDestroyed() ) );

  d->m_findDialog = findDlg;

  d->m_paFind->setEnabled( false );
}

void KHTMLPart::slotFindDone()
{
    assert( d->m_findDialog );

    KHTMLPart *part = d->m_findDialog->part();

    // this code actually belongs into some saveState() method in
    // KHTMLFind, but as we're saving into the private data section of
    // the part we have to do it here (no way to access it from the outside
    // as it is defined only in khtml_part.cpp) (Simon)
    part->d->m_lastFindState.text = d->m_findDialog->getText();
    part->d->m_lastFindState.caseSensitive = d->m_findDialog->case_sensitive();
    part->d->m_lastFindState.direction = d->m_findDialog->get_direction();

    // will trigger the method below
    delete d->m_findDialog;
}

void KHTMLPart::slotFindDialogDestroyed()
{
    assert( sender() == d->m_findDialog );

    d->m_findDialog = 0;
    d->m_paFind->setEnabled( true );
}

void KHTMLPart::slotPrintFrame()
{
  if ( d->m_frames.count() == 0 )
    return;

  KParts::Part *frame = partManager()->activePart();

  KParts::BrowserExtension *ext = static_cast<KParts::BrowserExtension *>( frame->child( 0, "KParts::BrowserExtension" ) );

  if ( !ext )
    return;

  QMetaData *mdata = ext->metaObject()->slot( "print()" );
  if ( mdata )
    (ext->*(mdata->ptr))();
}

void KHTMLPart::slotSelectAll()
{
  KHTMLPart *part = this;

  if ( d->m_frames.count() > 0 )
    part = static_cast<KHTMLPart *>( partManager()->activePart() );

  assert( part );

  part->selectAll();
}

void KHTMLPart::startAutoScroll()
{
   connect(&d->m_scrollTimer, SIGNAL( timeout() ), this, SLOT( slotAutoScroll() ));
   d->m_scrollTimer.start(100, false);
}

void KHTMLPart::stopAutoScroll()
{
   disconnect(&d->m_scrollTimer, SIGNAL( timeout() ), this, SLOT( slotAutoScroll() ));
   if (d->m_scrollTimer.isActive())
       d->m_scrollTimer.stop();
}


void KHTMLPart::slotAutoScroll()
{
    if (d->m_view)
      d->m_view->doAutoScroll();
    else
      stopAutoScroll(); // Safety
}

void KHTMLPart::selectAll()
{
  NodeImpl *first = d->m_doc->body();
  NodeImpl *next;

  while ( first && first->id() != ID_TEXT )
  {
    next = first->firstChild();
    if ( !next ) next = first->nextSibling();
    while( first && !next )
    {
      first = first->parentNode();
      if ( first )
        next = first->nextSibling();
    }
    first = next;
  }

  NodeImpl *last = d->m_doc->body();
  while ( last && last->id() != ID_TEXT )
  {
    next = last->lastChild();
    if ( !next ) next = last->previousSibling();
    while ( last && !next )
    {
      last = last->parentNode();
      if ( last )
        next = last->previousSibling();
    }
    last = next;
  }

  if ( !first || !last )
    return;

  d->m_selectionStart = first;
  d->m_startOffset = 0;
  d->m_selectionEnd = last;
  d->m_endOffset = static_cast<TextImpl *>( last )->string()->l;
  d->m_startBeforeEnd = true;

  d->m_doc->setSelection( d->m_selectionStart.handle(), d->m_startOffset,
                          d->m_selectionEnd.handle(), d->m_endOffset );

  emitSelectionChanged();
}

bool KHTMLPart::checkLinkSecurity(KURL linkURL)
{
  // Security check on the link.
  // KURL u( url ); Wrong!! Relative URL could be mis-interpreted!!! (DA)
  if ( !linkURL.protocol().isNull() && !m_url.protocol().isNull() &&
       ( linkURL.protocol().lower() == "cgi" || linkURL.protocol().lower() == "file" ) &&
       m_url.protocol().lower() != "file" && m_url.protocol().lower() != "cgi" )
  {
    KMessageBox::error( 0,
                        i18n( "This page is untrusted\nbut it contains a link to your local file system."),
                        i18n( "Security Alert" ));
    return false;
  }
  return true;
}

void KHTMLPart::slotPartRemoved( KParts::Part *part )
{
    kdDebug(6050) << "KHTMLPart::slotPartRemoved " << part << endl;
    if ( part == d->m_activeFrame )
        d->m_activeFrame = 0L;
}

void KHTMLPart::slotActiveFrameChanged( KParts::Part *part )
{
    kdDebug(6050) << "KHTMLPart::slotActiveFrameChanged part=" << part << endl;
    if ( part == this )
    {
        kdDebug() << "strange error! we activated ourselves" << endl;
        assert( false );
        return;
    }
    kdDebug(6050) << "KHTMLPart::slotActiveFrameChanged d->m_activeFrame=" << d->m_activeFrame << endl;
    if ( d->m_activeFrame && d->m_activeFrame->widget()->inherits( "QFrame" ) )
    {
        QFrame *frame = static_cast<QFrame *>( d->m_activeFrame->widget() );
        frame->setLineWidth( frame->style().defaultFrameWidth() );
    }
    d->m_activeFrame = part;
    if ( d->m_activeFrame && d->m_activeFrame->widget()->inherits( "QFrame" ) )
    {
        QFrame *frame = static_cast<QFrame *>( d->m_activeFrame->widget() );
        frame->setLineWidth( frame->lineWidth() + 2 );
        kdDebug(6050) << "new active frame " << d->m_activeFrame << endl;
    }
    updateActions();
}

#include "khtml_part.moc"
