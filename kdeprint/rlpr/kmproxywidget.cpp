/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kmproxywidget.h"

#include <QtGui/QLineEdit>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QLayout>
#include <QtGui/QValidator>
#include <klocale.h>
#include <kconfiggroup.h>
#include <kcursor.h>

KMProxyWidget::KMProxyWidget(QWidget *parent)
        : QGroupBox(i18n("Proxy Settings"), parent)
{
    setLayout(new QVBoxLayout);

    QLabel *m_hostlabel = new QLabel(i18n("&Host:"), this);
    QLabel *m_portlabel = new QLabel(i18n("&Port:"), this);
    m_useproxy = new QCheckBox(i18n("&Use proxy server"), this);
    m_useproxy->setCursor(QCursor(Qt::PointingHandCursor));
    m_proxyhost = new QLineEdit(this);
    m_proxyport = new QLineEdit(this);
    m_proxyport->setValidator(new QIntValidator(m_proxyport));
    m_hostlabel->setBuddy(m_proxyhost);
    m_portlabel->setBuddy(m_proxyport);

    connect(m_useproxy, SIGNAL(toggled(bool)), m_proxyhost, SLOT(setEnabled(bool)));
    connect(m_useproxy, SIGNAL(toggled(bool)), m_proxyport, SLOT(setEnabled(bool)));
    m_proxyhost->setEnabled(false);
    m_proxyport->setEnabled(false);

    QGridLayout *lay0 = new QGridLayout();
    layout()->addItem(lay0);
    lay0->setMargin(10);
    lay0->setColumnStretch(1, 1);
    lay0->addWidget(m_useproxy, 0, 0, 1, 2);
    lay0->addWidget(m_hostlabel, 1, 0);
    lay0->addWidget(m_portlabel, 2, 0);
    lay0->addWidget(m_proxyhost, 1, 1);
    lay0->addWidget(m_proxyport, 2, 1);
}

void KMProxyWidget::loadConfig(KConfig *conf)
{
    KConfigGroup cg(conf, "RLPR");
    m_proxyhost->setText(cg.readEntry("ProxyHost"));
    m_proxyport->setText(cg.readEntry("ProxyPort"));
    m_useproxy->setChecked(!m_proxyhost->text().isEmpty());
}

void KMProxyWidget::saveConfig(KConfig *conf)
{
    KConfigGroup cg(conf, "RLPR");
    cg.writeEntry("ProxyHost", (m_useproxy->isChecked() ? m_proxyhost->text() : QString()));
    cg.writeEntry("ProxyPort", (m_useproxy->isChecked() ? m_proxyport->text() : QString()));
}
