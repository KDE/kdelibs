
#include "kevent.h"

using namespace KParts;

//the answer!
#define KPARTS_EVENT_MAGIC 42

Event::Event( const char *eventName )
 : QCustomEvent( (QEvent::Type)(QEvent::User + KPARTS_EVENT_MAGIC), (void *)eventName )
{
}

const char *Event::eventName() const
{
  if ( !test( this ) )
    return 0L;
  
  return (const char *)data();
} 

bool Event::test( const QEvent *event )
{
  if ( !event )
    return false;
  
  return ( event->type() == (QEvent::Type)(QEvent::User + KPARTS_EVENT_MAGIC ) );
} 

bool Event::test( const QEvent *event, const char *name )
{
  if ( !test( event ) )
    return false;
  
  return ( strcmp( name, (const char *)((QCustomEvent *)event)->data() ) == 0 );
} 
