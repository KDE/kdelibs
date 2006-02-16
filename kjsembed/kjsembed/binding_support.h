/*
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
*  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*  Boston, MA 02110-1301, USA.
*/

#ifndef BINDING_SUPPORT_H
#define BINDING_SUPPORT_H

#include <QString>
#include <QDateTime>
#include <QStringList>

#include <kjs/object.h>
#include <kdemacros.h>

#define KJS_BINDING( NAME ) \
class NAME \
{ \
    public: \
    static const KJSEmbed::Method p_methods[]; \
    static const KJSEmbed::Method p_statics[]; \
    static const KJSEmbed::Enumerator p_enums[]; \
    static const KJSEmbed::Constructor p_constructor; \
    static KJS::JSObject *ctorMethod( KJS::ExecState *exec, const KJS::List &args );\
    static const KJSEmbed::Enumerator *enums() { return p_enums;} \
    static const KJSEmbed::Method *methods() { return p_methods;} \
    static const KJSEmbed::Method *statics() { return p_statics;} \
    static const KJSEmbed::Constructor *constructor() { return &p_constructor;} \
    static const KJS::JSObject *construct(KJS::ExecState *exec, const KJS::List &args)\
                                    { return (*p_constructor.construct)(exec,args); } \
};

#define START_METHOD_LUT( TYPE ) \
const Method TYPE::p_methods[] = \
{

#define START_STATIC_METHOD_LUT( TYPE ) \
const Method TYPE::p_statics[] = \
{

#define END_METHOD_LUT \
,{0, 0, 0, 0 }\
};

#define START_ENUM_LUT( TYPE) \
const Enumerator TYPE::p_enums[] =\
{

#define END_ENUM_LUT \
,{0, 0 }\
};

#define NO_ENUMS( TYPE ) \
const Enumerator TYPE::p_enums[] = {{0, 0 }};

#define NO_METHODS( TYPE )\
const Method TYPE::p_methods[] = { {0, 0, 0, 0 } };

#define NO_STATICS( TYPE )\
const Method TYPE::p_statics[] = { {0, 0, 0, 0 } };

#define START_CTOR( TYPE, JSNAME, ARGS )\
const Constructor TYPE::p_constructor = \
{ \
#JSNAME, ARGS, KJS::DontDelete|KJS::ReadOnly, &TYPE::ctorMethod, p_statics, p_enums, p_methods };\
KJS::JSObject *TYPE::ctorMethod( KJS::ExecState *exec, const KJS::List &args )\
{\
        Q_UNUSED(exec);\
        Q_UNUSED(args);

#define END_CTOR \
}

namespace KJSEmbed
{

    /**
    * Throw an error to the KJS interpreter.  The error message will be returned inside the KJS::Exception.
    */
//    KJS::JSObject KDE_EXPORT throwError( KJS::ExecState *exec, const QString &error, KJS::ErrorType type = KJS::GeneralError );

    /**
    * This will extract a binding implementation from a KJS::JSValue
    * @code
    * KJSEmbed ObjectBindingImp *imp = extractBindingImp<ObjectBindingImp>(exec,val);
    * if( imp )
    * 	qDebug("it worked");
    * else
    * 	qDebug("it failed");
    * @endcode
    */
    template <typename T>
    T * extractBindingImp( KJS::ExecState *exec, KJS::JSValue *val)
    {
        return dynamic_cast<T*>(val->toObject(exec));
    }

    /**
    * This is just a helper function similar to the one above, only it takes a KJS::JSObject
    */
    template <typename T>
    T * extractBindingImp( KJS::JSObject *obj )
    {
        return dynamic_cast<T*>( obj );
    }

    /**
    * Method callback signature.
    * @arg exec - The execution state.
    * @arg object - The current object that the method is working on (equivelent of "this")
    * @arg args - A KJS::List of KJS::JSValue objects that represents the arguments that where
    * passed in from the javascript function signature.
    */
    typedef KJS::JSValue *(*callMethod)(KJS::ExecState*, KJS::JSObject *, const KJS::List&);

    /**
    * Method structure
    */
    struct KDE_EXPORT Method
    {
        /**
       * Method name as will appear in javascript
       */
        const char *name;
        /**
       * Number of arguments.
       */
        const int argc;
        /**
       * Flags for the member properties
       */
        const int flags;
        /**
       * The callback for the method.
       */
        const callMethod call;
    };

    /**
    * Enumerator structure
    */
    struct KDE_EXPORT Enumerator
    {
        /**
       * Method name as will appear in javascript
       */
        const char *name;
        /**
       * Integer value.
       */
        const unsigned int value;
    };

    /**
    * Constructor signature
    * @arg exec - the execution context
    * @arg args - A KJS::List of KJS::JSValue objects that represents the arguments that where
    * passed in from the javascript function signature.
    */
    typedef KJS::JSObject *(*callConstructor)(KJS::ExecState*, const KJS::List&);

    struct KDE_EXPORT Constructor
    {
        /**
       * The constructor name as it will appear in Javascript.  This will be the objects
       * name as far as Javascript is concerned.
       */
        const char *name;
        /**
       * Number of arguments.
       */
        const int argc;
        /**
       * Flags for the member properties
       */
        const int flags;
        /**
       * The callback for the constructor.
       */
        const callConstructor construct;
        /**
       * Static methods on the object.
       */
        const Method *staticMethods;
        /**
       * Enumerators for the object
       */
        const Enumerator *enumerators;
        /**
       * Member methods for the object
       */
        const Method *methods;
    };

    /**
    * Extracts a QString from an argument list.  If the argument is not present, or is not convertable to a string
    * the defaultValue is returned.
    */
    QString KDE_EXPORT extractQString( KJS::ExecState *exec, const KJS::List &args, int idx, const QString defaultValue = QString() );

    /**
    * Extract a QString from a value.  If the value cannot convert to a string the defaultValue is returned.
    */
    QString KDE_EXPORT extractQString( KJS::ExecState *exec, KJS::JSValue *value, const QString defaultValue = QString() );

    /**
    * Create a new KJS::JSValue with the value of the QString
    */
    KJS::JSValue* createQString( KJS::ExecState *exec, const QString &value );

    /**
    * Extracts a QByteArray from an argument list.  If the argument is not present, or is not convertable to a string
    * the defaultValue is returned.
    */
    QByteArray KDE_EXPORT extractQByteArray( KJS::ExecState *exec, const KJS::List &args, int idx, const QByteArray &defaultValue = QByteArray() );

    /**
    * Extract a QString from a value.  If the value cannot convert to a string the defaultValue is returned.
    */
    QByteArray KDE_EXPORT extractQByteArray( KJS::ExecState *exec, KJS::JSValue *value, const QByteArray &defaultValue = QByteArray() );

    /**
    * Create a new KJS::JSValue with the value of the QString
    */
    KJS::JSValue* createQByteArray( KJS::ExecState *exec, const QByteArray &value );


    /**
    * Extracts an integer from an argument list.  If the argument is not present, or is not convertable to an integer
    * the defaultValue is returned.
    */
    int KDE_EXPORT extractInt( KJS::ExecState *exec, const KJS::List &args, int idx, int defaultValue = 0 );
    /**
    * Extract an integer from a value.  If the value cannot convert to an integer the defaultValue is returned.
    */
    int KDE_EXPORT extractInt( KJS::ExecState *exec, KJS::JSValue *value, int defaultValue = 0 );
    /**
    * Create a new KJS::JSValue with the value of the integer.
    */
    KJS::JSValue* createInt( KJS::ExecState *exec, int value  );

    /**
    * Extracts a double from an argument list.  If the argument is not present, or is not convertable to a double
    * the defaultValue is returned.
    */
    double KDE_EXPORT extractDouble( KJS::ExecState *exec, const KJS::List &args, int idx, double defaultValue = 0 );
    /**
    * Extract a double from a value.  If the value cannot convert to a double the defaultValue is returned.
    */
    double KDE_EXPORT extractDouble( KJS::ExecState *exec, KJS::JSValue *value, double defaultValue = 0 );
    /**
    * Create a new KJS::JSValue with the value of the double.
    */
    KJS::JSValue* createDouble( KJS::ExecState *exec, double value  );

    /**
    * Extracts a float from an argument list.  If the argument is not present, or is not convertable to a float
    * the defaultValue is returned.
    */
    float KDE_EXPORT extractFloat( KJS::ExecState *exec, const KJS::List &args, int idx, float defaultValue = 0 );
    /**
    * Extract a float from a value.  If the value cannot convert to a float the defaultValue is returned.
    */
    float KDE_EXPORT extractFloat( KJS::ExecState *exec, KJS::JSValue *value, float defaultValue = 0 );
    /**
    * Create a new KJS::JSValue with the value of the float.
    */
    KJS::JSValue* createFloat( KJS::ExecState *exec, float value  );

    /**
    * Extracts a bool from an argument list.  If the argument is not present, or is not convertable to a bool
    * the defaultValue is returned.
    */
    bool KDE_EXPORT extractBool( KJS::ExecState *exec, const KJS::List &args, int idx, bool defaultValue = false );
    /**
    * Extract a bool from a value.  If the value cannot convert to a bool the defaultValue is returned.
    */
    bool extractBool( KJS::ExecState *exec, KJS::JSValue *value, bool defaultValue = false );
    /**
    * Create a new KJS::JSValue with the value of the bool.
    */
    KJS::JSValue* createBool( KJS::ExecState *exec, bool value  );

    /**
    * Extracts a QDateTime from an argument list.  If the argument is not present, or is not convertable to a QDateTime
    * the defaultValue is returned.
    */
    QDateTime KDE_EXPORT extractQDateTime( KJS::ExecState *exec, const KJS::List &args, int idx, const QDateTime &defaultValue = QDateTime() );
    /**
    * Extract a bool from a value.  If the value cannot convert to a QDateTime the defaultValue is returned.
    */
    QDateTime KDE_EXPORT extractQDateTime( KJS::ExecState *exec, KJS::JSValue *value, const QDateTime &defaultValue = QDateTime() );
    /**
    * Create a new KJS::JSValue with the value of the QDateTime.
    */
    KJS::JSValue* createQDateTime( KJS::ExecState *exec, const QDateTime &value  );

    /**
    * Extracts a QDate from an argument list.  If the argument is not present, or is not convertable to a QDate
    * the defaultValue is returned.
    */
    QDate KDE_EXPORT extractQDate( KJS::ExecState *exec, const KJS::List &args, int idx, const QDate &defaultValue = QDate() );
    /**
    * Extract a QDate from a value.  If the value cannot convert to a QDate the defaultValue is returned.
    */
    QDate KDE_EXPORT extractQDate( KJS::ExecState *exec, KJS::JSValue *value, const QDate &defaultValue = QDate() );
    /**
    * Create a new KJS::JSValue with the value of the QDate.
    */
    KJS::JSValue* createQDate( KJS::ExecState *exec, const QDate &value  );

    /**
    * Extracts a QTime from an argument list.  If the argument is not present, or is not convertable to a QTime
    * the defaultValue is returned.
    */
    QTime KDE_EXPORT extractQTime( KJS::ExecState *exec, const KJS::List &args, int idx, const QTime &defaultValue = QTime() );
    /**
    * Extract a QTime from a value.  If the value cannot convert to a QTime the defaultValue is returned.
    */
    QTime KDE_EXPORT extractQTime( KJS::ExecState *exec, KJS::JSValue *value, const QTime &defaultValue = QTime() );
    /**
    * Create a new KJS::JSValue with the value of the QTime.
    */
    KJS::JSValue* createQTime( KJS::ExecState *exec, const QTime &value  );

    /**
    * Extracts a QStringList from an argument list. If the argument is not present, or is not convertable to a QStringList
    * the defaultValue is returned.
    */
    QStringList KDE_EXPORT extractQStringList( KJS::ExecState *exec, const KJS::List &args, int idx, const QStringList &defaultValue = QStringList() );
    /**
    * Extract a QStringList from a value.  If the value cannot convert to a QStringList the defaultValue is returned.
    */
    QStringList KDE_EXPORT extractQStringList( KJS::ExecState *exec, KJS::JSValue *value, const QStringList &defaultValue = QStringList() );
    /**
    * Create a new KJS::JSValue with the value of the QStringList.
    */
    KJS::JSValue* createQStringList( KJS::ExecState *exec, const QStringList &value  );

}

#endif
