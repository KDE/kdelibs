#include "settings.h"
#include <QSettings>
#include "global.h"

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
    // QSettings::Format format = (QSettings::Format) KJSEmbed::extractValue<uint>(exec, args, 0);
    // QSettings::Scope scope = (QSettings::Scope) KJSEmbed::extractValue<uint>(exec, args, 1);
    QString path = KJSEmbed::extractQString(exec, args, 2);
    //QSettings::setSystemIniPath(format,scope,path);
    return KJS::Null();
END_STATIC_OBJECT_METHOD

START_METHOD_LUT( Settings )
    {"allKeys", 0, KJS::DontDelete|KJS::ReadOnly, &callAllKeys },
    {"setValue", 2, KJS::DontDelete|KJS::ReadOnly, &callSetValue },
    {"value", 1, KJS::DontDelete|KJS::ReadOnly, &callValue },
    {"clear", 0,KJS::DontDelete|KJS::ReadOnly, &callClear },
    {"sync", 0,KJS::DontDelete|KJS::ReadOnly, &callSync },
    {"remove", 1,KJS::DontDelete|KJS::ReadOnly, &callRemove }
END_METHOD_LUT

START_ENUM_LUT( Settings )
    {"NativeFormat", QSettings::NativeFormat},
    {"IniFormat", QSettings::IniFormat},
    //{"InvalidFormat", QSettings::InvalidFormat},
    {"UserScope", QSettings::UserScope},
    {"SystemScope", QSettings::SystemScope}
END_ENUM_LUT

START_STATIC_METHOD_LUT( Settings )
    {"setPath", 3, KJS::DontDelete|KJS::ReadOnly, &callSetPath }
END_METHOD_LUT

START_CTOR( Settings, QSettings, 1 )
    QSettings *settings = 0;
    if( args.size() == 1)
    {
        QObject *parent = KJSEmbed::extractObject<QObject>(exec,args,0);
        settings = new QSettings(parent);
    }
    else if ( args.size() == 3 )
    {
        QString fileName = KJSEmbed::extractQString(exec, args, 0);
        QSettings::Format format = (QSettings::Format) KJSEmbed::extractValue<uint>(exec, args, 1);
        QObject *parent = KJSEmbed::extractObject<QObject>(exec,args,2);
        settings = new QSettings(fileName,format,parent);
    }
    else if( args.size() == 4 )
    {
        QSettings::Scope scope = (QSettings::Scope) KJSEmbed::extractValue<uint>(exec, args, 0);
        QString organization = KJSEmbed::extractQString(exec, args, 1);
        QString application = KJSEmbed::extractQString(exec, args, 2);
        QObject *parent = KJSEmbed::extractObject<QObject>(exec,args,3);
        settings = new QSettings(scope, organization, application, parent);
    }

    if( settings )
    {
        KJS::JSObject *binding = new QObjectBinding( exec, settings);
        StaticBinding::publish( exec, binding, Settings::methods() );
        return binding;
    }
    else
    {
        KJS::throwError(exec, KJS::GeneralError, i18n("Could not create the settings object."));
        // KJSEmbed::throwError(exec,i18n("Could not create the settings object."));
        return new KJS::JSObject();
    }
END_CTOR
