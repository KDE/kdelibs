/* This file is part of the KDE libraries
    Copyright (c) 1999 Waldo Bastian <bastian@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QtCore/QString>
#include <QtCore/QHash>
#include <stdio.h>

class A { qint64 foo; };
class B { qint64 bar; };
class C : public A, public B { qint64 foobar; };

QHash<QString,A*> dictA;
QHash<QString,B*> dictB;

int main(int, char *[])
{
    // This test shows that different pointer values due to base class adjustment
    // are not a problem.
    C obj;
    A *pA = &obj;
    B *pB = &obj;
    C *pC = &obj;
    Q_ASSERT(pA == pC); // comparison is done using A*, so it works
    Q_ASSERT(pB == pC); // comparison is done using B*, so it works
    Q_ASSERT((void*)pA == (void*)pC);
    Q_ASSERT((void*)pB != (void*)pC); // yes, it's a different pointer value!

    dictA.insert("hello", pC);
    dictB.insert("hello", pC);

    Q_ASSERT(dictA["hello"] == pC);
    Q_ASSERT(dictB["hello"] == pC);
}
