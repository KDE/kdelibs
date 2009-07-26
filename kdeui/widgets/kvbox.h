/* This file is part of the KDE libraries
   Copyright (C) 2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KVBOX_H
#define KVBOX_H

#include <khbox.h>

/**
 * A container widget which arranges its children vertically.
 * When using a KVBox you don't need to create a layout nor
 * to add the child widgets to it.
 *
 * Both margin and spacing are initialized to 0. Use QVBoxLayout
 * if you need standard layout margins.
 *
 * \image html kvbox.png "KDE Vertical Box containing three buttons"
 *
 * @see KHBox
 */
class KDEUI_EXPORT KVBox : public KHBox
{
  Q_OBJECT
  
  public:
    /**
     * Creates a new vbox.
     *
     * @param parent The parent widget.
     */
    explicit KVBox( QWidget* parent = 0 );
  
    /**
     * Destructor.
     */
    ~KVBox();

  private:
    class Private;
    friend class Private;
    Private* const d;

    Q_DISABLE_COPY(KVBox)
};

#endif
