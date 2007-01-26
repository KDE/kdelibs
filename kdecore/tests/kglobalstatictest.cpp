/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.

*/

#include <kglobal.h>
#include <QtDebug>

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
            qDebug() << "B::~B()";
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
