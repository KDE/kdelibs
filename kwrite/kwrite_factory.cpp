#include <klocale.h>
#include <kstddirs.h>
#include <kinstance.h>
#include <kaboutdata.h>

#include <kwrite/kwrite_factory.h>
#include <kwrite/kwrite.h>
#include <kwrite/kwrite_part.h>

#include "highlight.h"

extern "C"
{
    void* init_libkwrite()
    {
	return new KWriteFactory;
    }
};

KInstance* KWriteFactory::s_instance = 0L;
KAboutData* KWriteFactory::s_about = 0L;

KWriteFactory::KWriteFactory( QObject* parent, const char* name )
    : KParts::Factory( parent, name )
{
  HlManager::incRef(); 
}

KWriteFactory::~KWriteFactory()
{
  HlManager::decRef(); 
  delete s_instance;
  delete s_about;
}

KParts::Part* KWriteFactory::createPart( QWidget *parentWidget, const char *, QObject* parent, const char* /*name*/, const char* classname, const QStringList & )
{
  bool bBrowser = ( strcmp( classname, "Browser/View" ) == 0 );

  bool bROP = ( strcmp( classname, "KParts::ReadOnlyPart" ) == 0 );

  KWrite* obj = new KWrite( parentWidget, parent, (bBrowser) ? KWrite::kBrowser : 0 );

  if ( bBrowser || bROP )
    obj->setReadWrite( false );

  emit objectCreated( obj );
  return obj;
}

KInstance* KWriteFactory::instance()
{
  if( !s_instance )
  {
    s_about = new KAboutData( "kwrite", I18N_NOOP( "KWrite" ), "2.0pre" );
    s_instance = new KInstance( s_about );
  }

  return s_instance;
}
