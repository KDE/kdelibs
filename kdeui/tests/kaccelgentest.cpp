#include "kaccelgen.h"

#include <qstring.h>
#include <qstringlist.h>

#include <kdebug.h>

void print_stringlist(const QStringList& list)
{
    for (QValueListConstIterator<QString> i = list.begin();
         i != list.end();
         ++i) {
        kdDebug() << *i << endl;
    }
}

int main()
{
    // Load up initial string list
    QStringList strlist;
    strlist.append("foo");
    strlist.append("bar item");
    strlist.append("&baz");
    strlist.append("bif");
    strlist.append("boz");
    strlist.append("boz 2");

    // Test stringlist -> stringlist generation
    QStringList target1;
    
    kdDebug() << "Testing QStringList value gen: " << endl;
    KAccelGen::generate(strlist, target1);
    print_stringlist(target1);


    // Test map generation
    QStringList target2, target3;

    // Load up map
    QMap<QString,QString> map;
    for (QValueListIterator<QString> i = strlist.begin();
         i != strlist.end();
         ++i) {
        map.insert(*i, *i);
    }

    kdDebug() << "Testing map value gen: " << endl;
    KAccelGen::generateFromValues(map, target2);
    print_stringlist(target2);

    kdDebug() << "Testing map key gen: " << endl;
    KAccelGen::generateFromKeys(map, target3);
    print_stringlist(target3);

    return 0;
}
