/*
    This file is part of libkabc.
    Copyright (c) 2002 Jost Schenck <jost@schenck.de>

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

#include <kdebug.h>

#include "addresseelist.h"

using namespace KABC;

//
//
// Traits
//
//

bool SortingTraits::Uid::eq( const Addressee &a1, const Addressee &a2 )
{
  // locale awareness doesn't make sense sorting ids
  return ( QString::compare( a1.uid(), a2.uid() ) == 0 );
}

bool SortingTraits::Uid::lt( const Addressee &a1, const Addressee &a2 )
{
  // locale awareness doesn't make sense sorting ids
  return ( QString::compare( a1.uid(), a2.uid() ) < 0 );
}

bool SortingTraits::Name::eq( const Addressee &a1, const Addressee &a2 )
{
  return ( QString::localeAwareCompare( a1.name(), a2.name() ) == 0 );
}

bool SortingTraits::Name::lt( const Addressee &a1, const Addressee &a2 )
{
  return ( QString::localeAwareCompare( a1.name(), a2.name() ) < 0 );
}

bool SortingTraits::FormattedName::eq( const Addressee &a1, const Addressee &a2 )
{
  return ( QString::localeAwareCompare( a1.formattedName(), a2.formattedName() ) == 0 );
}

bool SortingTraits::FormattedName::lt( const Addressee &a1, const Addressee &a2 )
{
  return ( QString::localeAwareCompare( a1.formattedName(), a2.formattedName() ) < 0 );
}

bool SortingTraits::FamilyName::eq( const Addressee &a1, const Addressee &a2 )
{
  return ( QString::localeAwareCompare( a1.familyName(), a2.familyName() ) == 0 
           && QString::localeAwareCompare( a1.givenName(), a2.givenName() ) == 0 );
}

bool SortingTraits::FamilyName::lt( const Addressee &a1, const Addressee &a2 )
{
  int family = QString::localeAwareCompare( a1.familyName(), a2.familyName() );
  if ( 0 == family ) {
    return ( QString::localeAwareCompare( a1.givenName(), a2.givenName() ) < 0 );
  } else {
    return family < 0;
  }
}

bool SortingTraits::GivenName::eq( const Addressee &a1, const Addressee &a2 )
{
  return ( QString::localeAwareCompare( a1.givenName(), a2.givenName() ) == 0 
           && QString::localeAwareCompare( a1.familyName(), a2.familyName() ) == 0 );
}

bool SortingTraits::GivenName::lt( const Addressee &a1, const Addressee &a2 )
{
  int given = QString::localeAwareCompare( a1.givenName(), a2.givenName() );
  if ( 0 == given ) {
    return ( QString::localeAwareCompare( a1.familyName(), a2.familyName() ) < 0 );
  } else {
    return given < 0;
  }
}

//
//
// AddresseeList
//
//

AddresseeList::AddresseeList()
  : QValueList<Addressee>()
{
  mReverseSorting = false;
  mActiveSortingCriterion = FormattedName;
}

AddresseeList::~AddresseeList()
{
}

AddresseeList::AddresseeList( const AddresseeList &l )
  : QValueList<Addressee>( l )
{
  mReverseSorting = l.reverseSorting();
  mActiveSortingCriterion = l.sortingCriterion();
}

AddresseeList::AddresseeList( const QValueList<Addressee> &l )
  : QValueList<Addressee>( l )
{
}

void AddresseeList::dump() const
{
  kdDebug(5700) << "AddresseeList {" << endl;
  kdDebug(5700) << "reverse order: " << ( mReverseSorting ? "true" : "false" ) << endl;

  QString crit;
  if ( Uid == mActiveSortingCriterion ) {
    crit = "Uid";
  } else if ( Name == mActiveSortingCriterion ) {
    crit = "Name";
  } else if ( FormattedName == mActiveSortingCriterion ) {
    crit = "FormattedName";
  } else if ( FamilyName == mActiveSortingCriterion ) {
    crit = "FamilyName";
  } else if ( GivenName == mActiveSortingCriterion ) {
    crit = "GivenName";
  } else {
    crit = "unknown -- update dump method";
  }

  kdDebug(5700) << "sorting criterion: " << crit << endl;

  for ( const_iterator it = begin(); it != end(); ++it ) {
    (*it).dump();
  }

  kdDebug(5700) << "}" << endl;
}

void AddresseeList::sortBy( SortingCriterion c )
{
  mActiveSortingCriterion = c;
  if ( Uid == c ) {
    sortByTrait<SortingTraits::Uid>();
  } else if ( Name == c ) {
    sortByTrait<SortingTraits::Name>();
  } else if ( FormattedName == c ) {
    sortByTrait<SortingTraits::FormattedName>();
  } else if ( FamilyName == c ) {
    sortByTrait<SortingTraits::FamilyName>();
  } else if ( GivenName==c ) {
    sortByTrait<SortingTraits::GivenName>();
  } else {
    kdError(5700) << "AddresseeList sorting criterion passed for which a trait is not known. No sorting done." << endl;
  }
}

void AddresseeList::sort()
{
  sortBy( mActiveSortingCriterion );
}

template<class Trait>
void AddresseeList::sortByTrait()
{
  // FIXME: better sorting algorithm, bubblesort is not acceptable for larger lists.
  //
  // for i := 1 to n - 1 
  //   do for j := 1 to n - i 
  //     do if A[j] > A[j+1] 
  //       then temp :=  A[j] 
  //         A[j] := A[j + 1] 
  //         A[j + 1 ] := temp 

  iterator i1 = begin();
  iterator endIt = end();
  --endIt;
  if ( i1 == endIt ) // don't need sorting
    return;

  iterator i2 = endIt;
  while( i1 != endIt ) {
    iterator j1 = begin();
    iterator j2 = j1;
    ++j2;
    while( j1 != i2 ) {
      if ( !mReverseSorting && Trait::lt( *j2, *j1 )
           || mReverseSorting && Trait::lt( *j1, *j2 ) ) {
        qSwap( *j1, *j2 );
      }
      ++j1;
      ++j2;
    }
    ++i1;
    --i2;
  }
}
