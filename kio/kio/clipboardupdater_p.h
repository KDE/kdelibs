/* This file is part of the KDE libraries
    Copyright (C) 2013 Dawit Alemayehu <adawit@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KIO_CLIPBOARDUPDATER_P_H
#define KIO_CLIPBOARDUPDATER_P_H

#include <QObject>

class KJob;
class KUrl;

namespace KIO  {
    class Job;

    /**
     * Updates the clipboard when it is affected by KIO operations.
     *
     * UpdateContent updates clipboard urls that were modified. This mode should
     * be the one preferred by default because it will not change the contents
     * of the clipboard if the urls modified by the job are not found in the
     * clipboard.
     *
     * OverwriteContent blindly replaces all urls in the clipboard with the ones
     * from the job. This mode should not be used unless you are 100% certain that
     * the urls in the clipboard are actually there for the purposes of carrying
     * out the specified job. This mode for example is used by the KIO::pasteClipboard
     * job when a user performs a cut+paste operation.
     *
     * This class also sets @ref job as its parent object. As such, when @ref job
     * is deleted the instance of ClipboardUpdater you create will also be deleted
     * as well.
     */
    class ClipboardUpdater : public QObject
    {
        Q_OBJECT

    public:
        enum Mode {
            UpdateContent,
            OverwriteContent,
            RemoveContent
        };

        /**
         * Returns an instance of clipboard updater if QApplication::type() does
         * not return a tty. Otherwise, it returns null.
         */
        static ClipboardUpdater* create(Job* job, Mode mode);

        /**
         * Convenience function that allows renaming of a single url in the clipboard.
         * Note that this function does nothing if QApplication::type() returns tty.
         */
        static void update(const KUrl& srcUrl, const KUrl& destUrl);

        /**
         * Sets the mode.
         */
        void setMode(Mode m);

    private Q_SLOTS:
        void slotResult(KJob* job);

    private:
        explicit ClipboardUpdater(Job* job, Mode mode);
        Mode m_mode;
    };
}

#endif
