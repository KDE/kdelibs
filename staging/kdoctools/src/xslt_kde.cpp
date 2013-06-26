#include "xslt_kde.h"
#include <kfilterdev.h>

bool saveToCache( const QString &contents, const QString &filename )
{
    KFilterDev fd(filename);

    if (!fd.open(QIODevice::WriteOnly))
    {
       return false;
    }

    fd.write( contents.toUtf8() );
    fd.close();
    return true;
}
