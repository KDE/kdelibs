/* This file is part of the KDE project
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999 Lars Knoll <knoll@kde.org>
 *                     1999 Antti Koivisto <koivisto@kde.org>
 *		       2000 Simon Hausmann <hausmann@kde.org>
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

#include "dom/html_document.h"
#include "html/html_documentimpl.h"
#include "misc/khtmldata.h"
#include "html/html_miscimpl.h"
#include "html/html_inlineimpl.h"
#include "rendering/render_frames.h"

#include "khtmlview.h"
#include "decoder.h"
#include "kjs.h"

#include <stdio.h>
#include <assert.h>

#include <kglobal.h>
#include <kstddirs.h>
#include <ltdl.h>
#include <kio/job.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kfiledialog.h>
#include <klibloader.h>
#include <ktrader.h>
#include <kparts/partmanager.h>

#include <unistd.h>

namespace khtml
{
  struct ChildFrame
  {
    ChildFrame() { m_bCompleted = false; m_frame = 0L; }

    RenderFrame *m_frame;
    QGuardedPtr<KParts::ReadOnlyPart> m_part;
    QGuardedPtr<KParts::BrowserExtension> m_extension;
    QString m_serviceType;
    QStringList m_services;
    bool m_bCompleted;
    QString m_name;
    KParts::URLArgs m_args;
    QGuardedPtr<KHTMLRun> m_run;
  };
};

class KHTMLPartPrivate
{
public:
  KHTMLPartPrivate()
  {
    m_doc = 0L;
    m_decoder = 0L;
    m_jscript = 0L;
    m_job = 0L;
    m_bComplete = false;
    m_bParsing = false;
    m_manager = 0L;
    m_settings = new khtml::Settings();
  }
  ~KHTMLPartPrivate()
  {
    //no need to delete m_widget here! kparts does it for us (: (Simon)
    if ( m_extension )
      delete m_extension;
    delete m_settings;
  }

  QMap<QString,khtml::ChildFrame> m_frames;

  QGuardedPtr<KHTMLWidget> m_widget;
  KHTMLPartBrowserExtension *m_extension;
  DOM::HTMLDocumentImpl *m_doc;
  KHTMLDecoder *m_decoder;
  KJSProxy *m_jscript;
  bool m_bJScriptEnabled;
  bool m_bJavaEnabled;
  bool lt_dl_initialized;

  khtml::Settings *m_settings;

  KIO::TransferJob * m_job;

  bool m_bComplete;
  bool m_bParsing;

  KURL m_workingURL;
  KURL m_baseURL;
  QString m_baseTarget;

  int m_delayRedirect;
  KURL m_redirectURL;

  KAction *m_paViewDocument;
  KAction *m_paViewFrame;
  KAction *m_paSaveBackground;
  KAction *m_paSaveDocument;
  KAction *m_paSaveFrame;

  KParts::PartManager *m_manager;
};

KHTMLPart::KHTMLPart( QWidget *parentWidget, const char *widgetname, QObject *parent, const char *name )
: KParts::ReadOnlyPart( parent ? parent : parentWidget, name ? name : widgetname )
{
  setInstance( KHTMLFactory::instance() );
  setXMLFile( "khtml.rc" );

  d = new KHTMLPartPrivate;

  d->m_widget = new KHTMLWidget( this, parentWidget, widgetname );
  setWidget( d->m_widget );

  d->m_extension = new KHTMLPartBrowserExtension( this );

  d->m_bJScriptEnabled = false;
  d->m_bJavaEnabled = false;
  d->lt_dl_initialized = false;

  d->m_paViewDocument = new KAction( i18n( "View Document Source" ), 0, this, SLOT( slotViewDocumentSource() ), actionCollection(), "viewDocumentSource" );
  d->m_paViewFrame = new KAction( i18n( "View Frame Source" ), 0, this, SLOT( slotViewFrameSource() ), actionCollection(), "viewFrameSource" );
  d->m_paSaveBackground = new KAction( i18n( "Save &Background Image As.." ), 0, this, SLOT( slotSaveBackground() ), actionCollection(), "saveBackground" );
  d->m_paSaveDocument = new KAction( i18n( "&Save As.." ), 0, this, SLOT( slotSaveDocument() ), actionCollection(), "saveDocument" );
  d->m_paSaveFrame = new KAction( i18n( "Save &Frame As.." ), 0, this, SLOT( slotSaveFrame() ), actionCollection(), "saveFrame" );

  connect( this, SIGNAL( completed() ),
	   this, SLOT( updateActions() ) );
  connect( this, SIGNAL( started( KIO::Job * ) ),
	   this, SLOT( updateActions() ) );
}

KHTMLPart::~KHTMLPart()
{
  if ( d->m_widget )
  {
    d->m_widget->hide();
    d->m_widget->viewport()->hide();
  }
  closeURL();

  clear();

  delete d;
}

bool KHTMLPart::openURL( const KURL &url )
{
  static QString http_protocol = QString::fromLatin1( "http" );

  if ( d->m_frames.count() > 0 && urlcmp( url.url(), m_url.url(), true, true ) && d->m_extension->urlArgs().postData.size() == 0 )
  {
    m_url = url;
    emit started( 0L );

    if ( !url.htmlRef().isEmpty() )
      gotoAnchor( url.htmlRef() );
    else
      d->m_widget->setContentsPos( 0, 0 );

    d->m_bComplete = true;
    d->m_bParsing = false;

    emit completed();
    return true;
  }

  if ( !closeURL() )
    return false;

  if ( d->m_extension->urlArgs().postData.size() > 0 && url.protocol() == http_protocol )
      d->m_job = KIO::http_post( url, d->m_extension->urlArgs().postData );
  else
      d->m_job = KIO::get( url, d->m_extension->urlArgs().reload );

  connect( d->m_job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotFinished( KIO::Job * ) ) );
  connect( d->m_job, SIGNAL( data( KIO::Job*, const QByteArray &)),
           SLOT( slotData( KIO::Job*, const QByteArray &)));

  //connect( job, SIGNAL( sigRedirection( int, const char * ) ), this, SLOT( slotRedirection( int, const char * ) ) );

  d->m_bComplete = false;

  d->m_workingURL = url;

  m_url = url;

  emit started( d->m_job );

  return true;
}

bool KHTMLPart::closeURL()
{
  if ( d->m_job )
  {
    d->m_job->kill();
    d->m_job = 0;
  }

  if ( d->m_bParsing )
  {
    end();
    d->m_bParsing = false;
  }

  d->m_workingURL = KURL();

  // ### cancel all file requests

  if ( !d->m_bComplete )
    emit canceled( QString::null );

  d->m_bComplete = true;

  return true;
}

DOM::HTMLDocument KHTMLPart::htmlDocument() const
{
  return d->m_doc;
}

KHTMLPartBrowserExtension *KHTMLPart::browserExtension() const
{
  return d->m_extension;
}

KHTMLWidget *KHTMLPart::htmlWidget() const
{
  return d->m_widget;
}

void KHTMLPart::enableJScript( bool enable )
{
  d->m_bJScriptEnabled = enable;
}

bool KHTMLPart::jScriptEnabled() const
{
  return d->m_bJScriptEnabled;
}

KJSProxy *KHTMLPart::jScript()
{
  if ( !d->m_bJScriptEnabled )
    return 0;

  if ( !d->m_jscript )
  {
    if(!d->lt_dl_initialized)
    {
      lt_dlinit();
      d->lt_dl_initialized = true;
    }
    // locate module
    QString module = KGlobal::dirs()->findResource("lib", "kjs_html.la");
    if(module.isNull())
    {
      fprintf(stderr, "didn't find kjs module\n");
      return 0;
    }
    // try to obtain a handle on the module
    lt_dlhandle handle = lt_dlopen(module.ascii());
    if(!handle)
    {
      fprintf(stderr, "error loading kjs module: %s\n", lt_dlerror());
      return 0;
    }
    // look for plain C init function
    lt_ptr_t sym = lt_dlsym(handle, "kjs_html_init");
    if (lt_dlerror() != 0L)
    {
      fprintf(stderr, "error finding init symbol: %s\n", lt_dlerror());
      return 0;
    }

    typedef KJSProxy* (*initFunction)(HTMLDocument);
    initFunction initSym = (initFunction) sym;
    d->m_jscript = (*initSym)(htmlDocument());
  }

  return d->m_jscript;
}

void KHTMLPart::executeScript( const QString &script )
{
  if ( !d->m_bJScriptEnabled )
    return;

  jScript()->evaluate( script.unicode(), script.length() );
}

void KHTMLPart::enableJava( bool enable )
{
  d->m_bJavaEnabled = enable;
}

bool KHTMLPart::javaEnabled() const
{
  return d->m_bJavaEnabled;
}

void KHTMLPart::clear()
{
  if ( d->m_doc )
  {
    d->m_doc->detach();
    d->m_doc->deref();
  }
  d->m_doc = 0;

  if ( d->m_decoder )
    delete d->m_decoder;

  d->m_decoder = 0;

  if ( d->m_jscript )
    delete d->m_jscript;

  d->m_jscript = 0;

  if ( d->m_widget )
    d->m_widget->clear();
  /*
  QMap<QString,khtml::ChildFrame>::ConstIterator it = d->m_frames.begin();
  QMap<QString,khtml::ChildFrame>::ConstIterator end = d->m_frames.end();
  for(; it != end; ++it )
    if ( it.data().m_part )
    {
      partManager()->removePart( it.data().m_part );
      delete (KParts::ReadOnlyPart *)it.data().m_part;
    }
  */
  d->m_frames.clear();

  d->m_baseURL = KURL();
  d->m_baseTarget = QString::null;
  d->m_delayRedirect = 0;
  d->m_redirectURL = KURL();
}

bool KHTMLPart::openFile()
{
  return true;
}

DOM::HTMLDocumentImpl *KHTMLPart::docImpl() const
{
  return d->m_doc;
}

void KHTMLPart::slotData( KIO::Job*, const QByteArray &data )
{
  kDebugInfo(1202, "slotData: %d", data.size() );

  // The first data ?
  if ( !d->m_workingURL.isEmpty() )
  {
    kDebugInfo(1202, "begin!" );
    d->m_bParsing = true;

    begin( d->m_workingURL, d->m_extension->urlArgs().xOffset, d->m_extension->urlArgs().yOffset );

    d->m_workingURL = KURL();
  }

  write( data.data(), data.size() );
}

void KHTMLPart::slotFinished( KIO::Job * job )
{
  if (job->error())
  {
    job->showErrorDialog();
    d->m_job = 0L;
    // TODO: what else ?
    return;
  }
  kDebugInfo(1202, "slotFinished" );

  d->m_workingURL = KURL();

  d->m_job = 0L;

  if ( d->m_bParsing )
  {
    kDebugInfo( 1202, "end()" );
    end(); //will emit completed()
  }
}

void KHTMLPart::begin( const KURL &url, int xOffset, int yOffset )
{
  clear();

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

  d->m_doc = new HTMLDocumentImpl( d->m_widget );
  d->m_doc->ref();
  d->m_doc->attach( d->m_widget );
  d->m_doc->setURL( m_url.url() ); //### Lars, why not make the DOM stuff use KURL? :-) (Simon)
  d->m_doc->open();
  // clear widget
  d->m_widget->resizeContents( 0, 0 );

  d->m_bParsing = true;
}

void KHTMLPart::write( const char *str, int len )
{
  if ( !d->m_decoder )
    d->m_decoder = new KHTMLDecoder();

  if ( len == 0 )
    return;

  if ( len == -1 )
    len = strlen( str );

  QString decoded = d->m_decoder->decode( str, len );
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
  d->m_bParsing = false;
  d->m_doc->close();

  if ( !m_url.htmlRef().isEmpty() )
    gotoAnchor( m_url.htmlRef() );
  else
    d->m_widget->setContentsPos( d->m_extension->urlArgs().xOffset, d->m_extension->urlArgs().yOffset );

  if ( !d->m_redirectURL.isEmpty() )
  {
    QTimer::singleShot( 1000 * d->m_delayRedirect, this, SLOT( slotRedirect() ) );
    return;
  }

  checkCompleted();
}

void KHTMLPart::checkCompleted()
{
  if ( d->m_bParsing )
    return;

  QMap<QString,khtml::ChildFrame>::ConstIterator it = d->m_frames.begin();
  QMap<QString,khtml::ChildFrame>::ConstIterator end = d->m_frames.end();
  for (; it != end; ++it )
    if ( !it.data().m_bCompleted )
      return;

  // ### check all additional data downloaded

  d->m_bComplete = true;

  emit completed();
}

const khtml::Settings *KHTMLPart::settings() const
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

KURL KHTMLPart::completeURL( const QString &url, const QString &target )
{
  KURL orig;
  /* ###
  if(url[0] == '#' && !target.isEmpty() && findFrame(target))
  {
    orig = KURL(findFrame(target)->url());
  }
  else */ if( d->m_baseURL.isEmpty())
  {
    orig = m_url;
  }
  else
    orig = d->m_baseURL;
  if(url[0] != '/')
  {
    KURL u( orig, url );
    return u;
  }	
  orig.setEncodedPathAndQuery( url );
  return orig;
}

void KHTMLPart::scheduleRedirection( int delay, const KURL &url )
{
  d->m_delayRedirect = delay;
  d->m_redirectURL = url;
}

void KHTMLPart::slotRedirect()
{
  KURL u = d->m_redirectURL;
  d->m_delayRedirect = 0;
  d->m_redirectURL = KURL();
  urlSelected( u.url() );
}

// ####
bool KHTMLPart::setCharset( const QString &name, bool override )
{
  // ### hack: FIXME, use QFontDatabase!!!!!
  KCharsets *c = KGlobal::charsets();
  if(!c->isAvailable(name))
  {
    printf("charset not available!\n");
    return false;
  }

  QFont f;
  c->setQFont(f, name);

  QFontInfo fi(f);
  printf("font has charset %d, real %d\n", f.charSet(), fi.charSet());

  d->m_settings->charset = f.charSet();
  return true;
}

bool KHTMLPart::setEncoding( const QString &name, bool override )
{
    // ###
}

void KHTMLPart::setUserStyleSheet(const KURL &url)
{
    // ###
}

void KHTMLPart::setUserStyleSheet(const QString &styleSheet)
{
    // ###
}


bool KHTMLPart::gotoAnchor( const QString &name )
{
  printf("gotoAnchor(%s)\n", name.latin1());
  HTMLCollectionImpl *anchors =
      new HTMLCollectionImpl( d->m_doc, HTMLCollectionImpl::DOC_ANCHORS);
  anchors->ref();
  NodeImpl *n = anchors->namedItem(name);
  anchors->deref();

  if(!n)
  {
    n = d->m_doc->getElementById(name);
    if(n) printf("found element with matching id\n");
  }
	
  if(!n) return false;
  printf("found anchor %p!\n", n);

  int x = 0, y = 0;
  HTMLAnchorElementImpl *a = static_cast<HTMLAnchorElementImpl *>(n);
  a->getAnchorPosition(x, y);
  printf("going to %d/%d\n", x, y);
  d->m_widget->setContentsPos(x-50, y-50);
  return true;
}

void KHTMLPart::setFontSizes( const int *newFontSizes, int numFontSizes )
{
  d->m_settings->setFontSizes( newFontSizes, numFontSizes );
}

const int *KHTMLPart::fontSizes() const
{
  return d->m_settings->fontSizes();
}

void KHTMLPart::resetFontSizes()
{
  d->m_settings->resetFontSizes();
}

void KHTMLPart::setStandardFont( const QString &name )
{
    d->m_settings->setDefaultFamily(name);
}

void KHTMLPart::setFixedFont( const QString &name )
{
    d->m_settings->setMonoSpaceFamily(name);
}

void KHTMLPart::setURLCursor( const QCursor &c )
{
  d->m_widget->setURLCursor( c );
}

const QCursor &KHTMLPart::urlCursor()
{
  return d->m_widget->urlCursor();
}

void KHTMLPart::overURL( const QString &url )
{
  emit onURL( url );

  if ( url.isEmpty() )
  {
    emit setStatusBarText( url );
    return;
  }

  KURL u( m_url, url );
  QString com;

  KMimeType::Ptr typ = KMimeType::findByURL( u );

  if ( typ )
    com = typ->comment( u, false );

  if ( u.isMalformed() )
  {
    QString decodedURL = url;
    KURL::decode( decodedURL );
    emit setStatusBarText( decodedURL );
    return;
  }

  if ( u.isLocalFile() )
  {
    // TODO : use KIO::stat() and create a KFileItem out of its result,
   // to use KFileItem::statusBarText()
    QString decodedPath( u.path() );
    QString decodedName( u.filename( true ) );
	
    struct stat buff;
    stat( decodedPath, &buff );

    struct stat lbuff;
    lstat( decodedPath, &lbuff );

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
      int n = readlink ( decodedPath, buff_two, 1022);
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
	text = QString("%1 (%2 %3)").arg(text2).arg((long) buff.st_size).arg(i18n("bytes"));
      else
      {
	float d = (float) buff.st_size/1024.0;
	text = QString("%1 (%2 K)").arg(text2).arg(d, 0, 'f', 2); // was %.2f
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
    emit setStatusBarText( u.decodedURL() );

}

void KHTMLPart::urlSelected( const QString &url, int button, const QString &_target )
{
  if ( !d->m_bComplete )
    closeURL();

  if ( url.isEmpty() )
    return;

  KURL u( url );

  // Security
  if ( ::strcmp( u.protocol(), "cgi" ) == 0 &&
       ::strcmp( m_url.protocol(), "file" ) != 0 && ::strcmp( m_url.protocol(), "cgi" ) != 0 )
  {
    KMessageBox::error( 0,
			i18n( "This page is untrusted\nbut it contains a link to your local file system."),
			i18n( "Security Alert" ));
    return;
  }

  QString target = _target;
  if ( target.isEmpty() )
    target = d->m_baseTarget;

  u = completeURL( url, target );

  KParts::URLArgs args;
  args.frameName = target;

  if ( !target.isEmpty() )
  {
    khtml::ChildFrame *frame = recursiveFrameRequest( u, args, false, false );
    if ( frame )
    {
      childRequest( frame, u, args );
      return;
    }
  }

  emit d->m_extension->openURLRequest( u, args );
}

void KHTMLPart::slotViewDocumentSource()
{
  emit d->m_extension->openURLRequest( m_url, KParts::URLArgs( false, 0, 0, QString::fromLatin1( "text/plain" ) ) );
}

void KHTMLPart::slotViewFrameSource()
{
  // ### frames
}

void KHTMLPart::slotSaveBackground()
{
  QString relURL = d->m_doc->body()->getAttribute( ATTR_BACKGROUND ).string();

  KURL backgroundURL( m_url, relURL );

  KFileDialog *dlg = new KFileDialog( QString::null, "*",
					d->m_widget , "filedialog", true );
  dlg->setCaption(i18n("Save background image as"));

  dlg->setSelection( backgroundURL.filename() );
  if ( dlg->exec() )
  {
    KURL destURL( dlg->selectedURL());
    if ( !destURL.isMalformed() )
    {
      KIO::Job *job = KIO::copy( m_url, destURL );
      // TODO connect job result, to display errors
    }
  }

  delete dlg;
}

void KHTMLPart::slotSaveDocument()
{
  // ### frames

  KURL srcURL( m_url );

  if ( srcURL.filename(false).isEmpty() )
    srcURL.setFileName( "index.html" );

  KFileDialog *dlg = new KFileDialog( QString::null, i18n("HTML files|* *.html *.htm"),
				      d->m_widget , "filedialog", true );
  dlg->setCaption(i18n("Save as"));

  dlg->setSelection( srcURL.filename() );
  if ( dlg->exec() )
  {
     KURL destURL( dlg->selectedURL() );
      if ( !destURL.isMalformed() )
      {
        KIO::Job *job = KIO::copy( url(), destURL );
        // TODO connect job result, to display errors
      }
  }

  delete dlg;
}

void KHTMLPart::slotSaveFrame()
{
  // ### frames
}

void KHTMLPart::updateActions()
{
  bool frames = d->m_frames.count() > 0;
  d->m_paViewFrame->setEnabled( frames );
  d->m_paSaveFrame->setEnabled( frames );

  QString bgURL;

  // ### frames

  if ( d->m_doc && d->m_doc->body() )
    bgURL = d->m_doc->body()->getAttribute( ATTR_BACKGROUND ).string();

  d->m_paSaveBackground->setEnabled( !bgURL.isEmpty() );
}

void KHTMLPart::childRequest( khtml::RenderFrame *frame, const QString &url, const QString &frameName )
{
  QMap<QString,khtml::ChildFrame>::Iterator it = d->m_frames.find( frameName );

  if ( it == d->m_frames.end() )
  {
    khtml::ChildFrame child;
    child.m_frame = frame;
    child.m_name = frameName;
    it = d->m_frames.insert( frameName, child );
  }

  childRequest( &it.data(), completeURL( url ) );
}

void KHTMLPart::childRequest( khtml::ChildFrame *child, const KURL &url, const KParts::URLArgs &args )
{
  child->m_args = args;

  if ( child->m_run )
    delete (KHTMLRun *)child->m_run;

  if ( args.serviceType.isEmpty() )
    child->m_run = new KHTMLRun( this, child, url );
  else
    processChildRequest( child, url, args.serviceType );
}

void KHTMLPart::processChildRequest( khtml::ChildFrame *child, const KURL &url, const QString &mimetype )
{
  qDebug( "trying to create part for %s", debugString( mimetype ) );

  if ( !child->m_services.contains( mimetype ) )
  {
    KParts::ReadOnlyPart *part = createFrame( d->m_widget->viewport(), child->m_name.ascii(), this, child->m_name.ascii(), mimetype, child->m_services );

    if ( !part )
      return;
    
    child->m_serviceType = mimetype;
    child->m_frame->setWidget( part->widget() );

    //CRITICAL STUFF
    if ( child->m_part )
    {
      partManager()->removePart( (KParts::ReadOnlyPart *)child->m_part );
      delete (KParts::ReadOnlyPart *)child->m_part;
    }

    partManager()->addPart( part );
    child->m_part = part;

    connect( part, SIGNAL( started( KIO::Job *) ),
	     this, SLOT( slotChildStarted( KIO::Job *) ) );
    connect( part, SIGNAL( completed() ),
	     this, SLOT( slotChildCompleted() ) );

    child->m_extension = (KParts::BrowserExtension *)part->child( 0L, "KParts::BrowserExtension" );

    if ( child->m_extension )
    {
      connect( child->m_extension, SIGNAL( openURLRequest( const KURL &, const KParts::URLArgs & ) ),
	       this, SLOT( slotChildURLRequest( const KURL &, const KParts::URLArgs & ) ) );
      connect( child->m_extension, SIGNAL( createNewWindow( const KURL &, const KParts::URLArgs & ) ),
	       d->m_extension, SIGNAL( createNewWindow( const KURL &, const KParts::URLArgs & ) ) );
    }

    connect( part, SIGNAL( setStatusBarText( const QString & ) ),
	     this, SIGNAL( setStatusBarText( const QString & ) ) );
  }

  child->m_bCompleted = false;
  if ( child->m_extension )
    child->m_extension->setURLArgs( child->m_args );

  child->m_part->openURL( url );
}

KParts::ReadOnlyPart *KHTMLPart::createFrame( QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, const QString &mimetype, QStringList &serviceTypes )
{
  KTrader::OfferList offers = KTrader::self()->query( mimetype, "'KParts/ReadOnlyPart' in ServiceTypes" );

  assert( offers.count() >= 1 );
  
  KService::Ptr service = *offers.begin();
  
  KLibFactory *factory = KLibLoader::self()->factory( service->library() );
  
  if ( !factory )
    return 0L;
  
  KParts::ReadOnlyPart *res = 0L;
  
  if ( factory->inherits( "KParts::Factory" ) )
    res = static_cast<KParts::ReadOnlyPart *>(static_cast<KParts::Factory *>( factory )->createPart( parentWidget, widgetName, parent, name, "KParts::ReadOnlyPart" ));
  else
  res = static_cast<KParts::ReadOnlyPart *>(factory->create( parentWidget, widgetName, "KParts::ReadOnlyPart" ));
  
  if ( !res )
    return res;
  
  serviceTypes = service->serviceTypes();
  
  return res;
}

KParts::PartManager *KHTMLPart::partManager()
{
  if ( !d->m_manager )
  {
    d->m_manager = new KParts::PartManager( d->m_widget );
    connect( d->m_manager, SIGNAL( activePartChanged( KParts::Part * ) ),
	     this, SLOT( updateActions() ) );
  }

  return d->m_manager;
}

void KHTMLPart::submitForm( const char *action, const QString &url, const QCString &formData, const QString &_target )
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

  if ( strcmp( action, "get" ) == 0 )
  {
    QString query = u.query();
    if ( !query.isEmpty() )
      query = query + "&";

    query.append( QString::fromLatin1( formData ) ); // HTMLFormElementImpl calls formData.latin1(), so I assume that this
                                                     // conversion is ok

    u.setQuery( query );

    KParts::URLArgs args;
    args.frameName = target;
    emit d->m_extension->openURLRequest( u, args );
  }
  else
  {
    QByteArray pdata( formData.length() );
    memcpy( pdata.data(), formData.data(), formData.length() );

    KParts::URLArgs args;
    args.postData = pdata;
    args.frameName = target;

    emit d->m_extension->openURLRequest( u, args );
  }
}

void KHTMLPart::popupMenu( const QString &url )
{
  KURL u( m_url );
  if ( !url.isEmpty() )
    u = url;

  mode_t mode = 0;
  if ( !u.isLocalFile() )
  {
    QString cURL = u.url( 1 );
    int i = cURL.length();
    // A url ending with '/' is always a directory
    if ( i >= 1 && cURL[ i - 1 ] == '/' )
      mode = S_IFDIR;
  }
  emit d->m_extension->popupMenu( QCursor::pos(), u, QString::fromLatin1( "text/html" ), mode );
}

void KHTMLPart::slotChildStarted( KIO::Job *job )
{
  khtml::ChildFrame *child = frame( sender() );

  assert( child );

  child->m_bCompleted = false;

  if ( d->m_bComplete )
    emit started( job );
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

  if ( !args.frameName.isEmpty() )
  {
    static QString _top = QString::fromLatin1( "_top" );
    static QString _self = QString::fromLatin1( "_self" );
    static QString _parent = QString::fromLatin1( "_parent" );
    static QString _blank = QString::fromLatin1( "_blank" );

    QString frameName = args.frameName.lower();

    if ( frameName == _top )
    {
      emit d->m_extension->openURLRequest( url, args );
      return;
    }
    else if ( frameName == _blank )
    {
      emit d->m_extension->createNewWindow( url, args );
      return;
    }
    else if ( frameName == _parent )
    {
      KParts::URLArgs newArgs( args );
      newArgs.frameName = QString::null;

      emit d->m_extension->openURLRequest( url, newArgs );
      return;
    }
    else if ( frameName != _self )
    {
      khtml::ChildFrame *_frame = recursiveFrameRequest( url, args );

      if ( !_frame )
        return;

      child = _frame;
    }
  }

  childRequest( child, url, args );
}

khtml::ChildFrame *KHTMLPart::frame( const QObject *obj )
{
  assert( obj->inherits( "KParts::ReadOnlyPart" ) );
  const KParts::ReadOnlyPart *part = (KParts::ReadOnlyPart *)obj;

  QMap<QString,khtml::ChildFrame>::Iterator it = d->m_frames.begin();
  QMap<QString,khtml::ChildFrame>::Iterator end = d->m_frames.end();
  for (; it != end; ++it )
    if ( (KParts::ReadOnlyPart *)it.data().m_part == part )
      return &it.data();

  return 0L;
}

KHTMLPart *KHTMLPart::parentPart()
{
  if ( !parent() || !parent()->inherits( "KHTMLPart" ) )
    return 0L;

  return (KHTMLPart *)parent();
}

khtml::ChildFrame *KHTMLPart::recursiveFrameRequest( const KURL &url, const KParts::URLArgs &args, bool callParent, bool newWin )
{
  QMap<QString,khtml::ChildFrame>::Iterator it = d->m_frames.find( args.frameName );

  if ( it != d->m_frames.end() )
    return &it.data();

  it = d->m_frames.begin();
  QMap<QString,khtml::ChildFrame>::Iterator end = d->m_frames.end();
  for (; it != end; ++it )
    if ( it.data().m_part && it.data().m_part->inherits( "KHTMLPart" ) )
    {
      KHTMLPart *childPart = (KHTMLPart *)(KParts::ReadOnlyPart *)it.data().m_part;

      khtml::ChildFrame *res = childPart->recursiveFrameRequest( url, args, false );
      if ( !res )
        continue;

      childPart->childRequest( res, url, args );
      return 0L;
    }

  if ( parentPart() && callParent )
  {
    khtml::ChildFrame *res = parentPart()->recursiveFrameRequest( url, args );

    if ( res )
      parentPart()->childRequest( res, url, args );

    return 0L;
  }

  if ( newWin )
  {
    KParts::URLArgs newArgs( args );
    newArgs.frameName = QString::null; //not really necessary, but safer ;-)
    emit d->m_extension->createNewWindow( url, newArgs );
  }
  return 0L;
}

KHTMLPartBrowserExtension::KHTMLPartBrowserExtension( KHTMLPart *parent, const char *name )
: KParts::BrowserExtension( parent, name )
{
  m_part = parent;
}

int KHTMLPartBrowserExtension::xOffset()
{
  return m_part->htmlWidget()->contentsX();
}

int KHTMLPartBrowserExtension::yOffset()
{
  return m_part->htmlWidget()->contentsY();
}

void KHTMLPartBrowserExtension::saveState( QDataStream &stream )
{
  qDebug( "saveState!" );
  KParts::BrowserExtension::saveState( stream );
}

void KHTMLPartBrowserExtension::restoreState( QDataStream &stream )
{
  qDebug( "restoreState!" );
  KParts::BrowserExtension::restoreState( stream );
}

