#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <libxml/xmlversion.h>
#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlIO.h>
#include <libxml/parserInternals.h>
#include <libxslt/xsltconfig.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <qstring.h>
#include <kstddirs.h>
#include <kinstance.h>
#include <kdebug.h>
#include <stdlib.h>
#include "kio_help.h"
#include <xslt.h>

extern int xmlLoadExtDtdDefaultValue;

extern "C"
{
    int kdemain( int argc, char **argv )
    {
        KInstance instance( "kio_help" );
        fillInstance(instance);
        (void)instance.config(); // we need this one to make sure system globals are read

        kdDebug(7101) << "Starting " << getpid() << endl;

        if (argc != 4)
        {
            fprintf(stderr, "Usage: kio_help protocol domain-socket1 domain-socket2\n");
            exit(-1);
        }

        LIBXML_TEST_VERSION
        xmlSubstituteEntitiesDefault(1);
        xmlLoadExtDtdDefaultValue = 1;
        xmlSetExternalEntityLoader(meinExternalEntityLoader);

        HelpProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();

        kdDebug(7101) << "Done" << endl;
        return 0;
    }
}



