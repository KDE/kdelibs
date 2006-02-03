#include <QDebug>

#include <kjsembed/object_binding.h>
#include <kjsembed/value_binding.h>
#include <kjs/object.h>

#include "bind_qaction.h"

using namespace KJSEmbed;

namespace ActionNS {
};

NO_ENUMS( Action )
NO_METHODS( Action )
NO_STATICS( Action )

START_CTOR( Action, QAction, 1 )
    QObject *parent = KJSEmbed::extractObject<QObject>( exec, args, 0 );
    QAction *a = new QAction( parent );
    return new KJSEmbed::QObjectBinding( exec, a );
END_CTOR


