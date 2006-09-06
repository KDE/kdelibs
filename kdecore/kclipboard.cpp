/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kclipboard.h"
#include "kconfig.h"
#include "kglobal.h"
#include "kstaticdeleter.h"

#include <qapplication.h>
#include <qmime.h>

/*
 * This class provides an automatic synchronization of the X11 Clipboard and Selection
 * buffers. There are two configuration options in the kdeglobals configuration file,
 * in the [General] section:
 * - SynchronizeClipboardAndSelection - whenever the Selection changes, Clipboard is
 *   set to the same value. This can be also enabled in Klipper.
 * - ClipboardSetSelection - whenever the Clipboard changes, Selection is set
 *   to the same value. This setting is only for die-hard fans of the old broken
 *   KDE1/2 behavior, which can potentionally lead to unexpected problems,
 *   and this setting therefore can be enabled only in the configuration file.
 *
 *  Whenever reporting any bug only remotely related to clipboard, first make
 *  sure you can reproduce it when both these two options are turned off,
 *  especially the second one.
 */

KClipboardSynchronizer * KClipboardSynchronizer::s_self = 0L;
KStaticDeleter<KClipboardSynchronizer> kclipsync_sd;
bool KClipboardSynchronizer::s_sync = false;
bool KClipboardSynchronizer::s_reverse_sync = false;
bool KClipboardSynchronizer::s_blocked = false;

KClipboardSynchronizer * KClipboardSynchronizer::self()
{
    if ( !s_self ) {
        kclipsync_sd.setObject( s_self, new KClipboardSynchronizer );
        s_self->setObjectName( "KDE Clipboard" );
    }

    return s_self;
}

KClipboardSynchronizer::KClipboardSynchronizer( QObject *parent )
    : QObject( parent )
{
    s_self = this;

    KConfigGroup config( KGlobal::config(), "General" );
    s_sync = config.readEntry( "SynchronizeClipboardAndSelection", s_sync);
    s_reverse_sync = config.readEntry( "ClipboardSetSelection",
                                       s_reverse_sync );

    setupSignals();
}

KClipboardSynchronizer::~KClipboardSynchronizer()
{
    if ( s_self == this )
        s_self = 0L;
}

void KClipboardSynchronizer::setupSignals()
{
    QClipboard *clip = QApplication::clipboard();
    disconnect( clip, NULL, this, NULL );
    if( s_sync )
        connect( clip, SIGNAL( selectionChanged() ),
                 SLOT( slotSelectionChanged() ));
    if( s_reverse_sync )
        connect( clip, SIGNAL( dataChanged() ),
                 SLOT( slotClipboardChanged() ));
}

void KClipboardSynchronizer::slotSelectionChanged()
{
    QClipboard *clip = QApplication::clipboard();

//     qDebug("*** sel changed: %i", s_blocked);
    if ( s_blocked || !clip->ownsSelection() )
        return;

    setClipboard( clip->mimeData( QClipboard::Selection ),
                  QClipboard::Clipboard );
}

void KClipboardSynchronizer::slotClipboardChanged()
{
    QClipboard *clip = QApplication::clipboard();

//     qDebug("*** clip changed : %i (implicit: %i, ownz: clip: %i, selection: %i)", s_blocked, s_implicitSelection, clip->ownsClipboard(), clip->ownsSelection());
    if ( s_blocked || !clip->ownsClipboard() )
        return;

    setClipboard( clip->mimeData( QClipboard::Clipboard ),
                  QClipboard::Selection );
}

void KClipboardSynchronizer::setClipboard( const QMimeData *data, QClipboard::Mode mode )
{
//     qDebug("---> setting clipboard: %p", data);

    QClipboard *clip = QApplication::clipboard();

    s_blocked = true;

    QMimeData* clipData = const_cast<QMimeData*>( data );
    if ( mode == QClipboard::Clipboard )
    {
        clip->setMimeData( clipData, QClipboard::Clipboard );
    }
    else if ( mode == QClipboard::Selection )
    {
        clip->setMimeData( clipData, QClipboard::Selection );
    }

    s_blocked = false;
}

void KClipboardSynchronizer::setSynchronizing( bool sync )
{
    s_sync = sync;
    self()->setupSignals();
}

void KClipboardSynchronizer::setReverseSynchronizing( bool enable )
{
    s_reverse_sync = enable;
    self()->setupSignals();
}

// private, called by KApplication
void KClipboardSynchronizer::newConfiguration( int config )
{
    s_sync = (config & Synchronize);
    self()->setupSignals();
}

#include "kclipboard.moc"
