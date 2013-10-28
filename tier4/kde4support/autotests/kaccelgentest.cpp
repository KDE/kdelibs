/*
    Copyright 2004  Frerich Raabe <raabe@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License, or (at your option) version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kaccelgen.h"

#include <QtCore/QStringList>

#include <iostream>

using std::cout;
using std::endl;

void check( const QString &what, const QStringList &expected, const QStringList &received )
{
    cout << "Testing " << qPrintable( what ) << ": ";
    if ( expected == received ) {
        cout << "ok" << endl;
    } else {
        cout << "ERROR!" << endl;
        cout << "Expected: " << qPrintable( expected.join( "," ) ) << endl;
        cout << "Received: " << qPrintable( received.join( "," ) ) << endl;
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
    for (QStringList::ConstIterator it = input.constBegin(); it != input.constEnd(); ++it) {
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
