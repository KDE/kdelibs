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

#ifndef KCLIPBOARD_H
#define KCLIPBOARD_H

#include <qmime.h>
#include <qobject.h>
#include <qstrlist.h>

class KClipboard : public QObject
{
    Q_OBJECT

public:
    enum Mode { Clipboard = 1, Selection = 2 };

    static KClipboard *self();

    static void setSynchronizing( bool sync )
    {
        s_sync = sync;
    }
    static bool isSynchronizing()
    {
        return s_sync;
    }

    static void setImplicitSelection( bool enable )
    {
        s_implicitSelection = enable;
    }
    static bool implicitSelection()
    {
        return s_implicitSelection;
    }

protected slots:
    void slotSelectionChanged();
    void slotClipboardChanged();

protected:
    ~KClipboard();

private:
    KClipboard( QObject *parent = 0, const char *name = 0L );

    // does not restore the old selection mode.
    static void setClipboard( QMimeSource* data, Mode mode );

    static KClipboard *s_self;
    static bool s_sync;
    static bool s_implicitSelection;
    static bool s_blocked;

    class MimeSource;
    
};

#endif // KCLIPBOARD_H
