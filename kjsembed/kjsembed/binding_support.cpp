#include "binding_support.h"
#include <kjs/interpreter.h>
#include <QString>

using namespace KJSEmbed;

/*
KJS::JSObject KJSEmbed::throwError( KJS::ExecState *exec, const QString &error, KJS::ErrorType type )
{
    KJS::JSObject *retValue;
    if(exec->context().imp() != 0)
    {
//        int sourceId = exec->context().sourceId();
        int sourceId = exec->context().thisValue()->sourceId();
        int startLine = exec->context().curStmtFirstLine();
        retValue = KJS::Error::create( exec, type, error.toAscii().data(), startLine, sourceId );
    }
    else
        retValue = KJS::Error::create( exec, type, error.toAscii().data() );
    exec->setException( retValue );
    qDebug("KJSEmbed exception: '%s'",error.toAscii().data());
    return retValue;
}
*/

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
    return value->toString(exec).qstring();
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
    return value->toString(exec).qstring().toLatin1();
}

KJS::JSValue *KJSEmbed::createQByteArray( KJS::ExecState *exec, const QByteArray &value )
{
    Q_UNUSED( exec );
    return KJS::String( value.data() );
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
    return (int) value->toNumber(exec);
}

KJS::JSValue *KJSEmbed::createQString( KJS::ExecState *exec, const QString &value )
{
    Q_UNUSED( exec );
    return KJS::String( value );
}

KJS::JSValue *KJSEmbed::createInt( KJS::ExecState *exec, int value  )
{
    Q_UNUSED( exec );
    return KJS::Number( value );
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
    return KJS::Number( value );
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
    return KJS::Number( value );
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
    return KJS::Boolean( value );
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


QTime KJSEmbed::extractQTime( KJS::ExecState */*exec*/, KJS::JSValue* /*value*/, const QTime &/*defaultValue*/  )
{
    return QTime();
}


KJS::JSValue *KJSEmbed::createQTime( KJS::ExecState */*exec*/, const QTime &/*value*/  )
{
//    return new KJS::JSValue();
    return 0;
}


QStringList KJSEmbed::extractQStringList( KJS::ExecState */*exec*/, const KJS::List &/*args*/, int /*idx*/, const QStringList &/*defaultValue*/ )
{
    return QStringList();
}


QStringList KJSEmbed::extractQStringList( KJS::ExecState */*exec*/, KJS::JSValue* /*value*/, const QStringList &/*defaultValue*/ )
{
    return QStringList();
}


KJS::JSValue *KJSEmbed::createQStringList( KJS::ExecState */*exec*/, const QStringList &/*value*/  )
{
//    return new KJS::JSValue();
    return 0;
}


