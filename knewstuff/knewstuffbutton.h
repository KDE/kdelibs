/*
    Copyright (c) 2004 Aaron J. Seigo <aseigo@kde.org>

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

#ifndef _khotnewbutton_h
#define _khotnewbutton_h

#include <kpushbutton.h>

namespace KNS
{

class DownloadDialog;

/**
 * KHotNewStuff push button that makes using KHNS in an application
 * more convenient by encapsulating most of the details involved in
 * using KHotNewStuff in the button itself.
 *
 * @since 3.4
 */
class Button : public KPushButton
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
               QWidget* parent, const char* name);

        /**
         * Constructor used when the details of the KHotNewStuff
         * download is not known in advance of the button being created.
         *
         * @param parent the parent widget
         * @param name the name to be used for this widget
         */
        Button(QWidget* parent, const char* name);

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

    signals:
        /**
         * emitted when the Hot New Stuff dialog is about to be shown, usually
         * as a result of the user having click on the button
         */
        void aboutToShowDialog();

        /**
         * emitted when the Hot New Stuff dialog has been closed
         */
        void dialogFinished();

    protected slots:
        void showDialog();

    private:
        void init();

        class ButtonPrivate;
        ButtonPrivate* const d;

        QString m_providerList;
        QString m_type;
        DownloadDialog* m_downloadDialog;
};

}

#endif
