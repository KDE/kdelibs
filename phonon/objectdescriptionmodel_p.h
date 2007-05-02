/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#ifndef PHONON_OBJECTDESCRIPTIONMODEL_P_H
#define PHONON_OBJECTDESCRIPTIONMODEL_P_H

#include "objectdescriptionmodel.h"
#include <QtCore/QList>
#include "objectdescription.h"

namespace Phonon
{

class ObjectDescriptionModelBasePrivate
{
    Q_DECLARE_PUBLIC(ObjectDescriptionModelBase)
    protected:
        virtual ~ObjectDescriptionModelBasePrivate() {}
        virtual int size() const = 0;
        virtual const ObjectDescriptionBase &at(int) const = 0;
        virtual void swap(int, int) = 0;
        ObjectDescriptionModelBase *q_ptr;
};

template<ObjectDescriptionType type>
class ObjectDescriptionModelPrivate : public ObjectDescriptionModelBasePrivate
{
    Q_DECLARE_PUBLIC(ObjectDescriptionModel<type>)
    protected:
        int size() const { return data.size(); }
        const ObjectDescriptionBase &at(int x) const { return data.at(x); }
        void swap(int x, int y) { data.swap(x, y); }
        QList<ObjectDescription<type> > data;
};

}
#endif // PHONON_OBJECTDESCRIPTIONMODEL_P_H
// vim: sw=4 ts=4 tw=80
