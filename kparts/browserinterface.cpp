
#include "browserinterface.h"

#include <qmetaobject.h>
#include <qstringlist.h>

#include <config.h>

using namespace KParts;

BrowserInterface::BrowserInterface( QObject *parent )
    : QObject( parent )
{
}

BrowserInterface::~BrowserInterface()
{
}

void BrowserInterface::callMethod( const char *name, const QVariant &argument )
{
    if ( metaObject()->indexOfSlot( name ) == -1 )
        return;

    switch ( argument.type() )
    {
        case QVariant::Invalid:
            break;
        case QVariant::String:
            QMetaObject::invokeMethod( this, name,
                                       Q_ARG( QString, argument.toString() ) );
            break;
        case QVariant::StringList:
        {
	    QStringList strLst = argument.toStringList();
            QMetaObject::invokeMethod( this, name,
                                       Q_ARG( QStringList *, &strLst ) );
            break;
        }
        case QVariant::Int:
            QMetaObject::invokeMethod( this, name,
                                       Q_ARG( int, argument.toInt() ) );
            break;
        case QVariant::UInt:
        {
	    unsigned int i = argument.toUInt();
            QMetaObject::invokeMethod( this, name,
                                       Q_ARG( unsigned int *, &i ) );
            break;
        }
        case QVariant::Bool:
            QMetaObject::invokeMethod( this, name,
                                       Q_ARG( bool, argument.toBool() ) );
            break;
        default:
            break;
    }
}

#include "browserinterface.moc"
