/* This file is part of the KDE libraries
    Copyright (C) 1997 Mario Weilguni (mweilguni@sime.com)
    Copyright (C) 2006 Olivier Goffart <ogoffart@kde.org>

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

#ifndef KDIALOGBUTTONBOX_H
#define KDIALOGBUTTONBOX_H

#include <kdeui_export.h>

#include <QtGui/QDialogButtonBox>

class QPushButton;
class KPushButton;
class KGuiItem;

class KDialogButtonBoxPrivate;
/**
 * Container widget for buttons.
 *
 * An extension of QDialogButtonBox which allow the use of KGuiItem and
 * conveniance slot connection.
 *
 * \image html kdialogbuttonbox.png "Various KDE Dialog Button Boxes with different buttons"
 *
 * @author Mario Weilguni <mweilguni@sime.com>
 * @author Olivier Goffart <ogoffart@kde.org>
 **/

class KDEUI_EXPORT KDialogButtonBox : public QDialogButtonBox
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
  explicit KDialogButtonBox(QWidget *parent, Qt::Orientation _orientation = Qt::Horizontal);

  /**
    * Free private data field
    */
  ~KDialogButtonBox();

  /**
    * Creates a push button with the given text, adds it to the button box for the specified role, and returns the corresponding push button. 
    * If role is invalid, no button is created, and zero is returned.
    *
    * The clicked signal of the button is automatically connected to the slot
    *
    * @param text the text of the button to add.
    * @param role the button role
    * @param receiver An object to connect to.
    * @param slot A Qt slot to connect the 'clicked()' signal to.
    *
    * @return A pointer to the new button.
    */
  QPushButton *addButton(const QString& text, ButtonRole role, QObject * receiver, const char * slot);

  /**
    * Creates a push button with the given KUiItem, adds it to the button box for the specified role, and returns the corresponding push button. 
    * If role is invalid, the button is created, but not added.
    *
    * The clicked signal of the button is automatically connected to the slot
    *
    * @param guiitem text and icon on the button
    * @param role the button role
    * @param receiver An object to connect to.
    * @param slot A Qt slot to connect the 'clicked()' signal to.
    *
    * @return A pointer to the new button.
    */
  KPushButton *addButton(const KGuiItem& guiitem,ButtonRole role, QObject * receiver=0, const char * slot=0);

  using QDialogButtonBox::addButton;

private:
  KDialogButtonBoxPrivate* const d;
};

#endif

