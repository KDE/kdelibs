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

#ifndef KCLIPBOARD_H
#define KCLIPBOARD_H

#include <qmime.h>
#include <qobject.h>
#include <qstrlist.h>

/**
 * This class is mostly of internal use. You probably don't need it :)
 *
 * It provides an automatic synchronization of the X11 Clipboard and Selection
 * buffers. It connects to the selectionChanged() and dataChanged() signals of
 * QClipboard and copies the buffer's contents to the other buffer, if configured.
 *
 * Additionally to keeping them in sync, there is the option to automatically copy
 * the clipboard buffer to the selection buffer, when your application sets the
 * clipboard buffer. That is the default behavior in KDE.
 *
 * If you don't want any synchronizing or implicit copying, you can disable this
 * with the methods below.
 *
 * @short Allowing to automatically synchronize the X11 Clipboard and Selection buffers.
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KClipboard : public QObject
{
    Q_OBJECT

public:
    friend class KlipperWidget;
    friend class KApplication;

    enum Mode { Clipboard = 1, Selection = 2 };
    /**
     * @returns the KClipboard singleton object.
     */
    static KClipboard *self();

    /**
     * Configures KClipboard to synchronize the Clipboard and Selection buffers
     * whenever one changes.
     *
     * Default is false.
     * @see #isSynchronizing
     */
    static void setSynchronizing( bool sync )
    {
        s_sync = sync;
    }

    /**
     * @returns whether Clipboard and Selection will be synchronized upon
     * changes.
     * @see #setSynchronizing
     */
    static bool isSynchronizing()
    {
        return s_sync;
    }

    /**
     * Configures KClipboard to copy the Clipboard buffer to the Selection
     * buffer whenever the Clipboard changes.
     *
     * Default is true.
     * @see #implicitSelection
     */
    static void setImplicitSelection( bool enable )
    {
        s_implicitSelection = enable;
    }

    /**
     * @returns whether the Clipboard buffer will be copied to the Selection
     * buffer upon changes.
     * @see #setImplicitSelection
     */
    static bool implicitSelection()
    {
        return s_implicitSelection;
    }

protected:
    ~KClipboard();

private slots:
    void slotSelectionChanged();
    void slotClipboardChanged();

private:
    KClipboard( QObject *parent = 0, const char *name = 0L );

    // does not restore the old selection mode.
    static void setClipboard( QMimeSource* data, Mode mode );

    static KClipboard *s_self;
    static bool s_sync;
    static bool s_implicitSelection;
    static bool s_blocked;

    class MimeSource;

private:
    // needed by klipper
    enum Configuration { Synchronize = 1, ImplicitSelection = 2 };
    // called by KApplication upon kipc message, invoked by klipper
    static void newConfiguration( int config );


};

#endif // KCLIPBOARD_H
