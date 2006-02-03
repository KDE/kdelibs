#include <QDebug>

#include <kjsembed/object_binding.h>
#include <kjsembed/value_binding.h>
#include <kjs/object.h>

#include "bind_qdialog.h"

using namespace KJSEmbed;

namespace DialogNS {
};

NO_ENUMS( Dialog )

NO_METHODS( Dialog )

NO_STATICS( Dialog )

START_CTOR( Dialog, QDialog, 0 )
    QWidget *parent = KJSEmbed::extractObject<QWidget>( exec, args, 0, 0 );
    QDialog *a = new QDialog( parent );
    return new KJSEmbed::QObjectBinding( exec, a );
END_CTOR


