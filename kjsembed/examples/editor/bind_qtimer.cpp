#include <QDebug>

#include <kjsembed/object_binding.h>
#include <kjsembed/value_binding.h>
#include <kjs/object.h>

#include "bind_qtimer.h"

using namespace KJSEmbed;

namespace TimerNS {
};

NO_ENUMS( Timer )
NO_METHODS( Timer )
NO_STATICS( Timer )

START_CTOR( Timer, QTimer, 0 )
    QObject *parent = KJSEmbed::extractObject<QObject>( exec, args, 0 );
    QTimer *a = new QTimer( parent );
    return new KJSEmbed::QObjectBinding( exec, a );
END_CTOR


