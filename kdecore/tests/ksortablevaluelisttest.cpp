#include <qstring.h>
#include <ksortablevaluelist.h>

int main( int argc, char **argv )
{
    KSortableValueList<QString> list;
    list.insert( 1,  "FOO           (1)" );
    list.insert( 2,  "Test          (2)" );
    list.insert( 1,  "Huba!         (1)" );
    list.insert( 5,  "MAAOOAM!      (5)" );
    list.insert( 10, "Teeheeest    (10)" );
    list.insert( 2,  "I was here :) (2)" );
    list.insert( 4,  "Yeehaa...     (4)" );
    
    QValueListIterator<KSortableItem<QString> > it = list.begin();

    qDebug("Insertion order:");
    qDebug("================");
    for ( ; it != list.end(); ++it )
        qDebug( "%i: %s", (*it).index(), (*it).value().latin1() );

    list.sort();
    
    qDebug("\nSorted:");
    qDebug("=======");
    
    it = list.begin();
    for ( ; it != list.end(); ++it )
        qDebug( "%i: %s", (*it).index(), (*it).value().latin1() );
    
    return 0;
}
