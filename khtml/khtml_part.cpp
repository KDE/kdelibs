// -*- c-basic-offset: 2 -*-
/* This file is part of the KDE project
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999 Lars Knoll <knoll@kde.org>
 *                     1999 Antti Koivisto <koivisto@kde.org>
 *                     2000 Simon Hausmann <hausmann@kde.org>
 *                     2000 Stefan Schimanski <1Stein@gmx.de>
 *                     2001-2003 George Staikos <staikos@kde.org>
 *                     2001-2003 Dirk Mueller <mueller@kde.org>
 *                     2002 Apple Computer, Inc.
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

#define SPEED_DEBUG
#include "khtml_part.h"

#include "khtml_pagecache.h"

#include "dom/dom_string.h"
#include "dom/dom_element.h"
#include "html/html_documentimpl.h"
#include "html/html_baseimpl.h"
#include "html/html_miscimpl.h"
#include "html/html_imageimpl.h"
#include "rendering/render_text.h"
#include "rendering/render_frames.h"
#include "rendering/render_layer.h"
#include "misc/htmlhashes.h"
#include "misc/loader.h"
#include "xml/dom2_eventsimpl.h"
#include "xml/dom2_rangeimpl.h"
#include "xml/xml_tokenizer.h"
#include "css/cssstyleselector.h"
#include "css/csshelper.h"
using namespace DOM;

#include "khtmlview.h"
#include <kparts/partmanager.h>
#include "ecma/kjs_proxy.h"
#include "khtml_settings.h"
#include "kjserrordlg.h"

#include <kjs/function.h>
#include <kjs/interpreter.h>

#include "htmlpageinfo.h"

#include <sys/types.h>
#include <assert.h>
#include <unistd.h>

#include <config.h>

#include <dcopclient.h>
#include <dcopref.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <kio/job.h>
#include <kio/global.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kmessagebox.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <ktrader.h>
#include <kdatastream.h>
#include <ktempfile.h>
#include <kglobalsettings.h>
#include <kurldrag.h>
#include <kapplication.h>
#include <kparts/browserinterface.h>
#if !defined(QT_NO_DRAGANDDROP)
#include <kmultipledrag.h>
#endif
#include "../kutils/kfinddialog.h"
#include "../kutils/kfind.h"

#include <ksslcertchain.h>
#include <ksslinfodlg.h>

#include <kfileitem.h>
#include <kurifilter.h>
#include <kstatusbar.h>
#include <kurllabel.h>

#include <qclipboard.h>
#include <qfile.h>
#include <qtooltip.h>
#include <qmetaobject.h>
#include <private/qucomextra_p.h>

#include "khtmlpart_p.h"
#include "kpopupmenu.h"
#include "rendering/render_form.h"
#include <kwin.h>

#define HINT_UTF8	106

namespace khtml {
    class PartStyleSheetLoader : public CachedObjectClient
    {
    public:
        PartStyleSheetLoader(KHTMLPart *part, DOM::DOMString url, DocLoader* dl)
        {
            m_part = part;
            m_cachedSheet = dl->requestStyleSheet(url, QString::null, "text/css",
                                                  true /* "user sheet" */);
            if (m_cachedSheet)
		m_cachedSheet->ref( this );
        }
        virtual ~PartStyleSheetLoader()
        {
            if ( m_cachedSheet ) m_cachedSheet->deref(this);
        }
        virtual void setStyleSheet(const DOM::DOMString&, const DOM::DOMString &sheet)
        {
          if ( m_part )
            m_part->setUserStyleSheet( sheet.string() );

            delete this;
        }
        virtual void error( int, const QString& ) {
          delete this;
        }
        QGuardedPtr<KHTMLPart> m_part;
        khtml::CachedCSSStyleSheet *m_cachedSheet;
    };
}


KHTMLFrameList::Iterator KHTMLFrameList::find( const QString &name )
{
    Iterator it = begin();
    Iterator e = end();

    for (; it!=e; ++it )
        if ( (*it).m_name==name )
            break;

    return it;
}

KHTMLPart::KHTMLPart( QWidget *parentWidget, const char *widgetname, QObject *parent, const char *name, GUIProfile prof )
: KParts::ReadOnlyPart( parent, name )
{
    d = 0;
    KHTMLFactory::registerPart( this );
    setInstance( KHTMLFactory::instance(), prof == BrowserViewGUI && !parentPart() );
    init( new KHTMLView( this, parentWidget, widgetname ), prof );
}

KHTMLPart::KHTMLPart( KHTMLView *view, QObject *parent, const char *name, GUIProfile prof )
: KParts::ReadOnlyPart( parent, name )
{
    d = 0;
    KHTMLFactory::registerPart( this );
    setInstance( KHTMLFactory::instance(), prof == BrowserViewGUI && !parentPart() );
    assert( view );
    init( view, prof );
}

void KHTMLPart::init( KHTMLView *view, GUIProfile prof )
{
  if ( prof == DefaultGUI )
    setXMLFile( "khtml.rc" );
  else if ( prof == BrowserViewGUI )
    setXMLFile( "khtml_browser.rc" );

  d = new KHTMLPartPrivate(parent());

  d->m_view = view;
  setWidget( d->m_view );

  d->m_guiProfile = prof;
  d->m_extension = new KHTMLPartBrowserExtension( this );
  d->m_hostExtension = new KHTMLPartBrowserHostExtension( this );
  d->m_statusBarExtension = new KParts::StatusBarExtension( this );
  d->m_statusBarIconLabel = 0L;

  d->m_bSecurityInQuestion = false;
  d->m_paLoadImages = 0;
  d->m_paDebugScript = 0;
  d->m_bMousePressed = false;
  d->m_bRightMousePressed = false;
  d->m_paViewDocument = new KAction( i18n( "View Do&cument Source" ), CTRL + Key_U, this, SLOT( slotViewDocumentSource() ), actionCollection(), "viewDocumentSource" );
  d->m_paViewFrame = new KAction( i18n( "View Frame Source" ), 0, this, SLOT( slotViewFrameSource() ), actionCollection(), "viewFrameSource" );
  d->m_paViewInfo = new KAction( i18n( "View Document Information" ), CTRL+Key_I, this, SLOT( slotViewPageInfo() ), actionCollection(), "viewPageInfo" );
  d->m_paSaveBackground = new KAction( i18n( "Save &Background Image As..." ), 0, this, SLOT( slotSaveBackground() ), actionCollection(), "saveBackground" );
  d->m_paSaveDocument = new KAction( i18n( "&Save As..." ), CTRL+Key_S, this, SLOT( slotSaveDocument() ), actionCollection(), "saveDocument" );
  if ( parentPart() )
      d->m_paSaveDocument->setShortcut( KShortcut() ); // avoid clashes
  d->m_paSaveFrame = new KAction( i18n( "Save &Frame As..." ), 0, this, SLOT( slotSaveFrame() ), actionCollection(), "saveFrame" );
  d->m_paSecurity = new KAction( i18n( "Security..." ), "decrypted", 0, this, SLOT( slotSecurity() ), actionCollection(), "security" );
  d->m_paSecurity->setWhatsThis( i18n( "Security Settings<p>"
                                       "Shows the certificate of the displayed page. Only "
				       "pages that have been transmitted using a secure, encrypted connection have a "
				       "certificate.<p> "
				       "Hint: If the image shows a closed lock, the page has been transmitted over a "
				       "secure connection.") );
  d->m_paDebugRenderTree = new KAction( i18n( "Print Rendering Tree to STDOUT" ), 0, this, SLOT( slotDebugRenderTree() ), actionCollection(), "debugRenderTree" );
  d->m_paDebugDOMTree = new KAction( i18n( "Print DOM Tree to STDOUT" ), 0, this, SLOT( slotDebugDOMTree() ), actionCollection(), "debugDOMTree" );
  d->m_paStopAnimations = new KAction( i18n( "Stop Animated Images" ), 0, this, SLOT( slotStopAnimations() ), actionCollection(), "stopAnimations" );

  d->m_paSetEncoding = new KActionMenu( i18n( "Set &Encoding" ), "charset", actionCollection(), "setEncoding" );
  d->m_paSetEncoding->setDelayed( false );

  d->m_automaticDetection = new KPopupMenu( 0L );

  d->m_automaticDetection->insertItem( i18n( "Semi-Automatic" ), 0 );
  d->m_automaticDetection->insertItem( i18n( "Arabic" ), 1 );
  d->m_automaticDetection->insertItem( i18n( "Baltic" ), 2 );
  d->m_automaticDetection->insertItem( i18n( "Central European" ), 3 );
  //d->m_automaticDetection->insertItem( i18n( "Chinese" ), 4 );
  d->m_automaticDetection->insertItem( i18n( "Greek" ), 5 );
  d->m_automaticDetection->insertItem( i18n( "Hebrew" ), 6 );
  d->m_automaticDetection->insertItem( i18n( "Japanese" ), 7 );
  //d->m_automaticDetection->insertItem( i18n( "Korean" ), 8 );
  d->m_automaticDetection->insertItem( i18n( "Russian" ), 9 );
  //d->m_automaticDetection->insertItem( i18n( "Thai" ), 10 );
  d->m_automaticDetection->insertItem( i18n( "Turkish" ), 11 );
  d->m_automaticDetection->insertItem( i18n( "Ukrainian" ), 12 );
  //d->m_automaticDetection->insertItem( i18n( "Unicode" ), 13 );
  d->m_automaticDetection->insertItem( i18n( "Western European" ), 14 );

  connect( d->m_automaticDetection, SIGNAL( activated( int ) ), this, SLOT( slotAutomaticDetectionLanguage( int ) ) );

  d->m_paSetEncoding->popupMenu()->insertItem( i18n( "Automatic Detection" ), d->m_automaticDetection, 0 );

  d->m_paSetEncoding->insert( new KActionSeparator( actionCollection() ) );


  d->m_manualDetection = new KSelectAction( i18n( "short for Manual Detection", "Manual" ), 0, this, SLOT( slotSetEncoding() ), actionCollection(), "manualDetection" );
  QStringList encodings = KGlobal::charsets()->descriptiveEncodingNames();
  d->m_manualDetection->setItems( encodings );
  d->m_manualDetection->setCurrentItem( -1 );
  d->m_paSetEncoding->insert( d->m_manualDetection );


  KConfig *config = KGlobal::config();
  if ( config->hasGroup( "HTML Settings" ) ) {
    config->setGroup( "HTML Settings" );
    khtml::Decoder::AutoDetectLanguage language;
    QCString name = QTextCodec::codecForLocale()->name();
    name = name.lower();

    if ( name == "cp1256" || name == "iso-8859-6" ) {
      language = khtml::Decoder::Arabic;
    }
    else if ( name == "cp1257" || name == "iso-8859-13" || name == "iso-8859-4" ) {
      language = khtml::Decoder::Baltic;
    }
    else if ( name == "cp1250" || name == "ibm852" || name == "iso-8859-2" || name == "iso-8859-3" ) {
      language = khtml::Decoder::CentralEuropean;
    }
    else if ( name == "cp1251" || name == "koi8-r" || name == "iso-8859-5" ) {
      language = khtml::Decoder::Russian;
    }
    else if ( name == "koi8-u" ) {
      language = khtml::Decoder::Ukrainian;
    }
    else if ( name == "cp1253" || name == "iso-8859-7" ) {
      language = khtml::Decoder::Greek;
    }
    else if ( name == "cp1255" || name == "iso-8859-8" || name == "iso-8859-8-i" ) {
      language = khtml::Decoder::Hebrew;
    }
    else if ( name == "jis7" || name == "eucjp" || name == "sjis"  ) {
      language = khtml::Decoder::Japanese;
    }
    else if ( name == "cp1254" || name == "iso-8859-9" ) {
      language = khtml::Decoder::Turkish;
    }
    else if ( name == "cp1252" || name == "iso-8859-1" || name == "iso-8859-15" ) {
      language = khtml::Decoder::WesternEuropean;
    }
    else
      language = khtml::Decoder::SemiautomaticDetection;

    int _id = config->readNumEntry( "AutomaticDetectionLanguage", language );
    d->m_automaticDetection->setItemChecked( _id, true );
    d->m_paSetEncoding->popupMenu()->setItemChecked( 0, true );

    d->m_autoDetectLanguage = static_cast< khtml::Decoder::AutoDetectLanguage >( _id );
  }


  d->m_paUseStylesheet = new KSelectAction( i18n( "Use S&tylesheet"), 0, this, SLOT( slotUseStylesheet() ), actionCollection(), "useStylesheet" );

  if ( prof == BrowserViewGUI ) {
      d->m_paIncZoomFactor = new KHTMLZoomFactorAction( this, true, i18n(
                  "Increase Font Sizes" ), "viewmag+", "CTRL++;CTRL+=", this,
              SLOT( slotIncZoom() ), actionCollection(), "incFontSizes" );
      d->m_paIncZoomFactor->setWhatsThis( i18n( "Increase Font Size<p>"
                                                "Make the font in this window bigger. "
                            "Click and hold down the mouse button for a menu with all available font sizes." ) );
      d->m_paDecZoomFactor = new KHTMLZoomFactorAction( this, false, i18n(
                  "Decrease Font Sizes" ), "viewmag-", CTRL + Key_Minus, this,
              SLOT( slotDecZoom() ), actionCollection(), "decFontSizes" );
      d->m_paDecZoomFactor->setWhatsThis( i18n( "Decrease Font Size<p>"
                                                "Make the font in this window smaller. "
                            "Click and hold down the mouse button for a menu with all available font sizes." ) );
  }

  d->m_paFind = KStdAction::find( this, SLOT( slotFind() ), actionCollection(), "find" );
  d->m_paFind->setWhatsThis( i18n( "Find text<p>"
				   "Shows a dialog that allows you to find text on the displayed page." ) );

  d->m_paFindNext = KStdAction::findNext( this, SLOT( slotFindNext() ), actionCollection(), "findNext" );
  d->m_paFindNext->setWhatsThis( i18n( "Find next<p>"
				       "Find the next occurrence of the text that you "
				       "have found using the <b>Find Text</b> function" ) );
  if ( parentPart() )
  {
      d->m_paFind->setShortcut( KShortcut() ); // avoid clashes
      d->m_paFindNext->setShortcut( KShortcut() ); // avoid clashes
  }

  d->m_paPrintFrame = new KAction( i18n( "Print Frame" ), "frameprint", 0, this, SLOT( slotPrintFrame() ), actionCollection(), "printFrame" );
  d->m_paPrintFrame->setWhatsThis( i18n( "Print Frame<p>"
					 "Some pages have several frames. To print only a single frame, click "
					 "on it and then use this function." ) );

  d->m_paSelectAll = KStdAction::selectAll( this, SLOT( slotSelectAll() ), actionCollection(), "selectAll" );
  if ( parentPart() )
      d->m_paSelectAll->setShortcut( KShortcut() ); // avoid clashes

  d->m_paToggleCaretMode = new KToggleAction(i18n("Toggle Caret Mode"),
  				Key_F7, this, SLOT(slotToggleCaretMode()),
                                actionCollection(), "caretMode");
  d->m_paToggleCaretMode->setChecked(isCaretMode());
  if (parentPart())
      d->m_paToggleCaretMode->setShortcut(KShortcut()); // avoid clashes

  // set the default java(script) flags according to the current host.
  d->m_bBackRightClick = d->m_settings->isBackRightClickEnabled();
  d->m_bJScriptEnabled = d->m_settings->isJavaScriptEnabled();
  setDebugScript( d->m_settings->isJavaScriptDebugEnabled() );
  d->m_bJavaEnabled = d->m_settings->isJavaEnabled();
  d->m_bPluginsEnabled = d->m_settings->isPluginsEnabled();

  // Set the meta-refresh flag...
  d->m_metaRefreshEnabled = d->m_settings->isAutoDelayedActionsEnabled ();

  connect( view, SIGNAL( zoomView( int ) ), SLOT( slotZoomView( int ) ) );

  connect( this, SIGNAL( completed() ),
           this, SLOT( updateActions() ) );
  connect( this, SIGNAL( completed( bool ) ),
           this, SLOT( updateActions() ) );
  connect( this, SIGNAL( started( KIO::Job * ) ),
           this, SLOT( updateActions() ) );

  d->m_popupMenuXML = KXMLGUIFactory::readConfigFile( locate( "data", "khtml/khtml_popupmenu.rc", KHTMLFactory::instance() ) );

  connect( khtml::Cache::loader(), SIGNAL( requestStarted( khtml::DocLoader*, khtml::CachedObject* ) ),
           this, SLOT( slotLoaderRequestStarted( khtml::DocLoader*, khtml::CachedObject* ) ) );
  connect( khtml::Cache::loader(), SIGNAL( requestDone( khtml::DocLoader*, khtml::CachedObject *) ),
           this, SLOT( slotLoaderRequestDone( khtml::DocLoader*, khtml::CachedObject *) ) );
  connect( khtml::Cache::loader(), SIGNAL( requestFailed( khtml::DocLoader*, khtml::CachedObject *) ),
           this, SLOT( slotLoaderRequestDone( khtml::DocLoader*, khtml::CachedObject *) ) );

  connect ( &d->m_progressUpdateTimer, SIGNAL( timeout() ), this, SLOT( slotProgressUpdate() ) );

  findTextBegin(); //reset find variables

  connect( &d->m_redirectionTimer, SIGNAL( timeout() ),
           this, SLOT( slotRedirect() ) );

  d->m_dcopobject = new KHTMLPartIface(this);

  // "khtml" catalog does not exist, our translations are in kdelibs.
  // removing this catalog from KGlobal::locale() prevents problems
  // with changing the language in applications at runtime -Thomas Reitelbach
  KGlobal::locale()->removeCatalogue("khtml");
}

KHTMLPart::~KHTMLPart()
{
  //kdDebug(6050) << "KHTMLPart::~KHTMLPart " << this << endl;

  KConfig *config = KGlobal::config();
  config->setGroup( "HTML Settings" );
  config->writeEntry( "AutomaticDetectionLanguage", d->m_autoDetectLanguage );

  delete d->m_automaticDetection;
  delete d->m_manualDetection;

  slotWalletClosed();
  if (!parentPart()) { // only delete it if the top khtml_part closes
    removeJSErrorExtension();
  }

  d->m_find = 0; // deleted by its parent, the view.

  if ( d->m_manager )
  {
    d->m_manager->setActivePart( 0 );
    // We specify "this" as parent qobject for d->manager, so no need to delete it.
  }

  stopAutoScroll();
  d->m_redirectionTimer.stop();

  if (!d->m_bComplete)
    closeURL();

  disconnect( khtml::Cache::loader(), SIGNAL( requestStarted( khtml::DocLoader*, khtml::CachedObject* ) ),
           this, SLOT( slotLoaderRequestStarted( khtml::DocLoader*, khtml::CachedObject* ) ) );
  disconnect( khtml::Cache::loader(), SIGNAL( requestDone( khtml::DocLoader*, khtml::CachedObject *) ),
           this, SLOT( slotLoaderRequestDone( khtml::DocLoader*, khtml::CachedObject *) ) );
  disconnect( khtml::Cache::loader(), SIGNAL( requestFailed( khtml::DocLoader*, khtml::CachedObject *) ),
           this, SLOT( slotLoaderRequestDone( khtml::DocLoader*, khtml::CachedObject *) ) );

  clear();

  if ( d->m_view )
  {
    d->m_view->hide();
    d->m_view->viewport()->hide();
    d->m_view->m_part = 0;
  }

  // Have to delete this here since we forward declare it in khtmlpart_p and
  // at least some compilers won't call the destructor in this case.
  delete d->m_jsedlg;
  d->m_jsedlg = 0;

  delete d; d = 0;
  KHTMLFactory::deregisterPart( this );
}

bool KHTMLPart::restoreURL( const KURL &url )
{
  kdDebug( 6050 ) << "KHTMLPart::restoreURL " << url.url() << endl;

  d->m_redirectionTimer.stop();

  /*
   * That's not a good idea as it will call closeURL() on all
   * child frames, preventing them from further loading. This
   * method gets called from restoreState() in case of a full frameset
   * restoral, and restoreState() calls closeURL() before restoring
   * anyway.
  kdDebug( 6050 ) << "closing old URL" << endl;
  closeURL();
  */

  d->m_bComplete = false;
  d->m_bLoadEventEmitted = false;
  d->m_workingURL = url;

  // set the java(script) flags according to the current host.
  d->m_bJScriptEnabled = KHTMLFactory::defaultHTMLSettings()->isJavaScriptEnabled(url.host());
  setDebugScript( KHTMLFactory::defaultHTMLSettings()->isJavaScriptDebugEnabled() );
  d->m_bJavaEnabled = KHTMLFactory::defaultHTMLSettings()->isJavaEnabled(url.host());
  d->m_bPluginsEnabled = KHTMLFactory::defaultHTMLSettings()->isPluginsEnabled(url.host());

  m_url = url;

  KHTMLPageCache::self()->fetchData( d->m_cacheId, this, SLOT(slotRestoreData(const QByteArray &)));

  emit started( 0L );

  return true;
}


bool KHTMLPart::openURL( const KURL &url )
{
  kdDebug( 6050 ) << "KHTMLPart(" << this << ")::openURL " << url.url() << endl;

  d->m_redirectionTimer.stop();

  // check to see if this is an "error://" URL. This is caused when an error
  // occurs before this part was loaded (e.g. KonqRun), and is passed to
  // khtmlpart so that it can display the error.
  if ( url.protocol() == "error" && url.hasSubURL() ) {
    closeURL();

    if(  d->m_bJScriptEnabled )
      d->m_statusBarText[BarOverrideText] = d->m_statusBarText[BarDefaultText] = QString::null;

    /**
     * The format of the error url is that two variables are passed in the query:
     * error = int kio error code, errText = QString error text from kio
     * and the URL where the error happened is passed as a sub URL.
     */
    KURL::List urls = KURL::split( url );
    //kdDebug(6050) << "Handling error URL. URL count:" << urls.count() << endl;

    if ( urls.count() > 1 ) {
      KURL mainURL = urls.first();
      int error = mainURL.queryItem( "error" ).toInt();
      // error=0 isn't a valid error code, so 0 means it's missing from the URL
      if ( error == 0 ) error = KIO::ERR_UNKNOWN;
      QString errorText = mainURL.queryItem( "errText", HINT_UTF8 );
      urls.pop_front();
      d->m_workingURL = KURL::join( urls );
      //kdDebug(6050) << "Emitting fixed URL " << d->m_workingURL.prettyURL() << endl;
      emit d->m_extension->setLocationBarURL( d->m_workingURL.prettyURL() );
      htmlError( error, errorText, d->m_workingURL );
      return true;
    }
  }

  KParts::URLArgs args( d->m_extension->urlArgs() );
  // in case we have a) no frameset (don't test m_frames.count(), iframes get in there)
  // b) the url is identical with the currently
  // displayed one (except for the htmlref!) , c) the url request is not a POST
  // operation and d) the caller did not request to reload the page we try to
  // be smart and instead of reloading the whole document we just jump to the
  // request html anchor
  bool isFrameSet = false;
  if ( d->m_doc && d->m_doc->isHTMLDocument() ) {
      HTMLDocumentImpl* htmlDoc = static_cast<HTMLDocumentImpl*>(d->m_doc);
      isFrameSet = htmlDoc->body() && (htmlDoc->body()->id() == ID_FRAMESET);
  }
  if ( !isFrameSet && !args.redirectedRequest() &&
       urlcmp( url.url(), m_url.url(), true, true ) &&
       url.hasRef() && !args.doPost() && !args.reload )
  {
    kdDebug( 6050 ) << "KHTMLPart::openURL, jumping to anchor. m_url = " << url.url() << endl;
    m_url = url;
    emit started( 0L );

    if ( !gotoAnchor( url.encodedHtmlRef()) )
       gotoAnchor( url.htmlRef() );

    d->m_bComplete = true;
    if (d->m_doc)
       d->m_doc->setParsing(false);

    kdDebug( 6050 ) << "completed..." << endl;
    emit completed();
    return true;
  }

  if (!d->m_restored)
    closeURL();

  // initializing m_url to the new url breaks relative links when opening such a link after this call and _before_ begin() is called (when the first
  // data arrives) (Simon)
  m_url = url;
  if(m_url.protocol().startsWith( "http" ) && !m_url.host().isEmpty() &&
     m_url.path().isEmpty()) {
    m_url.setPath("/");
    emit d->m_extension->setLocationBarURL( m_url.prettyURL() );
  }
  // copy to m_workingURL after fixing m_url above
  d->m_workingURL = m_url;

  args.metaData().insert("main_frame_request", parentPart() == 0 ? "TRUE" : "FALSE" );
  args.metaData().insert("ssl_parent_ip", d->m_ssl_parent_ip);
  args.metaData().insert("ssl_parent_cert", d->m_ssl_parent_cert);
  args.metaData().insert("PropagateHttpHeader", "true");
  args.metaData().insert("ssl_was_in_use", d->m_ssl_in_use ? "TRUE" : "FALSE" );
  args.metaData().insert("ssl_activate_warnings", "TRUE" );
  args.metaData().insert("cross-domain", toplevelURL().url());

  if (d->m_restored)
  {
     args.metaData().insert("referrer", d->m_pageReferrer);
     d->m_cachePolicy = KIO::CC_Cache;
  }
  else if (args.reload)
     d->m_cachePolicy = KIO::CC_Refresh;
  else
     d->m_cachePolicy = KIO::CC_Verify;

  if ( args.doPost() && (m_url.protocol().startsWith("http")) )
  {
      d->m_job = KIO::http_post( m_url, args.postData, false );
      d->m_job->addMetaData("content-type", args.contentType() );
  }
  else
  {
      d->m_job = KIO::get( m_url, false, false );
      d->m_job->addMetaData("cache", KIO::getCacheControlString(d->m_cachePolicy));
  }

  if (widget())
     d->m_job->setWindow(widget()->topLevelWidget());
  d->m_job->addMetaData(args.metaData());

  connect( d->m_job, SIGNAL( result( KIO::Job* ) ),
           SLOT( slotFinished( KIO::Job* ) ) );
  connect( d->m_job, SIGNAL( data( KIO::Job*, const QByteArray& ) ),
           SLOT( slotData( KIO::Job*, const QByteArray& ) ) );
  connect ( d->m_job, SIGNAL( infoMessage( KIO::Job*, const QString& ) ),
           SLOT( slotInfoMessage(KIO::Job*, const QString& ) ) );
  connect( d->m_job, SIGNAL(redirection(KIO::Job*, const KURL& ) ),
           SLOT( slotRedirection(KIO::Job*, const KURL&) ) );

  d->m_bComplete = false;
  d->m_bLoadEventEmitted = false;

  // delete old status bar msg's from kjs (if it _was_ activated on last URL)
  if( d->m_bJScriptEnabled )
    d->m_statusBarText[BarOverrideText] = d->m_statusBarText[BarDefaultText] = QString::null;

  // set the javascript flags according to the current url
  d->m_bJScriptEnabled = KHTMLFactory::defaultHTMLSettings()->isJavaScriptEnabled(url.host());
  setDebugScript( KHTMLFactory::defaultHTMLSettings()->isJavaScriptDebugEnabled() );
  d->m_bJavaEnabled = KHTMLFactory::defaultHTMLSettings()->isJavaEnabled(url.host());
  d->m_bPluginsEnabled = KHTMLFactory::defaultHTMLSettings()->isPluginsEnabled(url.host());


  connect( d->m_job, SIGNAL( speed( KIO::Job*, unsigned long ) ),
           this, SLOT( slotJobSpeed( KIO::Job*, unsigned long ) ) );

  connect( d->m_job, SIGNAL( percent( KIO::Job*, unsigned long ) ),
           this, SLOT( slotJobPercent( KIO::Job*, unsigned long ) ) );

  connect( d->m_job, SIGNAL( result( KIO::Job* ) ),
           this, SLOT( slotJobDone( KIO::Job* ) ) );

  d->m_jobspeed = 0;

  // If this was an explicit reload and the user style sheet should be used,
  // do a stat to see whether the stylesheet was changed in the meanwhile.
  if ( args.reload && !settings()->userStyleSheet().isEmpty() ) {
    KURL url( settings()->userStyleSheet() );
    KIO::StatJob *job = KIO::stat( url, false /* don't show progress */ );
    connect( job, SIGNAL( result( KIO::Job * ) ),
             this, SLOT( slotUserSheetStatDone( KIO::Job * ) ) );
  }
  emit started( 0L );

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

  if ( d->m_doc && d->m_doc->isHTMLDocument() ) {
    HTMLDocumentImpl* hdoc = static_cast<HTMLDocumentImpl*>( d->m_doc );

    if ( hdoc->body() && d->m_bLoadEventEmitted ) {
      hdoc->body()->dispatchWindowEvent( EventImpl::UNLOAD_EVENT, false, false );
      if ( d->m_doc )
        d->m_doc->updateRendering();
      d->m_bLoadEventEmitted = false;
    }
  }

  d->m_bComplete = true; // to avoid emitting completed() in slotFinishedParsing() (David)
  d->m_bLoadEventEmitted = true; // don't want that one either
  d->m_cachePolicy = KIO::CC_Verify; // reset cache policy

  KHTMLPageCache::self()->cancelFetch(this);
  if ( d->m_doc && d->m_doc->parsing() )
  {
    kdDebug( 6050 ) << " was still parsing... calling end " << endl;
    slotFinishedParsing();
    d->m_doc->setParsing(false);
  }

  if ( !d->m_workingURL.isEmpty() )
  {
    // Aborted before starting to render
    kdDebug( 6050 ) << "Aborted before starting to render, reverting location bar to " << m_url.prettyURL() << endl;
    emit d->m_extension->setLocationBarURL( m_url.prettyURL() );
  }

  d->m_workingURL = KURL();

  if ( d->m_doc && d->m_doc->docLoader() )
    khtml::Cache::loader()->cancelRequests( d->m_doc->docLoader() );

  // tell all subframes to stop as well
  ConstFrameIt it = d->m_frames.begin();
  ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it )
  {
    if ( (*it).m_run )
      (*it).m_run->abort();
    if ( !( *it ).m_part.isNull() )
      ( *it ).m_part->closeURL();
  }
  // tell all objects to stop as well
  for (it = d->m_objects.begin(); it != d->m_objects.end(); ++it )
  {
    if ( !( *it ).m_part.isNull() )
      ( *it ).m_part->closeURL();
  }

  // Stop any started redirections as well!! (DA)
  if ( d && d->m_redirectionTimer.isActive() )
    d->m_redirectionTimer.stop();

  // null node activated.
  emit nodeActivated(Node());

  // make sure before clear() runs, we pop out of a dialog's message loop
  if ( d->m_view )
    d->m_view->closeChildDialogs();

  return true;
}

DOM::HTMLDocument KHTMLPart::htmlDocument() const
{
  if (d->m_doc && d->m_doc->isHTMLDocument())
    return static_cast<HTMLDocumentImpl*>(d->m_doc);
  else
    return static_cast<HTMLDocumentImpl*>(0);
}

DOM::Document KHTMLPart::document() const
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

void KHTMLPart::setStatusMessagesEnabled( bool enable )
{
  d->m_statusMessagesEnabled = enable;
}

KJS::Interpreter *KHTMLPart::jScriptInterpreter()
{
  KJSProxy *proxy = jScript();
  if (!proxy || proxy->paused())
    return 0;

  return proxy->interpreter();
}

bool KHTMLPart::statusMessagesEnabled() const
{
  return d->m_statusMessagesEnabled;
}

void KHTMLPart::setJScriptEnabled( bool enable )
{
  if ( !enable && jScriptEnabled() && d->m_jscript ) {
    d->m_jscript->clear();
  }
  d->m_bJScriptForce = enable;
  d->m_bJScriptOverride = true;
}

bool KHTMLPart::jScriptEnabled() const
{
  if ( d->m_bJScriptOverride )
      return d->m_bJScriptForce;
  return d->m_bJScriptEnabled;
}

void KHTMLPart::setMetaRefreshEnabled( bool enable )
{
  d->m_metaRefreshEnabled = enable;
}

bool KHTMLPart::metaRefreshEnabled() const
{
  return d->m_metaRefreshEnabled;
}

// Define this to disable dlopening kjs_html, when directly linking to it.
// You need to edit khtml/Makefile.am to add ./ecma/libkjs_html.la to LIBADD
// and to edit khtml/ecma/Makefile.am to s/kjs_html/libkjs_html/, remove libkhtml from LIBADD,
//        remove LDFLAGS line, and replace kde_module with either lib (shared) or noinst (static)
//        Also, change the order of "ecma" and "." in khtml's SUBDIRS line.
// OK - that's the default now, use the opposite of the above instructions to go back
// to "dlopening it" - but it breaks exception catching in kjs_binding.cpp
#define DIRECT_LINKAGE_TO_ECMA

#ifdef DIRECT_LINKAGE_TO_ECMA
extern "C" { KJSProxy *kjs_html_init(KHTMLPart *khtmlpart); }
#endif

KJSProxy *KHTMLPart::jScript()
{
  if (!jScriptEnabled()) return 0;

  if ( !d->m_jscript )
  {
#ifndef DIRECT_LINKAGE_TO_ECMA
    KLibrary *lib = KLibLoader::self()->library("kjs_html");
    if ( !lib ) {
      setJScriptEnabled( false );
      return 0;
    }
    // look for plain C init function
    void *sym = lib->symbol("kjs_html_init");
    if ( !sym ) {
      lib->unload();
      setJScriptEnabled( false );
      return 0;
    }
    typedef KJSProxy* (*initFunction)(KHTMLPart *);
    initFunction initSym = (initFunction) sym;
    d->m_jscript = (*initSym)(this);
    d->m_kjs_lib = lib;
#else
    d->m_jscript = kjs_html_init(this);
    // d->m_kjs_lib remains 0L.
#endif
    if (d->m_bJScriptDebugEnabled)
        d->m_jscript->setDebugEnabled(true);
  }

  return d->m_jscript;
}

QVariant KHTMLPart::crossFrameExecuteScript(const QString& target,  const QString& script)
{
  KHTMLPart* destpart = this;

  QString trg = target.lower();

  if (target == "_top") {
    while (destpart->parentPart())
      destpart = destpart->parentPart();
  }
  else if (target == "_parent") {
    if (parentPart())
      destpart = parentPart();
  }
  else if (target == "_self" || target == "_blank")  {
    // we always allow these
  }
  else {
    while (destpart->parentPart())
      destpart = destpart->parentPart();
    destpart = destpart->findFrame(target);

    if (!destpart)
      destpart = this; // ### doesn't make sense, does it?
  }

  // easy way out?
  if (destpart == this)
    return executeScript(DOM::Node(), script);


  // now compare the domains
  if (!destpart->htmlDocument().isNull() &&
      !htmlDocument().isNull())  {
    DOM::DOMString actDomain = htmlDocument().domain();
    DOM::DOMString destDomain = destpart->htmlDocument().domain();

    if (actDomain == destDomain)
      return destpart->executeScript(DOM::Node(), script);
  }


  // eww, something went wrong. better execute it in our frame
  return executeScript(DOM::Node(), script);
}

//Enable this to see all JS scripts being executed
//#define KJS_VERBOSE

KJSErrorDlg *KHTMLPart::jsErrorExtension() {
  if (!d->m_settings->jsErrorsEnabled()) {
    return 0L;
  }

  if (parentPart()) {
    return parentPart()->jsErrorExtension();
  }

  if (!d->m_statusBarJSErrorLabel) {
    d->m_statusBarJSErrorLabel = new KURLLabel(d->m_statusBarExtension->statusBar());
    d->m_statusBarJSErrorLabel->setFixedHeight(instance()->iconLoader()->currentSize(KIcon::Small));
    d->m_statusBarJSErrorLabel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    d->m_statusBarJSErrorLabel->setUseCursor(false);
    d->m_statusBarExtension->addStatusBarItem(d->m_statusBarJSErrorLabel, 0, false);
    QToolTip::add(d->m_statusBarJSErrorLabel, i18n("This web page contains coding errors."));
    d->m_statusBarJSErrorLabel->setPixmap(SmallIcon("bug", instance()));
    connect(d->m_statusBarJSErrorLabel, SIGNAL(leftClickedURL()), SLOT(launchJSErrorDialog()));
    connect(d->m_statusBarJSErrorLabel, SIGNAL(rightClickedURL()), SLOT(jsErrorDialogContextMenu()));
  }
  if (!d->m_jsedlg) {
    d->m_jsedlg = new KJSErrorDlg;
    d->m_jsedlg->setURL(m_url.prettyURL());
  }
  return d->m_jsedlg;
}

void KHTMLPart::removeJSErrorExtension() {
  if (parentPart()) {
    parentPart()->removeJSErrorExtension();
    return;
  }
  if (d->m_statusBarJSErrorLabel != 0) {
    d->m_statusBarExtension->removeStatusBarItem( d->m_statusBarJSErrorLabel );
    delete d->m_statusBarJSErrorLabel;
    d->m_statusBarJSErrorLabel = 0;
  }
  delete d->m_jsedlg;
  d->m_jsedlg = 0;
}

void KHTMLPart::disableJSErrorExtension() {
  removeJSErrorExtension();
  // These two lines are really kind of hacky, and it sucks to do this inside
  // KHTML but I don't know of anything that's reasonably easy as an alternative
  // right now.  It makes me wonder if there should be a more clean way to
  // contact all running "KHTML" instance as opposed to Konqueror instances too.
  d->m_settings->setJSErrorsEnabled(false);
  DCOPClient::mainClient()->send("konqueror*", "KonquerorIface", "reparseConfiguration()", QByteArray());
}

void KHTMLPart::jsErrorDialogContextMenu() {
  KPopupMenu *m = new KPopupMenu(0L);
  m->insertItem(i18n("&Hide Errors"), this, SLOT(removeJSErrorExtension()));
  m->insertItem(i18n("&Disable Error Reporting"), this, SLOT(disableJSErrorExtension()));
  m->popup(QCursor::pos());
}

void KHTMLPart::launchJSErrorDialog() {
  KJSErrorDlg *dlg = jsErrorExtension();
  if (dlg) {
    dlg->show();
    dlg->raise();
  }
}

QVariant KHTMLPart::executeScript(const QString& filename, int baseLine, const DOM::Node& n, const QString& script)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "executeScript: caller='" << name() << "' filename=" << filename << " baseLine=" << baseLine << " script=" << script << endl;
#endif
  KJSProxy *proxy = jScript();

  if (!proxy || proxy->paused())
    return QVariant();

  KJS::Completion comp;

  QVariant ret = proxy->evaluate(filename, baseLine, script, n, &comp);

  /*
   *  Error handling
   */
  if (comp.complType() == KJS::Throw && !comp.value().isNull()) {
    KJSErrorDlg *dlg = jsErrorExtension();
    if (dlg) {
      KJS::UString msg = comp.value().toString(proxy->interpreter()->globalExec());
      dlg->addError(i18n("<b>Error</b>: %1: %2").arg(filename, msg.qstring()));
    }
  }

  return ret;
}

QVariant KHTMLPart::executeScript( const QString &script )
{
    return executeScript( DOM::Node(), script );
}

QVariant KHTMLPart::executeScript( const DOM::Node &n, const QString &script )
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "KHTMLPart::executeScript caller='" << name() << "' node=" << n.nodeName().string().latin1() << "(" << (n.isNull() ? 0 : n.nodeType()) << ") " << script << endl;
#endif
  KJSProxy *proxy = jScript();

  if (!proxy || proxy->paused())
    return QVariant();
  d->m_runningScripts++;
  KJS::Completion comp;
  QVariant ret = proxy->evaluate( QString::null, 1, script, n, &comp );
  d->m_runningScripts--;

  /*
   *  Error handling
   */
  if (comp.complType() == KJS::Throw && !comp.value().isNull()) {
    KJSErrorDlg *dlg = jsErrorExtension();
    if (dlg) {
      KJS::UString msg = comp.value().toString(proxy->interpreter()->globalExec());
      dlg->addError(i18n("<b>Error</b>: node %1: %2").arg(n.nodeName().string()).arg(msg.qstring()));
    }
  }

  if (!d->m_runningScripts && d->m_doc && !d->m_doc->parsing() && d->m_submitForm )
      submitFormAgain();

#ifdef KJS_VERBOSE
  kdDebug(6070) << "KHTMLPart::executeScript - done" << endl;
#endif
  return ret;
}

bool KHTMLPart::scheduleScript(const DOM::Node &n, const QString& script)
{
    //kdDebug(6050) << "KHTMLPart::scheduleScript "<< script << endl;

    d->scheduledScript = script;
    d->scheduledScriptNode = n;

    return true;
}

QVariant KHTMLPart::executeScheduledScript()
{
  if( d->scheduledScript.isEmpty() )
    return QVariant();

  //kdDebug(6050) << "executing delayed " << d->scheduledScript << endl;

  QVariant ret = executeScript( d->scheduledScriptNode, d->scheduledScript );
  d->scheduledScript = QString();
  d->scheduledScriptNode = DOM::Node();

  return ret;
}

void KHTMLPart::setJavaEnabled( bool enable )
{
  d->m_bJavaForce = enable;
  d->m_bJavaOverride = true;
}

bool KHTMLPart::javaEnabled() const
{
#ifndef Q_WS_QWS
  if( d->m_bJavaOverride )
      return d->m_bJavaForce;
  return d->m_bJavaEnabled;
#else
  return false;
#endif
}

KJavaAppletContext *KHTMLPart::javaContext()
{
  return 0;
}

KJavaAppletContext *KHTMLPart::createJavaContext()
{
  return 0;
}

void KHTMLPart::setPluginsEnabled( bool enable )
{
  d->m_bPluginsForce = enable;
  d->m_bPluginsOverride = true;
}

bool KHTMLPart::pluginsEnabled() const
{
  if ( d->m_bPluginsOverride )
      return d->m_bPluginsForce;
  return d->m_bPluginsEnabled;
}

void KHTMLPart::slotDebugDOMTree()
{
  if ( d->m_doc && d->m_doc->firstChild() )
    qDebug("%s", d->m_doc->firstChild()->toHTML().latin1());
}

void KHTMLPart::slotDebugScript()
{
  if (jScript())
    jScript()->showDebugWindow();
}

void KHTMLPart::slotDebugRenderTree()
{
#ifndef NDEBUG
  if ( d->m_doc ) {
    d->m_doc->renderer()->printTree();
    // dump out the contents of the rendering & DOM trees
//    QString dumps;
//    QTextStream outputStream(dumps,IO_WriteOnly);
//    d->m_doc->renderer()->layer()->dump( outputStream );
//    kdDebug() << "dump output:" << "\n" + dumps;
  }
#endif
}

void KHTMLPart::slotStopAnimations()
{
  stopAnimations();
}

void KHTMLPart::setAutoloadImages( bool enable )
{
  if ( d->m_doc && d->m_doc->docLoader()->autoloadImages() == enable )
    return;

  if ( d->m_doc )
    d->m_doc->docLoader()->setAutoloadImages( enable );

  unplugActionList( "loadImages" );

  if ( enable ) {
    delete d->m_paLoadImages;
    d->m_paLoadImages = 0;
  }
  else if ( !d->m_paLoadImages )
    d->m_paLoadImages = new KAction( i18n( "Display Images on Page" ), "images_display", 0, this, SLOT( slotLoadImages() ), actionCollection(), "loadImages" );

  if ( d->m_paLoadImages ) {
    QPtrList<KAction> lst;
    lst.append( d->m_paLoadImages );
    plugActionList( "loadImages", lst );
  }
}

bool KHTMLPart::autoloadImages() const
{
  if ( d->m_doc )
    return d->m_doc->docLoader()->autoloadImages();

  return true;
}

void KHTMLPart::clear()
{
  if ( d->m_bCleared )
    return;
  d->m_bCleared = true;

  d->m_bClearing = true;

  {
    ConstFrameIt it = d->m_frames.begin();
    ConstFrameIt end = d->m_frames.end();
    for(; it != end; ++it )
    {
      // Stop HTMLRun jobs for frames
      if ( (*it).m_run )
        (*it).m_run->abort();
    }
  }

  {
    QValueList<khtml::ChildFrame>::ConstIterator it = d->m_objects.begin();
    QValueList<khtml::ChildFrame>::ConstIterator end = d->m_objects.end();
    for(; it != end; ++it )
    {
      // Stop HTMLRun jobs for objects
      if ( (*it).m_run )
        (*it).m_run->abort();
    }
  }


  findTextBegin(); // resets d->m_findNode and d->m_findPos
  d->m_mousePressNode = DOM::Node();


  if ( d->m_doc )
    d->m_doc->detach();

  // Moving past doc so that onUnload works.
  if ( d->m_jscript )
    d->m_jscript->clear();

  if ( d->m_view )
    d->m_view->clear();

  // do not dereference the document before the jscript and view are cleared, as some destructors
  // might still try to access the document.
  if ( d->m_doc ) {
    d->m_doc->deref();
  }
  d->m_doc = 0;

  delete d->m_decoder;
  d->m_decoder = 0;

  {
    ConstFrameIt it = d->m_frames.begin();
    ConstFrameIt end = d->m_frames.end();
    for(; it != end; ++it )
    {
      if ( (*it).m_part )
      {
        partManager()->removePart( (*it).m_part );
        delete (KParts::ReadOnlyPart *)(*it).m_part;
      }
    }
  }

  d->m_frames.clear();
  d->m_objects.clear();

  d->m_delayRedirect = 0;
  d->m_redirectURL = QString::null;
  d->m_redirectLockHistory = true;
  d->m_bClearing = false;
  d->m_frameNameId = 1;
  d->m_bFirstData = true;

  d->m_bMousePressed = false;

  d->m_selectionStart = DOM::Node();
  d->m_selectionEnd = DOM::Node();
  d->m_startOffset = 0;
  d->m_endOffset = 0;
#ifndef QT_NO_CLIPBOARD
  connect( kapp->clipboard(), SIGNAL( selectionChanged()), SLOT( slotClearSelection()));
#endif

  d->m_jobPercent = 0;

  if ( !d->m_haveEncoding )
    d->m_encoding = QString::null;
#ifdef SPEED_DEBUG
  d->m_parsetime.restart();
#endif
}

bool KHTMLPart::openFile()
{
  return true;
}

DOM::HTMLDocumentImpl *KHTMLPart::docImpl() const
{
    if ( d && d->m_doc && d->m_doc->isHTMLDocument() )
        return static_cast<HTMLDocumentImpl*>(d->m_doc);
    return 0;
}

DOM::DocumentImpl *KHTMLPart::xmlDocImpl() const
{
    if ( d )
        return d->m_doc;
    return 0;
}

void KHTMLPart::slotInfoMessage(KIO::Job* kio_job, const QString& msg)
{
  assert(d->m_job == kio_job);

  if (!parentPart())
    setStatusBarText(msg, BarDefaultText);
}

void KHTMLPart::setPageSecurity( PageSecurity sec )
{
  if ( sec != NotCrypted && !d->m_statusBarIconLabel && !parentPart() ) {
    d->m_statusBarIconLabel = new KURLLabel( d->m_statusBarExtension->statusBar() );
    d->m_statusBarIconLabel->setFixedHeight( instance()->iconLoader()->currentSize(KIcon::Small) );
    d->m_statusBarIconLabel->setSizePolicy(QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ));
    d->m_statusBarIconLabel->setUseCursor( false );
    d->m_statusBarExtension->addStatusBarItem( d->m_statusBarIconLabel, 0, false );
    connect( d->m_statusBarIconLabel, SIGNAL( leftClickedURL() ), SLOT( slotSecurity() ) );
  } else if (d->m_statusBarIconLabel) {
    QToolTip::remove(d->m_statusBarIconLabel);
  }

  if (d->m_statusBarIconLabel) {
    if (d->m_ssl_in_use)
      QToolTip::add(d->m_statusBarIconLabel,
		    i18n("Session is secured with %1 bit %2.").arg(d->m_ssl_cipher_used_bits).arg(d->m_ssl_cipher));
    else QToolTip::add(d->m_statusBarIconLabel, i18n("Session is not secured."));
  }

  QString iconName;
  switch (sec)  {
  case NotCrypted:
    iconName = "decrypted";
    if ( d->m_statusBarIconLabel )  {
      d->m_statusBarExtension->removeStatusBarItem( d->m_statusBarIconLabel );
      delete d->m_statusBarIconLabel;
      d->m_statusBarIconLabel = 0L;
    }
    break;
  case Encrypted:
    iconName = "encrypted";
    break;
  case Mixed:
    iconName = "halfencrypted";
    break;
  }
  d->m_paSecurity->setIcon( iconName );
  if ( d->m_statusBarIconLabel )
    d->m_statusBarIconLabel->setPixmap( SmallIcon( iconName, instance() ) );
}

void KHTMLPart::slotData( KIO::Job* kio_job, const QByteArray &data )
{
  assert ( d->m_job == kio_job );

  //kdDebug( 6050 ) << "slotData: " << data.size() << endl;
  // The first data ?
  if ( !d->m_workingURL.isEmpty() )
  {
      //kdDebug( 6050 ) << "begin!" << endl;

    // We must suspend KIO while we're inside begin() because it can cause
    // crashes if a window (such as kjsdebugger) goes back into the event loop,
    // more data arrives, and begin() gets called again (re-entered).
    d->m_job->suspend();
    begin( d->m_workingURL, d->m_extension->urlArgs().xOffset, d->m_extension->urlArgs().yOffset );
    d->m_job->resume();

    d->m_doc->docLoader()->setCachePolicy(d->m_cachePolicy);
    d->m_workingURL = KURL();

    d->m_cacheId = KHTMLPageCache::self()->createCacheEntry();

    // When the first data arrives, the metadata has just been made available
    d->m_httpHeaders = d->m_job->queryMetaData("HTTP-Headers");
    time_t cacheCreationDate =  d->m_job->queryMetaData("cache-creation-date").toLong();
    d->m_doc->docLoader()->setCacheCreationDate(cacheCreationDate);

    d->m_pageServices = d->m_job->queryMetaData("PageServices");
    d->m_pageReferrer = d->m_job->queryMetaData("referrer");

    d->m_bSecurityInQuestion = false;
    d->m_ssl_in_use = (d->m_job->queryMetaData("ssl_in_use") == "TRUE");

    {
    KHTMLPart *p = parentPart();
    if (p && p->d->m_ssl_in_use != d->m_ssl_in_use) {
	while (p->parentPart()) p = p->parentPart();

        p->setPageSecurity( Mixed );
        p->d->m_bSecurityInQuestion = true;
    }
    }

    setPageSecurity( d->m_ssl_in_use ? Encrypted : NotCrypted );

    // Shouldn't all of this be done only if ssl_in_use == true ? (DF)
    d->m_ssl_parent_ip = d->m_job->queryMetaData("ssl_parent_ip");
    d->m_ssl_parent_cert = d->m_job->queryMetaData("ssl_parent_cert");
    d->m_ssl_peer_certificate = d->m_job->queryMetaData("ssl_peer_certificate");
    d->m_ssl_peer_chain = d->m_job->queryMetaData("ssl_peer_chain");
    d->m_ssl_peer_ip = d->m_job->queryMetaData("ssl_peer_ip");
    d->m_ssl_cipher = d->m_job->queryMetaData("ssl_cipher");
    d->m_ssl_cipher_desc = d->m_job->queryMetaData("ssl_cipher_desc");
    d->m_ssl_cipher_version = d->m_job->queryMetaData("ssl_cipher_version");
    d->m_ssl_cipher_used_bits = d->m_job->queryMetaData("ssl_cipher_used_bits");
    d->m_ssl_cipher_bits = d->m_job->queryMetaData("ssl_cipher_bits");
    d->m_ssl_cert_state = d->m_job->queryMetaData("ssl_cert_state");

    if (d->m_statusBarIconLabel) {
      QToolTip::remove(d->m_statusBarIconLabel);
      if (d->m_ssl_in_use) {
        QToolTip::add(d->m_statusBarIconLabel, i18n("Session is secured with %1 bit %2.").arg(d->m_ssl_cipher_used_bits).arg(d->m_ssl_cipher));
      } else {
        QToolTip::add(d->m_statusBarIconLabel, i18n("Session is not secured."));
      }
    }

    // Check for charset meta-data
    QString qData = d->m_job->queryMetaData("charset");
    if ( !qData.isEmpty() && !d->m_haveEncoding ) // only use information if the user didn't override the settings
       d->m_encoding = qData;

    // Support for http-refresh
    qData = d->m_job->queryMetaData("http-refresh");
    if( !qData.isEmpty())
      d->m_doc->processHttpEquiv("refresh", qData);

    // Support Content-Location per section 14.14 of RFC 2616.
    QString baseURL = d->m_job->queryMetaData ("content-location");
    if (!baseURL.isEmpty())
      d->m_doc->setBaseURL(KURL( d->m_doc->completeURL(baseURL) ));


    if ( !m_url.isLocalFile() ) {
        // Support for http last-modified
        d->m_lastModified = d->m_job->queryMetaData("modified");
    } else
        d->m_lastModified = QString::null; // done on-demand by lastModified()

    // Reset contents position
    d->m_view->setContentsPos( 0, 0 );
  }

  KHTMLPageCache::self()->addData(d->m_cacheId, data);
  write( data.data(), data.size() );
  if (d->m_jscript)
    d->m_jscript->dataReceived();
}

void KHTMLPart::slotRestoreData(const QByteArray &data )
{
  // The first data ?
  if ( !d->m_workingURL.isEmpty() )
  {
     long saveCacheId = d->m_cacheId;
     QString savePageReferrer = d->m_pageReferrer;
     begin( d->m_workingURL, d->m_extension->urlArgs().xOffset, d->m_extension->urlArgs().yOffset );
     d->m_pageReferrer = savePageReferrer;
     d->m_cacheId = saveCacheId;
     d->m_workingURL = KURL();
  }

  //kdDebug( 6050 ) << "slotRestoreData: " << data.size() << endl;
  write( data.data(), data.size() );

  if (data.size() == 0)
  {
      //kdDebug( 6050 ) << "slotRestoreData: <<end of data>>" << endl;
     // End of data.
    if (d->m_doc && d->m_doc->parsing())
        end(); //will emit completed()
  }
}

void KHTMLPart::showError( KIO::Job* job )
{
  kdDebug(6050) << "KHTMLPart::showError d->m_bParsing=" << (d->m_doc && d->m_doc->parsing()) << " d->m_bComplete=" << d->m_bComplete
                << " d->m_bCleared=" << d->m_bCleared << endl;

  if (job->error() == KIO::ERR_NO_CONTENT)
	return;

  if ( (d->m_doc && d->m_doc->parsing()) || d->m_workingURL.isEmpty() ) // if we got any data already
    job->showErrorDialog( /*d->m_view*/ );
  else
  {
    htmlError( job->error(), job->errorText(), d->m_workingURL );
  }
}

// This is a protected method, placed here because of it's relevance to showError
void KHTMLPart::htmlError( int errorCode, const QString& text, const KURL& reqUrl )
{
  kdDebug(6050) << "KHTMLPart::htmlError errorCode=" << errorCode << " text=" << text << endl;
  // make sure we're not executing any embedded JS
  bool bJSFO = d->m_bJScriptForce;
  bool bJSOO = d->m_bJScriptOverride;
  d->m_bJScriptForce = false;
  d->m_bJScriptOverride = true;
  begin();
  QString errText = QString::fromLatin1( "<HTML><HEAD><TITLE>" );
  errText += i18n( "Error while loading %1" ).arg( reqUrl.htmlURL() );
  errText += QString::fromLatin1( "</TITLE></HEAD><BODY><P>" );
  errText += i18n( "An error occurred while loading <B>%1</B>:" ).arg( reqUrl.htmlURL() );
  errText += QString::fromLatin1( "</P><P>" );
  QString kioErrString = KIO::buildErrorString( errorCode, text );

  kioErrString.replace(QString::fromLatin1("&"), QString("&amp;"));
  kioErrString.replace(QString::fromLatin1("<"), QString("&lt;"));
  kioErrString.replace(QString::fromLatin1(">"), QString("&gt;"));

  // In case the error string has '\n' in it, replace with <BR/>
  kioErrString.replace( "\n", "<BR/>" );

  errText += kioErrString;
  errText += QString::fromLatin1( "</P></BODY></HTML>" );
  write(errText);
  end();

  d->m_bJScriptForce = bJSFO;
  d->m_bJScriptOverride = bJSOO;

  // make the working url the current url, so that reload works and
  // emit the progress signals to advance one step in the history
  // (so that 'back' works)
  m_url = reqUrl; // same as d->m_workingURL
  d->m_workingURL = KURL();
  emit started( 0 );
  emit completed();
  return;
  // following disabled until 3.1

  QString errorName, techName, description;
  QStringList causes, solutions;

  QByteArray raw = KIO::rawErrorDetail( errorCode, text, &reqUrl );
  QDataStream stream(raw, IO_ReadOnly);

  stream >> errorName >> techName >> description >> causes >> solutions;

  QString url, protocol, datetime;
  url = reqUrl.prettyURL();
  protocol = reqUrl.protocol();
  datetime = KGlobal::locale()->formatDateTime( QDateTime::currentDateTime(),
                                                false );

  QString doc = QString::fromLatin1( "<html><head><title>" );
  doc += i18n( "Error: " );
  doc += errorName;
  doc += QString::fromLatin1( " - %1</title></head><body><h1>" ).arg( url );
  doc += i18n( "The requested operation could not be completed" );
  doc += QString::fromLatin1( "</h1><h2>" );
  doc += errorName;
  doc += QString::fromLatin1( "</h2>" );
  if ( !techName.isNull() ) {
    doc += QString::fromLatin1( "<h2>" );
    doc += i18n( "Technical Reason: " );
    doc += techName;
    doc += QString::fromLatin1( "</h2>" );
  }
  doc += QString::fromLatin1( "<h3>" );
  doc += i18n( "Details of the Request:" );
  doc += QString::fromLatin1( "</h3><ul><li>" );
  doc += i18n( "URL: %1" ).arg( url );
  doc += QString::fromLatin1( "</li><li>" );
  if ( !protocol.isNull() ) {
    // uncomment for 3.1... i18n change
    // doc += i18n( "Protocol: %1" ).arg( protocol ).arg( protocol );
    doc += QString::fromLatin1( "</li><li>" );
  }
  doc += i18n( "Date and Time: %1" ).arg( datetime );
  doc += QString::fromLatin1( "</li><li>" );
  doc += i18n( "Additional Information: %1" ).arg( text );
  doc += QString::fromLatin1( "</li></ul><h3>" );
  doc += i18n( "Description:" );
  doc += QString::fromLatin1( "</h3><p>" );
  doc += description;
  doc += QString::fromLatin1( "</p>" );
  if ( causes.count() ) {
    doc += QString::fromLatin1( "<h3>" );
    doc += i18n( "Possible Causes:" );
    doc += QString::fromLatin1( "</h3><ul><li>" );
    doc += causes.join( "</li><li>" );
    doc += QString::fromLatin1( "</li></ul>" );
  }
  if ( solutions.count() ) {
    doc += QString::fromLatin1( "<h3>" );
    doc += i18n( "Possible Solutions:" );
    doc += QString::fromLatin1( "</h3><ul><li>" );
    doc += solutions.join( "</li><li>" );
    doc += QString::fromLatin1( "</li></ul>" );
  }
  doc += QString::fromLatin1( "</body></html>" );

  write( doc );
  end();
}

void KHTMLPart::slotFinished( KIO::Job * job )
{
  d->m_job = 0L;
  d->m_jobspeed = 0L;

  if (job->error())
  {
    KHTMLPageCache::self()->cancelEntry(d->m_cacheId);

    // The following catches errors that occur as a result of HTTP
    // to FTP redirections where the FTP URL is a directory. Since
    // KIO cannot change a redirection request from GET to LISTDIR,
    // we have to take care of it here once we know for sure it is
    // a directory...
    if (job->error() == KIO::ERR_IS_DIRECTORY)
    {
      KParts::URLArgs args;
      emit d->m_extension->openURLRequest( d->m_workingURL, args );
    }
    else
    {
      emit canceled( job->errorString() );
      // TODO: what else ?
      checkCompleted();
      showError( job );
    }

    return;
  }
  //kdDebug( 6050 ) << "slotFinished" << endl;

  KHTMLPageCache::self()->endData(d->m_cacheId);
  if (d->m_jscript)
    d->m_jscript->dataReceived();

  if ( d->m_doc && d->m_doc->docLoader()->expireDate() && m_url.protocol().lower().startsWith("http"))
      KIO::http_update_cache(m_url, false, d->m_doc->docLoader()->expireDate());

  d->m_workingURL = KURL();

  if ( d->m_doc && d->m_doc->parsing())
    end(); //will emit completed()
}

void KHTMLPart::begin( const KURL &url, int xOffset, int yOffset )
{
  clear();
  d->m_bCleared = false;
  d->m_cacheId = 0;
  d->m_bComplete = false;
  d->m_bLoadEventEmitted = false;
  d->m_bWalletOpened = false;

  if(url.isValid()) {
      QString urlString = url.url();
      KHTMLFactory::vLinks()->insert( urlString );
      QString urlString2 = url.prettyURL();
      if ( urlString != urlString2 ) {
          KHTMLFactory::vLinks()->insert( urlString2 );
      }
  }

  // No need to show this for a new page until an error is triggered
  if (!parentPart()) {
    removeJSErrorExtension();
  }

  // ###
  //stopParser();

  KParts::URLArgs args( d->m_extension->urlArgs() );
  args.xOffset = xOffset;
  args.yOffset = yOffset;
  d->m_extension->setURLArgs( args );

  d->m_pageReferrer = QString::null;

  KURL ref(url);
  d->m_referrer = ref.protocol().startsWith("http") ? ref.url() : "";

  m_url = url;
  KURL baseurl;

  if ( !m_url.isEmpty() )
  {
    KURL title( baseurl );
    title.setRef( QString::null );
    title.setQuery( QString::null );
    emit setWindowCaption( title.prettyURL() );
  }
  else
    emit setWindowCaption( i18n( "[Untitled]" ) );

  // ### not sure if XHTML documents served as text/xml should use DocumentImpl or HTMLDocumentImpl
  if (args.serviceType == "text/xml")
    d->m_doc = DOMImplementationImpl::instance()->createDocument( d->m_view );
  else
    d->m_doc = DOMImplementationImpl::instance()->createHTMLDocument( d->m_view );
#ifndef KHTML_NO_CARET
//  d->m_view->initCaret();
#endif

  d->m_doc->ref();
  d->m_doc->setURL( m_url.url() );
  if (!d->m_doc->attached())
    d->m_doc->attach( );
  // We prefer m_baseURL over m_url because m_url changes when we are
  // about to load a new page.
  d->m_doc->setBaseURL( baseurl );
  d->m_doc->docLoader()->setShowAnimations( KHTMLFactory::defaultHTMLSettings()->showAnimations() );
  emit docCreated(); // so that the parent can set the domain

  d->m_paUseStylesheet->setItems(QStringList());
  d->m_paUseStylesheet->setEnabled( false );

  setAutoloadImages( KHTMLFactory::defaultHTMLSettings()->autoLoadImages() );
  QString userStyleSheet = KHTMLFactory::defaultHTMLSettings()->userStyleSheet();
  if ( !userStyleSheet.isEmpty() )
    setUserStyleSheet( KURL( userStyleSheet ) );

  d->m_doc->setRestoreState(args.docState);
  d->m_doc->open();
  connect(d->m_doc,SIGNAL(finishedParsing()),this,SLOT(slotFinishedParsing()));

  emit d->m_extension->enableAction( "print", true );

  d->m_doc->setParsing(true);
}

void KHTMLPart::write( const char *str, int len )
{
  if ( !d->m_decoder )
    d->m_decoder = createDecoder();

  if ( len == 0 )
    return;

  if ( len == -1 )
    len = strlen( str );

  QString decoded = d->m_decoder->decode( str, len );

  if(decoded.isEmpty()) return;

  if(d->m_bFirstData) {
      // determine the parse mode
      d->m_doc->determineParseMode( decoded );
      d->m_bFirstData = false;

  //kdDebug(6050) << "KHTMLPart::write haveEnc = " << d->m_haveEncoding << endl;
      // ### this is still quite hacky, but should work a lot better than the old solution
      if(d->m_decoder->visuallyOrdered()) d->m_doc->setVisuallyOrdered();
      d->m_doc->setDecoderCodec(d->m_decoder->codec());
      d->m_doc->recalcStyle( NodeImpl::Force );
  }

  khtml::Tokenizer* t = d->m_doc->tokenizer();
  if(t)
    t->write( decoded, true );
}

void KHTMLPart::write( const QString &str )
{
  if ( str.isNull() )
    return;

  if(d->m_bFirstData) {
      // determine the parse mode
      d->m_doc->setParseMode( DocumentImpl::Strict );
      d->m_bFirstData = false;
  }
  khtml::Tokenizer* t = d->m_doc->tokenizer();
  if(t)
    t->write( str, true );
}

void KHTMLPart::end()
{
    // make sure nothing's left in there...
    if(d->m_decoder)
        write(d->m_decoder->flush());
    if (d->m_doc)
        d->m_doc->finishParsing();
}

bool KHTMLPart::doOpenStream( const QString& mimeType )
{
    if ( mimeType == "text/html" || mimeType == "text/xml" || mimeType == "application/xhtml+xml" )
    {
        begin( url() );
        return true;
    }
    return false;
}

bool KHTMLPart::doWriteStream( const QByteArray& data )
{
    write( data.data(), data.size() );
    return true;
}

bool KHTMLPart::doCloseStream()
{
    end();
    return true;
}


void KHTMLPart::paint(QPainter *p, const QRect &rc, int yOff, bool *more)
{
    if (!d->m_view) return;
    d->m_view->paint(p, rc, yOff, more);
}

void KHTMLPart::stopAnimations()
{
  if ( d->m_doc )
    d->m_doc->docLoader()->setShowAnimations( KHTMLSettings::KAnimationDisabled );

  ConstFrameIt it = d->m_frames.begin();
  ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it )
    if ( !( *it ).m_part.isNull() && ( *it ).m_part->inherits( "KHTMLPart" ) ) {
      KParts::ReadOnlyPart* p = ( *it ).m_part;
      static_cast<KHTMLPart*>( p )->stopAnimations();
    }
}

void KHTMLPart::slotFinishedParsing()
{
  d->m_doc->setParsing(false);
  checkEmitLoadEvent();
  disconnect(d->m_doc,SIGNAL(finishedParsing()),this,SLOT(slotFinishedParsing()));

  if (!d->m_view)
    return; // We are probably being destructed.
  // check if the scrollbars are really needed for the content
  // if not, remove them, relayout, and repaint

  d->m_view->restoreScrollBar();

  checkCompleted();
}

void KHTMLPart::slotLoaderRequestStarted( khtml::DocLoader* dl, khtml::CachedObject *obj )
{
  if ( obj && obj->type() == khtml::CachedObject::Image && d->m_doc && d->m_doc->docLoader() == dl ) {
    KHTMLPart* p = this;
    while ( p ) {
      KHTMLPart* op = p;
      p->d->m_totalObjectCount++;
      p = p->parentPart();
      if ( !p && op->d->m_loadedObjects <= op->d->m_totalObjectCount
        && !op->d->m_progressUpdateTimer.isActive())
	op->d->m_progressUpdateTimer.start( 200, true );
    }
  }
}

void KHTMLPart::slotLoaderRequestDone( khtml::DocLoader* dl, khtml::CachedObject *obj )
{
  if ( obj && obj->type() == khtml::CachedObject::Image && d->m_doc && d->m_doc->docLoader() == dl ) {
    KHTMLPart* p = this;
    while ( p ) {
      KHTMLPart* op = p;
      p->d->m_loadedObjects++;
      p = p->parentPart();
      if ( !p && op->d->m_loadedObjects <= op->d->m_totalObjectCount && op->d->m_jobPercent <= 100
        && !op->d->m_progressUpdateTimer.isActive())
	op->d->m_progressUpdateTimer.start( 200, true );
    }
  }

  checkCompleted();
}

void KHTMLPart::slotProgressUpdate()
{
  int percent;
  if ( d->m_loadedObjects < d->m_totalObjectCount )
    percent = d->m_jobPercent / 4 + ( d->m_loadedObjects*300 ) / ( 4*d->m_totalObjectCount );
  else
    percent = d->m_jobPercent;

  if( d->m_bComplete )
    percent = 100;

  if (d->m_statusMessagesEnabled) {
    if( d->m_bComplete )
      emit d->m_extension->infoMessage( i18n( "Page loaded." ));
    else if ( d->m_loadedObjects < d->m_totalObjectCount && percent >= 75 )
      emit d->m_extension->infoMessage( i18n( "%n Image of %1 loaded.", "%n Images of %1 loaded.", d->m_loadedObjects).arg(d->m_totalObjectCount) );
  }

  emit d->m_extension->loadingProgress( percent );
}

void KHTMLPart::slotJobSpeed( KIO::Job* /*job*/, unsigned long speed )
{
  d->m_jobspeed = speed;
  if (!parentPart())
    setStatusBarText(jsStatusBarText(), BarOverrideText);
}

void KHTMLPart::slotJobPercent( KIO::Job* /*job*/, unsigned long percent )
{
  d->m_jobPercent = percent;

  if ( !parentPart() )
    d->m_progressUpdateTimer.start( 0, true );
}

void KHTMLPart::slotJobDone( KIO::Job* /*job*/ )
{
  d->m_jobPercent = 100;

  if ( !parentPart() )
    d->m_progressUpdateTimer.start( 0, true );
}

void KHTMLPart::slotUserSheetStatDone( KIO::Job *_job )
{
  using namespace KIO;

  if ( _job->error() ) {
    showError( _job );
    return;
  }

  const UDSEntry entry = dynamic_cast<KIO::StatJob *>( _job )->statResult();
  UDSEntry::ConstIterator it = entry.begin();
  UDSEntry::ConstIterator end = entry.end();
  for ( ; it != end; ++it ) {
    if ( ( *it ).m_uds == UDS_MODIFICATION_TIME ) {
     break;
    }
  }

  // If the filesystem supports modification times, only reload the
  // user-defined stylesheet if necessary - otherwise always reload.
  if ( it != end ) {
    const time_t lastModified = static_cast<time_t>( ( *it ).m_long );
    if ( d->m_userStyleSheetLastModified >= lastModified ) {
      return;
    }
    d->m_userStyleSheetLastModified = lastModified;
  }

  setUserStyleSheet( KURL( settings()->userStyleSheet() ) );
}

void KHTMLPart::checkCompleted()
{
//   kdDebug( 6050 ) << "KHTMLPart::checkCompleted() " << this << " " << name() << endl;
//   kdDebug( 6050 ) << "                           parsing: " << (d->m_doc && d->m_doc->parsing()) << endl;
//   kdDebug( 6050 ) << "                           complete: " << d->m_bComplete << endl;

  // restore the cursor position
  if (d->m_doc && !d->m_doc->parsing() && !d->m_focusNodeRestored)
  {
      if (d->m_focusNodeNumber >= 0)
          d->m_doc->setFocusNode(d->m_doc->nodeWithAbsIndex(d->m_focusNodeNumber));

      d->m_focusNodeRestored = true;
  }

  bool bPendingChildRedirection = false;
  // Any frame that hasn't completed yet ?
  ConstFrameIt it = d->m_frames.begin();
  ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it ) {
    if ( !(*it).m_bCompleted )
    {
      //kdDebug( 6050 ) << this << " is waiting for " << ( *it ).m_part << endl;
      return;
    }
    // Check for frames with pending redirections
    if ( (*it).m_bPendingRedirection )
      bPendingChildRedirection = true;
  }

  // Any object that hasn't completed yet ?
  for (it = d->m_objects.begin(); it != d->m_objects.end(); ++it )
    if ( !(*it).m_bCompleted )
      return;

  // Are we still parsing - or have we done the completed stuff already ?
  if ( d->m_bComplete || (d->m_doc && d->m_doc->parsing()) )
    return;

  // Still waiting for images/scripts from the loader ?
  int requests = 0;
  if ( d->m_doc && d->m_doc->docLoader() )
    requests = khtml::Cache::loader()->numRequests( d->m_doc->docLoader() );

  if ( requests > 0 )
  {
    //kdDebug(6050) << "still waiting for images/scripts from the loader - requests:" << requests << endl;
    return;
  }

  // OK, completed.
  // Now do what should be done when we are really completed.
  d->m_bComplete = true;
  d->m_cachePolicy = KIO::CC_Verify; // reset cache policy
  d->m_totalObjectCount = 0;
  d->m_loadedObjects = 0;

  KHTMLPart* p = this;
  while ( p ) {
    KHTMLPart* op = p;
    p = p->parentPart();
    if ( !p && !op->d->m_progressUpdateTimer.isActive())
      op->d->m_progressUpdateTimer.start( 0, true );
  }

  checkEmitLoadEvent(); // if we didn't do it before

  // check that the view has not been moved by the user

  if ( m_url.encodedHtmlRef().isEmpty() && d->m_view->contentsY() == 0 )
      d->m_view->setContentsPos( d->m_extension->urlArgs().xOffset,
                                 d->m_extension->urlArgs().yOffset );

  d->m_view->complete();

  if ( !d->m_redirectURL.isEmpty() )
  {
    // Do not start redirection for frames here! That action is
    // deferred until the parent emits a completed signal.
    if ( parentPart() == 0 )
      d->m_redirectionTimer.start( 1000 * d->m_delayRedirect, true );

    emit completed( true );
  }
  else
  {
    if ( bPendingChildRedirection )
      emit completed( true );
    else
      emit completed();
  }

  // find the alternate stylesheets
  QStringList sheets;
  if (d->m_doc)
     sheets = d->m_doc->availableStyleSheets();
  sheets.prepend( i18n( "Automatic Detection" ) );
  d->m_paUseStylesheet->setItems( sheets );

  d->m_paUseStylesheet->setEnabled( sheets.count() > 2);
  if (sheets.count() > 2)
  {
    d->m_paUseStylesheet->setCurrentItem(kMax(sheets.findIndex(d->m_sheetUsed), 0));
    slotUseStylesheet();
  }

  setJSDefaultStatusBarText(QString::null);

  if ( !m_url.encodedHtmlRef().isEmpty() )
    if ( !gotoAnchor( m_url.encodedHtmlRef()) )
       gotoAnchor( m_url.htmlRef() );

#ifdef SPEED_DEBUG
  kdDebug(6050) << "DONE: " <<d->m_parsetime.elapsed() << endl;
#endif
}

void KHTMLPart::checkEmitLoadEvent()
{
  if ( d->m_bLoadEventEmitted || !d->m_doc || d->m_doc->parsing() ) return;

  ConstFrameIt it = d->m_frames.begin();
  ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it )
    if ( !(*it).m_bCompleted ) // still got a frame running -> too early
      return;

  for (it = d->m_objects.begin(); it != d->m_objects.end(); ++it )
    if ( !(*it).m_bCompleted ) // still got a object running -> too early
      return;

  // Still waiting for images/scripts from the loader ?
  // (onload must happen afterwards, #45607)
  // ## This makes this method very similar to checkCompleted. A brave soul should try merging them.
  int requests = 0;
  if ( d->m_doc && d->m_doc->docLoader() )
    requests = khtml::Cache::loader()->numRequests( d->m_doc->docLoader() );

  if ( requests > 0 )
    return;

  d->m_bLoadEventEmitted = true;
  if (d->m_doc)
    d->m_doc->close();
}

const KHTMLSettings *KHTMLPart::settings() const
{
  return d->m_settings;
}

#ifndef KDE_NO_COMPAT
KURL KHTMLPart::baseURL() const
{
  if ( !d->m_doc ) return KURL();

  return d->m_doc->baseURL();
}

QString KHTMLPart::baseTarget() const
{
  if ( !d->m_doc ) return QString::null;

  return d->m_doc->baseTarget();
}
#endif

KURL KHTMLPart::completeURL( const QString &url )
{
  if ( !d->m_doc ) return KURL( url );

  if (d->m_decoder)
    return KURL(d->m_doc->completeURL(url), d->m_decoder->codec()->mibEnum());

  return KURL( d->m_doc->completeURL( url ) );
}

void KHTMLPart::scheduleRedirection( int delay, const QString &url, bool doLockHistory )
{
  kdDebug(6050) << "KHTMLPart::scheduleRedirection delay=" << delay << " url=" << url << endl;
  kdDebug(6050) << "current redirectURL=" << d->m_redirectURL << " with delay " << d->m_delayRedirect <<  endl;
  if( delay < 24*60*60 &&
      ( d->m_redirectURL.isEmpty() || delay < d->m_delayRedirect ) ) {
    d->m_delayRedirect = delay;
    d->m_redirectURL = url;
    d->m_redirectLockHistory = doLockHistory;
    kdDebug(6050) << " d->m_bComplete=" << d->m_bComplete << endl;
    if ( d->m_bComplete ) {
      d->m_redirectionTimer.stop();
      d->m_redirectionTimer.start( kMax(0, 1000 * d->m_delayRedirect), true );
    }
  }
}

void KHTMLPart::slotRedirect()
{
  kdDebug() << k_funcinfo << endl;
  QString u = d->m_redirectURL;
  d->m_delayRedirect = 0;
  d->m_redirectURL = QString::null;

  // SYNC check with ecma/kjs_window.cpp::goURL !
  if ( u.find( QString::fromLatin1( "javascript:" ), 0, false ) == 0 )
  {
    QString script = KURL::decode_string( u.right( u.length() - 11 ) );
    kdDebug( 6050 ) << "KHTMLPart::slotRedirect script=" << script << endl;
    QVariant res = executeScript( DOM::Node(), script );
    if ( res.type() == QVariant::String ) {
      begin( url() );
      write( res.asString() );
      end();
    }
    return;
  }
  KParts::URLArgs args;
  // Redirecting to the current URL leads to a reload.
  // But jumping to an anchor never leads to a reload.
  KURL cUrl( m_url );
  KURL url( u );

  // handle windows opened by JS
  if ( openedByJS() && d->m_opener )
      cUrl = d->m_opener->url();

  if (!kapp || !kapp->authorizeURLAction("redirect", cUrl, url))
  {
    kdWarning(6050) << "KHTMLPart::scheduleRedirection: Redirection from " << cUrl.prettyURL() << " to " << url.prettyURL() << " REJECTED!" << endl;
    return;
  }

  if ( urlcmp( u, m_url.url(), true, true ) )
  {
    if (!url.hasRef())
       args.reload = true;
    args.metaData().insert("referrer", d->m_pageReferrer);
  }

  // Indicate that this request is due to a redirection.
  args.setRedirectedRequest(true);

  args.setLockHistory( d->m_redirectLockHistory );
  // _self: make sure we don't use any <base target=>'s
  urlSelected( u, 0, 0, "_self", args );
}

void KHTMLPart::slotRedirection(KIO::Job*, const KURL& url)
{
  // the slave told us that we got redirected
  //kdDebug( 6050 ) << "redirection by KIO to " << url.url() << endl;
  emit d->m_extension->setLocationBarURL( url.prettyURL() );
  d->m_workingURL = url;
}

bool KHTMLPart::setEncoding( const QString &name, bool override )
{
    d->m_encoding = name;
    d->m_haveEncoding = override;

    if( !m_url.isEmpty() ) {
        // reload document
        closeURL();
        KURL url = m_url;
        m_url = 0;
        d->m_restored = true;
        openURL(url);
        d->m_restored = false;
    }

    return true;
}

QString KHTMLPart::encoding() const
{
    if(d->m_haveEncoding && !d->m_encoding.isEmpty())
        return d->m_encoding;

    if(d->m_decoder && d->m_decoder->encoding())
        return QString(d->m_decoder->encoding());

    return(settings()->encoding());
}

void KHTMLPart::setUserStyleSheet(const KURL &url)
{
  if ( d->m_doc && d->m_doc->docLoader() )
    (void) new khtml::PartStyleSheetLoader(this, url.url(), d->m_doc->docLoader());
}

void KHTMLPart::setUserStyleSheet(const QString &styleSheet)
{
  if ( d->m_doc )
    d->m_doc->setUserStyleSheet( styleSheet );
}

bool KHTMLPart::gotoAnchor( const QString &name )
{
  if (!d->m_doc)
    return false;

  HTMLCollectionImpl *anchors =
      new HTMLCollectionImpl( d->m_doc, HTMLCollectionImpl::DOC_ANCHORS);
  anchors->ref();
  NodeImpl *n = anchors->namedItem(name);
  anchors->deref();

  if(!n) {
      n = d->m_doc->getElementById( name );
  }

  if(!n) {
      kdDebug(6050) << "KHTMLPart::gotoAnchor node '" << name << "' not found" << endl;
      return false;
  }

  int x = 0, y = 0;
  int gox, dummy;
  HTMLElementImpl *a = static_cast<HTMLElementImpl *>(n);

  a->getUpperLeftCorner(x, y);
  if (x <= d->m_view->contentsX())
    gox = x - 10;
  else {
    gox = d->m_view->contentsX();
    if ( x + 10 > d->m_view->contentsX()+d->m_view->visibleWidth()) {
      a->getLowerRightCorner(x, dummy);
      gox = x - d->m_view->visibleWidth() + 10;
    }
  }

  d->m_view->setContentsPos(gox, y-20);

  return true;
}

bool KHTMLPart::nextAnchor()
{
  if (!d->m_doc)
    return false;
  d->m_view->focusNextPrevNode ( true );

  return true;
}

bool KHTMLPart::prevAnchor()
{
  if (!d->m_doc)
    return false;
  d->m_view->focusNextPrevNode ( false );

  return true;
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

QCursor KHTMLPart::urlCursor() const
{
  return d->m_linkCursor;
}

bool KHTMLPart::onlyLocalReferences() const
{
  return d->m_onlyLocalReferences;
}

void KHTMLPart::setOnlyLocalReferences(bool enable)
{
  d->m_onlyLocalReferences = enable;
}

void KHTMLPartPrivate::setFlagRecursively(
	bool KHTMLPartPrivate::*flag, bool value)
{
  // first set it on the current one
  this->*flag = value;

  // descend into child frames recursively
  QValueList<khtml::ChildFrame>::Iterator it = m_frames.begin();
  for (; it != m_frames.end(); ++it) {
    KHTMLPart *part = static_cast<KHTMLPart *>((KParts::ReadOnlyPart *)(*it).m_part);
    if (part->inherits("KHTMLPart"))
      part->d->setFlagRecursively(flag, value);
  }/*next it*/

  // do the same again for objects
  it = m_objects.begin();
  for (; it != m_objects.end(); ++it) {
    KHTMLPart *part = static_cast<KHTMLPart *>((KParts::ReadOnlyPart *)(*it).m_part);
    if (part->inherits("KHTMLPart"))
      part->d->setFlagRecursively(flag, value);
  }/*next it*/
}

void KHTMLPart::setCaretMode(bool enable)
{
#ifndef KHTML_NO_CARET
  kdDebug(6200) << "setCaretMode(" << enable << ")" << endl;
  if (isCaretMode() == enable) return;
  d->setFlagRecursively(&KHTMLPartPrivate::m_caretMode, enable);
  // FIXME: this won't work on frames as expected
  if (!isEditable()) {
    if (enable) {
      view()->initCaret(true);
      view()->ensureCaretVisible();
    } else
      view()->caretOff();
  }/*end if*/
#endif // KHTML_NO_CARET
}

bool KHTMLPart::isCaretMode() const
{
  return d->m_caretMode;
}

void KHTMLPart::setEditable(bool enable)
{
#ifndef KHTML_NO_CARET
  if (isEditable() == enable) return;
  d->setFlagRecursively(&KHTMLPartPrivate::m_designMode, enable);
  // FIXME: this won't work on frames as expected
  if (!isCaretMode()) {
    if (enable) {
      view()->initCaret(true);
      view()->ensureCaretVisible();
    } else
      view()->caretOff();
  }/*end if*/
#endif // KHTML_NO_CARET
}

bool KHTMLPart::isEditable() const
{
  return d->m_designMode;
}

void KHTMLPart::setCaretPosition(DOM::Node node, long offset, bool extendSelection)
{
#ifndef KHTML_NO_CARET
#if 0
  kdDebug(6200) << k_funcinfo << "node: " << node.handle() << " nodeName: "
  	<< node.nodeName().string() << " offset: " << offset
	<< " extendSelection " << extendSelection << endl;
#endif
  if (view()->moveCaretTo(node.handle(), offset, !extendSelection))
    emitSelectionChanged();
  view()->ensureCaretVisible();
#endif // KHTML_NO_CARET
}

KHTMLPart::CaretDisplayPolicy KHTMLPart::caretDisplayPolicyNonFocused() const
{
#ifndef KHTML_NO_CARET
  return (CaretDisplayPolicy)view()->caretDisplayPolicyNonFocused();
#else // KHTML_NO_CARET
  return CaretInvisible;
#endif // KHTML_NO_CARET
}

void KHTMLPart::setCaretDisplayPolicyNonFocused(CaretDisplayPolicy policy)
{
#ifndef KHTML_NO_CARET
  view()->setCaretDisplayPolicyNonFocused(policy);
#endif // KHTML_NO_CARET
}

void KHTMLPart::setCaretVisible(bool show)
{
#ifndef KHTML_NO_CARET
  if (show) {

    NodeImpl *caretNode = xmlDocImpl()->focusNode();
    if (isCaretMode() || isEditable()
	|| (caretNode && caretNode->contentEditable())) {
      view()->caretOn();
    }/*end if*/

  } else {

    view()->caretOff();

  }/*end if*/
#endif // KHTML_NO_CARET
}

void KHTMLPart::findTextBegin()
{
  d->m_findPos = -1;
  d->m_findNode = 0;
  d->m_findPosEnd = -1;
  d->m_findNodeEnd= 0;
  delete d->m_find;
  d->m_find = 0L;
}

bool KHTMLPart::initFindNode( bool selection, bool reverse, bool fromCursor )
{
    if ( !d->m_doc )
        return false;

    DOM::NodeImpl* firstNode = 0L;
    if (d->m_doc->isHTMLDocument())
      firstNode = static_cast<HTMLDocumentImpl*>(d->m_doc)->body();
    else
      firstNode = d->m_doc;

    if ( !firstNode )
    {
      //kdDebug(6050) << k_funcinfo << "no first node (body or doc) -> return false" << endl;
      return false;
    }
    if ( firstNode->id() == ID_FRAMESET )
    {
      //kdDebug(6050) << k_funcinfo << "FRAMESET -> return false" << endl;
      return false;
    }

    if ( selection && hasSelection() )
    {
      //kdDebug(6050) << k_funcinfo << "using selection" << endl;
      if ( !fromCursor )
      {
        d->m_findNode = reverse ? d->m_selectionEnd.handle() : d->m_selectionStart.handle();
        d->m_findPos = reverse ? d->m_endOffset : d->m_startOffset;
      }
      d->m_findNodeEnd = reverse ? d->m_selectionStart.handle() : d->m_selectionEnd.handle();
      d->m_findPosEnd = reverse ? d->m_startOffset : d->m_endOffset;
    }
    else // whole document
    {
      //kdDebug(6050) << k_funcinfo << "whole doc" << endl;
      if ( !fromCursor )
      {
        d->m_findNode = reverse ? 0 : firstNode;
        d->m_findPos = reverse ? -1 : 0;
      }
      d->m_findNodeEnd = reverse ? firstNode : 0;
      d->m_findPosEnd = reverse ? 0 : -1;
      if ( d->m_findNode == 0 ) {
        d->m_findNode = firstNode; // body or doc
        // Need to find out the really last object, to start from it
        while ( d->m_findNode->lastChild() )
          d->m_findNode = d->m_findNode->lastChild();
      }
    }
    return true;
}

// Old method (its API limits the available features - remove in KDE-4)
bool KHTMLPart::findTextNext( const QString &str, bool forward, bool caseSensitive, bool isRegExp )
{
    if ( !initFindNode( false, !forward, false ) )
      return false;
    while(1)
    {
        if( (d->m_findNode->nodeType() == Node::TEXT_NODE || d->m_findNode->nodeType() == Node::CDATA_SECTION_NODE) && d->m_findNode->renderer() )
        {
            DOMString nodeText = d->m_findNode->nodeValue();
            DOMStringImpl *t = nodeText.implementation();
            QConstString s(t->s, t->l);

            int matchLen = 0;
            if ( isRegExp ) {
              QRegExp matcher( str );
              matcher.setCaseSensitive( caseSensitive );
              d->m_findPos = matcher.search(s.string(), d->m_findPos+1);
              if ( d->m_findPos != -1 )
                matchLen = matcher.matchedLength();
            }
            else {
              d->m_findPos = s.string().find(str, d->m_findPos+1, caseSensitive);
              matchLen = str.length();
            }

            if(d->m_findPos != -1)
            {
                int x = 0, y = 0;
                if(static_cast<khtml::RenderText *>(d->m_findNode->renderer())
                  ->posOfChar(d->m_findPos, x, y))
                    d->m_view->setContentsPos(x-50, y-50);

                d->m_selectionStart = d->m_findNode;
                d->m_startOffset = d->m_findPos;
                d->m_selectionEnd = d->m_findNode;
                d->m_endOffset = d->m_findPos + matchLen;
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


void KHTMLPart::slotFind()
{
  KParts::ReadOnlyPart *part = currentFrame();
  if (!part)
    return;
  if (!part->inherits("KHTMLPart") )
  {
      kdError(6000) << "slotFind: part is a " << part->className() << ", can't do a search into it" << endl;
      return;
  }
  static_cast<KHTMLPart *>( part )->findText();
}

void KHTMLPart::slotFindNext()
{
  KParts::ReadOnlyPart *part = currentFrame();
  if (!part)
    return;
  if (!part->inherits("KHTMLPart") )
  {
      kdError(6000) << "slotFindNext: part is a " << part->className() << ", can't do a search into it" << endl;
      return;
  }
  static_cast<KHTMLPart *>( part )->findTextNext();
}

void KHTMLPart::slotFindDone()
{
  // ### remove me
}

void KHTMLPart::slotFindDialogDestroyed()
{
  d->m_lastFindState.options = d->m_findDialog->options();
  d->m_lastFindState.history = d->m_findDialog->findHistory();
  d->m_findDialog->deleteLater();
  d->m_findDialog = 0L;
}

void KHTMLPart::findText()
{
  // First do some init to make sure we can search in this frame
  if ( !d->m_doc )
    return;

  // Raise if already opened
  if ( d->m_findDialog )
  {
    KWin::activateWindow( d->m_findDialog->winId() );
    return;
  }

  // The lineedit of the dialog would make khtml lose its selection, otherwise
#ifndef QT_NO_CLIPBOARD
  disconnect( kapp->clipboard(), SIGNAL(selectionChanged()), this, SLOT(slotClearSelection()) );
#endif

  // Now show the dialog in which the user can choose options.
  d->m_findDialog = new KFindDialog( false /*non-modal*/, widget(), "khtmlfind" );
  d->m_findDialog->setHasSelection( hasSelection() );
  d->m_findDialog->setHasCursor( d->m_findNode != 0 );
  if ( d->m_findNode ) // has a cursor -> default to 'FromCursor'
    d->m_lastFindState.options |= KFindDialog::FromCursor;

  // TODO? optionsDialog.setPattern( d->m_lastFindState.text );
  d->m_findDialog->setFindHistory( d->m_lastFindState.history );
  d->m_findDialog->setOptions( d->m_lastFindState.options );

  d->m_lastFindState.options = -1; // force update in findTextNext

  d->m_findDialog->show();
  connect( d->m_findDialog, SIGNAL(okClicked()), this, SLOT(slotFindNext()) );
  connect( d->m_findDialog, SIGNAL(finished()), this, SLOT(slotFindDialogDestroyed()) );

#ifndef QT_NO_CLIPBOARD
  connect( kapp->clipboard(), SIGNAL(selectionChanged()), SLOT(slotClearSelection()) );
#endif

  // Create the KFind object
  delete d->m_find;
  d->m_find = new KFind( d->m_findDialog->pattern(), 0 /*options*/, widget(), d->m_findDialog );
  d->m_find->closeFindNextDialog(); // we use KFindDialog non-modal, so we don't want another dlg popping up
  connect(d->m_find, SIGNAL( highlight( const QString &, int, int ) ),
          this, SLOT( slotHighlight( const QString &, int, int ) ) );
  //connect(d->m_find, SIGNAL( findNext() ),
  //        this, SLOT( slotFindNext() ) );
}

// New method
void KHTMLPart::findTextNext()
{
  if (!d->m_find)
  {
    // We didn't show the find dialog yet, let's do it then (#49442)
    findText();
    return;
  }

  long options = 0;
  if ( d->m_findDialog ) // 0 when we close the dialog
  {
    if ( d->m_find->pattern() != d->m_findDialog->pattern() ) {
	    d->m_find->setPattern( d->m_findDialog->pattern() );
	    d->m_find->resetCounts();
    }
    long options = d->m_findDialog->options();
    if ( d->m_lastFindState.options != options )
    {
      d->m_find->setOptions( options );

      if ( options & KFindDialog::SelectedText )
        Q_ASSERT( hasSelection() );

      long difference = d->m_lastFindState.options ^ options;
      if ( difference & (KFindDialog::SelectedText | KFindDialog::FromCursor ) )
      {
          // Important options changed -> reset search range
        (void) initFindNode( options & KFindDialog::SelectedText,
                             options & KFindDialog::FindBackwards,
                             options & KFindDialog::FromCursor );
      }
      d->m_lastFindState.options = options;
    }
  } else
    options = d->m_lastFindState.options;

  KFind::Result res = KFind::NoMatch;
  khtml::RenderObject* obj = d->m_findNode ? d->m_findNode->renderer() : 0;
  khtml::RenderObject* end = d->m_findNodeEnd ? d->m_findNodeEnd->renderer() : 0;
  khtml::RenderTextArea *tmpTextArea=0L;
  //kdDebug(6050) << k_funcinfo << "obj=" << obj << " end=" << end << endl;
  while( res == KFind::NoMatch )
  {
    if ( d->m_find->needData() )
    {
      if ( !obj ) {
        //kdDebug(6050) << k_funcinfo << "obj=0 -> done" << endl;
        break; // we're done
      }
      //kdDebug(6050) << k_funcinfo << " gathering data" << endl;
      // First make up the QString for the current 'line' (i.e. up to \n)
      // We also want to remember the DOMNode for every portion of the string.
      // We store this in an index->node list.

      d->m_stringPortions.clear();
      int newLinePos = -1;
      QString str;
      DOM::NodeImpl* lastNode = d->m_findNode;
      while ( obj && newLinePos == -1 )
      {
        // Grab text from render object
        QString s;
        bool renderAreaText = (QCString(obj->parent()->renderName())== "RenderTextArea");
        bool renderLineText = (QCString(obj->renderName())== "RenderLineEdit");
        if ( renderAreaText )
        {
          khtml::RenderTextArea *parent= static_cast<khtml::RenderTextArea *>(obj->parent());
          s = parent->text();
          s = s.replace(0xa0, ' ');
          tmpTextArea = parent;
        }
        else if ( renderLineText )
        {
          khtml::RenderLineEdit *parentLine= static_cast<khtml::RenderLineEdit *>(obj);
          s = parentLine->widget()->text();
          s = s.replace(0xa0, ' ');
        }
        else if ( obj->isText() )
        {
          if ( obj->parent()!=tmpTextArea )
          {
            s = static_cast<khtml::RenderText *>(obj)->data().string();
            s = s.replace(0xa0, ' ');
          }
        }
        else if ( obj->isBR() )
          s = '\n';
        else if ( !obj->isInline() && !str.isEmpty() )
          s = '\n';
        if ( lastNode == d->m_findNodeEnd )
          s.truncate( d->m_findPosEnd );
        if ( !s.isEmpty() )
        {
          newLinePos = s.find( '\n' ); // did we just get a newline?
          int index = str.length();
          if ( newLinePos != -1 )
            newLinePos += index;
          str += s;
          //kdDebug(6050) << "StringPortion: " << index << "-" << index+s.length()-1 << " -> " << lastNode << endl;
          d->m_stringPortions.append( KHTMLPartPrivate::StringPortion( index, lastNode ) );
        }
        // Compare obj and end _after_ we processed the 'end' node itself
        if ( obj == end )
          obj = 0L;
        else
        {
          // Move on to next object (note: if we found a \n already, then obj (and lastNode)
          // will point to the _next_ object, i.e. they are in advance.
          do {
            // We advance until the next RenderObject that has a NodeImpl as its element().
            // Otherwise (if we keep the 'last node', and it has a '\n') we might be stuck
            // on that object forever...
            obj = (options & KFindDialog::FindBackwards) ? obj->objectAbove() : obj->objectBelow();
          } while ( obj && !obj->element() );
        }
        if ( obj )
          lastNode = obj->element();
        else
          lastNode = 0;
      } // end while
      //kdDebug()<<" str : "<<str<<endl;
      if ( !str.isEmpty() )
      {
        d->m_find->setData( str, d->m_findPos );
      }

      d->m_findPos = -1; // not used during the findnext loops. Only during init.
      d->m_findNode = lastNode;
    }
    if ( !d->m_find->needData() ) // happens if str was empty
    {
      // Let KFind inspect the text fragment, and emit highlighted if a match is found
      res = d->m_find->find();
    }
  } // end while

  if ( res == KFind::NoMatch ) // i.e. we're done
  {
    if ( d->m_find->shouldRestart() )
    {
      //kdDebug(6050) << "Restarting" << endl;
      initFindNode( false, options & KFindDialog::FindBackwards, false );
      findTextNext();
    }
    else // really done
    {
      //kdDebug(6050) << "Finishing" << endl;
      //delete d->m_find;
      //d->m_find = 0L;
      initFindNode( false, options & KFindDialog::FindBackwards, false );
      d->m_find->resetCounts();
      slotClearSelection();
    }
  }
}

void KHTMLPart::slotHighlight( const QString& /*text*/, int index, int length )
{
  //kdDebug(6050) << "slotHighlight index=" << index << " length=" << length << endl;
  QValueList<KHTMLPartPrivate::StringPortion>::Iterator it = d->m_stringPortions.begin();
  QValueList<KHTMLPartPrivate::StringPortion>::Iterator prev = it;
  // We stop at the first portion whose index is 'greater than', and then use the previous one
  while ( it != d->m_stringPortions.end() && (*it).index <= index )
  {
    prev = it;
    ++it;
  }
  Q_ASSERT ( prev != d->m_stringPortions.end() );
  DOM::NodeImpl* node = (*prev).node;
  Q_ASSERT( node );

  d->m_selectionStart = node;
  d->m_startOffset = index - (*prev).index;

  khtml::RenderObject* obj = node->renderer();
  khtml::RenderTextArea *parent = 0L;
  khtml::RenderLineEdit *parentLine = 0L;
  bool renderLineText =false;

  QRect highlightedRect;
  bool renderAreaText =false;
  Q_ASSERT( obj );
  if ( obj )
  {
    int x = 0, y = 0;
    renderAreaText = (QCString(obj->parent()->renderName())== "RenderTextArea");
    renderLineText = (QCString(obj->renderName())== "RenderLineEdit");


    if( renderAreaText )
      parent= static_cast<khtml::RenderTextArea *>(obj->parent());
    if ( renderLineText )
      parentLine= static_cast<khtml::RenderLineEdit *>(obj);
    if ( !renderLineText )
      //if (static_cast<khtml::RenderText *>(node->renderer())
      //    ->posOfChar(d->m_startOffset, x, y))
      {
        int dummy;
        static_cast<khtml::RenderText *>(node->renderer())
          ->caretPos( d->m_startOffset, false, x, y, dummy, dummy ); // more precise than posOfChar
        //kdDebug(6050) << "topleft: " << x << "," << y << endl;
        if ( x != -1 || y != -1 )
        {
          d->m_view->setContentsPos(x-50, y-50);
          highlightedRect.setTopLeft( d->m_view->mapToGlobal(QPoint(x, y)) );
        }
      }
  }
  // Now look for end node
  it = prev; // no need to start from beginning again
  while ( it != d->m_stringPortions.end() && (*it).index < index + length )
  {
    prev = it;
    ++it;
  }
  Q_ASSERT ( prev != d->m_stringPortions.end() );

  d->m_selectionEnd = (*prev).node;
  d->m_endOffset = index + length - (*prev).index;
  d->m_startBeforeEnd = true;

#if 0
  kdDebug(6050) << "slotHighlight: " << d->m_selectionStart.handle() << "," << d->m_startOffset << " - " <<
    d->m_selectionEnd.handle() << "," << d->m_endOffset << endl;
  it = d->m_stringPortions.begin();
  for ( ; it != d->m_stringPortions.end() ; ++it )
    kdDebug(6050) << "  StringPortion: from index=" << (*it).index << " -> node=" << (*it).node << endl;
#endif
  if( renderAreaText )
  {
    if( parent )
      parent->highLightWord( length, d->m_endOffset-length );
  }
  else if ( renderLineText )
  {
    if( parentLine )
      parentLine->highLightWord( length, d->m_endOffset-length );
  }
  else
  {
    d->m_doc->setSelection( d->m_selectionStart.handle(), d->m_startOffset,
                            d->m_selectionEnd.handle(), d->m_endOffset );
    if (d->m_selectionEnd.handle()->renderer() )
    {
      int x, y, height, dummy;
      static_cast<khtml::RenderText *>(d->m_selectionEnd.handle()->renderer())
          ->caretPos( d->m_endOffset, false, x, y, dummy, height ); // more precise than posOfChar
      //kdDebug(6050) << "bottomright: " << x << "," << y+height << endl;
      if ( x != -1 || y != -1 )
      {
        // if ( static_cast<khtml::RenderText *>(d->m_selectionEnd.handle()->renderer())
        //  ->posOfChar(d->m_endOffset-1, x, y))
        highlightedRect.setBottomRight( d->m_view->mapToGlobal( QPoint(x, y+height) ) );
      }
    }
  }
  emitSelectionChanged();

  // make the finddialog move away from the selected area
  if ( d->m_findDialog && !highlightedRect.isNull() )
  {
    highlightedRect.moveBy( -d->m_view->contentsX(), -d->m_view->contentsY() );
    //kdDebug(6050) << "avoiding " << highlightedRect << endl;
    KDialog::avoidArea( d->m_findDialog, highlightedRect );
  }
}

QString KHTMLPart::selectedText() const
{
  bool hasNewLine = true;
  QString text;
  DOM::Node n = d->m_selectionStart;
  while(!n.isNull()) {
      if(n.nodeType() == DOM::Node::TEXT_NODE && n.handle()->renderer()) {
        QString str = n.nodeValue().string();
        hasNewLine = false;
        if(n == d->m_selectionStart && n == d->m_selectionEnd)
          text = str.mid(d->m_startOffset, d->m_endOffset - d->m_startOffset);
        else if(n == d->m_selectionStart)
          text = str.mid(d->m_startOffset);
        else if(n == d->m_selectionEnd)
          text += str.left(d->m_endOffset);
        else
          text += str;
      }
      else {
        // This is our simple HTML -> ASCII transformation:
        unsigned short id = n.elementId();
        switch(id) {
          case ID_BR:
            text += "\n";
            hasNewLine = true;
            break;

          case ID_TD:
          case ID_TH:
          case ID_HR:
          case ID_OL:
          case ID_UL:
          case ID_LI:
          case ID_DD:
          case ID_DL:
          case ID_DT:
          case ID_PRE:
          case ID_BLOCKQUOTE:
          case ID_DIV:
            if (!hasNewLine)
               text += "\n";
            hasNewLine = true;
            break;
          case ID_P:
          case ID_TR:
          case ID_H1:
          case ID_H2:
          case ID_H3:
          case ID_H4:
          case ID_H5:
          case ID_H6:
            if (!hasNewLine)
               text += "\n";
            text += "\n";
            hasNewLine = true;
            break;
        }
      }
      if(n == d->m_selectionEnd) break;
      DOM::Node next = n.firstChild();
      if(next.isNull()) next = n.nextSibling();
      while( next.isNull() && !n.parentNode().isNull() ) {
        n = n.parentNode();
        next = n.nextSibling();
        unsigned short id = n.elementId();
        switch(id) {
          case ID_TD:
          case ID_TH:
          case ID_HR:
          case ID_OL:
          case ID_UL:
          case ID_LI:
          case ID_DD:
          case ID_DL:
          case ID_DT:
          case ID_PRE:
          case ID_BLOCKQUOTE:
          case ID_DIV:
            if (!hasNewLine)
               text += "\n";
            hasNewLine = true;
            break;
          case ID_P:
          case ID_TR:
          case ID_H1:
          case ID_H2:
          case ID_H3:
          case ID_H4:
          case ID_H5:
          case ID_H6:
            if (!hasNewLine)
               text += "\n";
            text += "\n";
            hasNewLine = true;
            break;
        }
      }

      n = next;
    }

    if(text.isEmpty())
        return QString::null;

    int start = 0;
    int end = text.length();

    // Strip leading LFs
    while ((start < end) && (text[start] == '\n'))
       start++;

    // Strip excessive trailing LFs
    while ((start < (end-1)) && (text[end-1] == '\n') && (text[end-2] == '\n'))
       end--;

    return text.mid(start, end-start);
}

bool KHTMLPart::hasSelection() const
{
  if ( d->m_selectionStart.isNull() || d->m_selectionEnd.isNull() )
      return false;
  if ( d->m_selectionStart == d->m_selectionEnd &&
       d->m_startOffset == d->m_endOffset )
      return false; // empty
  return true;
}

DOM::Range KHTMLPart::selection() const
{
    DOM::Range r = document().createRange();DOM::Range();
    r.setStart( d->m_selectionStart, d->m_startOffset );
    r.setEnd( d->m_selectionEnd, d->m_endOffset );
    return r;
}

void KHTMLPart::selection(DOM::Node &s, long &so, DOM::Node &e, long &eo) const
{
    s = d->m_selectionStart;
    so = d->m_startOffset;
    e = d->m_selectionEnd;
    eo = d->m_endOffset;
}

void KHTMLPart::setSelection( const DOM::Range &r )
{
    d->m_selectionStart = r.startContainer();
    d->m_startOffset = r.startOffset();
    d->m_selectionEnd = r.endContainer();
    d->m_endOffset = r.endOffset();
    d->m_doc->setSelection(d->m_selectionStart.handle(),d->m_startOffset,
                           d->m_selectionEnd.handle(),d->m_endOffset);
#ifndef KHTML_NO_CARET
    bool v = d->m_view->placeCaret();
    emitCaretPositionChanged(v ? d->caretNode() : 0, d->caretOffset());
#endif
}

void KHTMLPart::slotClearSelection()
{
    bool hadSelection = hasSelection();
#ifndef KHTML_NO_CARET
    //kdDebug(6000) << "d->m_selectionStart " << d->m_selectionStart.handle()
    //		<< " d->m_selectionEnd " << d->m_selectionEnd.handle() << endl;
    // nothing, leave selection parameters as is
#else
    d->m_selectionStart = 0;
    d->m_startOffset = 0;
    d->m_selectionEnd = 0;
    d->m_endOffset = 0;
#endif
    if ( d->m_doc ) d->m_doc->clearSelection();
    if ( hadSelection )
      emitSelectionChanged();
#ifndef KHTML_NO_CARET
    bool v = d->m_view->placeCaret();
    emitCaretPositionChanged(v ? d->caretNode() : 0, d->caretOffset());
#endif
}

void KHTMLPart::overURL( const QString &url, const QString &target, bool /*shiftPressed*/ )
{
  KURL u = completeURL(url);

  // special case for <a href="">
  if ( url.isEmpty() )
    u.setFileName( url );

  emit onURL( url );

  if ( url.isEmpty() ) {
    setStatusBarText(u.htmlURL(), BarHoverText);
    return;
  }

  if (url.find( QString::fromLatin1( "javascript:" ),0, false ) == 0 ) {
    QString jscode = KURL::decode_string( url.mid( url.find( "javascript:", 0, false ) ) );
    jscode = KStringHandler::rsqueeze( jscode, 80 ); // truncate if too long
    setStatusBarText( QStyleSheet::escape( jscode ), BarHoverText );
    return;
  }

  KFileItem item(u, QString::null, KFileItem::Unknown);
  emit d->m_extension->mouseOverInfo(&item);

  QString com;

  KMimeType::Ptr typ = KMimeType::findByURL( u );

  if ( typ )
    com = typ->comment( u, false );

  if ( !u.isValid() ) {
    setStatusBarText(u.htmlURL(), BarHoverText);
    return;
  }

  if ( u.isLocalFile() )
  {
    // TODO : use KIO::stat() and create a KFileItem out of its result,
    // to use KFileItem::statusBarText()
    QCString path = QFile::encodeName( u.path() );

    struct stat buff;
    bool ok = !stat( path.data(), &buff );

    struct stat lbuff;
    if (ok) ok = !lstat( path.data(), &lbuff );

    QString text = u.htmlURL();
    QString text2 = text;

    if (ok && S_ISLNK( lbuff.st_mode ) )
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
        setStatusBarText(text2, BarHoverText);
        return;
      }
      buff_two[n] = 0;

      text += buff_two;
      text += "  ";
      text += tmp;
    }
    else if ( ok && S_ISREG( buff.st_mode ) )
    {
      if (buff.st_size < 1024)
        text = i18n("%2 (%1 bytes)").arg((long) buff.st_size).arg(text2); // always put the URL last, in case it contains '%'
      else
      {
        float d = (float) buff.st_size/1024.0;
        text = i18n("%2 (%1 K)").arg(KGlobal::locale()->formatNumber(d, 2)).arg(text2); // was %.2f
      }
      text += "  ";
      text += com;
    }
    else if ( ok && S_ISDIR( buff.st_mode ) )
    {
      text += "  ";
      text += com;
    }
    else
    {
      text += "  ";
      text += com;
    }
    setStatusBarText(text, BarHoverText);
  }
  else
  {
    QString extra;
    if (target.lower() == "_blank")
    {
      extra = i18n(" (In new window)");
    }
    else if (!target.isEmpty() &&
             (target.lower() != "_top") &&
             (target.lower() != "_self") &&
             (target.lower() != "_parent"))
    {
      extra = i18n(" (In other frame)");
    }

    if (u.protocol() == QString::fromLatin1("mailto")) {
      QString mailtoMsg /* = QString::fromLatin1("<img src=%1>").arg(locate("icon", QString::fromLatin1("locolor/16x16/actions/mail_send.png")))*/;
      mailtoMsg += i18n("Email to: ") + KURL::decode_string(u.path());
      QStringList queries = QStringList::split('&', u.query().mid(1));
      for (QStringList::Iterator it = queries.begin(); it != queries.end(); ++it)
        if ((*it).startsWith(QString::fromLatin1("subject=")))
          mailtoMsg += i18n(" - Subject: ") + KURL::decode_string((*it).mid(8));
        else if ((*it).startsWith(QString::fromLatin1("cc=")))
          mailtoMsg += i18n(" - CC: ") + KURL::decode_string((*it).mid(3));
        else if ((*it).startsWith(QString::fromLatin1("bcc=")))
          mailtoMsg += i18n(" - BCC: ") + KURL::decode_string((*it).mid(4));
      mailtoMsg.replace(QString::fromLatin1("&"), QString("&amp;"));
      mailtoMsg.replace(QString::fromLatin1("<"), QString("&lt;"));
      mailtoMsg.replace(QString::fromLatin1(">"), QString("&gt;"));
      mailtoMsg.replace(QRegExp("([\n\r\t]|[ ]{10})"), QString::null);
      setStatusBarText("<qt>"+mailtoMsg, BarHoverText);
      return;
    }
   // Is this check necessary at all? (Frerich)
#if 0
    else if (u.protocol() == QString::fromLatin1("http")) {
        DOM::Node hrefNode = nodeUnderMouse().parentNode();
        while (hrefNode.nodeName().string() != QString::fromLatin1("A") && !hrefNode.isNull())
          hrefNode = hrefNode.parentNode();

        if (!hrefNode.isNull()) {
          DOM::Node hreflangNode = hrefNode.attributes().getNamedItem("HREFLANG");
          if (!hreflangNode.isNull()) {
            QString countryCode = hreflangNode.nodeValue().string().lower();
            // Map the language code to an appropriate country code.
            if (countryCode == QString::fromLatin1("en"))
              countryCode = QString::fromLatin1("gb");
            QString flagImg = QString::fromLatin1("<img src=%1>").arg(
                locate("locale", QString::fromLatin1("l10n/")
                + countryCode
                + QString::fromLatin1("/flag.png")));
            emit setStatusBarText(flagImg + u.prettyURL() + extra);
          }
        }
      }
#endif
    setStatusBarText(u.htmlURL() + extra, BarHoverText);
  }
}

//
// This executes in the active part on a click or other url selection action in
// that active part.
//
void KHTMLPart::urlSelected( const QString &url, int button, int state, const QString &_target, KParts::URLArgs args )
{
  kdDebug() << k_funcinfo << url << endl;
  bool hasTarget = false;

  QString target = _target;
  if ( target.isEmpty() && d->m_doc )
    target = d->m_doc->baseTarget();
  if ( !target.isEmpty() )
      hasTarget = true;

  if ( url.find( QString::fromLatin1( "javascript:" ), 0, false ) == 0 )
  {
    crossFrameExecuteScript( target, KURL::decode_string( url.mid( 11 ) ) );
    return;
  }

  KURL cURL = completeURL(url);
  // special case for <a href="">  (IE removes filename, mozilla doesn't)
  if ( url.isEmpty() )
    cURL.setFileName( url ); // removes filename

  if ( !cURL.isValid() )
    // ### ERROR HANDLING
    return;

  kdDebug( 6000 ) << "urlSelected: complete URL:" << cURL.url() << " target = " << target << endl;

  if ( button == LeftButton && ( state & ShiftButton ) )
  {
    KIO::MetaData metaData;
    metaData["referrer"] = d->m_referrer;
    KHTMLPopupGUIClient::saveURL( d->m_view, i18n( "Save As" ), cURL, metaData );
    return;
  }

  if (!checkLinkSecurity(cURL,
			 i18n( "<qt>The link <B>%1</B><BR>leads from this untrusted page to your local filesystem.<BR>Do you want to follow the link?" ),
			 i18n( "Follow" )))
    return;

  args.frameName = target;

  args.metaData().insert("main_frame_request",
                         parentPart() == 0 ? "TRUE":"FALSE");
  args.metaData().insert("ssl_parent_ip", d->m_ssl_parent_ip);
  args.metaData().insert("ssl_parent_cert", d->m_ssl_parent_cert);
  args.metaData().insert("PropagateHttpHeader", "true");
  args.metaData().insert("ssl_was_in_use", d->m_ssl_in_use ? "TRUE":"FALSE");
  args.metaData().insert("ssl_activate_warnings", "TRUE");
  // WABA: When we select the link explicitly we should treat this new URL as the
  // toplevel url and it should never be considered cross-domain.
  // However this function is also called for javascript and META-tag based
  // redirections:
  //   - In such case, we don't take cross-domain-ness in consideration if we are the
  //   toplevel frame because the new URL may be in a different domain as the current URL
  //   but that's ok.
  //   - If we are not the toplevel frame then we check against the toplevelURL()
  if (args.redirectedRequest() && parentPart())
      args.metaData().insert("cross-domain", toplevelURL().url());

  if ( hasTarget )
  {
    // unknown frame names should open in a new window.
    khtml::ChildFrame *frame = recursiveFrameRequest( cURL, args, false );
    if ( frame )
    {
      args.metaData()["referrer"] = d->m_referrer;
      requestObject( frame, cURL, args );
      return;
    }
  }

  if ( !d->m_bComplete && !hasTarget )
    closeURL();

  if (!d->m_referrer.isEmpty() && !args.metaData().contains("referrer"))
    args.metaData()["referrer"] = d->m_referrer;

  if ( button == 0 && (state & ShiftButton) && (state & ControlButton) )
  {
    emit d->m_extension->createNewWindow( cURL, args );
    return;
  }

  if ( state & ShiftButton)
  {
    KParts::WindowArgs winArgs;
    winArgs.lowerWindow = true;
    KParts::ReadOnlyPart *newPart = 0;
    emit d->m_extension->createNewWindow( cURL, args, winArgs, newPart );
    return;
  }

  if ( state & ControlButton )
  {
    args.setNewTab(true);
    emit d->m_extension->createNewWindow( cURL, args );
    return;
  }

  view()->viewport()->unsetCursor();
  emit d->m_extension->openURLRequest( cURL, args );
}

void KHTMLPart::slotViewDocumentSource()
{
  KURL url(m_url);
  bool isTempFile = false;
  if (!(url.isLocalFile()) && KHTMLPageCache::self()->isComplete(d->m_cacheId))
  {
     KTempFile sourceFile(QString::null, QString::fromLatin1(".html"));
     if (sourceFile.status() == 0)
     {
        KHTMLPageCache::self()->saveData(d->m_cacheId, sourceFile.dataStream());
        url = KURL();
        url.setPath(sourceFile.name());
        isTempFile = true;
     }
  }

  (void) KRun::runURL( url, QString::fromLatin1("text/plain"), isTempFile );
}

void KHTMLPart::slotViewPageInfo()
{
  KHTMLInfoDlg *dlg = new KHTMLInfoDlg(NULL, "KHTML Page Info Dialog", false, WDestructiveClose);

  if (d->m_doc)
     dlg->_title->setText(d->m_doc->title().string());

  // If it's a frame, set the caption to "Frame Information"
  if ( parentPart() && d->m_doc && d->m_doc->isHTMLDocument() ) {
     dlg->setCaption(i18n("Frame Information"));
  }

  QString editStr = QString::null;

  if (!d->m_pageServices.isEmpty())
    editStr = i18n("   <a href=\"%1\">[Properties]</a>").arg(d->m_pageServices);

  QString squeezedURL = KStringHandler::csqueeze( url().prettyURL(), 80 );
  dlg->_url->setText("<a href=\"" + url().url() + "\">" + squeezedURL + "</a>" + editStr);
  if (lastModified().isEmpty())
  {
    dlg->_lastModified->hide();
    dlg->_lmLabel->hide();
  }
  else
    dlg->_lastModified->setText(lastModified());

  /* populate the list view now */
  QStringList headers = QStringList::split("\n", d->m_httpHeaders);

  for (QStringList::Iterator it = headers.begin(); it != headers.end(); ++it) {
    QStringList header = QStringList::split(QRegExp(":[ ]+"), *it);
    if (header.count() != 2)
       continue;
    new QListViewItem(dlg->_headers, header[0], header[1]);
  }

  dlg->show();
  /* put no code here */
}


void KHTMLPart::slotViewFrameSource()
{
  KParts::ReadOnlyPart *frame = currentFrame();
  if ( !frame )
    return;

  KURL url = frame->url();
  bool isTempFile = false;
  if (!(url.isLocalFile()) && frame->inherits("KHTMLPart"))
  {
       long cacheId = static_cast<KHTMLPart *>(frame)->d->m_cacheId;

       if (KHTMLPageCache::self()->isComplete(cacheId))
       {
           KTempFile sourceFile(QString::null, QString::fromLatin1(".html"));
           if (sourceFile.status() == 0)
           {
               KHTMLPageCache::self()->saveData(cacheId, sourceFile.dataStream());
               url = KURL();
               url.setPath(sourceFile.name());
               isTempFile = true;
           }
     }
  }

  (void) KRun::runURL( url, QString::fromLatin1("text/plain"), isTempFile );
}

KURL KHTMLPart::backgroundURL() const
{
  // ### what about XML documents? get from CSS?
  if (!d->m_doc || !d->m_doc->isHTMLDocument())
    return KURL();

  QString relURL = static_cast<HTMLDocumentImpl*>(d->m_doc)->body()->getAttribute( ATTR_BACKGROUND ).string();

  return KURL( m_url, relURL );
}

void KHTMLPart::slotSaveBackground()
{
  KIO::MetaData metaData;
  metaData["referrer"] = d->m_referrer;
  KHTMLPopupGUIClient::saveURL( d->m_view, i18n("Save Background Image As"), backgroundURL(), metaData );
}

void KHTMLPart::slotSaveDocument()
{
  KURL srcURL( m_url );

  if ( srcURL.fileName(false).isEmpty() )
    srcURL.setFileName( "index.html" );

  KIO::MetaData metaData;
  // Referre unknown?
  KHTMLPopupGUIClient::saveURL( d->m_view, i18n( "Save As" ), srcURL, metaData, "text/html", d->m_cacheId );
}

void KHTMLPart::slotSecurity()
{
//   kdDebug( 6050 ) << "Meta Data:" << endl
//                   << d->m_ssl_peer_cert_subject
//                   << endl
//                   << d->m_ssl_peer_cert_issuer
//                   << endl
//                   << d->m_ssl_cipher
//                   << endl
//                   << d->m_ssl_cipher_desc
//                   << endl
//                   << d->m_ssl_cipher_version
//                   << endl
//                   << d->m_ssl_good_from
//                   << endl
//                   << d->m_ssl_good_until
//                   << endl
//                   << d->m_ssl_cert_state
//                   << endl;

  KSSLInfoDlg *kid = new KSSLInfoDlg(d->m_ssl_in_use, widget(), "kssl_info_dlg", true );

  if (d->m_bSecurityInQuestion)
	  kid->setSecurityInQuestion(true);

  if (d->m_ssl_in_use) {
    KSSLCertificate *x = KSSLCertificate::fromString(d->m_ssl_peer_certificate.local8Bit());
    if (x) {
       // Set the chain back onto the certificate
       QStringList cl = QStringList::split(QString("\n"), d->m_ssl_peer_chain);
       QPtrList<KSSLCertificate> ncl;

       ncl.setAutoDelete(true);
       for (QStringList::Iterator it = cl.begin(); it != cl.end(); ++it) {
          KSSLCertificate *y = KSSLCertificate::fromString((*it).local8Bit());
          if (y) ncl.append(y);
       }

       if (ncl.count() > 0)
          x->chain().setChain(ncl);

       kid->setup(x,
                  d->m_ssl_peer_ip,
                  m_url.url(),
                  d->m_ssl_cipher,
                  d->m_ssl_cipher_desc,
                  d->m_ssl_cipher_version,
                  d->m_ssl_cipher_used_bits.toInt(),
                  d->m_ssl_cipher_bits.toInt(),
                  (KSSLCertificate::KSSLValidation) d->m_ssl_cert_state.toInt()
                  );
        kid->exec();
        delete x;
     } else kid->exec();
  } else kid->exec();
}

void KHTMLPart::slotSaveFrame()
{
    if ( !d->m_activeFrame )
        return; // should never be the case, but one never knows :-)

    KURL srcURL( static_cast<KParts::ReadOnlyPart *>( d->m_activeFrame )->url() );

    if ( srcURL.fileName(false).isEmpty() )
        srcURL.setFileName( "index.html" );

    KIO::MetaData metaData;
    // Referrer unknown?
    KHTMLPopupGUIClient::saveURL( d->m_view, i18n( "Save As" ), srcURL, metaData, "text/html" );
}

void KHTMLPart::slotSetEncoding()
{
  d->m_automaticDetection->setItemChecked( int( d->m_autoDetectLanguage ), false );
  d->m_paSetEncoding->popupMenu()->setItemChecked( 0, false );
  d->m_paSetEncoding->popupMenu()->setItemChecked( d->m_paSetEncoding->popupMenu()->idAt( 2 ), true );

  QString enc = KGlobal::charsets()->encodingForName( d->m_manualDetection->currentText() );
  setEncoding( enc, true );
}

void KHTMLPart::slotUseStylesheet()
{
  if (d->m_doc)
  {
    bool autoselect = (d->m_paUseStylesheet->currentItem() == 0);
    d->m_sheetUsed = autoselect ? QString() : d->m_paUseStylesheet->currentText();
    d->m_doc->updateStyleSelector();
  }
}

void KHTMLPart::updateActions()
{
  bool frames = false;

  QValueList<khtml::ChildFrame>::ConstIterator it = d->m_frames.begin();
  QValueList<khtml::ChildFrame>::ConstIterator end = d->m_frames.end();
  for (; it != end; ++it )
      if ( (*it).m_type == khtml::ChildFrame::Frame )
      {
          frames = true;
          break;
      }

  d->m_paViewFrame->setEnabled( frames );
  d->m_paSaveFrame->setEnabled( frames );

  if ( frames )
    d->m_paFind->setText( i18n( "&Find in Frame..." ) );
  else
    d->m_paFind->setText( i18n( "&Find..." ) );

  KParts::Part *frame = 0;

  if ( frames )
    frame = currentFrame();

  bool enableFindAndSelectAll = true;

  if ( frame )
    enableFindAndSelectAll = frame->inherits( "KHTMLPart" );

  d->m_paFind->setEnabled( enableFindAndSelectAll );
  d->m_paSelectAll->setEnabled( enableFindAndSelectAll );

  bool enablePrintFrame = false;

  if ( frame )
  {
    QObject *ext = KParts::BrowserExtension::childObject( frame );
    if ( ext )
      enablePrintFrame = ext->metaObject()->slotNames().contains( "print()" );
  }

  d->m_paPrintFrame->setEnabled( enablePrintFrame );

  QString bgURL;

  // ### frames
  if ( d->m_doc && d->m_doc->isHTMLDocument() && static_cast<HTMLDocumentImpl*>(d->m_doc)->body() && !d->m_bClearing )
    bgURL = static_cast<HTMLDocumentImpl*>(d->m_doc)->body()->getAttribute( ATTR_BACKGROUND ).string();

  d->m_paSaveBackground->setEnabled( !bgURL.isEmpty() );

  if ( d->m_paDebugScript )
    d->m_paDebugScript->setEnabled( d->m_jscript );
}

KParts::LiveConnectExtension *KHTMLPart::liveConnectExtension( const khtml::RenderPart *frame) const {
    QValueList<khtml::ChildFrame>::ConstIterator it = d->m_objects.begin();
    QValueList<khtml::ChildFrame>::ConstIterator end = d->m_objects.end();
    for(; it != end; ++it )
        if ((*it).m_frame == frame)
            return (*it).m_liveconnect;
    return 0L;
}

bool KHTMLPart::requestFrame( khtml::RenderPart *frame, const QString &url, const QString &frameName,
                              const QStringList &params, bool isIFrame )
{
  //kdDebug( 6050 ) << this << " requestFrame( ..., " << url << ", " << frameName << " )" << endl;
  FrameIt it = d->m_frames.find( frameName );
  if ( it == d->m_frames.end() )
  {
    khtml::ChildFrame child;
    //kdDebug( 6050 ) << "inserting new frame into frame map " << frameName << endl;
    child.m_name = frameName;
    it = d->m_frames.append( child );
  }

  (*it).m_type = isIFrame ? khtml::ChildFrame::IFrame : khtml::ChildFrame::Frame;
  (*it).m_frame = frame;
  (*it).m_params = params;

  // Support for <frame src="javascript:string">
  if ( url.find( QString::fromLatin1( "javascript:" ), 0, false ) == 0 )
  {
      QVariant res = executeScript( DOM::Node(frame->element()), KURL::decode_string( url.right( url.length() - 11) ) );
      KURL myurl;
      myurl.setProtocol("javascript");
      if ( res.type() == QVariant::String )
	myurl.setPath(res.asString());
      return processObjectRequest(&(*it), myurl, QString("text/html") );
  }
  KURL u = url.isEmpty() ? KURL() : completeURL( url );
  return requestObject( &(*it), u );
}

QString KHTMLPart::requestFrameName()
{
   return QString::fromLatin1("<!--frame %1-->").arg(d->m_frameNameId++);
}

bool KHTMLPart::requestObject( khtml::RenderPart *frame, const QString &url, const QString &serviceType,
                               const QStringList &params )
{
    kdDebug( 6005 ) << "KHTMLPart::requestObject " << this << " frame=" << frame << endl;
  khtml::ChildFrame child;
  QValueList<khtml::ChildFrame>::Iterator it = d->m_objects.append( child );
  (*it).m_frame = frame;
  (*it).m_type = khtml::ChildFrame::Object;
  (*it).m_params = params;

  KParts::URLArgs args;
  args.serviceType = serviceType;
  if (!requestObject( &(*it), completeURL( url ), args ) && !(*it).m_run) {
      (*it).m_bCompleted = true;
      return false;
  }
  return true;
}

bool KHTMLPart::requestObject( khtml::ChildFrame *child, const KURL &url, const KParts::URLArgs &_args )
{
  if (!checkLinkSecurity(url))
  {
    kdDebug(6005) << this << " KHTMLPart::requestObject checkLinkSecurity refused" << endl;
    return false;
  }
  if ( child->m_bPreloaded )
  {
    kdDebug(6005) << "KHTMLPart::requestObject preload" << endl;
    if ( child->m_frame && child->m_part )
      child->m_frame->setWidget( child->m_part->widget() );

    child->m_bPreloaded = false;
    return true;
  }

  KParts::URLArgs args( _args );

  if ( child->m_run )
    child->m_run->abort();

  if ( child->m_part && !args.reload && urlcmp( child->m_part->url().url(), url.url(), true, true ) )
    args.serviceType = child->m_serviceType;

  child->m_args = args;
  child->m_args.reload = (d->m_cachePolicy == KIO::CC_Reload) || (d->m_cachePolicy == KIO::CC_Refresh);
  child->m_serviceName = QString::null;
  if (!d->m_referrer.isEmpty() && !child->m_args.metaData().contains( "referrer" ))
    child->m_args.metaData()["referrer"] = d->m_referrer;

  child->m_args.metaData().insert("PropagateHttpHeader", "true");
  child->m_args.metaData().insert("ssl_parent_ip", d->m_ssl_parent_ip);
  child->m_args.metaData().insert("ssl_parent_cert", d->m_ssl_parent_cert);
  child->m_args.metaData().insert("main_frame_request",
                                  parentPart() == 0 ? "TRUE":"FALSE");
  child->m_args.metaData().insert("ssl_was_in_use",
                                  d->m_ssl_in_use ? "TRUE":"FALSE");
  child->m_args.metaData().insert("ssl_activate_warnings", "TRUE");
  child->m_args.metaData().insert("cross-domain", toplevelURL().url());

  // We want a KHTMLPart if the HTML says <frame src=""> or <frame src="about:blank">
  if ((url.isEmpty() || url.url() == "about:blank") && args.serviceType.isEmpty())
    args.serviceType = QString::fromLatin1( "text/html" );

  if ( args.serviceType.isEmpty() ) {
    kdDebug(6050) << "Running new KHTMLRun for " << this << " and child=" << child << endl;
    child->m_run = new KHTMLRun( this, child, url, child->m_args, true );
    d->m_bComplete = false; // ensures we stop it in checkCompleted...
    return false;
  } else {
    return processObjectRequest( child, url, args.serviceType );
  }
}

bool KHTMLPart::processObjectRequest( khtml::ChildFrame *child, const KURL &_url, const QString &mimetype )
{
  //kdDebug( 6050 ) << "KHTMLPart::processObjectRequest trying to create part for " << mimetype << endl;

  // IMPORTANT: create a copy of the url here, because it is just a reference, which was likely to be given
  // by an emitting frame part (emit openURLRequest( blahurl, ... ) . A few lines below we delete the part
  // though -> the reference becomes invalid -> crash is likely
  KURL url( _url );

  // khtmlrun called us this way to indicate a loading error
  if ( d->m_onlyLocalReferences || ( url.isEmpty() && mimetype.isEmpty() ) )
  {
      child->m_bCompleted = true;
      checkCompleted();
      return true;
  }

  if (child->m_bNotify)
  {
      child->m_bNotify = false;
      if ( !child->m_args.lockHistory() )
          emit d->m_extension->openURLNotify();
  }

  if ( child->m_serviceType != mimetype )
  {
    // Before attempting to load a part, check if the user wants that.
    // Many don't like getting ZIP files embedded.
    // However we don't want to ask for flash and other plugin things..
    if ( child->m_type != khtml::ChildFrame::Object )
    {
      KParts::BrowserRun::AskSaveResult res = KParts::BrowserRun::askEmbedOrSave(
        url, mimetype/*, suggestedFilename */ );
      switch( res ) {
      case KParts::BrowserRun::Save:
        KHTMLPopupGUIClient::saveURL( widget(), i18n( "Save As" ), url, child->m_args.metaData(), QString::null, 0 /*, suggestedFilename */ );
        // fall-through
      case KParts::BrowserRun::Cancel:
        child->m_bCompleted = true;
        checkCompleted();
        return true; // done
      default: // Open
        break;
      }
    }

    QStringList dummy; // the list of servicetypes handled by the part is now unused.
    KParts::ReadOnlyPart *part = createPart( d->m_view->viewport(), child->m_name.ascii(), this, child->m_name.ascii(), mimetype, child->m_serviceName, dummy, child->m_params );

    if ( !part )
    {
        if ( child->m_frame )
          if (child->m_frame->partLoadingErrorNotify( child, url, mimetype ))
            return true; // we succeeded after all (a fallback was used)

        checkEmitLoadEvent();
        return false;
    }

    //CRITICAL STUFF
    if ( child->m_part )
    {
      partManager()->removePart( (KParts::ReadOnlyPart *)child->m_part );
      delete (KParts::ReadOnlyPart *)child->m_part;
    }

    child->m_serviceType = mimetype;
    if ( child->m_frame )
      child->m_frame->setWidget( part->widget() );

    if ( child->m_type != khtml::ChildFrame::Object )
      partManager()->addPart( part, false );
//  else
//      kdDebug(6005) << "AH! NO FRAME!!!!!" << endl;

    child->m_part = part;
    assert( ((void*) child->m_part) != 0);

    connect( part, SIGNAL( started( KIO::Job *) ),
             this, SLOT( slotChildStarted( KIO::Job *) ) );
    connect( part, SIGNAL( completed() ),
             this, SLOT( slotChildCompleted() ) );
    if ( child->m_type != khtml::ChildFrame::Object )
    {
      connect( part, SIGNAL( completed(bool) ),
               this, SLOT( slotChildCompleted(bool) ) );
      connect( part, SIGNAL( setStatusBarText( const QString & ) ),
               this, SIGNAL( setStatusBarText( const QString & ) ) );
      if ( part->inherits( "KHTMLPart" ) )
      {
          connect( this, SIGNAL( completed() ),
                   part, SLOT( slotParentCompleted() ) );
          connect( this, SIGNAL( completed(bool) ),
                   part, SLOT( slotParentCompleted() ) );
          // As soon as the child's document is created, we need to set its domain
          // (but we do so only once, so it can't be simply done in the child)
          connect( part, SIGNAL( docCreated() ),
                   this, SLOT( slotChildDocCreated() ) );
      }
    }

    child->m_extension = KParts::BrowserExtension::childObject( part );

    if ( child->m_extension )
    {
      connect( child->m_extension, SIGNAL( openURLNotify() ),
               d->m_extension, SIGNAL( openURLNotify() ) );

      connect( child->m_extension, SIGNAL( openURLRequestDelayed( const KURL &, const KParts::URLArgs & ) ),
               this, SLOT( slotChildURLRequest( const KURL &, const KParts::URLArgs & ) ) );

      connect( child->m_extension, SIGNAL( createNewWindow( const KURL &, const KParts::URLArgs & ) ),
               d->m_extension, SIGNAL( createNewWindow( const KURL &, const KParts::URLArgs & ) ) );
      connect( child->m_extension, SIGNAL( createNewWindow( const KURL &, const KParts::URLArgs &, const KParts::WindowArgs &, KParts::ReadOnlyPart *& ) ),
               d->m_extension, SIGNAL( createNewWindow( const KURL &, const KParts::URLArgs & , const KParts::WindowArgs &, KParts::ReadOnlyPart *&) ) );

      connect( child->m_extension, SIGNAL( popupMenu( const QPoint &, const KFileItemList & ) ),
               d->m_extension, SIGNAL( popupMenu( const QPoint &, const KFileItemList & ) ) );
      connect( child->m_extension, SIGNAL( popupMenu( KXMLGUIClient *, const QPoint &, const KFileItemList & ) ),
               d->m_extension, SIGNAL( popupMenu( KXMLGUIClient *, const QPoint &, const KFileItemList & ) ) );
      connect( child->m_extension, SIGNAL( popupMenu( KXMLGUIClient *, const QPoint &, const KFileItemList &, const KParts::URLArgs &, KParts::BrowserExtension::PopupFlags ) ),
               d->m_extension, SIGNAL( popupMenu( KXMLGUIClient *, const QPoint &, const KFileItemList &, const KParts::URLArgs &, KParts::BrowserExtension::PopupFlags ) ) );
      connect( child->m_extension, SIGNAL( popupMenu( const QPoint &, const KURL &, const QString &, mode_t ) ),
               d->m_extension, SIGNAL( popupMenu( const QPoint &, const KURL &, const QString &, mode_t ) ) );
      connect( child->m_extension, SIGNAL( popupMenu( KXMLGUIClient *, const QPoint &, const KURL &, const QString &, mode_t ) ),
               d->m_extension, SIGNAL( popupMenu( KXMLGUIClient *, const QPoint &, const KURL &, const QString &, mode_t ) ) );
      connect( child->m_extension, SIGNAL( popupMenu( KXMLGUIClient *, const QPoint &, const KURL &, const KParts::URLArgs &, KParts::BrowserExtension::PopupFlags, mode_t ) ),
               d->m_extension, SIGNAL( popupMenu( KXMLGUIClient *, const QPoint &, const KURL &, const KParts::URLArgs &, KParts::BrowserExtension::PopupFlags, mode_t ) ) );

      connect( child->m_extension, SIGNAL( infoMessage( const QString & ) ),
               d->m_extension, SIGNAL( infoMessage( const QString & ) ) );

      child->m_extension->setBrowserInterface( d->m_extension->browserInterface() );
    }
    child->m_liveconnect = KParts::LiveConnectExtension::childObject( part );
  }

  checkEmitLoadEvent();
  // Some JS code in the load event may have destroyed the part
  // In that case, abort
  if ( !child->m_part )
    return false;

  if ( child->m_bPreloaded )
  {
    if ( child->m_frame && child->m_part )
      child->m_frame->setWidget( child->m_part->widget() );

    child->m_bPreloaded = false;
    return true;
  }

  child->m_args.reload = (d->m_cachePolicy == KIO::CC_Reload) || (d->m_cachePolicy == KIO::CC_Refresh);

  // make sure the part has a way to find out about the mimetype.
  // we actually set it in child->m_args in requestObject already,
  // but it's useless if we had to use a KHTMLRun instance, as the
  // point the run object is to find out exactly the mimetype.
  child->m_args.serviceType = mimetype;

  // if not a frame set child as completed
  child->m_bCompleted = child->m_type == khtml::ChildFrame::Object;

  if ( child->m_extension )
    child->m_extension->setURLArgs( child->m_args );

  if(url.protocol() == "javascript" || url.url() == "about:blank") {
      if (!child->m_part->inherits("KHTMLPart"))
          return false;

      KHTMLPart* p = static_cast<KHTMLPart*>(static_cast<KParts::ReadOnlyPart *>(child->m_part));

      p->begin();
      if (d->m_doc && p->d->m_doc)
        p->d->m_doc->setBaseURL(d->m_doc->baseURL());
      if (!url.url().startsWith("about:")) {
        p->write(url.path());
      } else {
	p->m_url = url;
        // we need a body element. testcase: <iframe id="a"></iframe><script>alert(a.document.body);</script>
        p->write("<HTML><BODY></BODY></HTML>");
      }
      p->end();
      return true;
  }
  else if ( !url.isEmpty() )
  {
      //kdDebug( 6050 ) << "opening " << url.url() << " in frame " << child->m_part << endl;
      bool b = child->m_part->openURL( url );
      if (child->m_bCompleted)
          checkCompleted();
      return b;
  }
  else
  {
      child->m_bCompleted = true;
      checkCompleted();
      return true;
  }
}

KParts::ReadOnlyPart *KHTMLPart::createPart( QWidget *parentWidget, const char *widgetName,
                                             QObject *parent, const char *name, const QString &mimetype,
                                             QString &serviceName, QStringList &serviceTypes,
                                             const QStringList &params )
{
  QString constr;
  if ( !serviceName.isEmpty() )
    constr.append( QString::fromLatin1( "Name == '%1'" ).arg( serviceName ) );

  KTrader::OfferList offers = KTrader::self()->query( mimetype, "KParts/ReadOnlyPart", constr, QString::null );

  if ( offers.isEmpty() )
    return 0L;

  KTrader::OfferList::Iterator it = offers.begin();
  for (  ; it != offers.end() ; ++it )
  {
    KService::Ptr service = (*it);

    KLibFactory *factory = KLibLoader::self()->factory( QFile::encodeName(service->library()) );
    if ( factory ) {
      KParts::ReadOnlyPart *res = 0L;

      const char *className = "KParts::ReadOnlyPart";
      if ( service->serviceTypes().contains( "Browser/View" ) )
        className = "Browser/View";

      if ( factory->inherits( "KParts::Factory" ) )
        res = static_cast<KParts::ReadOnlyPart *>(static_cast<KParts::Factory *>( factory )->createPart( parentWidget, widgetName, parent, name, className, params ));
      else
        res = static_cast<KParts::ReadOnlyPart *>(factory->create( parentWidget, widgetName, className ));

      if ( res ) {
        serviceTypes = service->serviceTypes();
        serviceName = service->name();
        return res;
      }
    } else {
      // TODO KMessageBox::error and i18n, like in KonqFactory::createView?
      kdWarning() << QString("There was an error loading the module %1.\nThe diagnostics is:\n%2")
                      .arg(service->name()).arg(KLibLoader::self()->lastErrorMessage()) << endl;
    }
  }
  return 0;
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

void KHTMLPart::submitFormAgain()
{
  if( d->m_doc && !d->m_doc->parsing() && d->m_submitForm)
    KHTMLPart::submitForm( d->m_submitForm->submitAction, d->m_submitForm->submitUrl, d->m_submitForm->submitFormData, d->m_submitForm->target, d->m_submitForm->submitContentType, d->m_submitForm->submitBoundary );

  delete d->m_submitForm;
  d->m_submitForm = 0;
  disconnect(this, SIGNAL(completed()), this, SLOT(submitFormAgain()));
}

void KHTMLPart::submitFormProxy( const char *action, const QString &url, const QByteArray &formData, const QString &_target, const QString& contentType, const QString& boundary )
{
  submitForm(action, url, formData, _target, contentType, boundary);
}

void KHTMLPart::submitForm( const char *action, const QString &url, const QByteArray &formData, const QString &_target, const QString& contentType, const QString& boundary )
{
  kdDebug(6000) << this << ": KHTMLPart::submitForm target=" << _target << " url=" << url << endl;
  if (d->m_formNotification == KHTMLPart::Only) {
    emit formSubmitNotification(action, url, formData, _target, contentType, boundary);
    return;
  } else if (d->m_formNotification == KHTMLPart::Before) {
    emit formSubmitNotification(action, url, formData, _target, contentType, boundary);
  }

  KURL u = completeURL( url );

  if ( !u.isValid() )
  {
    // ### ERROR HANDLING!
    return;
  }

  // Form security checks
  //
  /*
   * If these form security checks are still in this place in a month or two
   * I'm going to simply delete them.
   */

  /* This is separate for a reason.  It has to be _before_ all script, etc,
   * AND I don't want to break anything that uses checkLinkSecurity() in
   * other places.
   */

  if (!d->m_submitForm) {
    if (u.protocol() != "https" && u.protocol() != "mailto") {
      if (d->m_ssl_in_use) {    // Going from SSL -> nonSSL
        int rc = KMessageBox::warningContinueCancel(NULL, i18n("Warning:  This is a secure form but it is attempting to send your data back unencrypted."
                                                               "\nA third party may be able to intercept and view this information."
                                                               "\nAre you sure you wish to continue?"),
                                                    i18n("SSL"));
        if (rc == KMessageBox::Cancel)
          return;
      } else {                  // Going from nonSSL -> nonSSL
        KSSLSettings kss(true);
        if (kss.warnOnUnencrypted()) {
          int rc = KMessageBox::warningContinueCancel(NULL,
                                                      i18n("Warning: Your data is about to be transmitted across the network unencrypted."
                                                           "\nAre you sure you wish to continue?"),
                                                      i18n("KDE"),
                                                      QString::null,
                                                      "WarnOnUnencryptedForm");
          // Move this setting into KSSL instead
          KConfig *config = kapp->config();
          QString grpNotifMsgs = QString::fromLatin1("Notification Messages");
          KConfigGroupSaver saver( config, grpNotifMsgs );

          if (!config->readBoolEntry("WarnOnUnencryptedForm", true)) {
            config->deleteEntry("WarnOnUnencryptedForm");
            config->sync();
            kss.setWarnOnUnencrypted(false);
            kss.save();
          }
          if (rc == KMessageBox::Cancel)
            return;
      	}
      }
    }

    if (u.protocol() == "mailto") {
      int rc = KMessageBox::warningContinueCancel(NULL,
                                                  i18n("This site is attempting to submit form data via email.\n"
                                                       "Do you want to continue?"),
                                                  i18n("KDE"),
                                                  QString::null,
                                                  "WarnTriedEmailSubmit");

      if (rc == KMessageBox::Cancel) {
        return;
      }
    }
  }

  // End form security checks
  //

  QString urlstring = u.url();

  if ( urlstring.find( QString::fromLatin1( "javascript:" ), 0, false ) == 0 ) {
    urlstring = KURL::decode_string(urlstring);
    crossFrameExecuteScript( _target, urlstring.right( urlstring.length() - 11) );
    return;
  }

  if (!checkLinkSecurity(u,
			 i18n( "<qt>The form will be submitted to <BR><B>%1</B><BR>on your local filesystem.<BR>Do you want to submit the form?" ),
			 i18n( "Submit" )))
    return;

  KParts::URLArgs args;

  if (!d->m_referrer.isEmpty())
     args.metaData()["referrer"] = d->m_referrer;

  args.metaData().insert("PropagateHttpHeader", "true");
  args.metaData().insert("ssl_parent_ip", d->m_ssl_parent_ip);
  args.metaData().insert("ssl_parent_cert", d->m_ssl_parent_cert);
  args.metaData().insert("main_frame_request",
                         parentPart() == 0 ? "TRUE":"FALSE");
  args.metaData().insert("ssl_was_in_use", d->m_ssl_in_use ? "TRUE":"FALSE");
  args.metaData().insert("ssl_activate_warnings", "TRUE");
//WABA: When we post a form we should treat it as the main url
//the request should never be considered cross-domain
//args.metaData().insert("cross-domain", toplevelURL().url());
  args.frameName = _target.isEmpty() ? d->m_doc->baseTarget() : _target ;

  // Handle mailto: forms
  if (u.protocol() == "mailto") {
      // 1)  Check for attach= and strip it
      QString q = u.query().mid(1);
      QStringList nvps = QStringList::split("&", q);
      bool triedToAttach = false;

      for (QStringList::Iterator nvp = nvps.begin(); nvp != nvps.end(); ++nvp) {
         QStringList pair = QStringList::split("=", *nvp);
         if (pair.count() >= 2) {
            if (pair.first().lower() == "attach") {
               nvp = nvps.remove(nvp);
               triedToAttach = true;
            }
         }
      }

      if (triedToAttach)
         KMessageBox::information(NULL, i18n("This site attempted to attach a file from your computer in the form submission. The attachment was removed for your protection."), i18n("KDE"), "WarnTriedAttach");

      // 2)  Append body=
      QString bodyEnc;
      if (contentType.lower() == "multipart/form-data") {
         // FIXME: is this correct?  I suspect not
         bodyEnc = KURL::encode_string(QString::fromLatin1(formData.data(),
                                                           formData.size()));
      } else if (contentType.lower() == "text/plain") {
         // Convention seems to be to decode, and s/&/\n/
         QString tmpbody = QString::fromLatin1(formData.data(),
                                               formData.size());
         tmpbody.replace(QRegExp("[&]"), "\n");
         tmpbody.replace(QRegExp("[+]"), " ");
         tmpbody = KURL::decode_string(tmpbody);  // Decode the rest of it
         bodyEnc = KURL::encode_string(tmpbody);  // Recode for the URL
      } else {
         bodyEnc = KURL::encode_string(QString::fromLatin1(formData.data(),
                                                           formData.size()));
      }

      nvps.append(QString("body=%1").arg(bodyEnc));
      q = nvps.join("&");
      u.setQuery(q);
  }

  if ( strcmp( action, "get" ) == 0 ) {
    if (u.protocol() != "mailto")
       u.setQuery( QString::fromLatin1( formData.data(), formData.size() ) );
    args.setDoPost( false );
  }
  else {
    args.postData = formData;
    args.setDoPost( true );

    // construct some user headers if necessary
    if (contentType.isNull() || contentType == "application/x-www-form-urlencoded")
      args.setContentType( "Content-Type: application/x-www-form-urlencoded" );
    else // contentType must be "multipart/form-data"
      args.setContentType( "Content-Type: " + contentType + "; boundary=" + boundary );
  }

  if ( d->m_doc->parsing() || d->m_runningScripts > 0 ) {
    if( d->m_submitForm ) {
      kdDebug(6000) << "KHTMLPart::submitForm ABORTING!" << endl;
      return;
    }
    d->m_submitForm = new KHTMLPartPrivate::SubmitForm;
    d->m_submitForm->submitAction = action;
    d->m_submitForm->submitUrl = url;
    d->m_submitForm->submitFormData = formData;
    d->m_submitForm->target = _target;
    d->m_submitForm->submitContentType = contentType;
    d->m_submitForm->submitBoundary = boundary;
    connect(this, SIGNAL(completed()), this, SLOT(submitFormAgain()));
  }
  else
  {
    emit d->m_extension->openURLRequest( u, args );
  }
}

void KHTMLPart::popupMenu( const QString &linkUrl )
{
  KURL popupURL;
  KURL linkKURL;
  QString referrer;
  KParts::BrowserExtension::PopupFlags itemflags=KParts::BrowserExtension::ShowBookmark | KParts::BrowserExtension::ShowReload;

  if ( linkUrl.isEmpty() ) { // click on background
    KHTMLPart* khtmlPart = this;
    while ( khtmlPart->parentPart() )
    {
      khtmlPart=khtmlPart->parentPart();
    }
    popupURL = khtmlPart->url();
    referrer = khtmlPart->pageReferrer();
    itemflags |= KParts::BrowserExtension::ShowNavigationItems;
  } else {               // click on link
    popupURL = completeURL( linkUrl );
    linkKURL = popupURL;
    referrer = this->referrer();
  }

  // Danger, Will Robinson. The Popup might stay around for a much
  // longer time than KHTMLPart. Deal with it.
  KHTMLPopupGUIClient* client = new KHTMLPopupGUIClient( this, d->m_popupMenuXML, linkKURL );
  QGuardedPtr<QObject> guard( client );

  KParts::URLArgs args;
  args.serviceType = QString::fromLatin1( "text/html" );
  args.metaData()["referrer"] = referrer;

  emit d->m_extension->popupMenu( client, QCursor::pos(), popupURL, args, itemflags, S_IFREG /*always a file*/);

  if ( !guard.isNull() ) {
     delete client;
     emit popupMenu(linkUrl, QCursor::pos());
     d->m_strSelectedURL = d->m_strSelectedURLTarget = QString::null;
  }
}

void KHTMLPart::slotParentCompleted()
{
  if ( !d->m_redirectURL.isEmpty() && !d->m_redirectionTimer.isActive() )
  {
    // kdDebug(6050) << this << ": Child redirection -> " << d->m_redirectURL << endl;
    d->m_redirectionTimer.start( 1000 * d->m_delayRedirect, true );
  }
}

void KHTMLPart::slotChildStarted( KIO::Job *job )
{
  khtml::ChildFrame *child = frame( sender() );

  assert( child );

  child->m_bCompleted = false;

  if ( d->m_bComplete )
  {
#if 0
    // WABA: Looks like this belongs somewhere else
    if ( !parentPart() ) // "toplevel" html document? if yes, then notify the hosting browser about the document (url) changes
    {
      emit d->m_extension->openURLNotify();
    }
#endif
    d->m_bComplete = false;
    emit started( job );
  }
}

void KHTMLPart::slotChildCompleted()
{
  slotChildCompleted( false );
}

void KHTMLPart::slotChildCompleted( bool pendingAction )
{
  khtml::ChildFrame *child = frame( sender() );

  if ( child ) {
    kdDebug(6050) << this << " slotChildCompleted child=" << child << " m_frame=" << child->m_frame << endl;
    child->m_bCompleted = true;
    child->m_bPendingRedirection = pendingAction;
    child->m_args = KParts::URLArgs();
  }
  checkCompleted();
}

void KHTMLPart::slotChildDocCreated()
{
  const KHTMLPart* htmlFrame = static_cast<const KHTMLPart *>(sender());
  // Set domain to the frameset's domain
  // This must only be done when loading the frameset initially (#22039),
  // not when following a link in a frame (#44162).
  if ( d->m_doc && d->m_doc->isHTMLDocument() )
  {
    if ( sender()->inherits("KHTMLPart") )
    {
      DOMString domain = static_cast<HTMLDocumentImpl*>(d->m_doc)->domain();
      if (htmlFrame->d->m_doc && htmlFrame->d->m_doc->isHTMLDocument() )
        //kdDebug(6050) << "KHTMLPart::slotChildDocCreated: url: " << htmlFrame->m_url.url() << endl;
        static_cast<HTMLDocumentImpl*>(htmlFrame->d->m_doc)->setDomain( domain );
    }
  }
  // So it only happens once
  disconnect( htmlFrame, SIGNAL( docCreated() ), this, SLOT( slotChildDocCreated() ) );
}

void KHTMLPart::slotChildURLRequest( const KURL &url, const KParts::URLArgs &args )
{
  khtml::ChildFrame *child = frame( sender()->parent() );

  // TODO: handle child target correctly! currently the script are always executed fur the parent
  QString urlStr = url.url();
  if ( urlStr.find( QString::fromLatin1( "javascript:" ), 0, false ) == 0 ) {
      QString script = KURL::decode_string( urlStr.right( urlStr.length() - 11 ) );
      executeScript( DOM::Node(), script );
      return;
  }

  QString frameName = args.frameName.lower();
  if ( !frameName.isEmpty() ) {
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

  if ( child && child->m_type != khtml::ChildFrame::Object ) {
      // Inform someone that we are about to show something else.
      child->m_bNotify = true;
      requestObject( child, url, args );
  }  else if ( frameName== "_self" ) // this is for embedded objects (via <object>) which want to replace the current document
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
      if ( (KParts::ReadOnlyPart *)(*it).m_part == part )
        return &(*it);

    for (it = d->m_objects.begin(); it != d->m_objects.end(); ++it )
      if ( (KParts::ReadOnlyPart *)(*it).m_part == part )
        return &(*it);

    return 0L;
}

//#define DEBUG_FINDFRAME

KHTMLPart *KHTMLPart::findFrame( const QString &f )
{
#ifdef DEBUG_FINDFRAME
  kdDebug(6050) << "KHTMLPart::findFrame '" << f << "'" << endl;
  FrameIt it2 = d->m_frames.begin();
  FrameIt end = d->m_frames.end();
  for (; it2 != end; ++it2 )
      kdDebug(6050) << "  - having frame '" << (*it2).m_name << "'" << endl;
#endif
  // ### http://www.w3.org/TR/html4/appendix/notes.html#notes-frames
  ConstFrameIt it = d->m_frames.find( f );
  if ( it == d->m_frames.end() )
  {
#ifdef DEBUG_FINDFRAME
    kdDebug(6050) << "KHTMLPart::findFrame frame " << f << " not found" << endl;
#endif
    return 0L;
  }
  else {
    KParts::ReadOnlyPart *p = (*it).m_part;
    if ( p && p->inherits( "KHTMLPart" ))
    {
#ifdef DEBUG_FINDFRAME
      kdDebug(6050) << "KHTMLPart::findFrame frame " << f << " is a KHTMLPart, ok" << endl;
#endif
      return (KHTMLPart*)p;
    }
    else
    {
#ifdef DEBUG_FINDFRAME
      if (p)
        kdWarning() << "KHTMLPart::findFrame frame " << f << " found but isn't a KHTMLPart ! " << p->className() << endl;
      else
        kdWarning() << "KHTMLPart::findFrame frame " << f << " found but m_part=0L" << endl;
#endif
      return 0L;
    }
  }
}

KParts::ReadOnlyPart *KHTMLPart::currentFrame() const
{
  KParts::ReadOnlyPart* part = (KParts::ReadOnlyPart*)(this);
  // Find active part in our frame manager, in case we are a frameset
  // and keep doing that (in case of nested framesets).
  // Just realized we could also do this recursively, calling part->currentFrame()...
  while ( part && part->inherits("KHTMLPart") &&
          static_cast<KHTMLPart *>(part)->d->m_frames.count() > 0 ) {
    KHTMLPart* frameset = static_cast<KHTMLPart *>(part);
    part = static_cast<KParts::ReadOnlyPart *>(frameset->partManager()->activePart());
    if ( !part ) return frameset;
  }
  return part;
}

bool KHTMLPart::frameExists( const QString &frameName )
{
  ConstFrameIt it = d->m_frames.find( frameName );
  if ( it == d->m_frames.end() )
    return false;

  // WABA: We only return true if the child actually has a frame
  // set. Otherwise we might find our preloaded-selve.
  // This happens when we restore the frameset.
  return (!(*it).m_frame.isNull());
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
    return &(*it);

  it = d->m_frames.begin();
  FrameIt end = d->m_frames.end();
  for (; it != end; ++it )
    if ( (*it).m_part && (*it).m_part->inherits( "KHTMLPart" ) )
    {
      KHTMLPart *childPart = (KHTMLPart *)(KParts::ReadOnlyPart *)(*it).m_part;

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

  stream << m_url << (Q_INT32)d->m_view->contentsX() << (Q_INT32)d->m_view->contentsY()
         << (Q_INT32) d->m_view->contentsWidth() << (Q_INT32) d->m_view->contentsHeight() << (Q_INT32) d->m_view->marginWidth() << (Q_INT32) d->m_view->marginHeight();

  // save link cursor position
  int focusNodeNumber;
  if (!d->m_focusNodeRestored)
      focusNodeNumber = d->m_focusNodeNumber;
  else if (d->m_doc && d->m_doc->focusNode())
      focusNodeNumber = d->m_doc->nodeAbsIndex(d->m_doc->focusNode());
  else
      focusNodeNumber = -1;
  stream << focusNodeNumber;

  // Save the doc's cache id.
  stream << d->m_cacheId;

  // Save the state of the document (Most notably the state of any forms)
  QStringList docState;
  if (d->m_doc)
  {
     docState = d->m_doc->docState();
  }
  stream << d->m_encoding << d->m_sheetUsed << docState;

  stream << d->m_zoomFactor;

  stream << d->m_httpHeaders;
  stream << d->m_pageServices;
  stream << d->m_pageReferrer;

  // Save ssl data
  stream << d->m_ssl_in_use
         << d->m_ssl_peer_certificate
         << d->m_ssl_peer_chain
         << d->m_ssl_peer_ip
         << d->m_ssl_cipher
         << d->m_ssl_cipher_desc
         << d->m_ssl_cipher_version
         << d->m_ssl_cipher_used_bits
         << d->m_ssl_cipher_bits
         << d->m_ssl_cert_state
         << d->m_ssl_parent_ip
         << d->m_ssl_parent_cert;


  QStringList frameNameLst, frameServiceTypeLst, frameServiceNameLst;
  KURL::List frameURLLst;
  QValueList<QByteArray> frameStateBufferLst;

  ConstFrameIt it = d->m_frames.begin();
  ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it )
  {
    if ( !(*it).m_part )
       continue;

    frameNameLst << (*it).m_name;
    frameServiceTypeLst << (*it).m_serviceType;
    frameServiceNameLst << (*it).m_serviceName;
    frameURLLst << (*it).m_part->url();

    QByteArray state;
    QDataStream frameStream( state, IO_WriteOnly );

    if ( (*it).m_extension )
      (*it).m_extension->saveState( frameStream );

    frameStateBufferLst << state;
  }

  // Save frame data
  stream << (Q_UINT32) frameNameLst.count();
  stream << frameNameLst << frameServiceTypeLst << frameServiceNameLst << frameURLLst << frameStateBufferLst;
}

void KHTMLPart::restoreState( QDataStream &stream )
{
  KURL u;
  Q_INT32 xOffset, yOffset, wContents, hContents, mWidth, mHeight;
  Q_UINT32 frameCount;
  QStringList frameNames, frameServiceTypes, docState, frameServiceNames;
  KURL::List frameURLs;
  QValueList<QByteArray> frameStateBuffers;
  QValueList<int> fSizes;
  QString encoding, sheetUsed;
  long old_cacheId = d->m_cacheId;

  stream >> u >> xOffset >> yOffset >> wContents >> hContents >> mWidth >> mHeight;

  d->m_view->setMarginWidth( mWidth );
  d->m_view->setMarginHeight( mHeight );

  // restore link cursor position
  // nth node is active. value is set in checkCompleted()
  stream >> d->m_focusNodeNumber;
  d->m_focusNodeRestored = false;

  stream >> d->m_cacheId;

  stream >> encoding >> sheetUsed >> docState;

  d->m_encoding = encoding;
  d->m_sheetUsed = sheetUsed;

  int zoomFactor;
  stream >> zoomFactor;
  setZoomFactor(zoomFactor);

  stream >> d->m_httpHeaders;
  stream >> d->m_pageServices;
  stream >> d->m_pageReferrer;

  // Restore ssl data
  stream >> d->m_ssl_in_use
         >> d->m_ssl_peer_certificate
         >> d->m_ssl_peer_chain
         >> d->m_ssl_peer_ip
         >> d->m_ssl_cipher
         >> d->m_ssl_cipher_desc
         >> d->m_ssl_cipher_version
         >> d->m_ssl_cipher_used_bits
         >> d->m_ssl_cipher_bits
         >> d->m_ssl_cert_state
         >> d->m_ssl_parent_ip
         >> d->m_ssl_parent_cert;

  setPageSecurity( d->m_ssl_in_use ? Encrypted : NotCrypted );

  stream >> frameCount >> frameNames >> frameServiceTypes >> frameServiceNames
         >> frameURLs >> frameStateBuffers;

  d->m_bComplete = false;
  d->m_bLoadEventEmitted = false;

//   kdDebug( 6050 ) << "restoreStakte() docState.count() = " << docState.count() << endl;
//   kdDebug( 6050 ) << "m_url " << m_url.url() << " <-> " << u.url() << endl;
//   kdDebug( 6050 ) << "m_frames.count() " << d->m_frames.count() << " <-> " << frameCount << endl;

  if (d->m_cacheId == old_cacheId)
  {
    // Partial restore
    d->m_redirectionTimer.stop();

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

//      kdDebug( 6050 ) <<  *fNameIt  << " ---- " <<  *fServiceTypeIt << endl;

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
        if ( child->m_extension && !(*fBufferIt).isEmpty() )
        {
          QDataStream frameStream( *fBufferIt, IO_ReadOnly );
          child->m_extension->restoreState( frameStream );
        }
        else
          child->m_part->openURL( *fURLIt );
      }
    }

    KParts::URLArgs args( d->m_extension->urlArgs() );
    args.xOffset = xOffset;
    args.yOffset = yOffset;
    args.docState = docState;
    d->m_extension->setURLArgs( args );

    d->m_view->resizeContents( wContents,  hContents);
    d->m_view->setContentsPos( xOffset, yOffset );

    m_url = u;
  }
  else
  {
    // Full restore.
    closeURL();
    // We must force a clear because we want to be sure to delete all
    // frames.
    d->m_bCleared = false;
    clear();
    d->m_encoding = encoding;
    d->m_sheetUsed = sheetUsed;

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

//      kdDebug( 6050 ) << *fNameIt << " ---- " << *fServiceTypeIt << endl;

      FrameIt childFrame = d->m_frames.append( newChild );

      processObjectRequest( &(*childFrame), *fURLIt, *fServiceTypeIt );

      (*childFrame).m_bPreloaded = true;

      if ( (*childFrame).m_part )
      {
        if ( (*childFrame).m_extension )
        if ( (*childFrame).m_extension && !(*fBufferIt).isEmpty() )
        {
          QDataStream frameStream( *fBufferIt, IO_ReadOnly );
          (*childFrame).m_extension->restoreState( frameStream );
        }
        else
          (*childFrame).m_part->openURL( *fURLIt );
      }
    }

    KParts::URLArgs args( d->m_extension->urlArgs() );
    args.xOffset = xOffset;
    args.yOffset = yOffset;
    args.docState = docState;

    d->m_view->resizeContents( wContents,  hContents);
    d->m_view->setContentsPos( xOffset, yOffset );

    d->m_extension->setURLArgs( args );
    if (!KHTMLPageCache::self()->isComplete(d->m_cacheId))
    {
       d->m_restored = true;
       openURL( u );
       d->m_restored = false;
    }
    else
    {
       restoreURL( u );
    }
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
  if ( d->m_findDialog )
       d->m_findDialog->setHasSelection( hasSelection() );

  emit d->m_extension->selectionInfo( selectedText() );
  emit selectionChanged();
}

int KHTMLPart::zoomFactor() const
{
  return d->m_zoomFactor;
}

// ### make the list configurable ?
static const int zoomSizes[] = { 20, 40, 60, 80, 90, 95, 100, 105, 110, 120, 140, 160, 180, 200, 250, 300 };
static const int zoomSizeCount = (sizeof(zoomSizes) / sizeof(int));
static const int minZoom = 20;
static const int maxZoom = 300;

void KHTMLPart::slotIncZoom()
{
  int zoomFactor = d->m_zoomFactor;

  if (zoomFactor < maxZoom) {
    // find the entry nearest to the given zoomsizes
    for (int i = 0; i < zoomSizeCount; ++i)
      if (zoomSizes[i] > zoomFactor) {
        zoomFactor = zoomSizes[i];
        break;
      }
    setZoomFactor(zoomFactor);
  }
}

void KHTMLPart::slotDecZoom()
{
    int zoomFactor = d->m_zoomFactor;
    if (zoomFactor > minZoom) {
      // find the entry nearest to the given zoomsizes
      for (int i = zoomSizeCount-1; i >= 0; --i)
        if (zoomSizes[i] < zoomFactor) {
          zoomFactor = zoomSizes[i];
          break;
        }
      setZoomFactor(zoomFactor);
    }
}

void KHTMLPart::setZoomFactor (int percent)
{
  if (percent < minZoom) percent = minZoom;
  if (percent > maxZoom) percent = maxZoom;
  if (d->m_zoomFactor == percent) return;
  d->m_zoomFactor = percent;

  if(d->m_doc) {
      QApplication::setOverrideCursor( waitCursor );
    if (d->m_doc->styleSelector())
      d->m_doc->styleSelector()->computeFontSizes(d->m_doc->paintDeviceMetrics(), d->m_zoomFactor);
    d->m_doc->recalcStyle( NodeImpl::Force );
    QApplication::restoreOverrideCursor();
  }

  ConstFrameIt it = d->m_frames.begin();
  ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it )
    if ( !( *it ).m_part.isNull() && ( *it ).m_part->inherits( "KHTMLPart" ) ) {
      KParts::ReadOnlyPart* p = ( *it ).m_part;
      static_cast<KHTMLPart*>( p )->setZoomFactor(d->m_zoomFactor);
    }

  if ( d->m_guiProfile == BrowserViewGUI ) {
      d->m_paDecZoomFactor->setEnabled( d->m_zoomFactor > minZoom );
      d->m_paIncZoomFactor->setEnabled( d->m_zoomFactor < maxZoom );
  }
}

void KHTMLPart::slotZoomView( int delta )
{
  if ( delta < 0 )
    slotIncZoom();
  else
    slotDecZoom();
}

void KHTMLPart::setStatusBarText( const QString& text, StatusBarPriority p)
{
  if (!d->m_statusMessagesEnabled)
    return;

  d->m_statusBarText[p] = text;

  // shift handling ?
  QString tobe = d->m_statusBarText[BarHoverText];
  if (tobe.isEmpty())
    tobe = d->m_statusBarText[BarOverrideText];
  if (tobe.isEmpty()) {
    tobe = d->m_statusBarText[BarDefaultText];
    if (!tobe.isEmpty() && d->m_jobspeed)
      tobe += " ";
    if (d->m_jobspeed)
      tobe += i18n( "(%1/s)" ).arg( KIO::convertSize( d->m_jobspeed ) );
  }
  tobe = "<qt>"+tobe;

  emit ReadOnlyPart::setStatusBarText(tobe);
}


void KHTMLPart::setJSStatusBarText( const QString &text )
{
  setStatusBarText(text, BarOverrideText);
}

void KHTMLPart::setJSDefaultStatusBarText( const QString &text )
{
  setStatusBarText(text, BarDefaultText);
}

QString KHTMLPart::jsStatusBarText() const
{
    return d->m_statusBarText[BarOverrideText];
}

QString KHTMLPart::jsDefaultStatusBarText() const
{
   return d->m_statusBarText[BarDefaultText];
}

QString KHTMLPart::referrer() const
{
   return d->m_referrer;
}

QString KHTMLPart::pageReferrer() const
{
   KURL referrerURL = KURL( d->m_pageReferrer );
   if (referrerURL.isValid())
   {
      QString protocol = referrerURL.protocol();

      if ((protocol == "http") ||
         ((protocol == "https") && (m_url.protocol() == "https")))
      {
          referrerURL.setRef(QString::null);
          referrerURL.setUser(QString::null);
          referrerURL.setPass(QString::null);
          return referrerURL.url();
      }
   }

   return QString::null;
}


QString KHTMLPart::lastModified() const
{
  if ( d->m_lastModified.isEmpty() && m_url.isLocalFile() ) {
    // Local file: set last-modified from the file's mtime.
    // Done on demand to save time when this isn't needed - but can lead
    // to slightly wrong results if updating the file on disk w/o reloading.
    QDateTime lastModif = QFileInfo( m_url.path() ).lastModified();
    d->m_lastModified = lastModif.toString( Qt::LocalDate );
  }
  //kdDebug(6050) << "KHTMLPart::lastModified: " << d->m_lastModified << endl;
  return d->m_lastModified;
}

void KHTMLPart::slotLoadImages()
{
  if (d->m_doc )
    d->m_doc->docLoader()->setAutoloadImages( !d->m_doc->docLoader()->autoloadImages() );

  ConstFrameIt it = d->m_frames.begin();
  ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it )
    if ( !( *it ).m_part.isNull() && ( *it ).m_part->inherits( "KHTMLPart" ) ) {
      KParts::ReadOnlyPart* p = ( *it ).m_part;
      static_cast<KHTMLPart*>( p )->slotLoadImages();
    }
}

void KHTMLPart::reparseConfiguration()
{
  KHTMLSettings *settings = KHTMLFactory::defaultHTMLSettings();
  settings->init();

  setAutoloadImages( settings->autoLoadImages() );
  if (d->m_doc)
     d->m_doc->docLoader()->setShowAnimations( settings->showAnimations() );

  d->m_bBackRightClick = settings->isBackRightClickEnabled();
  d->m_bJScriptEnabled = settings->isJavaScriptEnabled(m_url.host());
  setDebugScript( settings->isJavaScriptDebugEnabled() );
  d->m_bJavaEnabled = settings->isJavaEnabled(m_url.host());
  d->m_bPluginsEnabled = settings->isPluginsEnabled(m_url.host());
  d->m_metaRefreshEnabled = settings->isAutoDelayedActionsEnabled ();

  delete d->m_settings;
  d->m_settings = new KHTMLSettings(*KHTMLFactory::defaultHTMLSettings());

  QApplication::setOverrideCursor( waitCursor );
  khtml::CSSStyleSelector::reparseConfiguration();
  if(d->m_doc) d->m_doc->updateStyleSelector();
  QApplication::restoreOverrideCursor();
}

QStringList KHTMLPart::frameNames() const
{
  QStringList res;

  ConstFrameIt it = d->m_frames.begin();
  ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it )
    if (!(*it).m_bPreloaded)
      res += (*it).m_name;

  return res;
}

QPtrList<KParts::ReadOnlyPart> KHTMLPart::frames() const
{
  QPtrList<KParts::ReadOnlyPart> res;

  ConstFrameIt it = d->m_frames.begin();
  ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it )
    if (!(*it).m_bPreloaded)
      res.append( (*it).m_part );

  return res;
}

bool KHTMLPart::openURLInFrame( const KURL &url, const KParts::URLArgs &urlArgs )
{
    kdDebug( 6050 ) << this << "KHTMLPart::openURLInFrame " << url << endl;
  FrameIt it = d->m_frames.find( urlArgs.frameName );

  if ( it == d->m_frames.end() )
    return false;

  // Inform someone that we are about to show something else.
  if ( !urlArgs.lockHistory() )
      emit d->m_extension->openURLNotify();

  requestObject( &(*it), url, urlArgs );

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

void KHTMLPart::customEvent( QCustomEvent *event )
{
  if ( khtml::MousePressEvent::test( event ) )
  {
    khtmlMousePressEvent( static_cast<khtml::MousePressEvent *>( event ) );
    return;
  }

  if ( khtml::MouseDoubleClickEvent::test( event ) )
  {
    khtmlMouseDoubleClickEvent( static_cast<khtml::MouseDoubleClickEvent *>( event ) );
    return;
  }

  if ( khtml::MouseMoveEvent::test( event ) )
  {
    khtmlMouseMoveEvent( static_cast<khtml::MouseMoveEvent *>( event ) );
    return;
  }

  if ( khtml::MouseReleaseEvent::test( event ) )
  {
    khtmlMouseReleaseEvent( static_cast<khtml::MouseReleaseEvent *>( event ) );
    return;
  }

  if ( khtml::DrawContentsEvent::test( event ) )
  {
    khtmlDrawContentsEvent( static_cast<khtml::DrawContentsEvent *>( event ) );
    return;
  }

  KParts::ReadOnlyPart::customEvent( event );
}

void KHTMLPart::khtmlMousePressEvent( khtml::MousePressEvent *event )
{
  DOM::DOMString url = event->url();
  QMouseEvent *_mouse = event->qmouseEvent();
  DOM::Node innerNode = event->innerNode();
  d->m_mousePressNode = innerNode;

   d->m_dragStartPos = _mouse->pos();

   if ( !event->url().isNull() ) {
     d->m_strSelectedURL = event->url().string();
     d->m_strSelectedURLTarget = event->target().string();
   }
   else
     d->m_strSelectedURL = d->m_strSelectedURLTarget = QString::null;

  if ( _mouse->button() == LeftButton ||
       _mouse->button() == MidButton )
  {
    d->m_bMousePressed = true;

#ifndef KHTML_NO_SELECTION
    if ( _mouse->button() == LeftButton )
    {
      if ( (!d->m_strSelectedURL.isNull() && !isEditable())
	        || (!d->m_mousePressNode.isNull() && d->m_mousePressNode.elementId() == ID_IMG) )
	  return;
      if ( !innerNode.isNull()  && innerNode.handle()->renderer()) {
          int offset = 0;
          DOM::NodeImpl* node = 0;
          khtml::RenderObject::SelPointState state;
          innerNode.handle()->renderer()->checkSelectionPoint( event->x(), event->y(),
                                                               event->absX()-innerNode.handle()->renderer()->xPos(),
                                                               event->absY()-innerNode.handle()->renderer()->yPos(), node, offset, state );
#ifdef KHTML_NO_CARET
          d->m_selectionStart = node;
          d->m_startOffset = offset;
          //if ( node )
          //  kdDebug(6005) << "KHTMLPart::khtmlMousePressEvent selectionStart=" << d->m_selectionStart.handle()->renderer()
          //                << " offset=" << d->m_startOffset << endl;
          //else
          //  kdDebug(6005) << "KHTML::khtmlMousePressEvent selectionStart=(nil)" << endl;
          d->m_selectionEnd = d->m_selectionStart;
          d->m_endOffset = d->m_startOffset;
          d->m_doc->clearSelection();
#else // KHTML_NO_CARET
	  d->m_view->moveCaretTo(node, offset, (_mouse->state() & ShiftButton) == 0);
#endif // KHTML_NO_CARET
      }
      else
      {
#ifndef KHTML_NO_CARET
        // simply leave it. Is this a good idea?
#else
        d->m_selectionStart = DOM::Node();
        d->m_selectionEnd = DOM::Node();
#endif
      }
      emitSelectionChanged();
      startAutoScroll();
    }
#else
    d->m_dragLastPos = _mouse->globalPos();
#endif
  }

  if ( _mouse->button() == RightButton && parentPart() != 0 && d->m_bBackRightClick )
  {
    d->m_bRightMousePressed = true;
  } else if ( _mouse->button() == RightButton )
  {
    popupMenu( d->m_strSelectedURL );
    // might be deleted, don't touch "this"
  }
}

void KHTMLPart::khtmlMouseDoubleClickEvent( khtml::MouseDoubleClickEvent *event )
{
  QMouseEvent *_mouse = event->qmouseEvent();
  if ( _mouse->button() == LeftButton )
  {
    d->m_bMousePressed = true;
    DOM::Node innerNode = event->innerNode();
    // Find selectionStart again, khtmlMouseReleaseEvent lost it
    if ( !innerNode.isNull() && innerNode.handle()->renderer()) {
      int offset = 0;
      DOM::NodeImpl* node = 0;
      khtml::RenderObject::SelPointState state;
      innerNode.handle()->renderer()->checkSelectionPoint( event->x(), event->y(),
                                                           event->absX()-innerNode.handle()->renderer()->xPos(),
                                                           event->absY()-innerNode.handle()->renderer()->yPos(), node, offset, state);

      //kdDebug() << k_funcinfo << "checkSelectionPoint returned node=" << node << " offset=" << offset << endl;

      if ( node && node->renderer() )
      {
        // Extend selection to a complete word (double-click) or paragraph (triple-click)
        bool selectParagraph = (event->clickCount() == 3);

        // Extend to the left
        extendSelection( node, offset, d->m_selectionStart, d->m_startOffset, false, selectParagraph );
        // Extend to the right
        extendSelection( node, offset, d->m_selectionEnd, d->m_endOffset, true, selectParagraph );

        d->m_endOffset++; // the last char must be in
        //kdDebug() << d->m_selectionStart.handle() << " " << d->m_startOffset << "  -  " <<
        //  d->m_selectionEnd.handle() << " " << d->m_endOffset << endl;

        d->m_startBeforeEnd = true;
        emitSelectionChanged();
        d->m_doc
          ->setSelection(d->m_selectionStart.handle(),d->m_startOffset,
                         d->m_selectionEnd.handle(),d->m_endOffset);
#ifndef KHTML_NO_CARET
        bool v = d->m_view->placeCaret();
        emitCaretPositionChanged(v ? d->caretNode() : 0, d->caretOffset());
#endif
      }
    }
  }
}

void KHTMLPart::extendSelection( DOM::NodeImpl* node, long offset, DOM::Node& selectionNode, long& selectionOffset, bool right, bool selectParagraph )
{
  khtml::RenderObject* obj = node->renderer();
  QString str;
  int len = 0;
  if ( obj->isText() ) { // can be false e.g. when double-clicking on a disabled submit button
    str = static_cast<khtml::RenderText *>(obj)->data().string();
    len = str.length();
  }
  //kdDebug() << "extendSelection right=" << right << " offset=" << offset << " len=" << len << " Starting at obj=" << obj << endl;
  QChar ch;
  do {
    // Last char was ok, point to it
    if ( node ) {
      selectionNode = node;
      selectionOffset = offset;
    }

    // Get another char
    while ( obj && ( (right && offset >= len-1) || (!right && offset <= 0) ) )
    {
      obj = right ? obj->objectBelow() : obj->objectAbove();
      //kdDebug() << "obj=" << obj << endl;
      if ( obj ) {
        //kdDebug() << "isText=" << obj->isText() << endl;
        str = QString::null;
        if ( obj->isText() )
          str = static_cast<khtml::RenderText *>(obj)->data().string();
        else if ( obj->isBR() )
          str = '\n';
        else if ( !obj->isInline() ) {
          obj = 0L; // parag limit -> done
          break;
        }
        len = str.length();
        //kdDebug() << "str=" << str << " length=" << len << endl;
        // set offset - note that the first thing will be a ++ or -- on it.
        if ( right )
          offset = -1;
        else
          offset = len;
      }
    }
    if ( !obj ) // end of parag or document
      break;
    node = obj->element();
    if ( right )
    {
      Q_ASSERT( offset < len-1 );
      offset++;
    }
    else
    {
      Q_ASSERT( offset > 0 );
      offset--;
    }

    // Test that char
    ch = str[ offset ];
    //kdDebug() << " offset=" << offset << " ch=" << QString(ch) << endl;
  } while ( selectParagraph || (!ch.isSpace() && !ch.isPunct()) );
}

#ifndef KHTML_NO_SELECTION
void KHTMLPart::extendSelectionTo(int x, int y, int absX, int absY, const DOM::Node &innerNode)
{
      int offset;
      //kdDebug(6000) << "KHTMLPart::khtmlMouseMoveEvent x=" << event->x() << " y=" << event->y() << endl;
      DOM::NodeImpl* node=0;
      khtml::RenderObject::SelPointState state;
      innerNode.handle()->renderer()->checkSelectionPoint( x, y,
                                                           absX-innerNode.handle()->renderer()->xPos(),
                                                           absY-innerNode.handle()->renderer()->yPos(), node, offset, state);
      if (!node) return;

      d->m_selectionEnd = node;
      d->m_endOffset = offset;
      //kdDebug( 6000 ) << "setting end of selection to " << d->m_selectionEnd.handle() << "/" << d->m_endOffset << endl;

      // we have to get to know if end is before start or not...
#if 0
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
#else
      // shouldn't be null but it can happen with dynamic updating of nodes
      if (d->m_selectionStart.isNull() || d->m_selectionEnd.isNull() ||
          !d->m_selectionStart.handle()->renderer() ||
          !d->m_selectionEnd.handle()->renderer()) return;
      d->m_startBeforeEnd = RangeImpl::compareBoundaryPoints(
      			d->m_selectionStart.handle(), d->m_startOffset,
			d->m_selectionEnd.handle(), d->m_endOffset) <= 0;
#endif

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
#ifndef KHTML_NO_CARET
      d->m_view->placeCaret();
#endif
}

bool KHTMLPart::isExtendingSelection() const
{
  // This is it, the whole detection. khtmlMousePressEvent only sets this
  // on LMB or MMB, but never on RMB. As text selection doesn't work for MMB,
  // it's sufficient to only rely on this flag to detect selection extension.
  return d->m_bMousePressed;
}
#endif // KHTML_NO_SELECTION

void KHTMLPart::khtmlMouseMoveEvent( khtml::MouseMoveEvent *event )
{
  QMouseEvent *_mouse = event->qmouseEvent();

  if( d->m_bRightMousePressed && parentPart() != 0 && d->m_bBackRightClick )
  {
    popupMenu( d->m_strSelectedURL );
    d->m_strSelectedURL = d->m_strSelectedURLTarget = QString::null;
    d->m_bRightMousePressed = false;
  }
#ifndef QT_NO_DRAGANDDROP
  if( d->m_bDnd && d->m_bMousePressed &&
      ( (!d->m_strSelectedURL.isEmpty() && !isEditable())
        || (!d->m_mousePressNode.isNull() && d->m_mousePressNode.elementId() == ID_IMG) ) ) {
    if ( ( d->m_dragStartPos - _mouse->pos() ).manhattanLength() <= KGlobalSettings::dndEventDelay() )
      return;

    QPixmap pix;
    HTMLImageElementImpl *img = 0L;
    QDragObject *drag = 0;
    KURL u;
    if ( d->m_mousePressNode.handle() && d->m_mousePressNode.handle()->id() == ID_IMG )
    {
      // Normal image
      img = static_cast<HTMLImageElementImpl *>(d->m_mousePressNode.handle());
      u = KURL( completeURL( khtml::parseURL(img->getAttribute(ATTR_SRC)).string() ) );
      pix = KMimeType::mimeType("image/png")->pixmap(KIcon::Desktop);
    }
    else if ( event->innerNode().handle() && event->innerNode().handle()->id() == ID_IMG )
    {
      // Image inside a link?
      img = static_cast<HTMLImageElementImpl *>(event->innerNode().handle());
      u = completeURL( d->m_strSelectedURL );
      pix = KMimeType::mimeType("image/png")->pixmap(KIcon::Desktop);
    }
    else
    {
      // Text link
      u = completeURL( d->m_strSelectedURL );
      pix = KMimeType::pixmapForURL(u, 0, KIcon::Desktop, KIcon::SizeMedium);
    }

    KURLDrag* urlDrag = new KURLDrag( u, img ? 0 : d->m_view->viewport() );
    if ( !d->m_referrer.isEmpty() )
      urlDrag->metaData()["referrer"] = d->m_referrer;

    if( img ) {
      KMultipleDrag *mdrag = new KMultipleDrag( d->m_view->viewport() );
      mdrag->addDragObject( new QImageDrag( img->currentImage(), 0L ) );
      mdrag->addDragObject( urlDrag );
      drag = mdrag;
    }
    else
      drag = urlDrag;

    if ( !pix.isNull() )
      drag->setPixmap( pix );

    stopAutoScroll();
    if(drag)
        drag->drag();

    // when we finish our drag, we need to undo our mouse press
    d->m_bMousePressed = false;
    d->m_strSelectedURL = d->m_strSelectedURLTarget = QString::null;
    return;
  }
#endif

  DOM::DOMString url = event->url();
  DOM::DOMString target = event->target();
  DOM::Node innerNode = event->innerNode();

  // Not clicked -> mouse over stuff
  if ( !d->m_bMousePressed )
  {
    // The mouse is over something
    if ( url.length() )
    {
      bool shiftPressed = ( _mouse->state() & ShiftButton );

      // Image map
      if ( !innerNode.isNull() && innerNode.elementId() == ID_IMG )
      {
        HTMLImageElementImpl *i = static_cast<HTMLImageElementImpl *>(innerNode.handle());
        if ( i && i->isServerMap() )
        {
          khtml::RenderObject *r = i->renderer();
          if(r)
          {
            int absx, absy, vx, vy;
            r->absolutePosition(absx, absy);
            view()->contentsToViewport( absx, absy, vx, vy );

            int x(_mouse->x() - vx), y(_mouse->y() - vy);

            d->m_overURL = url.string() + QString("?%1,%2").arg(x).arg(y);
            d->m_overURLTarget = target.string();
            overURL( d->m_overURL, target.string(), shiftPressed );
            return;
          }
        }
      }

      // normal link
      if ( d->m_overURL.isEmpty() || d->m_overURL != url || d->m_overURLTarget != target )
      {
        d->m_overURL = url.string();
        d->m_overURLTarget = target.string();
        overURL( d->m_overURL, target.string(), shiftPressed );
      }
    }
    else  // Not over a link...
    {
      if( !d->m_overURL.isEmpty() ) // and we were over a link  -> reset to "default statusbar text"
      {
        d->m_overURL = d->m_overURLTarget = QString::null;
        emit onURL( QString::null );
        // revert to default statusbar text
        setStatusBarText(QString::null, BarHoverText);
        emit d->m_extension->mouseOverInfo(0);
     }
    }
  }
  else {
#ifndef KHTML_NO_SELECTION
    // selection stuff
    if( d->m_bMousePressed && innerNode.handle() && innerNode.handle()->renderer() &&
        ( (_mouse->state() & LeftButton) != 0 )) {
      extendSelectionTo(event->x(), event->y(),
                        event->absX(), event->absY(), innerNode);
#else
      if ( d->m_doc && d->m_view ) {
        QPoint diff( _mouse->globalPos() - d->m_dragLastPos );

        if ( abs( diff.x() ) > 64 || abs( diff.y() ) > 64 ) {
          d->m_view->scrollBy( -diff.x(), -diff.y() );
          d->m_dragLastPos = _mouse->globalPos();
        }
#endif
    }
  }

}

void KHTMLPart::khtmlMouseReleaseEvent( khtml::MouseReleaseEvent *event )
{
  DOM::Node innerNode = event->innerNode();
  d->m_mousePressNode = DOM::Node();

  if ( d->m_bMousePressed ) {
    setStatusBarText(QString::null, BarHoverText);
    stopAutoScroll();
  }

  // Used to prevent mouseMoveEvent from initiating a drag before
  // the mouse is pressed again.
  d->m_bMousePressed = false;

  QMouseEvent *_mouse = event->qmouseEvent();
  if ( _mouse->button() == RightButton && parentPart() != 0 && d->m_bBackRightClick )
  {
    d->m_bRightMousePressed = false;
    KParts::BrowserInterface *tmp_iface = d->m_extension->browserInterface();
    if( tmp_iface ) {
      tmp_iface->callMethod( "goHistory(int)", -1 );
    }
  }
#ifndef QT_NO_CLIPBOARD
  if ((d->m_guiProfile == BrowserViewGUI) && (_mouse->button() == MidButton) && (event->url().isNull())) {
    KHTMLPart *p = this;
    while (p->parentPart()) p = p->parentPart();
    p->d->m_extension->pasteRequest();
  }
#endif

#ifndef KHTML_NO_SELECTION
  // delete selection in case start and end position are at the same point
  if(d->m_selectionStart == d->m_selectionEnd && d->m_startOffset == d->m_endOffset) {
#ifndef KHTML_NO_CARET
    d->m_extendAtEnd = true;
#else
    d->m_selectionStart = 0;
    d->m_selectionEnd = 0;
    d->m_startOffset = 0;
    d->m_endOffset = 0;
#endif
    emitSelectionChanged();
  } else /*if ((_mouse->state() & ShiftButton) == 0)*/ {
    // we have to get to know if end is before start or not...
    DOM::Node n = d->m_selectionStart;
    d->m_startBeforeEnd = false;
    d->m_extendAtEnd = true;
    if( d->m_selectionStart == d->m_selectionEnd ) {
      if( d->m_startOffset < d->m_endOffset )
        d->m_startBeforeEnd = true;
    } else {
#if 0
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
#else
      // shouldn't be null but it can happen with dynamic updating of nodes
      if (d->m_selectionStart.isNull() || d->m_selectionEnd.isNull() ||
          !d->m_selectionStart.handle()->renderer() ||
          !d->m_selectionEnd.handle()->renderer()) return;
      d->m_startBeforeEnd = RangeImpl::compareBoundaryPoints(
      			d->m_selectionStart.handle(), d->m_startOffset,
			d->m_selectionEnd.handle(), d->m_endOffset) <= 0;
#endif
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
      d->m_extendAtEnd = false;
    }
#ifndef KHTML_NO_CARET
    bool v = d->m_view->placeCaret();
    emitCaretPositionChanged(v ? d->caretNode() : 0, d->caretOffset());
#endif
    // get selected text and paste to the clipboard
#ifndef QT_NO_CLIPBOARD
    QString text = selectedText();
    text.replace(QChar(0xa0), ' ');
    disconnect( kapp->clipboard(), SIGNAL( selectionChanged()), this, SLOT( slotClearSelection()));
    kapp->clipboard()->setText(text,QClipboard::Selection);
    connect( kapp->clipboard(), SIGNAL( selectionChanged()), SLOT( slotClearSelection()));
#endif
    //kdDebug( 6000 ) << "selectedText = " << text << endl;
    emitSelectionChanged();
  }
#endif

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
        QPtrList<KAction> lst;
        lst.append( d->m_paLoadImages );
        plugActionList( "loadImages", lst );
    }
  }
}

void KHTMLPart::slotPrintFrame()
{
  if ( d->m_frames.count() == 0 )
    return;

  KParts::ReadOnlyPart *frame = currentFrame();
  if (!frame)
    return;

  KParts::BrowserExtension *ext = KParts::BrowserExtension::childObject( frame );

  if ( !ext )
    return;

  QMetaObject *mo = ext->metaObject();

  int idx = mo->findSlot( "print()", true );
  if ( idx >= 0 ) {
    QUObject o[ 1 ];
    ext->qt_invoke( idx, o );
  }
}

void KHTMLPart::slotSelectAll()
{
  KParts::ReadOnlyPart *part = currentFrame();
  if (part && part->inherits("KHTMLPart"))
    static_cast<KHTMLPart *>(part)->selectAll();
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
  if (!d->m_doc) return;

  NodeImpl *first;
  if (d->m_doc->isHTMLDocument())
    first = static_cast<HTMLDocumentImpl*>(d->m_doc)->body();
  else
    first = d->m_doc;
  NodeImpl *next;

  // Look for first text/cdata node that has a renderer,
  // or first childless replaced element
  while ( first && !(first->renderer()
  	&& ((first->nodeType() == Node::TEXT_NODE || first->nodeType() == Node::CDATA_SECTION_NODE)
		|| (first->renderer()->isReplaced() && !first->renderer()->firstChild()))))
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

  NodeImpl *last;
  if (d->m_doc->isHTMLDocument())
    last = static_cast<HTMLDocumentImpl*>(d->m_doc)->body();
  else
    last = d->m_doc;
  // Look for last text/cdata node that has a renderer,
  // or last childless replaced element
  // ### Instead of changing this loop, use findLastSelectableNode
  // in render_table.cpp (LS)
  while ( last && !(last->renderer()
  	&& ((last->nodeType() == Node::TEXT_NODE || last->nodeType() == Node::CDATA_SECTION_NODE)
		|| (last->renderer()->isReplaced() && !last->renderer()->lastChild()))))
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
  Q_ASSERT(first->renderer());
  Q_ASSERT(last->renderer());
  d->m_selectionStart = first;
  d->m_startOffset = 0;
  d->m_selectionEnd = last;
  d->m_endOffset = last->nodeValue().length();
  d->m_startBeforeEnd = true;

  d->m_doc->setSelection( d->m_selectionStart.handle(), d->m_startOffset,
                          d->m_selectionEnd.handle(), d->m_endOffset );

  emitSelectionChanged();
}

bool KHTMLPart::checkLinkSecurity(const KURL &linkURL,const QString &message, const QString &button)
{
  // Security check on the link.
  // KURL u( url ); Wrong!! Relative URL could be mis-interpreted!!! (DA)
  QString linkProto = linkURL.protocol().lower();
  QString proto = m_url.protocol().lower();

  if ( !linkProto.isEmpty() && !proto.isEmpty() &&
       ( linkProto == "cgi" || linkProto == "file" ) &&
       proto != "file" && proto != "cgi" && proto != "man" && proto != "about")
  {
      khtml::Tokenizer *tokenizer = d->m_doc->tokenizer();
    if (tokenizer)
      tokenizer->setOnHold(true);

    int response = KMessageBox::Cancel;
    if (!message.isEmpty())
    {
	    response = KMessageBox::warningContinueCancel( 0,
							   message.arg(linkURL.url()),
							   i18n( "Security Warning" ),
							   button);
    }
    else
    {
	    KMessageBox::error( 0,
				i18n( "<qt>This untrusted page contains a link<BR><B>%1</B><BR>to your local file system.").arg(linkURL.url()),
				i18n( "Security Alert" ));
    }

    if (tokenizer)
       tokenizer->setOnHold(false);
    return (response==KMessageBox::Continue);
  }
  return true;
}

void KHTMLPart::slotPartRemoved( KParts::Part *part )
{
//    kdDebug(6050) << "KHTMLPart::slotPartRemoved " << part << endl;
    if ( part == d->m_activeFrame )
    {
        d->m_activeFrame = 0L;
        if ( !part->inherits( "KHTMLPart" ) )
        {
            if (factory()) {
                factory()->removeClient( part );
            }
            if (childClients()->containsRef(part)) {
                removeChildClient( part );
            }
        }
    }
}

void KHTMLPart::slotActiveFrameChanged( KParts::Part *part )
{
//    kdDebug(6050) << "KHTMLPart::slotActiveFrameChanged part=" << part << endl;
    if ( part == this )
    {
        kdError(6050) << "strange error! we activated ourselves" << endl;
        assert( false );
        return;
    }
//    kdDebug(6050) << "KHTMLPart::slotActiveFrameChanged d->m_activeFrame=" << d->m_activeFrame << endl;
    if ( d->m_activeFrame && d->m_activeFrame->widget() && d->m_activeFrame->widget()->inherits( "QFrame" ) )
    {
        QFrame *frame = static_cast<QFrame *>( d->m_activeFrame->widget() );
        if (frame->frameStyle() != QFrame::NoFrame)
        {
           frame->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken);
           frame->repaint();
        }
    }

    if( d->m_activeFrame && !d->m_activeFrame->inherits( "KHTMLPart" ) )
    {
        if (factory()) {
            factory()->removeClient( d->m_activeFrame );
        }
        removeChildClient( d->m_activeFrame );
    }
    if( part && !part->inherits( "KHTMLPart" ) )
    {
        if (factory()) {
            factory()->addClient( part );
        }
        insertChildClient( part );
    }


    d->m_activeFrame = part;

    if ( d->m_activeFrame && d->m_activeFrame->widget()->inherits( "QFrame" ) )
    {
        QFrame *frame = static_cast<QFrame *>( d->m_activeFrame->widget() );
        if (frame->frameStyle() != QFrame::NoFrame)
        {
           frame->setFrameStyle( QFrame::StyledPanel | QFrame::Plain);
           frame->repaint();
        }
        kdDebug(6050) << "new active frame " << d->m_activeFrame << endl;
    }

    updateActions();

    // (note: childObject returns 0 if the argument is 0)
    d->m_extension->setExtensionProxy( KParts::BrowserExtension::childObject( d->m_activeFrame ) );
}

void KHTMLPart::setActiveNode(const DOM::Node &node)
{
    if (!d->m_doc || !d->m_view)
        return;

    // Set the document's active node
    d->m_doc->setFocusNode(node.handle());

    // Scroll the view if necessary to ensure that the new focus node is visible
    QRect rect  = node.handle()->getRect();
    d->m_view->ensureVisible(rect.right(), rect.bottom());
    d->m_view->ensureVisible(rect.left(), rect.top());
}

DOM::Node KHTMLPart::activeNode() const
{
    return DOM::Node(d->m_doc?d->m_doc->focusNode():0);
}

DOM::EventListener *KHTMLPart::createHTMLEventListener( QString code, QString name )
{
  KJSProxy *proxy = jScript();

  if (!proxy)
    return 0;

  return proxy->createHTMLEventHandler( m_url.url(), name, code );
}

KHTMLPart *KHTMLPart::opener()
{
    return d->m_opener;
}

void KHTMLPart::setOpener(KHTMLPart *_opener)
{
    d->m_opener = _opener;
}

bool KHTMLPart::openedByJS()
{
    return d->m_openedByJS;
}

void KHTMLPart::setOpenedByJS(bool _openedByJS)
{
    d->m_openedByJS = _openedByJS;
}

void KHTMLPart::preloadStyleSheet(const QString &url, const QString &stylesheet)
{
    khtml::Cache::preloadStyleSheet(url, stylesheet);
}

void KHTMLPart::preloadScript(const QString &url, const QString &script)
{
    khtml::Cache::preloadScript(url, script);
}

QCString KHTMLPart::dcopObjectId() const
{
  QCString id;
  id.sprintf("html-widget%d", d->m_dcop_counter);
  return id;
}

long KHTMLPart::cacheId() const
{
  return d->m_cacheId;
}

bool KHTMLPart::restored() const
{
  return d->m_restored;
}

bool KHTMLPart::pluginPageQuestionAsked(const QString& mimetype) const
{
  // parentPart() should be const!
  KHTMLPart* parent = const_cast<KHTMLPart *>(this)->parentPart();
  if ( parent )
    return parent->pluginPageQuestionAsked(mimetype);

  return d->m_pluginPageQuestionAsked.contains(mimetype);
}

void KHTMLPart::setPluginPageQuestionAsked(const QString& mimetype)
{
  if ( parentPart() )
    parentPart()->setPluginPageQuestionAsked(mimetype);

  d->m_pluginPageQuestionAsked.append(mimetype);
}

void KHTMLPart::slotAutomaticDetectionLanguage( int _id )
{
  d->m_automaticDetection->setItemChecked( _id, true );

  switch ( _id ) {
    case 0 :
      d->m_autoDetectLanguage = khtml::Decoder::SemiautomaticDetection;
      break;
    case 1 :
      d->m_autoDetectLanguage = khtml::Decoder::Arabic;
      break;
    case 2 :
      d->m_autoDetectLanguage = khtml::Decoder::Baltic;
      break;
    case 3 :
      d->m_autoDetectLanguage = khtml::Decoder::CentralEuropean;
      break;
    case 4 :
      d->m_autoDetectLanguage = khtml::Decoder::Chinese;
      break;
    case 5 :
      d->m_autoDetectLanguage = khtml::Decoder::Greek;
      break;
    case 6 :
      d->m_autoDetectLanguage = khtml::Decoder::Hebrew;
      break;
    case 7 :
      d->m_autoDetectLanguage = khtml::Decoder::Japanese;
      break;
    case 8 :
      d->m_autoDetectLanguage = khtml::Decoder::Korean;
      break;
    case 9 :
      d->m_autoDetectLanguage = khtml::Decoder::Russian;
      break;
    case 10 :
      d->m_autoDetectLanguage = khtml::Decoder::Thai;
      break;
    case 11 :
      d->m_autoDetectLanguage = khtml::Decoder::Turkish;
      break;
    case 12 :
      d->m_autoDetectLanguage = khtml::Decoder::Ukrainian;
      break;
    case 13 :
      d->m_autoDetectLanguage = khtml::Decoder::Unicode;
      break;
    case 14 :
      d->m_autoDetectLanguage = khtml::Decoder::WesternEuropean;
      break;
    default :
      d->m_autoDetectLanguage = khtml::Decoder::SemiautomaticDetection;
      break;
  }

  for ( int i = 0; i <= 14; ++i ) {
    if ( i != _id )
      d->m_automaticDetection->setItemChecked( i, false );
  }

  d->m_paSetEncoding->popupMenu()->setItemChecked( 0, true );

  setEncoding( QString::null, false );

  if( d->m_manualDetection )
    d->m_manualDetection->setCurrentItem( -1 );
  d->m_paSetEncoding->popupMenu()->setItemChecked( d->m_paSetEncoding->popupMenu()->idAt( 2 ), false );
}

khtml::Decoder *KHTMLPart::createDecoder()
{
  khtml::Decoder *dec = new khtml::Decoder();
  if( !d->m_encoding.isNull() )
    dec->setEncoding( d->m_encoding.latin1(), true );
  else
    dec->setEncoding( settings()->encoding().latin1(), d->m_haveEncoding );

  dec->setAutoDetectLanguage( d->m_autoDetectLanguage );
  return dec;
}

void KHTMLPart::emitCaretPositionChanged(const DOM::Node &node, long offset) {
  emit caretPositionChanged(node, offset);
}

KWallet::Wallet* KHTMLPart::wallet()
{
  // ### close wallet after a certain timeout period automatically
  //      No - KWallet already does this based on user preferences. (GS)
  // ### close wallet after screensaver was enabled
  //      No - KWalletD should do this, if anything. (GS)

  KHTMLPart* p;

  for (p = parentPart(); p && p->parentPart(); p = p->parentPart())
    ;

  if (p)
    return p->wallet();

  if (!d->m_wallet && !d->m_bWalletOpened) {
    d->m_wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), widget() ? widget()->topLevelWidget()->winId() : 0);
    d->m_bWalletOpened = true;
    if (d->m_wallet) {
      connect(d->m_wallet, SIGNAL(walletClosed()), SLOT(slotWalletClosed()));
      d->m_statusBarWalletLabel = new KURLLabel(d->m_statusBarExtension->statusBar());
      d->m_statusBarWalletLabel->setFixedHeight(instance()->iconLoader()->currentSize(KIcon::Small));
      d->m_statusBarWalletLabel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
      d->m_statusBarWalletLabel->setUseCursor(false);
      d->m_statusBarExtension->addStatusBarItem(d->m_statusBarWalletLabel, 0, false);
      QToolTip::add(d->m_statusBarWalletLabel, i18n("The wallet '%1' is open and being used for form data and passwords.").arg(KWallet::Wallet::NetworkWallet()));
      d->m_statusBarWalletLabel->setPixmap(SmallIcon("wallet_open", instance()));
      connect(d->m_statusBarWalletLabel, SIGNAL(leftClickedURL()), SLOT(launchWalletManager()));
      connect(d->m_statusBarWalletLabel, SIGNAL(rightClickedURL()), SLOT(walletMenu()));
    } else if (d->m_statusBarWalletLabel) {
      d->m_statusBarExtension->removeStatusBarItem(d->m_statusBarWalletLabel);
      delete d->m_statusBarWalletLabel;
      d->m_statusBarWalletLabel = 0L;
    }
  }
  return d->m_wallet;
}

void KHTMLPart::slotWalletClosed()
{
  if (d->m_wallet) {
    d->m_wallet->deleteLater();
    d->m_wallet = 0L;
  }
  d->m_bWalletOpened = false;
  if (d->m_statusBarWalletLabel) {
    d->m_statusBarExtension->removeStatusBarItem(d->m_statusBarWalletLabel);
    delete d->m_statusBarWalletLabel;
    d->m_statusBarWalletLabel = 0L;
  }
}

void KHTMLPart::launchWalletManager()
{
  if (!DCOPClient::mainClient()->isApplicationRegistered("kwalletmanager")) {
    KApplication::startServiceByDesktopName("kwalletmanager_show");
  } else {
    DCOPRef r("kwalletmanager", "kwalletmanager-mainwindow#1");
    r.send("show");
    r.send("raise");
  }
}

void KHTMLPart::walletMenu()
{
  KPopupMenu *m = new KPopupMenu(0L);
  m->insertItem(i18n("&Close Wallet"), this, SLOT(slotWalletClosed()));
  m->popup(QCursor::pos());
}

void KHTMLPart::slotToggleCaretMode()
{
  setCaretMode(d->m_paToggleCaretMode->isChecked());
}

void KHTMLPart::setFormNotification(KHTMLPart::FormNotification fn) {
  d->m_formNotification = fn;
}

KHTMLPart::FormNotification KHTMLPart::formNotification() const {
  return d->m_formNotification;
}

KURL KHTMLPart::toplevelURL()
{
  KHTMLPart* part = this;
  while (part->parentPart())
    part = part->parentPart();

  if (!part)
    return KURL();

  return part->url();
}

void KHTMLPart::setDebugScript( bool enable )
{
  unplugActionList( "debugScriptList" );
  if ( enable ) {
    if (!d->m_paDebugScript) {
      d->m_paDebugScript = new KAction( i18n( "JavaScript &Debugger" ), 0, this, SLOT( slotDebugScript() ), actionCollection(), "debugScript" );
    }
    d->m_paDebugScript->setEnabled( d->m_jscript );
    QPtrList<KAction> lst;
    lst.append( d->m_paDebugScript );
    plugActionList( "debugScriptList", lst );
  }
  d->m_bJScriptDebugEnabled = enable;
}

using namespace KParts;
#include "khtml_part.moc"
