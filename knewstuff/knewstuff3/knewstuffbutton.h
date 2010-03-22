/*
    This file is part of KNewStuff2.
    Copyright (c) 2004 Aaron J. Seigo <aseigo@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KNEWSTUFF3BUTTON_H
#define KNEWSTUFF3BUTTON_H

#include <kpushbutton.h>

#include <knewstuff3/knewstuff_export.h>
#include <knewstuff3/entry.h>

namespace KNS3
{

/**
 * KHotNewStuff push button that makes using KHNS in an application
 * more convenient by encapsulating most of the details involved in
 * using KHotNewStuff in the button itself.
 *
 * @since 4.4
 */
class KNEWSTUFF_EXPORT Button : public KPushButton
{
    Q_OBJECT

public:
    /**
     * Constructor used when the details of the KHotNewStuff
     * download is known when the button is created.
     *
     * @param text describing what is being downloaded.
     *        It should be a text beginning with "Download New ..." for consistency
     * @param configFile the name of the .knsrc file
     * @param parent the parent widget
     */
    Button(const QString& text,
           const QString& configFile,
           QWidget* parent);

    /**
     * Constructor used when the details of the KHotNewStuff
     * download is not known in advance of the button being created.
     *
     * @param parent the parent widget
     */
    Button(QWidget* parent);

    ~Button();

    /**
     * set the name of the .knsrc file to use
     */
    void setConfigFile(const QString& configFile);

    /**
     * set the text that should appear on the button. will be prefaced
     * with i18n("Download New")
     */
    void setButtonText(const QString& text);

Q_SIGNALS:
    /**
     * emitted when the Hot New Stuff dialog is about to be shown, usually
     * as a result of the user having click on the button
     */
    void aboutToShowDialog();

    /**
     * emitted when the Hot New Stuff dialog has been closed
     */
    void dialogFinished(const KNS3::Entry::List& changedEntries);

protected Q_SLOTS:
    void showDialog();

private:
    void init();

    class Private;
    Private* const d;
};

}

#endif // KNEWSTUFFBUTTON_H
