#include <QDebug>

#include <kjsembed/value_binding.h>
#include <kjs/object.h>

#include "bind_qicon.h"

using namespace KJSEmbed;

namespace IconNS {
};

NO_ENUMS( Icon )
NO_METHODS( Icon )
NO_STATICS( Icon )

START_CTOR( Icon, QIcon, 0 )

    qDebug() << "QIcon called args " << args.size();

    QIcon i;
    if ( args.size() == 0 )
    {
        i = QIcon();
    }
    else if ( args.size() >= 1 )
    {
        i = QIcon( KJSEmbed::extractQString( exec, args, 0 ) );
    }

    return new KJSEmbed::ValueBinding( exec, i );
END_CTOR


