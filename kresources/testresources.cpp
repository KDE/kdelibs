#include <kdebug.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include "resource.h"
#include "resourcemanager.h"

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
    
    void dump() const { kdDebug() << "DUMP: TestSubResource" << endl; }
};

int main( int argc, char **argv )
{
  KAboutData aboutData( "testresources", "Kresource Test", "0" );
  KCmdLineArgs::init( argc, argv, &aboutData );

  KApplication app;

  ResourceManager<TestResource> manager( "test" );
  
  TestResource *resource1 = new TestResource;  
  manager.add( resource1 );
  resource1->dump();

  TestResource *resource2 = new TestSubResource;  
  manager.add( resource2 );
  resource2->dump();

  kdDebug() << "List Resources:" << endl;
  ResourceManager<TestResource>::Iterator it;
  for( it = manager.begin(); it != manager.end(); ++it ) {
    (*it)->dump();
  }
}
