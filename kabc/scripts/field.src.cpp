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

#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>

#include "field.h"

using namespace KABC;

class Field::FieldImpl
{
  public:
    FieldImpl( int fieldId, int category = 0 )
      : mFieldId( fieldId ), mCategory( category ) {}
  
    enum FieldId
    {
      --ENUMS--
    };

    int fieldId() { return mFieldId; }
    int category() { return mCategory; }

  private:
    int mFieldId;
    int mCategory;
};


Field::List Field::mAllFields;
Field::List Field::mDefaultFields;


Field::Field( FieldImpl *impl )
{
  mImpl = impl;
}

Field::~Field()
{
  delete mImpl;
}

QString Field::label()
{
  switch ( mImpl->fieldId() ) {
    --CASELABEL--
    default:
      return i18n("Unknown Field");
  }
}

int Field::category()
{
  return Field::All;
}

QString Field::value( const KABC::Addressee &a )
{
  switch ( mImpl->fieldId() ) {
    --CASEVALUE--
    case FieldImpl::Email:
      return a.preferredEmail();
    case FieldImpl::Birthday:
      if ( a.birthday().isValid() )
        return KGlobal::locale()->formatDate( a.birthday().date() );
      else
        return QString::null;
    case FieldImpl::Url:
      return a.url().prettyURL();
    default:
      return QString::null;
  }
}

bool Field::setValue( KABC::Addressee &a, const QString &value )
{
  switch ( mImpl->fieldId() ) {
    --CASESETVALUE--
    case FieldImpl::Email:
    default:
      return false;
  }
}

bool Field::isCustom()
{
  return false;
}

Field::List Field::allFields()
{
  if ( mAllFields.isEmpty() ) {
    --CREATEFIELDS--
  }

  return mAllFields;
}

Field::List Field::defaultFields()
{
  if ( mDefaultFields.isEmpty() ) {
    createDefaultField( FieldImpl::GivenName );
    createDefaultField( FieldImpl::FamilyName );
    createDefaultField( FieldImpl::Email );
  }

  return mDefaultFields;
}

void Field::createField( int id, int category )
{
  mAllFields.append( new Field( new FieldImpl( id, category ) ) );
}

void Field::createDefaultField( int id, int category )
{
  mDefaultFields.append( new Field( new FieldImpl( id, category ) ) );
}

void Field::deleteFields()
{
  Field::List::ConstIterator it;

  for( it = mAllFields.begin(); it != mAllFields.end(); ++it ) {
    delete (*it);
  }
  mAllFields.clear();

  for( it = mDefaultFields.begin(); it != mDefaultFields.end(); ++it ) {
    delete (*it);
  }
  mDefaultFields.clear();
}

void Field::saveFields( const QString &identifier,
                        const Field::List &fields )
{
  KConfig *cfg = KGlobal::config();
  KConfigGroupSaver( cfg, "KABCFields" );

  saveFields( cfg, identifier, fields );
}

void Field::saveFields( KConfig *cfg, const QString &identifier,
                        const Field::List &fields )
{
  QValueList<int> fieldIds;
  
  Field::List::ConstIterator it;
  for( it = fields.begin(); it != fields.end(); ++it ) {
    fieldIds.append( (*it)->mImpl->fieldId() );
  }
  
  cfg->writeEntry( identifier, fieldIds );
}

Field::List Field::restoreFields( const QString &identifier )
{
  KConfig *cfg = KGlobal::config();
  KConfigGroupSaver( cfg, "KABCFields" );
 
  return restoreFields( cfg, identifier );
}

Field::List Field::restoreFields( KConfig *cfg, const QString &identifier )
{
  QValueList<int> fieldIds = cfg->readIntListEntry( identifier );

  Field::List fields;

  QValueList<int>::ConstIterator it;
  for( it = fieldIds.begin(); it != fieldIds.end(); ++it ) {
    fields.append( new Field( new FieldImpl( *it ) ) );
  }
  
  return fields;
}

bool Field::equals( Field *field )
{
  return mImpl->fieldId() == field->mImpl->fieldId();
}
