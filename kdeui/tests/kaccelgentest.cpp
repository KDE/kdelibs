#include "kaccelgen.h"

#include <qstringlist.h>

#include <iostream>

using std::cout;
using std::endl;

void check( const QString &what, const QStringList &expected, const QStringList &received )
{
    cout << "Testing " << what.latin1() << ": ";
    if ( expected == received ) {
        cout << "ok" << endl;
    } else {
        cout << "ERROR!" << endl;
        cout << "Expected: " << expected.join( "," ).latin1() << endl;
        cout << "Received: " << received.join( "," ).latin1() << endl;
    }
}

int main()
{
    QStringList input;
    input << "foo" << "bar item" << "&baz" << "bif" << "boz" << "boz 2"
	      << "yoyo && dyne";

    QStringList expected;
    expected << "&foo" << "bar &item" << "&baz" << "bif" << "b&oz" << "boz &2"
	         << "&yoyo && dyne";

    QStringList output;
    KAccelGen::generate( input, output );
    check( "QStringList value generation", expected, output );

    QMap<QString,QString> map;
    for (QStringList::ConstIterator it = input.begin(); it != input.end(); ++it) {
        map.insert(*it, *it);
    }
    input.sort();
    expected.clear();
    KAccelGen::generate( input, expected );

    output.clear();
    KAccelGen::generateFromValues( map, output );
    check( "map value generation", expected, output );

    output.clear();
    KAccelGen::generateFromKeys( map, output );
    check( "map key generation", expected, output );
}
