/*
 * klangbutton.h - Button with language selection drop down menu.
 *                 Derived from the KLangCombo class by Hans Petter Bieker.
 *
 * Copyright (c) 1999-2003 Hans Petter Bieker <bieker@kde.org>
 *           (c) 2001      Martijn Klingens <klingens@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef __KLANGBUTTON_H__
#define __KLANGBUTTON_H__

#include <qwidget.h>

#include <kdelibs_export.h>

class KLanguageButtonPrivate;
class QIconSet;
class QPopupMenu;

/**
 * KLanguageButton provides a combobox with a 2-D dataset. It also supports icons.
 * It is also possible to construct a non combobox version.
 *
 * All items are identified using strings, not integers.
 *
 * Combined version of KTagCombo and KLanguageCombo but using a QPushButton
 * instead.
 */
class KDEUI_EXPORT KLanguageButton : public QWidget
{
  Q_OBJECT

public:
  /**
   * Constructs a combobox widget with parent parent called name.
   *
   * @param parent The parent of the combo box
   * @param name The name of the combo box
   */
  KLanguageButton(QWidget * parent = 0, const char * name = 0);
  
  /**
   * Constructs a version with static text. The parent parent is called name.
   *
   * @param text The text of the button
   * @param parent The parent of the button
   * @param name The name of the button
   */
  KLanguageButton(const QString & text, QWidget * parent = 0, const char * name = 0);
  
  /**
   * Deconstructor
   */ 
  virtual ~KLanguageButton();

  /**
   * Inserts an item into the combo box. A negative index will append the item.
   *
   * @param icon The icon used used when displaying the item.
   * @param text The text string used when displaying the item.
   * @param id The text string used to identify the item.
   * @param submenu The place where the item should be placed.
   * @param index The visual position in the submenu.
   */
  void insertItem( const QIconSet& icon, const QString &text,
                   const QString & id, const QString &submenu = QString::null,
                   int index = -1 );
  /**
   * Inserts an item into the combo box. A negative index will append the item.
   *
   * @param text The text string used when displaying the item.
   * @param id The text string used to identify the item.
   * @param submenu The place where the item should be placed.
   * @param index The visual position in the submenu.
   */
  void insertItem( const QString &text, const QString & id,
                   const QString &submenu = QString::null, int index = -1 );
  /**
   * Inserts a seperator item into the combo box. A negative index will append the item.
   *
   * @param submenu The place where the item should be placed.
   * @param index The visual position in the submenu.
   */
  void insertSeparator( const QString &submenu = QString::null,
                        int index = -1 );
  /**
   * Inserts a submenu into the combo box. A negative index will append the item.
   *
   * @param icon The icon used used when displaying the item.
   * @param text The text string used when displaying the item.
   * @param id The text string used to identify the item.
   * @param submenu The place where the item should be placed.
   * @param index The visual position in the submenu.
   */
  void insertSubmenu( const QIconSet & icon, const QString &text,
                      const QString & id, const QString &submenu = QString::null,
                      int index = -1);
  /**
   * Inserts a submenu into the combo box. A negative index will append the item.
   *
   * @param text The text string used when displaying the item.
   * @param id The text string used to identify the item.
   * @param submenu The place where the item should be placed.
   * @param index The visual position in the submenu.
   */
  void insertSubmenu( const QString &text, const QString & id,
                      const QString &submenu = QString::null, int index = -1);


  void insertLanguage( const QString& path, const QString& name,
                       const QString& sub = QString::null,
                       const QString &submenu = QString::null, int index = -1);


  int count() const;
  /**
   * Removes all combobox items.
   */
  void clear();
  /**
   * Changes the current text item of the combobox, and makes the text static.
   */
  void setText(const QString & text);
  /**
   * Returns the id of the combobox's current item.
   */
  QString current() const;
  /**
   * Returns TRUE if the combobox contains id.
   */
  bool contains( const QString & id ) const;
  /**
   * Sets id as current item.
   */
  void setCurrentItem( const QString & id );

    QString id( int i ) const;

signals:
  /**
   * This signal is emitted when a new item is activated. The id is
   * the identificator of the selected item.
   */
  void activated( const QString & id );
  void highlighted( const QString & id );

private slots:
  void slotActivated( int );
  void slotHighlighted( int );

private:
  int currentItem() const;
  void setCurrentItem( int );
  void init(const char * name);

  // work space for the new class
  QStringList *m_ids;
  QPopupMenu *m_popup, *m_oldPopup;
  int m_current;

  KLanguageButtonPrivate * d;
};

#endif
