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

#ifndef KCLIPBOARD_H
#define KCLIPBOARD_H

#include <kdeui_export.h>

#include <QtCore/QObject>
#include <QtGui/QClipboard>

class QMimeData;

/**
 * This class is only for internal use.
 *
 * @short Allowing to automatically synchronize the X11 Clipboard and Selection buffers.
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KDEUI_EXPORT KClipboardSynchronizer : public QObject
{
    Q_OBJECT

public:
    friend class KlipperWidget;

    /**
     * Returns the KClipboardSynchronizer singleton object.
     * @return the KClipboardSynchronizer singleton object.
     */
    static KClipboardSynchronizer *self();

    /**
     * Configures KClipboardSynchronizer to synchronize the Selection to Clipboard whenever
     * it changes.
     *
     * Default is false.
     * @see isSynchronizing
     */
    static void setSynchronizing( bool sync );

    /**
     * Checks whether Clipboard and Selection will be synchronized upon changes.
     * @returns whether Clipboard and Selection will be synchronized upon
     * changes.
     * @see setSynchronizing
     */
    static bool isSynchronizing();

    /**
     * Configures KClipboardSynchronizer to copy the Clipboard buffer to the Selection
     * buffer whenever the Clipboard changes.
     *
     *
     * @param enable true to enable implicit selection, false otherwise.
     * Default is true.
     * @see selectionSetting
     */
    static void setReverseSynchronizing( bool enable );

    /**
     * Checks whether the  Clipboard buffer will be copied to the Selection
     * buffer upon changes.
     * @returns whether the Clipboard buffer will be copied to the Selection
     * buffer upon changes.
     * @see setSelectionSetting
     */
    static bool isReverseSynchronizing();

protected:
    ~KClipboardSynchronizer();

private:
    // needed by klipper
    enum Configuration { Synchronize = 1 };

    explicit KClipboardSynchronizer( QObject *parent = 0 );

    class Private;
    Private* const d;

    Q_PRIVATE_SLOT(d, void _k_slotSelectionChanged())
    Q_PRIVATE_SLOT(d, void _k_slotClipboardChanged())
    Q_PRIVATE_SLOT(d, void _k_slotNotifyChange(int, int))
};

#endif // KCLIPBOARD_H
