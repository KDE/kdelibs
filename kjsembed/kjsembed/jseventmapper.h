// -*- c++ -*-

/*
*  Copyright (C) 2001-2003, Richard J. Moore <rich@kde.org>
*  Copyright (C) 2005, Ian Reinhart Geiser <geiseri@kde.org>
*  Copyright (C) 2006, Matt Broadstone <mbroadst@gmail.com>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Library General Public License for more details.
*
*  You should have received a copy of the GNU Library General Public License
*  along with this library; see the file COPYING.LIB.  If not, write to
*  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
*  Boston, MA 02111-1307, USA.
*/

#ifndef JSEVENTMAPPER_H
#define JSEVENTMAPPER_H

#include <QHash>
#include <QEvent>

#include <kjs/identifier.h>

namespace KJSEmbed {

/**
* Maintains a map between the types of QEvent and the names of their event
* handlers.  This class handles all of the built in Qt events. To add your own custom events you can simply
* call mapper()->addEvent(KJS::Identifier("onMyCustomEvent"), QEvent::User + 1).  Then in your javascript
* you can just do the following:
* @code
* var myQObject = new QObjectBinding(this);
* myQObject.onMyCustomEvent = function () { //do stuff ;}
* @endcode
* 
* In cases where you need to handle properties on your custom event, you will need to provide a binding to
* that event and KJSEmbed will create an instance of it for you as the argument in your event handler.
* @author Richard Moore, rich@kde.org
* @author Ian Reinhart Geiser, geiseri@kde.org
*/
class JSEventMapper
{
public:
    virtual ~JSEventMapper();

    /**
    * Adds an event to the map. The event handler has the specified name, and
    * the event has the specified type.
    */
    void addEvent( const KJS::Identifier &name, QEvent::Type t );

    /** Returns true iff the specified name is the identifier for an event handler. */
    bool isEventHandler( const KJS::Identifier &name ) const;

    /** Returns the type of the events handled by the specified handler. */
    QEvent::Type findEventType( const KJS::Identifier &name ) const;

    KJS::Identifier findEventHandler( QEvent::Type t ) const;

    /**
    * Return the global event mapper.
    */
    static JSEventMapper *mapper();

private:
    JSEventMapper();
    static JSEventMapper *m_inst;
    QHash<QString,QEvent::Type> m_handlerToEvent;
    QHash<int,KJS::Identifier> m_eventToHandler;
};

} // namespace KJSEmbed

#endif // JSEVENTMAPPER_H

