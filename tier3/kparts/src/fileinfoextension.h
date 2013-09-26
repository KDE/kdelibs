/* This file is part of the KDE project
   Copyright (C) 2010 Dawit Alemayehu <adawit@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef KPARTS_FILEINFOEXTENSION_H
#define KPARTS_FILEINFOEXTENSION_H

#include <QtCore/QObject>

#include <kfileitem.h>
#include <kparts/kparts_export.h>

class KFileItemList;
class FileInfoExtensionPrivate;

namespace KParts
{

class ReadOnlyPart;

/**
 * @short an extension for obtaining file information from the part.
 *
 * This extension provides information about file and directory resources
 * that are present in the part the implements it.
 *
 * The main purpose of for this extension is to provide information about
 * files and directories located on remote servers so that download managers
 * such as kget can easily retrieve these resources.
 * 
 * @since 4.6
 */
class KPARTS_EXPORT FileInfoExtension : public QObject
{
    Q_OBJECT

public:

    /**
     * Supported file information retrieval modes.
     */
    enum QueryMode  {
        None = 0x00,              /*!< Querying for file information is NOT possible */
        AllItems = 0x01,          /*!< Retrieve or can retrieve file information for all items.*/
        SelectedItems = 0x02      /*!< Retrieve or can retrieve file information for selected items.*/
    };

    Q_DECLARE_FLAGS(QueryModes, QueryMode)

    /*! Constructor */
    FileInfoExtension(KParts::ReadOnlyPart* parent);

    /*! Destructor */
    virtual ~FileInfoExtension();

    /**
     * Queries @p obj for a child object which inherits from this class.
     */
    static FileInfoExtension *childObject( QObject *obj );

    /**
     * Returns true if any of the items in the current view of the part that
     * implements this extension are selected.
     *
     * By default this function returns false.
     */
    virtual bool hasSelection() const;

    /**
     * Returns the file information retrieve modes supported by the part
     * that implements this extension.
     *
     * By default this function returns None.
     */
    virtual QueryModes supportedQueryModes() const;

    /**
     * Returns a information for files that match the specified query @p mode.
     *
     * If the mode specified by @p mode is not supported or cannot be
     * handled, then an empty list is returned.
     */
    virtual KFileItemList queryFor(QueryMode mode) const = 0;

private:
    FileInfoExtensionPrivate* const d;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KParts::FileInfoExtension::QueryModes)

#endif /* KPARTS_FILEINFOEXTENSION_H */
