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
#include "settings.h"
#include <QtCore/QSettings>
#include "kjseglobal.h"

using namespace KJSEmbed;

START_QOBJECT_METHOD( callAllKeys, QSettings )
    QStringList keys = object->allKeys();
    result = KJSEmbed::convertToValue( exec, keys );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( callSetValue, QSettings )
    QString key = KJSEmbed::extractQString(exec, args, 0);
    QVariant value = KJSEmbed::convertToVariant(exec, args[1]);
    object->setValue(key,value);
END_QOBJECT_METHOD

START_QOBJECT_METHOD( callValue, QSettings )
    QVariant value;
    QString key = KJSEmbed::extractQString(exec, args, 0);
    if( args.size() == 2 )
    {
        value = KJSEmbed::convertToVariant(exec, args[1] );
    }
    value = object->value(key,value);
    result = KJSEmbed::convertToValue( exec, value );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( callClear, QSettings )
    object->clear();
END_QOBJECT_METHOD

START_QOBJECT_METHOD( callSync, QSettings )
    object->sync();
END_QOBJECT_METHOD

START_QOBJECT_METHOD( callRemove, QSettings )
    QString key = KJSEmbed::extractQString(exec, args, 0);
    object->remove(key);
END_QOBJECT_METHOD

START_STATIC_OBJECT_METHOD( callSetPath )
    // QSettings::Format format = (QSettings::Format) KJSEmbed::extractVariant<uint>(exec, args, 0);
    // QSettings::Scope scope = (QSettings::Scope) KJSEmbed::extractVariant<uint>(exec, args, 1);
    QString path = KJSEmbed::extractQString(exec, args, 2);
    //QSettings::setSystemIniPath(format,scope,path);
    return KJS::jsNull();
END_STATIC_OBJECT_METHOD

START_METHOD_LUT( SettingsBinding )
    {"allKeys", 0, KJS::DontDelete|KJS::ReadOnly, &callAllKeys },
    {"setValue", 2, KJS::DontDelete|KJS::ReadOnly, &callSetValue },
    {"value", 1, KJS::DontDelete|KJS::ReadOnly, &callValue },
    {"clear", 0,KJS::DontDelete|KJS::ReadOnly, &callClear },
    {"sync", 0,KJS::DontDelete|KJS::ReadOnly, &callSync },
    {"remove", 1,KJS::DontDelete|KJS::ReadOnly, &callRemove }
END_METHOD_LUT

START_ENUM_LUT( SettingsBinding )
    {"NativeFormat", QSettings::NativeFormat},
    {"IniFormat", QSettings::IniFormat},
    //{"InvalidFormat", QSettings::InvalidFormat},
    {"UserScope", QSettings::UserScope},
    {"SystemScope", QSettings::SystemScope}
END_ENUM_LUT

START_STATIC_METHOD_LUT( SettingsBinding )
    {"setPath", 3, KJS::DontDelete|KJS::ReadOnly, &callSetPath }
END_METHOD_LUT

KJSO_SIMPLE_BINDING_CTOR( SettingsBinding, QSettings, QObjectBinding )
KJSO_QOBJECT_BIND( SettingsBinding, QSettings )

KJSO_START_CTOR( SettingsBinding, QSettings, 1 )
    QSettings *settings = 0;
    if( args.size() == 1)
    {
        QObject *parent = KJSEmbed::extractObject<QObject>(exec,args,0);
        settings = new QSettings(parent);
    }
    else if ( args.size() == 3 )
    {
        QString fileName = KJSEmbed::extractQString(exec, args, 0);
        QSettings::Format format = (QSettings::Format) KJSEmbed::extractVariant<uint>(exec, args, 1);
        QObject *parent = KJSEmbed::extractObject<QObject>(exec,args,2);
        settings = new QSettings(fileName,format,parent);
    }
    else if( args.size() == 4 )
    {
        QSettings::Scope scope = (QSettings::Scope) KJSEmbed::extractVariant<uint>(exec, args, 0);
        QString organization = KJSEmbed::extractQString(exec, args, 1);
        QString application = KJSEmbed::extractQString(exec, args, 2);
        QObject *parent = KJSEmbed::extractObject<QObject>(exec,args,3);
        settings = new QSettings(scope, organization, application, parent);
    }
    else
        settings = new QSettings();
    
    return new SettingsBinding(exec,settings);
KJSO_END_CTOR

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
