/*
   This file is part of the KDB libraries
   Copyright (c) 2000 Praduroux Alessandro <pradu@thekompany.com>
 
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/     

#include <iostream.h>
#include <kdebug.h>

#include <kdb/object.h>

#include "object.moc"

using namespace KDB;

QQueue<Exception> Object::m_errors;

#if 0
QPtrDict<char> Object::d(1009);
int Object::allocs = 0;
#endif

Object::Object( QObject *parent, const char *name )
    : QObject(0L, name), m_parent(parent)
{
#if 0
    cerr << "KDB (core): Allocating object " << name
         << ": allocations so far: " << ++allocs << endl;
    d.insert(this,name);
#endif
}

Object::~Object()
{
#if 0
    cerr << "KDB (core): Destroying object " << name()
         << ": allocations so far: " << --allocs << endl;
    d.remove(this);
#endif
}

#if 0
void
Object::printPendingObjects()
{
    cerr << "List of pending objects:" << endl;
    QPtrDictIterator<char> iter(d);
    while (iter.current()) {
        Object * o = static_cast<Object *> (iter.currentKey());
        cerr << "\t" << o->className() << "(" << iter.currentKey() << "): " << o->name() << endl;
        ++iter;
    }
}
#endif

void
Object::clearErrors() const
{
    m_errors.clear();
}

void
Object::pushError(Exception *e) const
{
    m_errors.enqueue(e);
}

Exception *
Object::popError() const
{
    return m_errors.dequeue();
}

Exception *
Object::getError() const
{
    return m_errors.head();
}

bool
Object::error() const
{
    return !m_errors.isEmpty();
}




