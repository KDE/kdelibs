#ifndef __kparts_event_h__
#define __kparts_event_h__

#include <qevent.h>

namespace KParts
{

class Event : public QCustomEvent
{
public:
  Event( const char *eventName );

  virtual const char *eventName() const;

  static bool test( const QEvent *event );
  static bool test( const QEvent *event, const char *name );
};

};

#endif
