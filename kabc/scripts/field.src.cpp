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
      FamilyName,
      GivenName,
      AdditionalName,
      Prefix,
      Suffix,
      NickName,
      Email
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
    case FieldImpl::FamilyName:
      return Addressee::familyNameLabel();
    case FieldImpl::GivenName:
      return Addressee::givenNameLabel();
    case FieldImpl::AdditionalName:
      return Addressee::additionalNameLabel();
    case FieldImpl::Prefix:
      return Addressee::prefixLabel();
    case FieldImpl::Suffix:
      return Addressee::suffixLabel();
    case FieldImpl::NickName:
      return Addressee::nickNameLabel();
    case FieldImpl::Email:
      return i18n("Email");
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
    case FieldImpl::FamilyName:
      return a.familyName();
    case FieldImpl::GivenName:
      return a.givenName();
    case FieldImpl::AdditionalName:
      return a.additionalName();
    case FieldImpl::Prefix:
      return a.prefix();
    case FieldImpl::Suffix:
      return a.suffix();
    case FieldImpl::NickName:
      return a.nickName();
    case FieldImpl::Email:
      return a.preferredEmail();
    default:
      return QString::null;
  }
}

bool Field::setValue( KABC::Addressee &a, const QString &value )
{
  switch ( mImpl->fieldId() ) {
    case FieldImpl::FamilyName:
      a.setFamilyName( value );
      return true;
    case FieldImpl::GivenName:
      a.setGivenName( value );
      return true;
    case FieldImpl::AdditionalName:
      a.setAdditionalName( value );
      return true;
    case FieldImpl::Prefix:
      a.setPrefix( value );
      return true;
    case FieldImpl::Suffix:
      a.setSuffix( value );
      return true;
    case FieldImpl::NickName:
      a.setNickName( value );
      return true;
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
    createField( FieldImpl::FamilyName );
    createField( FieldImpl::GivenName );
    createField( FieldImpl::AdditionalName );
    createField( FieldImpl::Prefix );
    createField( FieldImpl::Suffix );
    createField( FieldImpl::NickName );
    createField( FieldImpl::Email );
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
