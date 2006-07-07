/*
    This file is part of the KDE Libraries

    Copyright (C) 2006 Pino Toscano <toscano.pino@tiscali.it>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KBUTTONGROUP_H
#define KBUTTONGROUP_H

#include <QGroupBox>

#include <kdelibs_export.h>

/**
 * @short Group box with index of the selected radio button
 * KGroupBox is a simple group box that can keep track of the current selected
 * radio button of the ones added to it.
 *
 * Use normally as you would with a QGroupBox.
 *
 * @author Pino Toscano <toscano.pino@tiscali.it>
 */
class KDEUI_EXPORT KButtonGroup
  : public QGroupBox
{
  Q_OBJECT

  Q_PROPERTY(int current READ selected WRITE setSelected)

  public:
    /**
     * Construct a new empty KGroupBox.
     */
    explicit KButtonGroup( QWidget* parent = 0 );
    /**
     * Destroys the widget.
     */
    ~KButtonGroup();

    /**
     * Return the index of the selected QRadioButton, among the QRadioButton's
     * added to the widget.
     * @return the index of the selected radio button
     */
    int selected() const;

  public Q_SLOTS:
    /**
     * Select the \p id -th radio button
     */
    void setSelected( int id );

  Q_SIGNALS:
    /**
     * The radio button with index \p id was clicked
     */
    void clicked( int id );
    /**
     * The radio button with index \p id was pressed
     */
    void pressed( int id );
    /**
     * The radio button with index \p id was released
     */
    void released( int id );
    /**
     * Emitted when anything (a click on a radio button, or calling setSelected())
     * change the id of the current selected. \p id is the index of the new
     * selected radio button.
     */
    void changed( int id );

  protected:
    /**
     * Reimplemented from QGroupBox.
     */
    virtual void childEvent( QChildEvent* event );

  private Q_SLOTS:
    void slotClicked( int id );

  private:
    class Private;
    Private * const d;
};

#endif

