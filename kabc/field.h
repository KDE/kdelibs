/*
    This file is part of libkabc.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef KABC_FIELD_H
#define KABC_FIELD_H

#include <qstring.h>

#include "addressee.h"

class KConfigBase;

namespace KABC {

/**
 * @short Represents a field in the Addressbook
 *
 * This class represents a field in the Addressbook database.
 * It provides methods for accessing meta-information about
 * the field (such as label() or category()) as well as
 * getting or setting the field's value in an instance of
 * Addressee (value(), setValue()).
 *
 * Furthermore, some static methods are provided for getting
 * a list of all fields (allFields(), defaultFields()), for
 * creating new fields (createCustomField()) and for saving
 * or loading fields to/from a config file (saveFields(),
 * restoreFields()).
 */
class KABC_EXPORT Field
{
  class FieldImpl;
  friend class FieldImpl;

public:
  /**
   * This type is used for a list of fields.
   */
  typedef QList<Field *> List;

  /**
   * Represents the category a field belongs to.
   */
  enum FieldCategory
  {
    /**
     * All fields
     */
    All = 0x0,
    /**
     * Frequently used fields
     */
    Frequent = 0x01,
    /**
     * Fields which belong to the address, such as Street, City, Zip, etc.
     */
    Address = 0x02,
    /**
     * Fields which store information about the e-mail contact, such as
     * e-mail address or mail client
     */
    Email = 0x04,
    /**
     * Personal fields, such as Birthday, Home Address fields, IM Address, etc.
     */
    Personal = 0x08,
    /**
     * Fields about the organization, such as Business Address fields, Department,
     * Profession, etc.
     */
    Organization = 0x10,
    /**
     * Custom (user-defined) fields
     */
    CustomCategory = 0x20
  };

  /**
   * Returns the translated label for this field.
   */
  virtual QString label();

  /**
   * Returns the  ored categories the field belongs to.
   */
  virtual int category();

  /**
   * Returns the translated label for @p category.
   *
   * @param category the category of type FieldCategory
   * @return the translated label
   */
  static QString categoryLabel( int category );

  /**
   * Returns a string representation of the value the field has in the given
   * Addressee.
   *
   * @return the string representation of the value or QString::null, if it
   * is not possible to convert the value to a string.
   */
  virtual QString value( const KABC::Addressee & );

  /**
   * Sets the value of the field in the given Addressee.
   *
   * @return @c true on success or @c false, if the given string couldn't
   * be converted to a valid value.
   */
  virtual bool setValue( KABC::Addressee &, const QString & );

  /**
   * Returns a string, that can be used for sorting.
   */
  QString sortKey( const KABC::Addressee & );

  /**
   * Returns, if the field is a user-defined field.
   *
   * @return @c true if this is a custom field, @c false otherwise
   */
  virtual bool isCustom();

  /**
   * Returns, if the field is equal with @p field.
   *
   * @param field the field to compare this field to
   * @return @c true if the fields are equal, @c false otherwise
   */
  virtual bool equals( Field *field );

  /**
   * Returns a list of all fields.
   */
  static Field::List allFields();

  /**
   * Returns a list of the default fields.
   */
  static Field::List defaultFields();

  /**
   * Creates a custom field.
   *
   * @param label    The label for this field
   * @param category The category of this field
   * @param key      Unique key for this field
   * @param app      Unique app name for this field
   */
  static Field *createCustomField( const QString &label, int category,
                                   const QString &key, const QString &app );

  /**
   * Delete all fields from list.
   */
  static void deleteFields();

  /**
   * Save the field settings to a config file.
   *
   * @param cfg        The config file object
   * @param identifier The unique identifier
   * @param fields     The list of the fields
   */
  static void saveFields( KConfigBase *cfg, const QString &identifier,
                          const Field::List &fields );
  /**
   * @overload
   *
   * Here, the list is stored in KGlobal::config() in group "KABCFields".
   *
   * @param identifier The unique identifier
   * @param fields     The list of the fields
   */
  static void saveFields( const QString &identifier,
                          const Field::List &fields );

  /**
   * Load the field settings from a config file.
   *
   * @param cfg        The config file object
   * @param identifier The unique identifier
   */
  static Field::List restoreFields( KConfigBase *cfg, const QString &identifier );

  /**
   * @overload
   *
   * Here, the list is loaded from KGlobal::config() from group "KABCFields".
   *
   * @param identifier The unique identifier
   */
  static Field::List restoreFields( const QString &identifier );

protected:
  /**
   * @internal
   */
  static void createField( int id, int category = 0 );
  /**
   * @internal
   */
  static void createDefaultField( int id, int category = 0 );

private:
  Field( FieldImpl * );
  virtual ~Field();

  FieldImpl *mImpl;

  static Field::List mAllFields;
  static Field::List mDefaultFields;
  static Field::List mCustomFields;
};

}
#endif
