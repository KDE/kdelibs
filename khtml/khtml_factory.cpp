/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
 * Copyright (C) 2007 David Faure <faure@kde.org>
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
#include "khtml_global.h"
#include "khtml_part.h"

KHTMLFactory::KHTMLFactory()
{
    kDebug(6000) << this;
}

KHTMLFactory::~KHTMLFactory()
{
    kDebug(6000) << this;
    // Called when khtml part is unloaded; check that we didn't leak anything
    KHTMLGlobal::finalCheck();
}

KParts::Part * KHTMLFactory::createPartObject( QWidget *parentWidget, QObject *parent, const char *className, const QStringList &args )
{
    KHTMLPart::GUIProfile prof = KHTMLPart::DefaultGUI;
    if ( strcmp(className, "Browser/View") == 0 ) // old hack
        prof = KHTMLPart::BrowserViewGUI;
    if (args.contains("Browser/View"))
        prof = KHTMLPart::BrowserViewGUI;

    return new KHTMLPart( parentWidget, parent, prof );
}

extern "C" KDE_EXPORT void *init_libkhtmlpart()
{
    return new KHTMLFactory;
}

#include "khtml_factory.moc"
