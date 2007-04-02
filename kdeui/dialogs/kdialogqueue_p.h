/*  This file is part of the KDE Libraries
 *  Copyright (C) 1998 Thomas Tanghus (tanghus@earthling.net)
 *  Additions 1999-2000 by Espen Sand (espen@kde.org)
 *                      and Holger Freyther <freyther@kde.org>
 *            2005-2006   Olivier Goffart <ogoffart @ kde.org>
 *            2006      Tobias Koenig <tokoe@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef KDIALOGQUEUE_H
#define KDIALOGQUEUE_H

/**
 * \brief Queue for showing modal dialogs one after the other.
 *
 * This is useful if you want to show a modal dialog but are not in the
 * position to start a new event loop at that point in your code.
 *
 * The disadvantage is that you will not be able to get any information from
 * the dialog, so it can currently only be used for simple dialogs.
 *
 * You probably want to use KMessageBox::queueMessageBox() instead
 * of this class directly.
 *
 * @author Waldo Bastian <bastian@kde.org>
 */
class KDialogQueue : public QObject
{
  Q_OBJECT

  public:
    static void queueDialog(QDialog *);

    ~KDialogQueue();

  protected:
    KDialogQueue();
    static KDialogQueue *self();

  private:
    Q_PRIVATE_SLOT(d, void slotShowQueuedDialog())

  private:
    class Private;
    friend class Private;  
    Private* const d;

    Q_DISABLE_COPY(KDialogQueue)
};

#endif

