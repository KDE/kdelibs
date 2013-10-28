/*
    This file is part of KNewStuff2.
    Copyright (c) 2006, 2007 Josef Spillner <spillner@kde.org>

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

#ifndef KNEWSTUFF2_UI_KDXSVIEW_H
#define KNEWSTUFF2_UI_KDXSVIEW_H

#include <kdialog.h>

#include <knewstuff2/core/providerloader.h>
#include <knewstuff2/knewstuff_export.h>

class QComboBox;

namespace KNS
{

/* -- FIXME: not used at all by the library? -- */
class KDXSView : public KDialog
{
    Q_OBJECT
public:
    KDXSView(QWidget *parent = NULL);
private Q_SLOTS:
    void slotRun();
    void slotProvidersLoaded(KNS::Provider::List providers);
    void slotProvidersFailed();
private:
    enum Access {
        access_auto,
        access_http,
        access_webservice
    };
    QComboBox *m_url;
    QComboBox *m_type;
};

}

#endif
