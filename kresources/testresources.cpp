#include <kdebug.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include "resource.h"
#include "manager.h"

using namespace KRES;

class TestResource : public Resource
{
  public:
    TestResource() : Resource( 0 ) {}

};

class TestSubResource : public TestResource
{
  public:
    TestSubResource() : TestResource() {}
    
    void dump() const
    {
      kdDebug() << "TestSubResource" << endl;
      TestResource::dump();
    }
};

int main( int argc, char **argv )
{
  KAboutData aboutData( "testresources", "Kresource Test", "0" );
  KCmdLineArgs::init( argc, argv, &aboutData );

  KApplication app;

  Manager<TestResource> manager( "test" );
  
  TestResource *resource1 = new TestResource;  
  resource1->setResourceName( "One" );
  manager.add( resource1 );

  TestResource *resource2 = new TestSubResource;  
  resource2->setResourceName( "Two" );
  manager.add( resource2 );

  TestResource *resource3 = new TestSubResource;  
  resource3->setResourceName( "Three" );
  manager.add( resource3 );

  kdDebug() << "LIST ALL:" << endl;
  Manager<TestResource>::Iterator it;
  for( it = manager.begin(); it != manager.end(); ++it ) {
    (*it)->dump();
  }

  resource2->setActive( false );
  resource3->setActive( true );
  
  kdDebug() << "LIST ACTIVE" << endl;
  Manager<TestResource>::ActiveIterator it2;
  for( it2 = manager.activeBegin(); it2 != manager.activeEnd(); ++it2 ) {
    (*it2)->dump();
  }

  resource1->setActive( false );
  resource2->setActive( true );
  resource3->setActive( true );
  
  kdDebug() << "LIST ACTIVE" << endl;
  for( it2 = manager.activeBegin(); it2 != manager.activeEnd(); ++it2 ) {
    (*it2)->dump();
  }

  kdDebug() << "LIST ALL" << endl;
  for( it = manager.begin(); it != manager.end(); ++it ) {
    (*it)->dump();
  }


}
