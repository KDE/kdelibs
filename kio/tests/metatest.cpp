#include <kapplication.h>
#include <kfilemetainfo.h>

int main( int argc, char **argv )
{
    KApplication app( argc, argv, "kfilemetatest" );
    
    KFileMetaInfoProvider *prov = KFileMetaInfoProvider::self();
    KURL u;
    u.setPath("/tmp/metatest.txt");
    prov->metaInfo( u );
    
    return 0;
}
