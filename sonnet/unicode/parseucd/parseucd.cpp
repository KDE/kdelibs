/**
 * parseucd.cpp
 *
 * Copyright 2006 Jacob Rideout <kde@jacobrideout.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#include <QtCore>

int main()
{
    QString str;
    QTextStream in(stdin);

    QFile sout;
    sout.open(stdout, QIODevice::WriteOnly);
    QDataStream out(&sout);

    bool ok;
    QMultiMap<QString,QString> data;

    while ( ok )
    {
        QString line = in.readLine();
        ok =  !in.atEnd();

        if ( line.isEmpty() || line.startsWith('#') )
            continue;

        QRegExp rx(";");
        int split = rx.indexIn(line);

        data.insert( line.right( line.size() - split - 1 ).simplified() ,
                     line.left( split ).simplified() );
    }

    out << data;
}
