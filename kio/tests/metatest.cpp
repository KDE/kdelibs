#include <kapplication.h>
#include <kfilemetainfo.h>
#include <qstringlist.h>
#include <kdebug.h>

int main( int argc, char **argv )
{
    KApplication app( argc, argv, "kfilemetatest" );

    KFileMetaInfoProvider *prov = KFileMetaInfoProvider::self();
    KURL u;
    if (argv[1])
      u.setPath(argv[1]);
    else
      u.setPath("/tmp/metatest.txt");

    KFileMetaInfo* info = prov->metaInfo( u );

    if (!info) return 1;

    QStringList l = info->supportedKeys();
    QStringList::Iterator it;
    for (it = l.begin(); it!=l.end(); ++it)
    {
        KFileMetaInfoItem* item = info->item(*it);
        if ( item ) {
            kdDebug() << item->translatedKey() << " -> " << item->prefix()
                      << item->value().toString() << item->postfix() << endl;
        }
    }

    return 0;
}
