
#include "browserinterface.h"

#include <qmetaobject.h>

#include <config.h>
#if QT_VERSION >= 300
#ifdef HAVE_PRIVATE_QUCOMEXTRA_P_H
#include <private/qucomextra_p.h>
#else
#include <qucom.h>
#endif
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
#ifdef HAVE_PRIVATE_QUCOMEXTRA_P_H
            static_QUType_QString.set( o + 1, argument.toString() );
#else
	    pQUType_QString->set( o + 1, argument.toString() );
#endif
            break;
        case QVariant::StringList:
	    strLst = argument.toStringList();
#ifdef HAVE_PRIVATE_QUCOMEXTRA_P_H
            static_QUType_ptr.set( o + 1, &strLst );
#else
	    pQUType_ptr->set( o + 1, &strLst );
#endif
            break;
        case QVariant::Int:
#ifdef HAVE_PRIVATE_QUCOMEXTRA_P_H
            static_QUType_int.set( o + 1, argument.toInt() );
#else
	    pQUType_int->set( o + 1, argument.toInt() );
#endif
            break;
        case QVariant::UInt:
	    i = argument.toUInt();
#ifdef HAVE_PRIVATE_QUCOMEXTRA_P_H
	    static_QUType_ptr.set( o + 1, &i );
#else
	    pQUType_ptr->set( o + 1, &i );
#endif
            break;
        case QVariant::Bool:
#ifdef HAVE_PRIVATE_QUCOMEXTRA_P_H
	    static_QUType_bool.set( o + 1, argument.toBool() );
#else
	    pQUType_bool->set( o + 1, argument.toBool() );
#endif
            break;
        default: return;
    }
  
    qt_invoke( slot, o );
#endif
}

#include "browserinterface.moc"
