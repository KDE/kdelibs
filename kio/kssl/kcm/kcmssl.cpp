/*  This file is part of the KDE project
    Copyright (C) 2010 Andreas Hartmetz <ahartmetz@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "kcmssl.h"
#include "cacertificatespage.h"

#include <kaboutdata.h>
#include <kdeversion.h>

#include <QVBoxLayout>
#include <kpluginfactory.h>
#include <kpluginloader.h>


K_PLUGIN_FACTORY(KcmSslFactory, registerPlugin<KcmSsl>();)
K_EXPORT_PLUGIN(KcmSslFactory("kcm_ssl"))


KcmSsl::KcmSsl(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    KAboutData *about = new KAboutData(
        "kcm_ssl", 0, i18n("SSL Configuration Module"),
        KDE_VERSION_STRING, QString(), KAboutData::License_GPL,
        i18n("Copyright 2010 Andreas Hartmetz"));
    about->addAuthor(i18n("Andreas Hartmetz"), QString(), "ahartmetz@gmail.com");
    setAboutData(about);
    setButtons(Apply | Default | Help);

    m_tabs = new QTabWidget(this);
    // tell the tab widget to resize itself to fill all space, basically...
    setLayout(new QVBoxLayout);
    layout()->setMargin(0);
    layout()->setSpacing(0);
    layout()->addWidget(m_tabs);

    m_caCertificatesPage = new CaCertificatesPage(m_tabs);
    m_tabs->addTab(m_caCertificatesPage, i18n("SSL Signers"));

    connect(m_caCertificatesPage, SIGNAL(changed(bool)), SLOT(pageChanged(bool)));
}

void KcmSsl::load()
{
    m_caCertificatesPage->load();
}

void KcmSsl::save()
{
    m_caCertificatesPage->save();
}

void KcmSsl::defaults()
{
    m_caCertificatesPage->defaults();
}

// slot
void KcmSsl::pageChanged(bool isChanged)
{
    // HACK
    emit changed(isChanged);
}

#include "kcmssl.moc"
