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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
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
