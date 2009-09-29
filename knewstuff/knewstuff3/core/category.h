/*
    This file is part of KNewStuff2.
    Copyright (c) 2006, 2007 Josef Spillner <spillner@kde.org>

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
#ifndef KNEWSTUFF2_CATEGORY_H
#define KNEWSTUFF2_CATEGORY_H

#include <knewstuff2/core/ktranslatable.h>

#include <kurl.h>

namespace KNS
{

struct CategoryPrivate;

/**
 * @short KNewStuff category.
 *
 * @author Josef Spillner (spillner@kde.org)
 */
class KNEWSTUFF_EXPORT Category
{
public:
    /**
     * Constructor.
     */
    Category();

    /**
     * Destructor.
     */
    ~Category();

    /**
     * Sets the unique id for this category.
     */
    void setId(const QString& id);

    /**
     * Retrieve the unique id of the category.
     *
     * @return category id
     */
    QString id() const;

    /**
     * Sets the name for this category.
     */
    void setName(const KTranslatable& name);

    /**
     * Retrieve the name of the category.
     *
     * @return category name (potentially translated)
     */
    KTranslatable name() const;

    /**
     * Sets the category description.
     */
    void setDescription(const KTranslatable& type);

    /**
     * Retrieve the description of the category.
     *
     * @return category description
     */
    KTranslatable description() const;

    /**
     * Sets the icon.
     */
    void setIcon(const KUrl& icon);

    /**
     * Retrieve the icon of the category.
     *
     * @return category icon
     */
    KUrl icon() const;

private:
    Q_DISABLE_COPY(Category)

    struct CategoryPrivate * const d;
};

}

#endif
