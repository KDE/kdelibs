/*
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef _khotnewbutton_h
#define _khotnewbutton_h

#include <kpushbutton.h>

namespace KNS
{

class DownloadDialog;

class Button : public KPushButton
{
    Q_OBJECT

    public:
        Button(const QString& what,
               const QString& providerList,
               const QString& resourceType,
               QWidget* parent, const char* name);
        Button(QWidget* parent, const char* name);

        void setProviderList(const QString& providerList);
        void setResourceType(const QString& resourceType);
        void setButtonText(const QString& what);

    signals:
        void aboutToShowDialog();
        void dialogFinished();

    protected slots:
        void showDialog();

    private:
        void init();

        class ButtonPrivate;
        ButtonPrivate* d;

        QString m_providerList;
        QString m_type;
        DownloadDialog* m_downloadDialog;
};

}

#endif
