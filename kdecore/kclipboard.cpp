/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qapplication.h>
#include <qclipboard.h>

#include <kconfig.h>
#include <kglobal.h>

#include "kclipboard.h"

bool KClipboard::s_sync = false;
bool KClipboard::s_implicitSelection = true;

void KClipboard::setText( const QString& text, uint mode, bool honorConfiguration )
{
    setClipboard( text, 0L, mode, honorConfiguration );
}

void KClipboard::setData( QMimeSource *data, uint mode, bool honorConfiguration )
{
    setClipboard( QString::null, data, mode, honorConfiguration );
}

void KClipboard::setClipboard( const QString& text, QMimeSource *data,
                               uint mode, bool honorConfiguration )
{
    init();

    QClipboard *clip = QApplication::clipboard();
    bool oldMode = clip->selectionModeEnabled();

    mode = applyConfig( mode, honorConfiguration );

    if ( mode & Clipboard )
    {
        clip->setSelectionMode( false );
        if ( data )
            clip->setData( data );
        else
            clip->setText( text );
    }

    if ( mode & Selection )
    {
        clip->setSelectionMode( true );
        if ( data )
            clip->setData( data );
        else
            clip->setText( text );
    }

    clip->setSelectionMode( oldMode );
}

uint KClipboard::applyConfig( uint mode, bool honorConfiguration )
{
    if ( honorConfiguration )
    {
        if ( s_sync )
            mode = Clipboard | Selection;
        else if ( s_implicitSelection )
            mode |= Selection;
    }

    return mode;
}

void KClipboard::init()
{
    static bool initialized = false;
    if ( !initialized )
    {
        initialized = true;
        KConfigGroup config( KGlobal::config(), "General" );
        s_sync = config.readBoolEntry( "SynchronizeClipboardAndSelection",
                                       s_sync );
        s_implicitSelection = config.readBoolEntry( "ImplicitlySetSelection",
                                                    s_implicitSelection );
    }
}
