/*
    This file is part of libkabc.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#ifndef KABC_SORTMODE_H
#define KABC_SORTMODE_H

#include <kdelibs_export.h>

#include <kabc/addressee.h>

namespace KABC {

/**
  @short Sort method for sorting an addressee list.

  This interface should be reimplemented by classes which shall act as
  SortModes for KABC::AddresseeList.
*/
class KABC_EXPORT SortMode
{
  public:
	virtual ~SortMode(){}
    /**
      Reimplement this method and return whether the first contact is 'smaller'
      than the second.
     */
    virtual bool lesser( const KABC::Addressee &first, const KABC::Addressee &second ) const = 0;
};

/**
  @short Implements comparison by name.

  This class implements a comparison of two Addressee objects by name.
 */
class KABC_EXPORT NameSortMode : public SortMode
{
  public:
    /**
      Specifies which parts of the name are used for comparison.
     */
    enum NameType
    {
      FormattedName,    ///< use the formatted name, e.g. "John Doe"
      FamilyName,       ///< use the last name, e.g. "Doe"
      GivenName         ///< use the first name, e.g. "John"
    };

    /**
      Constructor.

      Creates a NameSortMethod with FormattedName as name type set.
     */
    NameSortMode();
	virtual ~NameSortMode(){}
    /**
      Constructor.

      Creates a NameSortMethod with the specified name type.

      @param type The name type.
      @param ascending if @c true, Addressee object are sorted in
             ascending order; if @c false, objects are sorted in
             descending order
     */
    NameSortMode( NameType type, bool ascending = true );

    /**
      Returns whether the first contact is 'smaller' then the second.
     */
    virtual bool lesser( const KABC::Addressee&, const KABC::Addressee& ) const;

  private:
    NameType mNameType;
    bool mAscendingOrder;

    class NameSortModePrivate;
    NameSortModePrivate *d;
};

/**
  @short Implements comparison by an arbitrary field.

  This class implements a comparison of two Addressee objects by the
  value of an arbitrary field.
 */
class KABC_EXPORT FieldSortMode : public SortMode
{
  public:
    /**
      Constructor.

      Creates a FieldSortMethod with the specified @p field.

      @param field The field.
      @param ascending if @c true, Addressee object are sorted in
             ascending order; if @c false, objects are sorted in
             descending order
     */
    FieldSortMode( KABC::Field *field, bool ascending = true );
	virtual ~FieldSortMode(){}
    /**
      Returns whether the first contact is 'smaller' then the second.
     */
    virtual bool lesser( const KABC::Addressee&, const KABC::Addressee& ) const;

  private:
    KABC::Field *mField;
    bool mAscendingOrder;

    class FieldSortModePrivate;
    FieldSortModePrivate *d;
};

}

#endif
