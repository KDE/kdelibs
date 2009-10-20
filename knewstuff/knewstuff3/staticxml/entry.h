/*
    knewstuff3/entry.h.
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
#ifndef KNEWSTUFF3_STATICXML_ENTRY_H
#define KNEWSTUFF3_STATICXML_ENTRY_H

#include <knewstuff3/core/author.h>
#include <knewstuff3/core/ktranslatable.h>
#include <knewstuff3/core/entry.h>

#include <QtXml/QDomElement>

namespace KNS3
{
class StaticXmlEntryPrivate;

/**
 * @short KNewStuff data entry container.
 *
 * This class provides accessor methods to the data objects
 * as used by KNewStuff.
 * It should probably not be used directly by the application.
 *
 * @author Cornelius Schumacher (schumacher@kde.org)
 * \par Maintainer:
 * Jeremy Whiting (jpwhiting@kde.org)
 */
class KNEWSTUFF_EXPORT StaticXmlEntry : public Entry
{
public:
    typedef QList<Entry*> List;
    
    /**
     * Constructor.
     */
    StaticXmlEntry();

    StaticXmlEntry(const StaticXmlEntry& other);
    StaticXmlEntry& operator=(const StaticXmlEntry& other);

    /**
     * Destructor.
     */
    virtual ~StaticXmlEntry();
    
    virtual bool setEntryXML(const QDomElement & xmldata);
    virtual QDomElement entryXML() const;
    
protected:
    StaticXmlEntryPrivate* const d_ptr;
    StaticXmlEntry(StaticXmlEntryPrivate &dd, QObject *parent);
private:
    Q_DECLARE_PRIVATE(StaticXmlEntry)
};

}

#endif
