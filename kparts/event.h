#ifndef __kparts_event_h__
#define __kparts_event_h__

#include <qevent.h>

class QWidget;

namespace KParts
{
class Part;

class Event : public QCustomEvent
{
public:
  Event( const char *eventName );

  virtual const char *eventName() const;

  static bool test( const QEvent *event );
  static bool test( const QEvent *event, const char *name );
};

class GUIActivateEvent : public Event
{
public:
  GUIActivateEvent( bool activated ) : Event( s_strGUIActivateEvent ), m_bActivated( activated ) {}

  bool activated() const { return m_bActivated; }

  static bool test( const QEvent *event ) { return Event::test( event, s_strGUIActivateEvent ); }

private:
  static const char *s_strGUIActivateEvent;
  bool m_bActivated;
};

class PartActivateEvent : public Event
{
public:
  PartActivateEvent( bool activated, Part *part, QWidget *widget ) : Event( s_strPartActivateEvent ), m_bActivated( activated ), m_part( part ), m_widget( widget ) {}

  bool activated() const { return m_bActivated; }

  Part *part() const { return m_part; }
  QWidget *widget() const { return m_widget; }

  static bool test( const QEvent *event ) { return Event::test( event, s_strPartActivateEvent ); }

private:
  static const char *s_strPartActivateEvent;
  bool m_bActivated;
  Part *m_part;
  QWidget *m_widget;
};

class PartSelectEvent : public Event
{
public:
  PartSelectEvent( bool selected, Part *part, QWidget *widget ) : Event( s_strPartSelectEvent ), m_bSelected( selected ), m_part( part ), m_widget( widget ) {}

  bool selected() const { return m_bSelected; }

  Part *part() const { return m_part; }
  QWidget *widget() const { return m_widget; }

  static bool test( const QEvent *event ) { return Event::test( event, s_strPartSelectEvent ); }

private:
  static const char *s_strPartSelectEvent;
  bool m_bSelected;
  Part *m_part;
  QWidget *m_widget;
};

};

#endif
