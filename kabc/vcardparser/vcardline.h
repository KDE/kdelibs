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

#ifndef VCARDLINE_H
#define VCARDLINE_H

#include <qstringlist.h>
#include <qvaluelist.h>
#include <qvariant.h>
#include <qmap.h>
#include <qstring.h>

namespace KABC {

class VCardLine
{
  public:
    typedef QValueList<VCardLine> List;
    typedef QMap<QString, QStringList> ParamMap;

    VCardLine();
    VCardLine( const QString &identifier );
    VCardLine( const QString &identifier, const QVariant &value );
    VCardLine( const VCardLine& );

    ~VCardLine();

    VCardLine& operator=( const VCardLine& );

    /**
     * Sets the identifier of this line e.g. UID, FN, CLASS
     */
    void setIdentifier( const QString& identifier );

    /**
     * Returns the identifier of this line.
     */
    QString identifier() const;

    /**
     * Sets the value of of this line.
     */
    void setValue( const QVariant& value );

    /**
     * Returns the value of this line.
     */
    QVariant value() const;

    /**
     * Returns all parameters.
     */
    QStringList parameterList() const;

    /**
     * Add a new parameter to the line.
     */
    void addParameter( const QString& param, const QString& value );

    /**
     * Returns the values of a special parameter.
     * You can get a list of all parameters with paramList().
     */
    QStringList parameters( const QString& param ) const;

    /**
     * Returns only the first value of a special parameter.
     * You can get a list of all parameters with paramList().
     */
    QString parameter( const QString& param ) const;

  private:
    ParamMap mParamMap;
    QString mIdentifier;
    QVariant mValue;

    class VCardLinePrivate;
    VCardLinePrivate *d;
};

}

#endif
