#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include <qstring.h>
#include <kstddirs.h>
#include <kinstance.h>
#include <xslt.h>
#include <qfileinfo.h>
#include <unistd.h>
#include <qfile.h>
#include <config.h>
#include <unistd.h>
#include <kdebug.h>

int main(int argc, char **argv) {
    if ( argc != 2 ) {
        fprintf( stderr, "usage: %s xml\n", argv[0] );
        ::exit( 1 );
    }
    KInstance ins("checkXML");
    fillInstance(ins);

    QFileInfo file( argv[1] );
    chdir( QFile::encodeName( file.dirPath( true ) ) );

    QString catalogs;
    catalogs += locate( "dtd", "customization/catalog" );
    catalogs += ":";
    catalogs += locate( "dtd", "docbook/xml-dtd-4.1.2/docbook.cat" );

    setenv( "SGML_CATALOG_FILES", QFile::encodeName( catalogs ).data(), 1);
    system( QString::fromLatin1( "xmllint --catalogs --valid --nowarning --noout %1" ).arg( file.fileName() ).local8Bit().data() );

    return(0);
}
