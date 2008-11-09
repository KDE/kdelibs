/*
 * Copyright (c) 2003 Ingo Kloecker <kloecker@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
*/

#include "dictionarycombobox.h"

#include <kdebug.h>
#include <sonnet/speller.h>

#include <QStringList>

namespace Sonnet {

  DictionaryComboBox::DictionaryComboBox( QWidget * parent )
    : QComboBox( parent ),
      mDefaultDictionary( 0 ),mspeller(0)
  {
    reloadCombo();
    connect( this, SIGNAL( activated( int ) ),
             this, SLOT( slotDictionaryChanged( int ) ) );
  }

  DictionaryComboBox::~DictionaryComboBox()
  {
    delete mspeller;
  }

  QString DictionaryComboBox::currentDictionaryName() const
  {
    return currentText();
  }

  QString DictionaryComboBox::currentDictionary() const
  {
    if ( mDictionaries.empty() )
      return "<default>";
    QString dict = mDictionaries[ currentIndex() ];
    if ( dict.isEmpty() )
      return "<default>";
    else
      return dict;
  }

  QString DictionaryComboBox::realDictionaryName() const
  {
    return mspeller->availableLanguages()[currentIndex()];
  }

  void DictionaryComboBox::setCurrentByDictionaryName( const QString & name )
  {
    if ( name.isEmpty() )
      return;

    for ( int i = 0; i < count(); ++i ) {
      if ( itemText( i ) == name ) {
        if ( i != currentIndex() ) {
          setCurrentIndex( i );
          slotDictionaryChanged( i );
        }
        return;
      }
    }
  }

  void DictionaryComboBox::setCurrentByDictionary( const QString & dictionary )
  {
    if ( !dictionary.isEmpty() ) {
      // first handle the special case of the default dictionary
      if ( dictionary == "<default>" ) {
        if ( 0 != currentIndex() ) {
          setCurrentIndex( 0 );
          slotDictionaryChanged( 0 );
        }
        return;
      }

      int i = 0;
      for ( QStringList::ConstIterator it = mDictionaries.begin();
            it != mDictionaries.end();
            ++it, ++i ) {
        if ( *it == dictionary ) {
          if ( i != currentIndex() ) {
            setCurrentIndex( i );
            slotDictionaryChanged( i );
          }
          return;
        }
      }
    }

    // If dictionary is empty or doesn't exist fall back to the global default
    if ( mDefaultDictionary != currentIndex() ) {
      setCurrentIndex( mDefaultDictionary );
      slotDictionaryChanged( mDefaultDictionary );
    }
  }

  void DictionaryComboBox::setCurrentByDictionaryCode( const QString &dictionaryCode )
  {
    int index = mspeller->availableLanguages().indexOf( dictionaryCode );
    if ( index != -1 && index != currentIndex() ) {
      setCurrentIndex( index );
      slotDictionaryChanged( index );
    }
  }

  void DictionaryComboBox::reloadCombo()
  {
    mspeller = new Sonnet::Speller();
    mDictionaries = mspeller->availableLanguageNames();
    insertItems( 0, mDictionaries );
    mDefaultDictionary = currentIndex();
  }

  void DictionaryComboBox::slotDictionaryChanged( int idx )
  {
    kDebug( 5006 ) << idx;
    if( !mDictionaries.isEmpty())
      {
        emit dictionaryChanged( mspeller->availableLanguages()[idx] );
        //kDebug(5006)<<"mDictionaries[idx]mDictionaries[idx] :"<<mDictionaries[idx];
        //kDebug(5006)<<" mspeller->availableLanguages()[i]  :"<<mspeller->availableLanguages()[idx];
      }
    emit dictionaryChanged( idx );
  }

}

#include "dictionarycombobox.moc"
