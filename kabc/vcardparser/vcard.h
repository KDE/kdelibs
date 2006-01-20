/*
    This file is part of libkabc.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#ifndef VCARDPARSER_VCARD_H
#define VCARDPARSER_VCARD_H

#include <QMap>
#include <QStringList>

#include "vcardline.h"

namespace KABC {

class VCard
{
  public:
    typedef QList<VCard> List;
    typedef QMap<QString, VCardLine::List> LineMap;

    enum Version { v2_1, v3_0 };

    VCard();
    VCard( const VCard& );

    ~VCard();

    VCard& operator=( const VCard& );

    /**
     * Removes all lines from the vCard.
     */
    void clear();

    /**
     * Returns a list of all identifiers that exists in the
     * vCard.
     */
    QStringList identifiers() const;

    /**
     * Adds a VCardLine to the VCard
     */
    void addLine( const VCardLine& line );

    /**
     * Returns all lines of the vcard with a special identifier.
     */
    VCardLine::List lines( const QString& identifier ) const;

    /**
     * Returns only the first line of the vcard with a special identifier.
     */
    VCardLine line( const QString& identifier ) const;

    /**
     * Set the version of the vCard.
     */
    void setVersion( Version version );

    /**
     * Returns the version of this vCard.
     */
    Version version() const;

  private:
    LineMap mLineMap;

    class VCardPrivate;
    VCardPrivate *d;
};

}

#endif
