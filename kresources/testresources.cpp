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

int main( int argc, char **argv )
{
  KAboutData aboutData( "testresources", "Kresource Test", "0" );
  KCmdLineArgs::init( argc, argv, &aboutData );

  KApplication app;

  ResourceManager<TestResource> manager( "test" );
  
  TestResource *resource1 = new TestResource;  
  manager.add( resource1 );

  TestResource *resource2 = new TestResource;  
  manager.add( resource2 );

  QPtrList<TestResource> resources = manager.resources();
  
  TestResource *r;
  for( r = resources.first(); r; r = resources.next() ) {
    r->dump();
  }  
}
