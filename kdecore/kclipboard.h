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

#include <qclipboard.h>
#include <qmime.h>
#include <qobject.h>
#include <qstrlist.h>

/**
 * This class is only for internal use.
 *
 * @short Allowing to automatically synchronize the X11 Clipboard and Selection buffers.
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 * @since 3.1
 */
class KClipboard : public QObject
{
    Q_OBJECT

public:
    friend class KlipperWidget;
    friend class KApplication;

    /**
     * Returns the KClipboard singleton object.
     * @return the KClipboard singleton object.
     */
    static KClipboard *self();

    /**
     * Configures KClipboard to synchronize the Selection to Clipboard whenever
     * it changes.
     *
     * Default is false.
     * @see #isSynchronizing
     */
    static void setSynchronizing( bool sync );

    /**
     * Checks whether Clipboard and Selection will be synchronized upon changes.
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
     *
     * @param enable true to enable implicit selection, false otherwise.
     * Default is true.
     * @see #selectionSetting
     */
    static void setReverseSynchronizing( bool enable );

    /**
     * Checks whether the  Clipboard buffer will be copied to the Selection
     * buffer upon changes.
     * @returns whether the Clipboard buffer will be copied to the Selection
     * buffer upon changes.
     * @see #setSelectionSetting
     */
    static bool isReverseSynchronizing()
    {
        return s_reverse_sync;
    }


protected:
    ~KClipboard();

private slots:
    void slotSelectionChanged();
    void slotClipboardChanged();

private:
    KClipboard( QObject *parent = 0, const char *name = 0L );
    void setupSignals();

    static void setClipboard( QMimeSource* data, QClipboard::Mode mode );

    static KClipboard *s_self;
    static bool s_sync;
    static bool s_reverse_sync;
    static bool s_blocked;

    class MimeSource;

private:
    // needed by klipper
    enum Configuration { Synchronize = 1 };
    // called by KApplication upon kipc message, invoked by klipper
    static void newConfiguration( int config );

};

#endif // KCLIPBOARD_H
