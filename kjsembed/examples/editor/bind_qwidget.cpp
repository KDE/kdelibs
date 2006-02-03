#include <QDebug>

#include <kjsembed/object_binding.h>
#include <kjsembed/value_binding.h>
#include <kjs/object.h>

#include <bind_qwidget.h>

using namespace KJSEmbed;

namespace WidgetNS {

START_QOBJECT_METHOD( addAction, QWidget )
    QAction *act = KJSEmbed::extractObject<QAction>( exec, args, 0 );
    object->addAction( act );
END_QOBJECT_METHOD


};

NO_ENUMS( Widget )

START_METHOD_LUT( Widget )
        {"addAction", 1, KJS::DontDelete|KJS::ReadOnly, &WidgetNS::addAction }
END_METHOD_LUT

NO_STATICS( Widget )

START_CTOR( Widget, QWidget, 0 )
    QWidget *parent = KJSEmbed::extractObject<QWidget>( exec, args, 0, 0 );
    QWidget *a = new QWidget( parent );
    return new KJSEmbed::QObjectBinding( exec, a );
END_CTOR


