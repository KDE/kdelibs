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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>

#include "kclipboard.h"

/*
 * This class provides an automatic synchronization of the X11 Clipboard and Selection
 * buffers. There are two configuration options in the kdeglobals configuration file,
 * in the [General] section:
 * - SynchronizeClipboardAndSelection - whenever the Selection changes, Clipboard is
 *   set to the same value. This can be also enabled in Klipper.
 * - ClipboardSetSelection - whenever the Clipboard changes, Selection is set
 *   to the same value. This setting is only for die-hard fans of the old broken
 *   KDE1/2 behavior, which can potentionally leads to unexpected problems,
 *   and this setting therefore can be enabled only in the configuration dialog.
 *
 *  Whenever reporting any bug only remotely related to clipboard, first make
 *  sure you can reproduce it when both these two options are turned off,
 *  especially the second one.
 */

class KClipboardSynchronizer::MimeSource : public QMimeSource
{
public:
    MimeSource( const QMimeSource * src )
        : QMimeSource(),
          m_formats( true ) // deep copies!
    {
        m_formats.setAutoDelete( true );
        m_data.setAutoDelete( true );

        if ( src )
        {
            QByteArray *byteArray;
            const char *format;
            int i = 0;
            while ( (format = src->format( i++ )) )
            {
                byteArray = new QByteArray();
                *byteArray = src->encodedData( format ).copy();
                m_data.append( byteArray );
                m_formats.append( format );
            }
        }
    }

    ~MimeSource() {}

    virtual const char *format( int i ) const {
        if ( i < (int) m_formats.count() )
            return m_formats.at( i );
        else
            return 0L;
    }
    virtual bool provides( const char *mimeType ) const {
        return ( m_formats.find( mimeType ) > -1 );
    }
    virtual QByteArray encodedData( const char *format ) const
    {
        int index = m_formats.find( format );
        if ( index > -1 )
        {
            // grmbl, gcc (2.95.3 at least) doesn't let me call m_data.at(),
            // due to it being non-const. Even if mutable.
            QPtrList<QByteArray> *list =
                const_cast<QPtrList<QByteArray> *>( &m_data );
            return *(list->at( index ));
        }

        return QByteArray();
    }

private:
    mutable QStrList m_formats;
    QPtrList<QByteArray> m_data;
};


KClipboardSynchronizer * KClipboardSynchronizer::s_self = 0L;
bool KClipboardSynchronizer::s_sync = false;
bool KClipboardSynchronizer::s_reverse_sync = false;
bool KClipboardSynchronizer::s_blocked = false;

KClipboardSynchronizer * KClipboardSynchronizer::self()
{
    if ( !s_self )
        s_self = new KClipboardSynchronizer( kapp, "KDE Clipboard" );

    return s_self;
}

KClipboardSynchronizer::KClipboardSynchronizer( QObject *parent, const char *name )
    : QObject( parent, name )
{
    s_self = this;

    KConfigGroup config( KGlobal::config(), "General" );
    s_sync = config.readBoolEntry( "SynchronizeClipboardAndSelection", s_sync);
    s_reverse_sync = config.readBoolEntry( "ClipboardSetSelection",
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

    setClipboard( new MimeSource( clip->data( QClipboard::Selection) ),
                  QClipboard::Clipboard );
}

void KClipboardSynchronizer::slotClipboardChanged()
{
    QClipboard *clip = QApplication::clipboard();

//     qDebug("*** clip changed : %i (implicit: %i, ownz: clip: %i, selection: %i)", s_blocked, s_implicitSelection, clip->ownsClipboard(), clip->ownsSelection());
    if ( s_blocked || !clip->ownsClipboard() )
        return;

    setClipboard( new MimeSource( clip->data( QClipboard::Clipboard ) ),
                  QClipboard::Selection );
}

void KClipboardSynchronizer::setClipboard( QMimeSource *data, QClipboard::Mode mode )
{
//     qDebug("---> setting clipboard: %p", data);

    QClipboard *clip = QApplication::clipboard();

    s_blocked = true;

    if ( mode == QClipboard::Clipboard )
    {
        clip->setData( data, QClipboard::Clipboard );
    }
    else if ( mode == QClipboard::Selection )
    {
        clip->setData( data, QClipboard::Selection );
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
