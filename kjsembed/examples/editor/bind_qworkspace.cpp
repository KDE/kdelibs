#include <QDebug>

#include <kjsembed/object_binding.h>
#include <kjsembed/value_binding.h>
#include <kjsembed/qobject_binding.h>
#include <kjs/object.h>

#include "bind_qworkspace.h"

using namespace KJSEmbed;

namespace WorkspaceNS {

START_QOBJECT_METHOD( activeWindow, QWorkspace )
    QWidget *w = object->activeWindow();
    qDebug() << "Workspace active window is " << w;

    result = KJSEmbed::createQObject( exec, w );

    qDebug() << "Elvis has left the building";

END_QOBJECT_METHOD


START_QOBJECT_METHOD( addWindow, QWorkspace )
    QWidget *arg0 = KJSEmbed::extractObject<QWidget>( exec, args, 0 );
    int arg1 = KJSEmbed::extractInt( exec, args, 1 );

    QWidget *w = object->addWindow( arg0, (Qt::WFlags) arg1 );
    result = KJSEmbed::createQObject( exec, w );
END_QOBJECT_METHOD


};

NO_ENUMS( Workspace )

START_METHOD_LUT( Workspace )
        {"activeWindow", 0, KJS::DontDelete|KJS::ReadOnly, &WorkspaceNS::activeWindow },
        {"addWindow", 1, KJS::DontDelete|KJS::ReadOnly, &WorkspaceNS::addWindow }
END_METHOD_LUT

NO_STATICS( Workspace )

START_CTOR( Workspace, QWorkspace, 0 )
    QWorkspace *parent = KJSEmbed::extractObject<QWorkspace>( exec, args, 0, 0 );
    QWorkspace *a = new QWorkspace( parent );
    KJSEmbed::QObjectBinding *obj = new KJSEmbed::QObjectBinding( exec, a );

    StaticBinding::publish( exec, obj, Workspace::methods() );
    return obj;
END_CTOR


