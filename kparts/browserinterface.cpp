
#include "browserinterface.h"

#include <qmetaobject.h>

#include <config.h>
#if QT_VERSION >= 300
#include <private/qucomextra_p.h>
#endif

using namespace KParts;

BrowserInterface::BrowserInterface( QObject *parent, const char *name )
    : QObject( parent, name )
{
}

BrowserInterface::~BrowserInterface()
{
}

void BrowserInterface::callMethod( const char *name, const QVariant &argument )
{
#if QT_VERSION < 300
    QMetaData *mdata = metaObject()->slot( name );

    if ( !mdata )
        return;

    // only the most common types for now
    typedef void (QObject::*VoidMethod)(void);
    typedef void (QObject::*StringMethod)(const QString &);
    typedef void (QObject::*StringListMethod)(const QStringList &);
    typedef void (QObject::*IntMethod)(int);
    typedef void (QObject::*UIntMethod)(uint);
    typedef void (QObject::*BoolMethod)(bool);

    switch ( argument.type() )
    {
        case QVariant::Invalid:
            (this->*((VoidMethod)mdata->ptr))();
            break;
        case QVariant::String:
            (this->*((StringMethod)mdata->ptr))( argument.toString() );
            break;
        case QVariant::StringList:
            (this->*((StringListMethod)mdata->ptr))( argument.toStringList() );
            break;
        case QVariant::Int:
            (this->*((IntMethod)mdata->ptr))( argument.toInt() );
            break;
        case QVariant::UInt:
            (this->*((UIntMethod)mdata->ptr))( argument.toUInt() );
            break;
        case QVariant::Bool:
            (this->*((BoolMethod)mdata->ptr))( argument.toBool() );
            break;
        default: break;
    }
#else
    int slot = metaObject()->findSlot( name );

    if ( slot == -1 )
        return;

    QUObject o[ 2 ];
    QStringList strLst;
    uint i;

    switch ( argument.type() )
    {
        case QVariant::Invalid:
            break;
        case QVariant::String:
            static_QUType_QString.set( o + 1, argument.toString() );
            break;
        case QVariant::StringList:
	    strLst = argument.toStringList();
            static_QUType_ptr.set( o + 1, &strLst );
            break;
        case QVariant::Int:
            static_QUType_int.set( o + 1, argument.toInt() );
            break;
        case QVariant::UInt:
	    i = argument.toUInt();
	    static_QUType_ptr.set( o + 1, &i );
            break;
        case QVariant::Bool:
	    static_QUType_bool.set( o + 1, argument.toBool() );
            break;
        default: return;
    }
  
    qt_invoke( slot, o );
#endif
}

#include "browserinterface.moc"
