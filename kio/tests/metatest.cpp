#include <kapplication.h>
#include <kfilemetainfo.h>
#include <qstringlist.h>
#include <kdebug.h>

int main( int argc, char **argv )
{
    KApplication app( argc, argv, "kfilemetatest" );

    QString path = argv[1] ? argv[1] : "/tmp/metatest.txt";
    
    KFileMetaInfo info( path );
    
    kdDebug() << "is it valid?\n";

    if (!info.isValid()) return 1;

    kdDebug() << "it is!\n";

    QStringList l = info.preferredKeys();
    
    kdDebug() << "found " << l.size() << " keys\n";
    
    QStringList::Iterator it;
    for (it = l.begin(); it!=l.end(); ++it)
    {
        KFileMetaInfoItem item = info.item(*it);
        if ( item.isValid() ) {
            kdDebug() << item.translatedKey() << " -> " << item.prefix()
                      << item.value().toString() << item.postfix() << endl;
        }
    }

    return 0;
}
