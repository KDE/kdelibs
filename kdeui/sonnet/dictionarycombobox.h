/*
 *  Copyright (c) 2003 Ingo Kloecker <kloecker@kde.org>
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

#ifndef SONNET_DICTIONARYCOMBOBOX_H
#define SONNET_DICTIONARYCOMBOBOX_H

#include "kdeui_export.h"

#include <QComboBox>

class QStringList;
class QString;

namespace Sonnet {

  class Speller;

  /**
   * @short A combo box for selecting the dictionary used for spell checking.
   * @author Ingo Kloecker <kloecker@kde.org>
   **/

  class KDEUI_EXPORT DictionaryComboBox : public QComboBox {
    Q_OBJECT
  public:

    /** 
     * Constructor
     */
    DictionaryComboBox( QWidget * parent=0 );

    /**
     * Destructor
     */
    ~DictionaryComboBox();

    /**
     * Returns the current dictionary name, for example "German (Switzerland)"
     */
    QString currentDictionaryName() const;
    
    /**
     * Returns the current dictionary name, for example "German (Switzerland)"
     */
    QString currentDictionary() const;

    /**
     * Returns the current dictionary name, for example "de_CH"
     */
    QString realDictionaryName() const;

    void setCurrentByDictionaryName( const QString & dictionaryName );
    void setCurrentByDictionary( const QString & dictionary );
    void setCurrentByDictionaryCode( const QString &dictionaryCode );

  signals:
    /** @em Emitted whenever the current dictionary changes. Either
     *  by user intervention or on setCurrentByDictionaryName() or on
     *  setCurrentByDictionary().
     **/
    void dictionaryChanged( const QString & dictionary );
    void dictionaryChanged( int );

  protected slots:
    void slotDictionaryChanged( int );

  protected:
    void reloadCombo();

  protected:
    QStringList mDictionaries;
    int mDefaultDictionary;
    Sonnet::Speller *mspeller;
  };

}

#endif
