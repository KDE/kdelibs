/* This file is part of the KDE libraries
    Copyright (C) 2003,2004,2005,2006 Ian Reinhart Geiser <geiseri@kde.org>
    Copyright (C) 2003,2004,2005,2006 Matt Broadstone <mbroadst@gmail.com>
    Copyright (C) 2003,2004,2005,2006 Richard J. Moore <rich@kde.org>
    Copyright (C) 2003,2004,2005,2006 Erik L. Bunce <kde@bunce.us>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "eventproxy.h"

#include  <QtCore/QCoreApplication>

#include "qobject_binding.h"
#include <kjs/interpreter.h>

#include "kjseglobal.h"
#include "jseventmapper.h"
#include "jseventutils.h"

using namespace KJSEmbed;

EventProxy::EventProxy( QObjectBinding *watch, KJS::Interpreter *interpreter ) :
    QObject(watch->object<QObject>()), m_watch(watch), m_interpreter(interpreter)
{
    m_refcount = 0l;
}

EventProxy::~EventProxy()
{
}

bool EventProxy::isFiltered( QEvent::Type t ) const
{
    if ( m_eventMask.size() <= t )
        return false;
    return m_eventMask.testBit( t );
}

void EventProxy::addFilter( QEvent::Type t )
{
    if( t == QEvent::None )
        return;
    if ( !m_refcount )
        m_watch->object<QObject>()->installEventFilter( this );

    if ( m_eventMask.size() <= t )
        m_eventMask.resize( t + 1);

    if ( !m_eventMask.testBit(t) )
    {
        m_refcount++;
        m_eventMask.setBit( t );
    }
}

void EventProxy::removeFilter( QEvent::Type t )
{
    if( t == QEvent::None )
        return;
    if ( m_eventMask.size() <= t )
        return;
    m_eventMask.clearBit( t );
    m_refcount--;
    if ( !m_refcount )
    {
        m_watch->object<QObject>()->removeEventFilter( this );
        deleteLater();
    }
}

bool EventProxy::eventFilter( QObject * /*watched*/, QEvent *e )
{
    if ( isFiltered(e->type()) )
    {
        return !callHandler( e );
    }
    return false;
}

bool EventProxy::callHandler( QEvent *e )
{
// Be careful enabling this as if there are a lot of events then the event loop times
// out and the app crashes with 'Alarm Clock'.
//    qDebug("JSObjectEventProxy::callHandler() event type %d" , e->type() );

    KJS::ExecState *exec = m_interpreter->globalExec();
    KJS::Identifier id = JSEventMapper::mapper()->findEventHandler( e->type() );

    KJS::JSObject *jsobj(m_watch);
    KJS::JSObject *fun = jsobj->get(exec, id )->toObject( exec );

    KJS::JSValue *retValue;
    if ( !fun->implementsCall() )
    {
        QString msg = i18n( "Bad event handler: Object %1 Identifier %2 Method %3 Type: %4.",
          jsobj->className().ascii(),
          id.ascii(),
          fun->className().ascii(),
          e->type());
        retValue = throwError(exec, KJS::TypeError, msg);
    }
    else
    {
        // Process args
        KJS::List args;
        args.append( JSEventUtils::event(exec, e) );
        
        // Call handler
        retValue = fun->call( exec, jsobj, args );
    }
    
    if ( exec->hadException() ) 
    {
        if (m_interpreter->shouldPrintExceptions())
        {
            KJS::JSLock lock;
            KJS::JSObject* exceptObj = retValue->toObject(exec);
            QString message = toQString(exceptObj->toString(exec));
            QString sourceURL = toQString(exceptObj->get(exec, "sourceURL")->toString(exec));
            int sourceId = exceptObj->get(exec, "sourceId")->toUInt32(exec);
            int line = exceptObj->get(exec, "line")->toUInt32(exec);
            (*KJSEmbed::conerr()) << i18n("Exception calling '%1' function from %2:%3:%4", id.ascii(), !sourceURL.isEmpty() ? sourceURL : QString::number(sourceId), line, message) << endl;
        }
        
        
        // clear it so it doesn't stop other things
        exec->clearException();
        return false;
    }
  
    return true;
}

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
