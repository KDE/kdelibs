/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>

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
#ifndef KNEWSTUFF2_ENTRY_HANDLER_H
#define KNEWSTUFF2_ENTRY_HANDLER_H

#include <knewstuff2/core/entry.h>

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtCore/QString>

namespace KNS
{

/**
 * @short Parser and dumper for KNewStuff data entries.
 *
 * This class can be used to construct data entry objects
 * from their XML representation. It can also be used
 * for the inverse.
 *
 * @author Josef Spillner (spillner@kde.org)
 *
 * @internal
 */
class KNEWSTUFF_EXPORT EntryHandler
{
public:
    EntryHandler(const QDomElement& entryxml);
    EntryHandler(const Entry& entry);

    bool isValid();
    void setCompatibilityFormat();

    Entry entry();
    Entry *entryptr();
    QDomElement entryXML();

private:
    void init();

    QDomElement serializeElement(const Entry& entry);
    Entry deserializeElement(const QDomElement& entryxml);
    QDomElement addElement(QDomDocument& doc, QDomElement& parent,
                           const QString& tag, const QString& value);

    Entry mEntry;
    QDomElement mEntryXML;
    bool mValid;
    bool mCompat;
};

}

#endif
