/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 Ian Reinhart Geiser <geiseri@kde.org>
    Copyright (C) 2005, 2006 Matt Broadstone <mbroadst@gmail.com>
    Copyright (C) 2005, 2006 Richard J. Moore <rich@kde.org>
    Copyright (C) 2005, 2006 Erik L. Bunce <kde@bunce.us>

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
#include "binding_support.h"
#include <kjs/interpreter.h>


using namespace KJSEmbed;

ProxyBinding::ProxyBinding( KJS::ExecState *exec )
    : KJS::JSObject(exec->lexicalInterpreter()->builtinObjectPrototype())
{

}

QString KJSEmbed::extractQString( KJS::ExecState *exec, const KJS::List &args, int idx, const QString defaultValue )
{
    if( args.size() > idx )
    {
        return extractQString( exec, args[idx] );
    }
    return defaultValue;
}

QString KJSEmbed::extractQString( KJS::ExecState *exec, KJS::JSValue *value, const QString defaultValue )
{
    if( !value )
        return defaultValue;
    return toQString(value->toString(exec));
}

QByteArray KJSEmbed::extractQByteArray( KJS::ExecState *exec, const KJS::List &args, int idx, const QByteArray &defaultValue )
{
    if( args.size() > idx )
    {
        return extractQByteArray( exec, args[idx] );
    }
    return defaultValue;
}

QByteArray KJSEmbed::extractQByteArray( KJS::ExecState *exec, KJS::JSValue *value, const QByteArray &defaultValue  )
{
    if( !value )
        return defaultValue;
    return toQString(value->toString(exec)).toLatin1();
}

KJS::JSValue *KJSEmbed::createQByteArray( KJS::ExecState *exec, const QByteArray &value )
{
    Q_UNUSED( exec );
    return KJS::jsString( value.data() );
}

int KJSEmbed::extractInt( KJS::ExecState *exec, const KJS::List &args, int idx, int defaultValue )
{
    if( args.size() > idx )
    {
        return extractInt( exec, args[idx] );
    }
    return defaultValue;
}

int KJSEmbed::extractInt( KJS::ExecState *exec, KJS::JSValue *value, int defaultValue )
{
    if( !value )
        return defaultValue;
    return int( value->toInteger(exec) );
}

KJS::JSValue *KJSEmbed::createQString( KJS::ExecState *exec, const QString &value )
{
    Q_UNUSED( exec );
    return KJS::jsString( toUString(value) );
}

KJS::JSValue *KJSEmbed::createInt( KJS::ExecState *exec, int value  )
{
    Q_UNUSED( exec );
    return KJS::jsNumber( value );
}

double KJSEmbed::extractDouble( KJS::ExecState *exec, const KJS::List &args, int idx, double defaultValue )
{
    if( args.size() > idx )
    {
        return extractDouble( exec, args[idx] );
    }
    return defaultValue;
}

double KJSEmbed::extractDouble( KJS::ExecState *exec, KJS::JSValue *value, double defaultValue )
{
    if( !value )
        return defaultValue;
    return (double) value->toNumber(exec);
}


KJS::JSValue *KJSEmbed::createDouble( KJS::ExecState *exec, double value  )
{
    Q_UNUSED( exec );
    return KJS::jsNumber( value );
}


float KJSEmbed::extractFloat( KJS::ExecState *exec, const KJS::List &args, int idx, float defaultValue  )
{
    if( args.size() > idx )
    {
        return extractFloat( exec, args[idx] );
    }
    return defaultValue;
}


float KJSEmbed::extractFloat( KJS::ExecState *exec, KJS::JSValue *value, float defaultValue )
{
    if( !value )
        return defaultValue;
    return (float) value->toNumber(exec);
}


KJS::JSValue *KJSEmbed::createFloat( KJS::ExecState *exec, float value  )
{
    Q_UNUSED( exec );
    return KJS::jsNumber( value );
}


bool KJSEmbed::extractBool( KJS::ExecState *exec, const KJS::List &args, int idx, bool defaultValue )
{
    if( args.size() > idx )
    {
        return extractBool( exec, args[idx] );
    }
    return defaultValue;
}


bool KJSEmbed::extractBool( KJS::ExecState *exec, KJS::JSValue *value, bool defaultValue )
{
    if( !value )
        return defaultValue;
    return value->toBoolean(exec);
}


KJS::JSValue *KJSEmbed::createBool( KJS::ExecState *exec, bool value  )
{
    Q_UNUSED( exec );
    return KJS::jsBoolean( value );
}


QDateTime KJSEmbed::extractQDateTime( KJS::ExecState* /* exec */, const KJS::List& /* args */, int /* idx */, const QDateTime& /* defaultValue */  )
{
    return QDateTime();
}


QDateTime KJSEmbed::extractQDateTime( KJS::ExecState* /* exec */, KJS::JSValue* /* value */, const QDateTime& /* defaultValue */ )
{
    return QDateTime();
}


KJS::JSValue *KJSEmbed::createQDateTime( KJS::ExecState* /* exec */, const QDateTime& /* value */  )
{
//    return new KJS::JSValue();
    return 0;
}


QDate KJSEmbed::extractQDate( KJS::ExecState* /* exec */, const KJS::List& /* args */, int /* idx */, const QDate& /* defaultValue */ )
{
    return QDate();
}


QDate KJSEmbed::extractQDate( KJS::ExecState* /*exec*/, KJS::JSValue* /*value*/, const QDate& /*defaultValue*/ )
{
    return QDate();
}


KJS::JSValue *KJSEmbed::createQDate( KJS::ExecState* /*exec*/, const QDate& /*value*/  )
{
//    return new KJS::JSValue();
    return 0;
}


QTime KJSEmbed::extractQTime( KJS::ExecState* /*exec*/, const KJS::List& /*args*/, int /*idx*/, const QTime& /*defaultValue*/  )
{
    return QTime();
}


QTime KJSEmbed::extractQTime( KJS::ExecState * /*exec*/, KJS::JSValue* /*value*/, const QTime &/*defaultValue*/  )
{
    return QTime();
}


KJS::JSValue *KJSEmbed::createQTime( KJS::ExecState * /*exec*/, const QTime &/*value*/  )
{
//    return new KJS::JSValue();
    return 0;
}


QStringList KJSEmbed::extractQStringList( KJS::ExecState * /*exec*/, const KJS::List &/*args*/, int /*idx*/, const QStringList &/*defaultValue*/ )
{
    return QStringList();
}


QStringList KJSEmbed::extractQStringList( KJS::ExecState * /*exec*/, KJS::JSValue* /*value*/, const QStringList &/*defaultValue*/ )
{
    return QStringList();
}


KJS::JSValue *KJSEmbed::createQStringList( KJS::ExecState * /*exec*/, const QStringList &/*value*/  )
{
//    return new KJS::JSValue();
    return 0;
}
//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;


