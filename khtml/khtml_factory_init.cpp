/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "khtml_factory.h"

extern "C" KDE_EXPORT void *init_libkhtmlpart()
{
    // We can't use a plain self() here, because that would
    // return the global factory, which might already exist
    // at the time init_libkhtmlpart is called! As soon as someone
    // does new KHTMLPart() in his application and loads up
    // an html document into that part which either embeds
    // embeds another KHTMLPart instance via <object> or
    // as html frame, then we cannot return self(), as
    // what we return here is what the KLibLoader deletes
    // in the end, and we don't want the libloader to
    // delete our global instance. Anyway, the new
    // KHTMLFactory we create here is very cheap :)
    // (Simon)
    return new KHTMLFactory( true );
}
