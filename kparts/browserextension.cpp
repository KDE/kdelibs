
#include "browserextension.h"

using namespace KParts;

const char *OpenURLEvent::s_strOpenURLEvent = "KParts/BrowserExtension/OpenURLevent";

namespace KParts
{

struct URLArgsPrivate
{
};

};

URLArgs::URLArgs()
{
  reload = false;
  xOffset = 0;
  yOffset = 0;
  d = 0;
}

URLArgs::URLArgs( const URLArgs &args )
{
  reload = args.reload;
  xOffset = args.xOffset;
  yOffset = args.yOffset;
  serviceType = args.serviceType;
  postData = args.postData;
  frameName = args.frameName;
  d = 0;
  
  //copy contents of args.d here
  /*
    if ( args.d )
    {
      d = new URLArgsPrivate;
      d->foo = args.foo;
      etc.
    }
   */
}

URLArgs::URLArgs( bool _reload, int _xOffset, int _yOffset, const QString &_serviceType )
{
  reload = _reload;
  xOffset = _xOffset;
  yOffset = _yOffset;
  serviceType = _serviceType;
  d = 0;
} 

URLArgs::~URLArgs()
{
  if ( d )
    delete d;
} 

namespace KParts
{

class BrowserExtensionPrivate
{
public: 
  BrowserExtensionPrivate()
  {
  }
  ~BrowserExtensionPrivate()
  {
  }
};

};

BrowserExtension::BrowserExtension( KParts::ReadOnlyPart *parent,
				    const char *name )
: QObject( parent, name), m_part( parent )
{
  d = new BrowserExtensionPrivate;

  connect( m_part, SIGNAL( completed() ),
	   this, SLOT( slotCompleted() ) );
}

BrowserExtension::~BrowserExtension()
{
  delete d; 
}

void BrowserExtension::setURLArgs( const URLArgs &args )
{
  m_args = args; 
} 

URLArgs BrowserExtension::urlArgs()
{
  return m_args;
}

void BrowserExtension::setXYOffset( int, int )
{
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
  stream << m_part->url() << (Q_INT32)xOffset() << (Q_INT32)yOffset();
} 

void BrowserExtension::restoreState( QDataStream &stream )
{
  KURL u;
  Q_INT32 xOfs, yOfs;
  stream >> u >> xOfs >> yOfs;
  m_part->openURL( u );
  setXYOffset( xOfs, yOfs );
}

void BrowserExtension::slotCompleted()
{
  //empty the argument stuff, to avoid bogus/invalid values when opening a new url
  setURLArgs( URLArgs() ); 
} 
