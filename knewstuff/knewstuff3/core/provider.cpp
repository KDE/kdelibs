/*
    knewstuff3/provider.cpp
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (c) 2009 Jeremy Whiting <jpwhiting@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "provider.h"
#include "knewstuff3/core/provider_p.h"

#include "xmlloader.h"
#include "core/feed.h"

#include <kdebug.h>
#include <klocale.h>
#include <kio/job.h>

namespace KNS3
{

Provider::Provider()
    :d_ptr(new ProviderPrivate)
{}

Provider::~Provider()
{
    delete d_ptr;
}

Provider::Provider(ProviderPrivate &dd)
    : d_ptr(&dd)
{
}
    
KTranslatable Provider::name() const
{
    Q_D(const Provider);
    return d->mName;
}

KUrl Provider::icon() const
{
    Q_D(const Provider);
    return d->mIcon;
}

}

#include "provider.moc"
