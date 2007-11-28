/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <kglobal.h>
#include <QtCore/QDebug>

class A
{
    public:
        A() : i(1) {}
        int i;
};

K_GLOBAL_STATIC(A, globalA)
K_GLOBAL_STATIC(A, globalA2)

class B
{
    public:
        ~B()
        {
            Q_ASSERT(globalA.isDestroyed());
            qDebug() << "All global statics were successfully destroyed.";
        }
};

K_GLOBAL_STATIC(B, globalB)

int main(int, char**)
{
    Q_ASSERT(globalB);

    Q_ASSERT(!globalA.isDestroyed());
    A *a = globalA;
    Q_ASSERT(a);
    A &x = *globalA;
    Q_ASSERT(a == &x);
    Q_ASSERT(a == globalA);
    Q_ASSERT(globalA->i == 1);
    Q_ASSERT(!globalA.isDestroyed());

    Q_ASSERT(!globalA2.isDestroyed());
    Q_ASSERT(globalA2);
    Q_ASSERT(globalA2->i == 1);
    Q_ASSERT(!globalA2.isDestroyed());
    globalA2.destroy();
    Q_ASSERT(globalA2.isDestroyed());

    return 0;
}
