
#include "factory.h"
#include "part.h"

#include <assert.h>

using namespace KParts;

Factory::Factory( QObject *parent, const char *name )
: KLibFactory( parent, name )
{
}

Factory::~Factory()
{
}

QObject *Factory::create( QObject *parent, const char *name, const char *classname, const QStringList &args )
{
  assert( parent->isWidgetType() );
  return createPart( (QWidget *)parent, name, parent, name, classname, args );
} 
