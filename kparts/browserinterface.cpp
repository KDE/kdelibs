
#include "browserinterface.h"

#include <qmetaobject.h>

#include <config.h>
#include <private/qucomextra_p.h>

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
}

#include "browserinterface.moc"
