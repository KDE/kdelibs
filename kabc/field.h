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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KABC_FIELD_H
#define KABC_FIELD_H

#include <qstring.h>
#include <qvaluelist.h>

#include "addressee.h"

class KConfig;

namespace KABC {

class Field
{
    class FieldImpl;
    friend class FieldImpl;

  public:
    typedef QValueList<Field *> List;
  
    enum FieldCategory { All = 0x0,
                         Frequent = 0x01,
                         Address = 0x02,
                         Email = 0x04,
                         Personal = 0x08,
                         Organization = 0x10,
                         CustomCategory = 0x20 };

    /**
      Return translated label for this field.
    */
    virtual QString label();
    /**
      Return ored categories the field belongs to.
    */
    virtual int category();

    /**
      Return label translated label for field category.
    */
    static QString categoryLabel( int category );

    /**
      Return a string representation of the value the field has in the given
      Addressee. Returns QString::null, if it is not possible to convert the
      value to a string.
    */
    virtual QString value( const KABC::Addressee & );
    /**
      Set the value of the field in the given Addressee. Returns true on success
      or false, if the given string couldn't be converted to a valid value.
    */
    virtual bool setValue( KABC::Addressee &, const QString & );

    /**
      Returns, if the field is a user-defined field.
    */
    virtual bool isCustom();

    virtual bool equals( Field * );

    static Field::List allFields();
    static Field::List defaultFields();

    static Field *createCustomField( const QString &label, int category,
                                     const QString &key, const QString &app );

    static void deleteFields();

    static void saveFields( KConfig *, const QString &identifier,
                            const Field::List &fields );
    static void saveFields( const QString &identifier,
                            const Field::List &fields );
    static Field::List restoreFields( KConfig *,const QString &identifier );
    static Field::List restoreFields( const QString &identifier );

  protected:
    static void createField( int id, int category = 0 );
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
