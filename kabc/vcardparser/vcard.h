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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef VCARD_H
#define VCARD_H

#include "vcardline.h"
#include <qmap.h>
#include <qstringlist.h>
#include <qvaluelist.h>

class VCard
{
  public:
    typedef QValueList<VCard> List;

    enum Version { v2_1, v3_0 };

    VCard();
    ~VCard();

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
    VCardLine::List lines( const QString& identifier );

    /**
     * Returns only the first line of the vcard with a special identifier.
     */
    VCardLine line( const QString& identifier );

    /**
     * Set the version of the vCard.
     */
    void setVersion( Version version );

    /**
     * Returns the version of this vCard.
     */
    Version version() const;

  private:
    class VCardPrivate;
    VCardPrivate *d;
};

#endif
