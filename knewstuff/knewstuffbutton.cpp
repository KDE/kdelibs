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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kiconloader.h>
#include <klocale.h>

#include "downloaddialog.h"
#include "knewstuffbutton.h"
#include "knewstuffbutton.moc"

namespace KNS
{

Button::Button(const QString& what,
               const QString& providerList,
               const QString& resourceType,
               QWidget* parent, const char* name)
    : KPushButton(parent, name),
      d(0),
      m_providerList(providerList),
      m_type(resourceType),
      m_downloadDialog(0)
{
    setButtonText(what);
    init();
}

Button::Button(QWidget* parent, const char* name)
    : KPushButton(parent, name),
      d(0),
      m_downloadDialog(0)
{
    setButtonText(i18n("Download New Stuff"));
    init();
}

void Button::init()
{
    setIcon(SmallIconSet("knewstuff"));
    connect(this, SIGNAL(clicked()), SLOT(showDialog()));
}

void Button::setButtonText(const QString& what)
{
    setText(i18n("Download New %1").arg(what));
}

void Button::setProviderList(const QString& providerList)
{
    m_providerList = providerList;
}

void Button::setResourceType(const QString& resourceType)
{
    m_type = resourceType;
}

void Button::showDialog()
{
    emit aboutToShowDialog();

    if (!m_downloadDialog)
    {
        m_downloadDialog = new DownloadDialog(0, this);
    }

    m_downloadDialog->setType(m_type);
    m_downloadDialog->load(m_providerList);

    m_downloadDialog->exec(); // TODO: make non-modal?
    emit dialogFinished();
}

}
