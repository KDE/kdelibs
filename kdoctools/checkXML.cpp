/*
 * tester.c : a small tester program for parsing using the SAX API.
 *
 * See Copyright for the status of this software.
 *
 * Daniel.Veillard@w3.org
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include <libxml/parser.h>
#include <libxml/parserInternals.h> /* only for xmlNewInputFromFile() */
#include <libxml/tree.h>
#include <libxml/debugXML.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlerror.h>

#include <qstring.h>
#include <kstddirs.h>
#include <kinstance.h>
#include <xslt.h>

extern int xmlLoadExtDtdDefaultValue;

int main(int argc, char **argv) {
    int i;
    int files = 0;

    if ( argc != 2 ) {
        fprintf( stderr, "usage: %s xml\n", argv[0] );
        ::exit( 1 );
    }
    xmlLoadExtDtdDefaultValue = 1;

    KInstance ins("meinproc");
    fillInstance(ins);

    LIBXML_TEST_VERSION
    xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;
    xmlSetExternalEntityLoader(meinExternalEntityLoader);

    xmlParseFile(argv[1]);

    xmlCleanupParser();
    xmlMemoryDump();

    return(0);
}
