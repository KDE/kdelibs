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
#include "ksharedconfig.h"
#include "kglobal.h"
#include "kglobalsettings.h"

#include <QtCore/QMimeData>
#include <QtDBus/QtDBus>
#include <QtGui/QApplication>
#include <kconfiggroup.h>

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

class KClipboardSynchronizer::Private
{
    public:
        Private(KClipboardSynchronizer *q)
            : q(q)
        {
        }

        void setupSignals();

        static void setClipboard( const QMimeData* data, QClipboard::Mode mode );

        void _k_slotSelectionChanged();
        void _k_slotClipboardChanged();
        void _k_slotNotifyChange(int, int);

        KClipboardSynchronizer *q;
        static bool s_sync;
        static bool s_reverse_sync;
        static bool s_blocked;
};

bool KClipboardSynchronizer::Private::s_sync = false;
bool KClipboardSynchronizer::Private::s_reverse_sync = false;
bool KClipboardSynchronizer::Private::s_blocked = false;

KClipboardSynchronizer * KClipboardSynchronizer::self()
{
    K_GLOBAL_STATIC(KClipboardSynchronizer, s_self)
    return s_self;
}

KClipboardSynchronizer::KClipboardSynchronizer( QObject *parent )
    : QObject( parent ), d(new Private(this))
{
    KConfigGroup config( KGlobal::config(), "General" );
    Private::s_sync = config.readEntry("SynchronizeClipboardAndSelection", Private::s_sync);
    Private::s_reverse_sync = config.readEntry("ClipboardSetSelection", Private::s_reverse_sync);

    d->setupSignals();
}

KClipboardSynchronizer::~KClipboardSynchronizer()
{
    delete d;
}

void KClipboardSynchronizer::Private::setupSignals()
{
    QClipboard *clip = QApplication::clipboard();
    disconnect( clip, NULL, q, NULL );
    if( s_sync )
        connect( clip, SIGNAL( selectionChanged() ),
                 q, SLOT( _k_slotSelectionChanged() ));
    if( s_reverse_sync )
        connect( clip, SIGNAL( dataChanged() ),
                 q, SLOT( _k_slotClipboardChanged() ));

    QDBusConnection::sessionBus().connect( QString(), "/KGlobalSettings", "org.kde.KGlobalSettings",
                                           "notifyChange", q, SLOT(_k_slotNotifyChange(int,int)) );
}

void KClipboardSynchronizer::Private::_k_slotSelectionChanged()
{
    QClipboard *clip = QApplication::clipboard();

//     qDebug("*** sel changed: %i", s_blocked);
    if ( s_blocked || !clip->ownsSelection() )
        return;

    setClipboard( clip->mimeData( QClipboard::Selection ),
                  QClipboard::Clipboard );
}

void KClipboardSynchronizer::Private::_k_slotClipboardChanged()
{
    QClipboard *clip = QApplication::clipboard();

//     qDebug("*** clip changed : %i (implicit: %i, ownz: clip: %i, selection: %i)", s_blocked, s_implicitSelection, clip->ownsClipboard(), clip->ownsSelection());
    if ( s_blocked || !clip->ownsClipboard() )
        return;

    setClipboard( clip->mimeData( QClipboard::Clipboard ),
                  QClipboard::Selection );
}

void KClipboardSynchronizer::Private::_k_slotNotifyChange(int changeType, int arg)
{
    if (changeType == KGlobalSettings::ClipboardConfigChanged) {
        s_sync = (arg & Synchronize);
        KClipboardSynchronizer::self()->d->setupSignals();
    }
}

void KClipboardSynchronizer::Private::setClipboard( const QMimeData *data, QClipboard::Mode mode )
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
    Private::s_sync = sync;
    self()->d->setupSignals();
}

bool KClipboardSynchronizer::isSynchronizing()
{
    return Private::s_sync;
}

void KClipboardSynchronizer::setReverseSynchronizing( bool enable )
{
    Private::s_reverse_sync = enable;
    self()->d->setupSignals();
}

bool KClipboardSynchronizer::isReverseSynchronizing()
{
    return Private::s_reverse_sync;
}

#include "kclipboard.moc"
