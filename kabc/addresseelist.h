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

#ifndef KABC_ADDRESSEELIST_H
#define KABC_ADDRESSEELIST_H

#include <qvaluelist.h>

#include "addressee.h"

namespace KABC {

class Field;
    
/**
 * Each trait must implement one static function for equality, one for "less 
 * than". Class name should be the field name. A trait does not necessarily 
 * have to stick to just one field: a trait sorting by family name can e.g. 
 * sort addressees with equal family name by given name.
 *
 * If you want to implement reverse sorting, you do not have to write another
 * trait, as AddresseeList takes care of that.
 */
namespace SortingTraits
{ 

class Uid
{
  public:
    static bool eq( const Addressee &, const Addressee & );
    static bool lt( const Addressee &, const Addressee & );
};

class Name
{
  public:
    static bool eq( const Addressee &, const Addressee & );
    static bool lt( const Addressee &, const Addressee & );
};

class FormattedName
{
  public:
    static bool eq( const Addressee &, const Addressee & );
    static bool lt( const Addressee &, const Addressee & );
};

class FamilyName // fallback to given name
{
  public: 
    static bool eq( const Addressee &, const Addressee & );
    static bool lt( const Addressee &, const Addressee & );
};

class GivenName  // fallback to family name
{
  public: 
    static bool eq( const Addressee &, const Addressee & );
    static bool lt( const Addressee &, const Addressee & );
};

};

/** 
 * Addressee attribute used for sorting. 
 */
typedef enum { Uid, Name, FormattedName, FamilyName, GivenName } SortingCriterion;

/**
 * @short  a QValueList of Addressee, with sorting functionality
 *
 * This class extends the functionality of QValueList with
 * sorting methods specific to the Addressee class. It can be used
 * just like any other QValueList but is no template class.
 *
 * An AddresseeList does not automatically keep sorted when addressees
 * are added or removed or the sorting order is changed, as this would
 * slow down larger operations by sorting after every step. So after
 * such operations you have to call {@link #sort} or {@link #sortBy} to 
 * create a defined order again.
 *
 * Iterator usage is inherited by QValueList and extensively documented
 * there. Please remember that the state of an iterator is undefined 
 * after any sorting operation.
 *
 * For the enumeration Type SortingCriterion, which specifies the
 * field by the collection will be sorted, the following values exist:
 * Uid, Name, FormattedName, FamilyName, GivenName.
 * 
 * @author Jost Schenck jost@schenck.de 
 */
class AddresseeList : public QValueList<Addressee>
{
  public:
    AddresseeList();
    ~AddresseeList();
    AddresseeList( const AddresseeList & );
    AddresseeList( const QValueList<Addressee> & );

    /**
     * Debug output.
     */
    void dump() const;
  
    /** 
     * Determines the direction of sorting. On change, the list
     * will <em>not</em> automatically be resorted.
     * @param r   <tt>true</tt> if sorting should be done reverse, <tt>false</tt> otherwise
     */
    void setReverseSorting( bool r = true ) { mReverseSorting = r; }

    /**
     * Returns the direction of sorting.
     * @return    <tt>true</tt> if sorting is done reverse, <tt>false</tt> otherwise
     */
    bool reverseSorting() const { return mReverseSorting; }

    /**
     * Sorts this list by a specific criterion.
     * @param c    the criterion by which should be sorted 
     */
    void sortBy( SortingCriterion c );

    /** 
     * Sorts this list by a specific field. If no parameter is given, the 
     * last used Field object will be used.
     * @param field    pointer to the Field object to be sorted by
     */
    void sortByField( Field *field = 0 );

    /**
     * Sorts this list by its active sorting criterion. This normally is the 
     * criterion of the last sortBy operation or <tt>FormattedName</tt> if up 
     * to now there has been no sortBy operation.
     *
     * Please note that the sorting trait of the last {@link #sortByTrait} 
     * method call is not remembered and thus the action can not be repeated 
     * by this method.
     */
    void sort();

    /**
     * Templated sort function. You normally will not want to use this but 
     * {@link #sortBy} and {@link #sort} instead as the existing sorting 
     * criteria completely suffice for most cases.
     *
     * However, if you do want to use some special sorting criterion, you can 
     * write a trait class that will be provided to this templated method. 
     * This trait class has to have a class declaration like the following:
     * <pre>
     * class MySortingTrait {
     *   public:
     *     // eq returns true if a1 and a2 are equal
     *     static bool eq(KABC::Addressee a1, KABC::Addressee a2);
     *     // lt returns true is a1 is "less than" a2
     *     static bool lt(KABC::Addressee a1, KABC::Addressee a2);
     * };
     * </pre>
     * You can then pass this class to the sortByTrait method like this:
     * <pre>
     * myAddresseelist.sortByTrait&lt;MySortingTrait&gt;();
     * </pre>
     * Please note that the {@link #sort} method can not be used to repeat the 
     * sorting of the last <tt>sortByTrait</tt> action.
     * 
     * Right now this method uses the bubble sort algorithm. This should be 
     * replaced for a better one when I have time. 
     */
    template<class Trait> void sortByTrait();

    /** 
     * Returns the active sorting criterion, ie the sorting criterion that 
     * will be used by a {@link #sort} call. 
     */
    SortingCriterion sortingCriterion() const { return mActiveSortingCriterion; }

    /**
     * Returns the active sorting field, ie a pointer to the Field object
     * which was used for the last {@link #sortByField} operation.
     */
    Field* sortingField() const { return mActiveSortingField; }

  private:
    bool mReverseSorting;
    SortingCriterion mActiveSortingCriterion;
    Field* mActiveSortingField;
};

};

#endif
// vim:tw=78 cin et sw=2 comments=sr\:/*,mb\:\ ,ex\:*/,\://
