#ifdef _WIN32
#define LIBXML_DLL_IMPORT __declspec(dllimport)
#else
extern "C" int xmlLoadExtDtdDefaultValue;
#endif

#include "kio_help.h"
#include "xslt.h"

#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include <kdebug.h>

#include <QtCore/QString>

#include <stdlib.h>
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

extern "C"
{
    KDE_EXPORT int kdemain( int argc, char **argv )
    {
        Q_ASSERT(!KGlobal::hasMainComponent()); // kdeinit is messed up if this is the case.
        KComponentData componentData( "kio_help", "kio_help4" );
        fillInstance(componentData);
        (void)componentData.config(); // we need this one to make sure system globals are read

        kDebug(7101) << "Starting " << getpid();

        if (argc != 4)
        {
            fprintf(stderr, "Usage: kio_help protocol domain-socket1 domain-socket2\n");
            exit(-1);
        }

        LIBXML_TEST_VERSION
        xmlSubstituteEntitiesDefault(1);
        xmlLoadExtDtdDefaultValue = 1;

        HelpProtocol slave(false, argv[2], argv[3]);
        slave.dispatchLoop();

        kDebug(7101) << "Done";
        return 0;
    }
}



