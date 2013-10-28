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

#include "kdialogqueue_p.h"
#include <QPointer>
#include <QTimer>

class KDialogQueue::Private
{
  public:
    Private(KDialogQueue *q): q(q) {}

    void slotShowQueuedDialog();

    KDialogQueue *q;
    QList< QPointer<QDialog> > queue;
    bool busy;

};

class KDialogQueueSingleton
{
public:
    KDialogQueue self;
};

Q_GLOBAL_STATIC(KDialogQueueSingleton, globalKDialogQueue)

KDialogQueue* KDialogQueue::self()
{
    return &globalKDialogQueue()->self;
}

KDialogQueue::KDialogQueue()
  : d( new Private(this) )
{
  d->busy = false;
}

KDialogQueue::~KDialogQueue()
{
  delete d;
}

// static
void KDialogQueue::queueDialog( QDialog *dialog )
{
  KDialogQueue *_this = self();
  _this->d->queue.append( dialog );

  QTimer::singleShot( 0, _this, SLOT(slotShowQueuedDialog()) );
}

void KDialogQueue::Private::slotShowQueuedDialog()
{
  if ( busy )
    return;

  QDialog *dialog;
  do {
    if ( queue.isEmpty() )
      return;
    dialog = queue.first();
    queue.pop_front();
  } while( !dialog );

  busy = true;
  dialog->exec();
  busy = false;
  delete dialog;

  if ( !queue.isEmpty() )
    QTimer::singleShot( 20, q, SLOT(slotShowQueuedDialog()) );
}

#include "moc_kdialogqueue_p.cpp"
