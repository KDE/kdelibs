/* This file is part of the KDE libraries
    Copyright (C) 1997 Mario Weilguni (mweilguni@sime.com)

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

#ifndef K3BUTTONBOX_H
#define K3BUTTONBOX_H

#include <kde3support_export.h>

#include <QtGui/QWidget>

class QPushButton;
class KGuiItem;

class K3ButtonBoxPrivate;
/**
 * Container widget for buttons.
 *
 * This class uses Qt layout control to place the buttons; can handle
 * both vertical and horizontal button placement.  The default border
 * is now @p 0 (making it easier to deal with layouts). The space
 * between buttons is now more Motif compliant.
 *
 * @deprecated use KDialogButtonBox
 *
 * @author Mario Weilguni <mweilguni@sime.com>
 **/

class KDE3SUPPORT_EXPORT K3ButtonBox : public QWidget
{
  Q_OBJECT

public:
  /**
    * Create an empty container for buttons.
    *
    * If @p _orientation is @p Vertical, the buttons inserted with
    * addButton() are laid out from top to bottom, otherwise they
    * are laid out from left to right.
    */
  explicit K3ButtonBox(QWidget *parent, Qt::Orientation _orientation = Qt::Horizontal,
                      int border = 0, int _autoborder = 6);

  /**
    * Free private data field
    */
  ~K3ButtonBox();

  /**
    * @return The minimum size needed to fit all buttons.
    *
    * This size is
    * calculated by the width/height of all buttons plus border/autoborder.
    */
  virtual QSize sizeHint() const;
  virtual QSizePolicy sizePolicy() const;
  virtual void resizeEvent(QResizeEvent *);

  /**
    * Add a new QPushButton.
    *
    * @param text the text of the button to add
    * @param noexpand If @p noexpand is @p false, the width
    * of the button is adjusted to fit the other buttons (the maximum
    * of all buttons is taken). If @p noexpand is @p true, the width of this
    * button will be set to the minimum width needed for the given text).
    *
    * @return A pointer to the new button.
    */
  QPushButton *addButton(const QString& text, bool noexpand = false);

  /**
    * Add a new QPushButton.
    *
    * @param text the text of the button to add.
    * @param receiver An object to connect to.
    * @param slot A Qt slot to connect the 'clicked()' signal to.
    * @param noexpand If @p noexpand is @p false, the width
    * of the button is adjusted to fit the other buttons (the maximum
    * of all buttons is taken). If @p noexpand @p true, the width of this
    * button will be set to the minimum width needed for the given text).
    *
    * @return A pointer to the new button.
    */
  QPushButton *addButton(const QString& text, QObject * receiver, const char * slot, bool noexpand = false);

  /**
    * Add a new QPushButton.
    *
    * @param guiitem text and icon on the button
    * @param noexpand If @p noexpand is @p false, the width
    * of the button is adjusted to fit the other buttons (the maximum
    * of all buttons is taken). If @p noexpand is @p true, the width of this
    * button will be set to the minimum width needed for the given text).
    *
    * @return A pointer to the new button.
    */
  QPushButton *addButton(const KGuiItem& guiitem, bool noexpand = false);

  /**
    * Add a new QPushButton.
    *
    * @param guiitem text and icon on the button
    * @param receiver An object to connect to.
    * @param slot A Qt slot to connect the 'clicked()' signal to.
    * @param noexpand If @p noexpand is @p false, the width
    * of the button is adjusted to fit the other buttons (the maximum
    * of all buttons is taken). If @p noexpand @p true, the width of this
    * button will be set to the minimum width needed for the given text).
    *
    * @return A pointer to the new button.
    */
  QPushButton *addButton(const KGuiItem& guiitem, QObject * receiver, const char * slot, bool noexpand = false);

  /**
    * Add a stretch to the buttonbox.
    *
    * Can be used to separate buttons.  That is, if you add the
    * buttons OK and Cancel, add a stretch, and then add the button Help,
    * the buttons OK and Cancel will be left-aligned (or top-aligned
    * for vertical) whereas Help will be right-aligned (or
    * bottom-aligned for vertical).
    *
    * @see QBoxLayout
    */
  void addStretch(int scale = 1);

  /**
    * This function must be called @em once after all buttons have been
    * inserted.
    *
    * It will start layout control.
    */
  void layout();

public: // as PrivateData needs Item, it has to be exported
  class Item;
protected:
  /**
    * @return the best size for a button. Checks all buttons and takes
    * the maximum width/height.
    */
  QSize bestButtonSize() const;
  void  placeButtons();
  QSize buttonSizeHint(QPushButton *) const;

private:
  K3ButtonBoxPrivate* const data;
};

#endif

