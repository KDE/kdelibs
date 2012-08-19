
#include <QWidget>
#include <QtCore/QTimer>
#include <stdlib.h>
#include "kcolorcollection.h"
#include <stdio.h>

#include <QApplication>
#include <QtCore/QStringList>


int main( int argc, char **argv )
{
    QApplication::setApplicationName("KColorCollectionTest");
    QApplication a(argc, argv);

    QStringList collections = KColorCollection::installedCollections();
    for(QStringList::ConstIterator it = collections.constBegin();
        it != collections.constEnd(); ++it)
    {
       printf("Palette = %s\n", (*it).toAscii().constData());

       KColorCollection myColorCollection = KColorCollection(*it);

       printf("Palette Name = \"%s\"\n", myColorCollection.name().toAscii().constData());
       printf("Description:\n\"%s\"\n", myColorCollection.description().toAscii().constData());
       printf("Nr of Colors = %d\n", myColorCollection.count());
       for(int i = 0; i < myColorCollection.count(); i++)
       {
         int r,g,b;
         myColorCollection.color(i).getRgb(&r, &g, &b);
         printf("#%d Name = \"%s\" #%02x%02x%02x\n",
                i, myColorCollection.name(i).toAscii().constData(), r,g,b);
       }
    }
}


