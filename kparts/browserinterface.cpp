
#include "browserinterface.h"

#include <qmetaobject.h>

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
}

#include "browserinterface.moc"
