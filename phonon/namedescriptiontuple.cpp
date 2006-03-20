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

#include "namedescriptiontuple.h"
#include "namedescriptiontuple_p.h"
#include <QString>

namespace Phonon
{

NameDescriptionTuple::NameDescriptionTuple( NameDescriptionTuplePrivate& dd,
		int index, const QString& name, const QString& description )
	: d_ptr( &dd )
{
	Q_D( NameDescriptionTuple );
	d->index = index;
	d->name = name;
	d->description = description;
}

NameDescriptionTuple::NameDescriptionTuple( NameDescriptionTuplePrivate& dd )
	: d_ptr( &dd )
{
}

NameDescriptionTuple::~NameDescriptionTuple()
{
	Q_D( NameDescriptionTuple );
	delete d;
	d_ptr = 0;
}

int NameDescriptionTuple::index() const
{
	return d_func()->index;
}

const QString& NameDescriptionTuple::name() const
{
	return d_func()->name;
}

const QString& NameDescriptionTuple::description() const
{
	return d_func()->description;
}

} //namespace Phonon
// vim: sw=4 ts=4 noet
