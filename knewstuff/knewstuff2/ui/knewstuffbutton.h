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

#ifndef KNEWSTUFFBUTTON_H
#define KNEWSTUFFBUTTON_H

#include <knewstuff2/knewstuff_export.h>

#include <kpushbutton.h>

namespace KNS
{

class Engine;

// FIXME: port properly to KNS2, less accessors for apps, more *.knsrc usage
/**
 * KHotNewStuff push button that makes using KHNS in an application
 * more convenient by encapsulating most of the details involved in
 * using KHotNewStuff in the button itself.*
 *
 * Deprecated, use knewstuff3!
 */
class KNEWSTUFF_EXPORT_DEPRECATED Button : public KPushButton
{
    Q_OBJECT

public:
    /**
     * Constructor used when the details of the KHotNewStuff
     * download is known when the button is created.
     *
     * @param what text describing what is being downloaded. will be
     *        shown on the button as "Download New <what>"
     * @param providerList the URL to the list of providers; if empty
     *        we first try the ProvidersUrl from KGlobal::config, then we
     *        fall back to a hardcoded value
     * @param resourceType the Hotstuff data type for this downlaod such
     *        as "korganizer/calendar"
     * @param parent the parent widget
     * @param name the name to be used for this widget
     */
    Button(const QString& what,
           const QString& providerList,
           const QString& resourceType,
           QWidget* parent);

    /**
     * Constructor used when the details of the KHotNewStuff
     * download is not known in advance of the button being created.
     *
     * @param parent the parent widget
     */
    Button(QWidget* parent);

    /**
     * set the URL to the list of providers for this button to use
     */
    void setProviderList(const QString& providerList);

    /**
     * the Hotstuff data type for this downlaod such as
     * "korganizer/calendar"
     */
    void setResourceType(const QString& resourceType);

    /**
     * set the text that should appear on the button. will be prefaced
     * with i18n("Download New")
     */
    void setButtonText(const QString& what);

Q_SIGNALS:
    /**
     * emitted when the Hot New Stuff dialog is about to be shown, usually
     * as a result of the user having click on the button
     */
    void aboutToShowDialog();

    /**
     * emitted when the Hot New Stuff dialog has been closed
     */
    void dialogFinished();

protected Q_SLOTS:
    void showDialog();

private:
    void init();

    class ButtonPrivate;
    ButtonPrivate* const d;

    QString m_providerList;
    QString m_type;
    Engine * m_engine;
};

}

#endif // KNEWSTUFFBUTTON_H
