#include <kapplication.h>
#include <kfilemetainfo.h>
#include <qstringlist.h>
#include <kdebug.h>
#include <qlabel.h>

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
        if ( item.isValid() && item.value().canCast(QVariant::String)) {
            kdDebug() << item.translatedKey() << " -> " << item.prefix()
                      << item.value().toString() << item.suffix() << endl;
        }
    }
    
    KFileMetaInfoItem item = info.item("Thumbnail");
    
    if (!item.isValid()) kdDebug() << "no thumbnail\n";
    else
        kdDebug() << "type of thumbnail is " << item.value().typeName() << endl;
    
    if (item.isValid() && item.value().canCast(QVariant::Pixmap))
    {
        QLabel* label = new QLabel(0);
        app.setMainWidget(label);
        label->setPixmap(item.value().toPixmap());
        label->show();
        app.exec();
    }

    return 0;
}
