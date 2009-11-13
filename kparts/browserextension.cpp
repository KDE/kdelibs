 /* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999 David Faure <faure@kde.org>

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
#include "browserextension.h"

#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtCore/QTimer>
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QMetaEnum>
#include <QtCore/QRegExp>
#include <QtGui/QTextDocument>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurifilter.h>
#include <kglobal.h>
#include <assert.h>

using namespace KParts;


class OpenUrlEvent::OpenUrlEventPrivate
{
public:
  OpenUrlEventPrivate( ReadOnlyPart *part,
                       const KUrl &url,
                       const OpenUrlArguments &args,
                       const BrowserArguments &browserArgs )
    : m_part( part )
    , m_url( url )
    , m_args(args)
    , m_browserArgs(browserArgs)
  {
  }
  ~OpenUrlEventPrivate()
  {
  }
  static const char *s_strOpenUrlEvent;
  ReadOnlyPart *m_part;
  KUrl m_url;
  OpenUrlArguments m_args;
  BrowserArguments m_browserArgs;
};

const char *OpenUrlEvent::OpenUrlEventPrivate::s_strOpenUrlEvent =
                        "KParts/BrowserExtension/OpenURLevent";

OpenUrlEvent::OpenUrlEvent( ReadOnlyPart *part, const KUrl &url,
                            const OpenUrlArguments &args,
                            const BrowserArguments &browserArgs )
    : Event( OpenUrlEventPrivate::s_strOpenUrlEvent )
    , d( new OpenUrlEventPrivate(part, url, args, browserArgs) )
{
}

OpenUrlEvent::~OpenUrlEvent()
{
    delete d;
}

ReadOnlyPart *OpenUrlEvent::part() const
{
    return d->m_part;
}

KUrl OpenUrlEvent::url() const
{
    return d->m_url;
}

OpenUrlArguments OpenUrlEvent::arguments() const
{
    return d->m_args;
}

BrowserArguments OpenUrlEvent::browserArguments() const
{
    return d->m_browserArgs;
}

bool OpenUrlEvent::test( const QEvent *event )
{
    return Event::test( event, OpenUrlEventPrivate::s_strOpenUrlEvent );
}

namespace KParts
{

struct BrowserArgumentsPrivate
{
    BrowserArgumentsPrivate() {
      doPost = false;
      redirectedRequest = false;
      lockHistory = false;
      newTab = false;
      forcesNewWindow = false;
    }
    QString contentType; // for POST
    bool doPost;
    bool redirectedRequest;
    bool lockHistory;
    bool newTab;
    bool forcesNewWindow;
};

}

BrowserArguments::BrowserArguments()
{
  softReload = false;
  trustedSource = false;
  d = 0; // Let's build it on demand for now
}

BrowserArguments::BrowserArguments( const BrowserArguments &args )
{
  d = 0;
  (*this) = args;
}

BrowserArguments &BrowserArguments::operator=(const BrowserArguments &args)
{
  if (this == &args) return *this;

  delete d; d= 0;

  softReload = args.softReload;
  postData = args.postData;
  frameName = args.frameName;
  docState = args.docState;
  trustedSource = args.trustedSource;

  if ( args.d )
      d = new BrowserArgumentsPrivate( * args.d );

  return *this;
}

BrowserArguments::~BrowserArguments()
{
  delete d;
  d = 0;
}

void BrowserArguments::setContentType( const QString & contentType )
{
  if (!d)
    d = new BrowserArgumentsPrivate;
  d->contentType = contentType;
}

void BrowserArguments::setRedirectedRequest( bool redirected )
{
  if (!d)
     d = new BrowserArgumentsPrivate;
  d->redirectedRequest = redirected;
}

bool BrowserArguments::redirectedRequest () const
{
  return d ? d->redirectedRequest : false;
}

QString BrowserArguments::contentType() const
{
  return d ? d->contentType : QString();
}

void BrowserArguments::setDoPost( bool enable )
{
    if ( !d )
        d = new BrowserArgumentsPrivate;
    d->doPost = enable;
}

bool BrowserArguments::doPost() const
{
    return d ? d->doPost : false;
}

void BrowserArguments::setLockHistory( bool lock )
{
  if (!d)
     d = new BrowserArgumentsPrivate;
  d->lockHistory = lock;
}

bool BrowserArguments::lockHistory() const
{
    return d ? d->lockHistory : false;
}

void BrowserArguments::setNewTab( bool newTab )
{
  if (!d)
     d = new BrowserArgumentsPrivate;
  d->newTab = newTab;
}

bool BrowserArguments::newTab() const
{
    return d ? d->newTab : false;
}

void BrowserArguments::setForcesNewWindow( bool forcesNewWindow )
{
  if (!d)
     d = new BrowserArgumentsPrivate;
  d->forcesNewWindow = forcesNewWindow;
}

bool BrowserArguments::forcesNewWindow() const
{
    return d ? d->forcesNewWindow : false;
}

namespace KParts
{

class WindowArgsPrivate : public QSharedData
{
public:
    WindowArgsPrivate()
        : x(-1), y(-1), width(-1), height(-1),
          fullscreen(false),
          menuBarVisible(true),
          toolBarsVisible(true),
          statusBarVisible(true),
          resizable(true),
          lowerWindow(false),
          scrollBarsVisible(true)
    {
    }

    // Position
    int x;
    int y;
    // Size
    int width;
    int height;
    bool fullscreen; //defaults to false
    bool menuBarVisible; //defaults to true
    bool toolBarsVisible; //defaults to true
    bool statusBarVisible; //defaults to true
    bool resizable; //defaults to true

    bool lowerWindow; //defaults to false
    bool scrollBarsVisible; //defaults to true
};

}

WindowArgs::WindowArgs()
    : d(new WindowArgsPrivate)
{
}

WindowArgs::WindowArgs( const WindowArgs &args )
    : d(args.d)
{
}

WindowArgs::~WindowArgs()
{
}

WindowArgs &WindowArgs::operator=( const WindowArgs &args )
{
    if ( this == &args ) return *this;

    d = args.d;
    return *this;
}

WindowArgs::WindowArgs( const QRect &_geometry, bool _fullscreen, bool _menuBarVisible,
                        bool _toolBarsVisible, bool _statusBarVisible, bool _resizable )
    : d(new WindowArgsPrivate)
{
    d->x = _geometry.x();
    d->y = _geometry.y();
    d->width = _geometry.width();
    d->height = _geometry.height();
    d->fullscreen = _fullscreen;
    d->menuBarVisible = _menuBarVisible;
    d->toolBarsVisible = _toolBarsVisible;
    d->statusBarVisible = _statusBarVisible;
    d->resizable = _resizable;
    d->lowerWindow = false;
}

WindowArgs::WindowArgs( int _x, int _y, int _width, int _height, bool _fullscreen,
                        bool _menuBarVisible, bool _toolBarsVisible,
                        bool _statusBarVisible, bool _resizable )
    : d(new WindowArgsPrivate)
{
    d->x = _x;
    d->y = _y;
    d->width = _width;
    d->height = _height;
    d->fullscreen = _fullscreen;
    d->menuBarVisible = _menuBarVisible;
    d->toolBarsVisible = _toolBarsVisible;
    d->statusBarVisible = _statusBarVisible;
    d->resizable = _resizable;
    d->lowerWindow = false;
}

void WindowArgs::setX(int x)
{
    d->x = x;
}

int WindowArgs::x() const
{
    return d->x;
}

void WindowArgs::setY(int y)
{
    d->y = y;
}

int WindowArgs::y() const
{
    return d->y;
}

void WindowArgs::setWidth(int w)
{
    d->width = w;
}

int WindowArgs::width() const
{
    return d->width;
}

void WindowArgs::setHeight(int h)
{
    d->height = h;
}

int WindowArgs::height() const
{
    return d->height;
}

void WindowArgs::setFullScreen(bool fs)
{
    d->fullscreen = fs;
}

bool WindowArgs::isFullScreen() const
{
    return d->fullscreen;
}

void WindowArgs::setMenuBarVisible(bool visible)
{
    d->menuBarVisible = visible;
}

bool WindowArgs::isMenuBarVisible() const
{
    return d->menuBarVisible;
}

void WindowArgs::setToolBarsVisible(bool visible)
{
    d->toolBarsVisible = visible;
}

bool WindowArgs::toolBarsVisible() const
{
    return d->toolBarsVisible;
}

void WindowArgs::setStatusBarVisible(bool visible)
{
    d->statusBarVisible = visible;
}

bool WindowArgs::isStatusBarVisible() const
{
    return d->statusBarVisible;
}

void WindowArgs::setResizable(bool resizable)
{
    d->resizable = resizable;
}

bool WindowArgs::isResizable() const
{
    return d->resizable;
}

void WindowArgs::setLowerWindow(bool lower)
{
    d->lowerWindow = lower;
}

bool WindowArgs::lowerWindow() const
{
    return d->lowerWindow;
}

void WindowArgs::setScrollBarsVisible(bool visible)
{
    d->scrollBarsVisible = visible;
}

bool WindowArgs::scrollBarsVisible() const
{
    return d->scrollBarsVisible;
}

namespace KParts
{

// Internal class, use to store the status of the actions
class KBitArray
{
public:
    int val;
    KBitArray() { val = 0; }
    bool operator [](int index) { return (val & (1 << index)) ? true : false; }
    void setBit(int index, bool value) {
        if (value) val = val | (1 << index);
        else val = val & ~(1 << index);
    }
};

class BrowserExtension::BrowserExtensionPrivate
{
public:
  BrowserExtensionPrivate( KParts::ReadOnlyPart *parent )
    : m_urlDropHandlingEnabled(false),
      m_browserInterface(0),
      m_part( parent )
  {}

  struct DelayedRequest {
    KUrl m_delayedURL;
    KParts::OpenUrlArguments m_delayedArgs;
    KParts::BrowserArguments m_delayedBrowserArgs;
  };

  QList<DelayedRequest> m_requests;
  bool m_urlDropHandlingEnabled;
  KBitArray m_actionStatus;
  QMap<int, QString> m_actionText;
  BrowserInterface *m_browserInterface;

  static void createActionSlotMap();

  KParts::ReadOnlyPart *m_part;
    OpenUrlArguments m_args;
    BrowserArguments m_browserArgs;
};

K_GLOBAL_STATIC(BrowserExtension::ActionSlotMap, s_actionSlotMap)
K_GLOBAL_STATIC(BrowserExtension::ActionNumberMap, s_actionNumberMap)

void BrowserExtension::BrowserExtensionPrivate::createActionSlotMap()
{
    s_actionSlotMap->insert( "cut", SLOT( cut() ) );
    s_actionSlotMap->insert( "copy", SLOT( copy() ) );
    s_actionSlotMap->insert( "paste", SLOT( paste() ) );
    s_actionSlotMap->insert( "print", SLOT( print() ) );
    // Tricky. Those aren't actions in fact, but simply methods that a browserextension
    // can have or not. No need to return them here.
    //s_actionSlotMap->insert( "reparseConfiguration", SLOT( reparseConfiguration() ) );
    //s_actionSlotMap->insert( "refreshMimeTypes", SLOT( refreshMimeTypes() ) );

    // Create the action-number map
    ActionSlotMap::ConstIterator it = s_actionSlotMap->constBegin();
    ActionSlotMap::ConstIterator itEnd = s_actionSlotMap->constEnd();
    for ( int i=0 ; it != itEnd ; ++it, ++i )
    {
        //kDebug(1202) << " action " << it.key() << " number " << i;
        s_actionNumberMap->insert( it.key(), i );
    }
}

}

BrowserExtension::BrowserExtension( KParts::ReadOnlyPart *parent )
: QObject( parent ), d( new BrowserExtensionPrivate(parent) )
{
  //kDebug() << "BrowserExtension::BrowserExtension() " << this;

  if (s_actionSlotMap->isEmpty())
      // Create the action-slot map
      BrowserExtensionPrivate::createActionSlotMap();

  // Build list with this extension's slot names.
  QList<QByteArray> slotNames;
  int methodCount = metaObject()->methodCount();
  int methodOffset = metaObject()->methodOffset();
  for ( int i=0 ; i < methodCount; ++i )
  {
      QMetaMethod method = metaObject()->method( methodOffset + i );
      if ( method.methodType() == QMetaMethod::Slot )
          slotNames.append( method.signature() );
  }

  // Set the initial status of the actions depending on whether
  // they're supported or not
  ActionSlotMap::ConstIterator it = s_actionSlotMap->constBegin();
  ActionSlotMap::ConstIterator itEnd = s_actionSlotMap->constEnd();
  for ( int i=0 ; it != itEnd ; ++it, ++i )
  {
      // Does the extension have a slot with the name of this action ?
      // ######### KDE4 TODO: use QMetaObject::indexOfMethod() #######
      d->m_actionStatus.setBit( i, slotNames.contains( it.key()+"()" ) );
  }

  connect( d->m_part, SIGNAL( completed() ),
           this, SLOT( slotCompleted() ) );
  connect( this, SIGNAL( openUrlRequest( const KUrl &, const KParts::OpenUrlArguments &, const KParts::BrowserArguments & ) ),
           this, SLOT( slotOpenUrlRequest( const KUrl &, const KParts::OpenUrlArguments &, const KParts::BrowserArguments & ) ) );
  connect( this, SIGNAL( enableAction( const char *, bool ) ),
           this, SLOT( slotEnableAction( const char *, bool ) ) );
  connect( this, SIGNAL( setActionText( const char *, const QString& ) ),
           this, SLOT( slotSetActionText( const char *, const QString& ) ) );
}

BrowserExtension::~BrowserExtension()
{
  //kDebug() << "BrowserExtension::~BrowserExtension() " << this;
  delete d;
}

void BrowserExtension::setBrowserArguments( const BrowserArguments &args )
{
  d->m_browserArgs = args;
}

BrowserArguments BrowserExtension::browserArguments() const
{
  return d->m_browserArgs;
}

int BrowserExtension::xOffset()
{
  return 0;
}

int BrowserExtension::yOffset()
{
  return 0;
}

void BrowserExtension::saveState( QDataStream &stream )
{
    // TODO add d->m_part->mimeType()
  stream << d->m_part->url() << (qint32)xOffset() << (qint32)yOffset();
}

void BrowserExtension::restoreState( QDataStream &stream )
{
  KUrl u;
  qint32 xOfs, yOfs;
  stream >> u >> xOfs >> yOfs;

    OpenUrlArguments args;
    args.setXOffset(xOfs);
    args.setYOffset(yOfs);
    // TODO add args.setMimeType
    d->m_part->setArguments(args);
    d->m_part->openUrl(u);
}

bool BrowserExtension::isURLDropHandlingEnabled() const
{
    return d->m_urlDropHandlingEnabled;
}

void BrowserExtension::setURLDropHandlingEnabled( bool enable )
{
    d->m_urlDropHandlingEnabled = enable;
}

void BrowserExtension::slotCompleted()
{
  //empty the argument stuff, to avoid bogus/invalid values when opening a new url
    setBrowserArguments( BrowserArguments() );
}

void BrowserExtension::pasteRequest()
{
    QString plain( "plain" );
    QString url = QApplication::clipboard()->text(plain, QClipboard::Selection).trimmed();
    // Remove linefeeds and any whitespace surrounding it.
    url.remove(QRegExp("[\\ ]*\\n+[\\ ]*"));

    // Check if it's a URL
    QStringList filters = KUriFilter::self()->pluginNames();
    filters.removeAll( "kuriikwsfilter" );
    filters.removeAll( "localdomainurifilter" );
    KUriFilterData filterData;
    filterData.setData( url );
    filterData.setCheckForExecutables( false );
    if ( KUriFilter::self()->filterUri( filterData, filters ) )
    {
        switch ( filterData.uriType() )
	{
	    case KUriFilterData::LocalFile:
	    case KUriFilterData::LocalDir:
	    case KUriFilterData::NetProtocol:
	        slotOpenUrlRequest( filterData.uri() );
		break;
	    case KUriFilterData::Error:
		KMessageBox::sorry( d->m_part->widget(), filterData.errorMsg() );
		break;
	    default:
		break;
	}
    }
    else if ( KUriFilter::self()->filterUri( filterData,
                    QStringList( QLatin1String( "kuriikwsfilter" ) ) ) &&
              url.length() < 250 )
    {
        if ( KMessageBox::questionYesNo( d->m_part->widget(),
		    i18n( "<qt>Do you want to search the Internet for <b>%1</b>?</qt>" ,  Qt::escape(url) ),
		    i18n( "Internet Search" ), KGuiItem( i18n( "&Search" ), "edit-find"),
		    KStandardGuiItem::cancel(), "MiddleClickSearch" ) == KMessageBox::Yes)
          slotOpenUrlRequest( filterData.uri() );
    }
}

void BrowserExtension::slotOpenUrlRequest( const KUrl &url, const KParts::OpenUrlArguments& args, const KParts::BrowserArguments &browserArgs )
{
    //kDebug() << this << " BrowserExtension::slotOpenURLRequest(): url=" << url.url();
    BrowserExtensionPrivate::DelayedRequest req;
    req.m_delayedURL = url;
    req.m_delayedArgs = args;
    req.m_delayedBrowserArgs = browserArgs;
    d->m_requests.append( req );
    QTimer::singleShot( 0, this, SLOT( slotEmitOpenUrlRequestDelayed() ) );
}

void BrowserExtension::slotEmitOpenUrlRequestDelayed()
{
    if (d->m_requests.isEmpty()) return;
    BrowserExtensionPrivate::DelayedRequest req = d->m_requests.front();
    d->m_requests.pop_front();
    emit openUrlRequestDelayed( req.m_delayedURL, req.m_delayedArgs, req.m_delayedBrowserArgs );
    // tricky: do not do anything here! (no access to member variables, etc.)
}

void BrowserExtension::setBrowserInterface( BrowserInterface *impl )
{
    d->m_browserInterface = impl;
}

BrowserInterface *BrowserExtension::browserInterface() const
{
    return d->m_browserInterface;
}

void BrowserExtension::slotEnableAction( const char * name, bool enabled )
{
    //kDebug() << "BrowserExtension::slotEnableAction " << name << " " << enabled;
    ActionNumberMap::ConstIterator it = s_actionNumberMap->constFind( name );
    if ( it != s_actionNumberMap->constEnd() )
    {
        d->m_actionStatus.setBit( it.value(), enabled );
        //kDebug() << "BrowserExtension::slotEnableAction setting bit " << it.data() << " to " << enabled;
    }
    else
        kWarning() << "BrowserExtension::slotEnableAction unknown action " << name;
}

bool BrowserExtension::isActionEnabled( const char * name ) const
{
    int actionNumber = (*s_actionNumberMap)[ name ];
    return d->m_actionStatus[ actionNumber ];
}

void BrowserExtension::slotSetActionText( const char * name, const QString& text )
{
    kDebug() << "BrowserExtension::slotSetActionText " << name << " " << text;
    ActionNumberMap::ConstIterator it = s_actionNumberMap->constFind( name );
    if ( it != s_actionNumberMap->constEnd() )
    {
        d->m_actionText[ it.value() ] = text;
    }
    else
        kWarning() << "BrowserExtension::slotSetActionText unknown action " << name;
}

QString BrowserExtension::actionText( const char * name ) const
{
    int actionNumber = (*s_actionNumberMap)[ name ];
    QMap<int, QString>::ConstIterator it = d->m_actionText.constFind( actionNumber );
    if ( it != d->m_actionText.constEnd() )
        return *it;
    return QString();
}

// for compatibility
BrowserExtension::ActionSlotMap BrowserExtension::actionSlotMap()
{
    return *actionSlotMapPtr();
}

BrowserExtension::ActionSlotMap * BrowserExtension::actionSlotMapPtr()
{
    if (s_actionSlotMap->isEmpty())
        BrowserExtensionPrivate::createActionSlotMap();
    return s_actionSlotMap;
}

BrowserExtension *BrowserExtension::childObject( QObject *obj )
{
    return KGlobal::findDirectChild<KParts::BrowserExtension *>(obj);
}

namespace KParts
{

class BrowserHostExtension::BrowserHostExtensionPrivate
{
public:
  BrowserHostExtensionPrivate()
  {
  }
  ~BrowserHostExtensionPrivate()
  {
  }

  KParts::ReadOnlyPart *m_part;
};

}

BrowserHostExtension::BrowserHostExtension( KParts::ReadOnlyPart *parent )
 : QObject( parent ), d( new BrowserHostExtensionPrivate )
{
  d->m_part = parent;
}

BrowserHostExtension::~BrowserHostExtension()
{
  delete d;
}

QStringList BrowserHostExtension::frameNames() const
{
  return QStringList();
}

const QList<KParts::ReadOnlyPart*> BrowserHostExtension::frames() const
{
  return QList<KParts::ReadOnlyPart*>();
}

bool BrowserHostExtension::openUrlInFrame( const KUrl &,
                                           const KParts::OpenUrlArguments&,
                                           const KParts::BrowserArguments & )
{
  return false;
}

BrowserHostExtension *BrowserHostExtension::childObject( QObject *obj )
{
    return KGlobal::findDirectChild<KParts::BrowserHostExtension *>(obj);
}

BrowserHostExtension *
BrowserHostExtension::findFrameParent(KParts::ReadOnlyPart *callingPart, const QString &frame)
{
    Q_UNUSED(callingPart);
    Q_UNUSED(frame);
    return 0;
}

LiveConnectExtension::LiveConnectExtension( KParts::ReadOnlyPart *parent )
 : QObject( parent ), d( 0 ) {}

LiveConnectExtension::~LiveConnectExtension() {}

bool LiveConnectExtension::get( const unsigned long, const QString &, Type &, unsigned long &, QString & ) {
    return false;
}

bool LiveConnectExtension::put( const unsigned long, const QString &, const QString & ) {
    return false;
}

bool LiveConnectExtension::call( const unsigned long, const QString &, const QStringList &, Type &, unsigned long &, QString & ) {
    return false;
}

void LiveConnectExtension::unregister( const unsigned long ) {}

LiveConnectExtension *LiveConnectExtension::childObject( QObject *obj )
{
    return KGlobal::findDirectChild<KParts::LiveConnectExtension *>(obj);
}

#include "browserextension.moc"
