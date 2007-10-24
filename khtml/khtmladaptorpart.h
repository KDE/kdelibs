/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2007 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef KHTMLADAPTORPART_H
#define KHTMLADAPTORPART_H

#include <khtml_export.h>
#include <kparts/part.h>
#include <kparts/browserextension.h>
#include <kparts/factory.h>

class KHTMLAdaptorPartFactory : public KParts::Factory {
    Q_OBJECT
public:
    KHTMLAdaptorPartFactory ();
    virtual KParts::Part *createPartObject(QWidget *wparent,
                                           QObject *parent,
                                           const char *className,
                                           const QStringList &args);
};

class AdaptorView : public KParts::ReadOnlyPart {
    Q_OBJECT
public:
    AdaptorView(QWidget* wparent, QObject* parent, const QStringList& args);

protected:
    bool openFile();
};

#endif
