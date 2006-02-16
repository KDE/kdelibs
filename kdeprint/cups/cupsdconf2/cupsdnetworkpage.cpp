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

#include "cupsdnetworkpage.h"
#include "cupsdconf.h"
#include "editlist.h"
#include "portdialog.h"
#include "sizewidget.h"

#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlayout.h>

#include <klocale.h>
#include <knuminput.h>

CupsdNetworkPage::CupsdNetworkPage(QWidget *parent)
	: CupsdPage(parent)
{
	setPageLabel(i18n("Network"));
	setHeader(i18n("Network Settings"));
	setPixmap("network");

	keepalive_ = new QCheckBox(i18n("Keep alive"), this);
	keepalivetimeout_ = new KIntNumInput(this);
	maxclients_ = new KIntNumInput(this);
	maxrequestsize_ = new SizeWidget(this);
	clienttimeout_ = new KIntNumInput(this);
	hostnamelookup_ = new QComboBox(this);
	listen_ = new EditList(this);

	keepalivetimeout_->setRange(0, 10000, 1, true);
	keepalivetimeout_->setSteps(1, 10);
	keepalivetimeout_->setSpecialValueText(i18n("Unlimited"));
	keepalivetimeout_->setSuffix(i18n(" sec"));

	maxclients_->setRange(1, 1000, 1, true);
	maxclients_->setSteps(1, 10);

	clienttimeout_->setRange(0, 10000, 1, true);
	clienttimeout_->setSteps(1, 10);
	clienttimeout_->setSpecialValueText(i18n("Unlimited"));
	clienttimeout_->setSuffix(i18n(" sec"));

	hostnamelookup_->addItem(i18n("Off"));
	hostnamelookup_->addItem(i18n("On"));
	hostnamelookup_->addItem(i18n("Double"));

	QLabel *l1 = new QLabel(i18n("Hostname lookups:"), this);
	QLabel *l2 = new QLabel(i18n("Keep-alive timeout:"), this);
	QLabel *l3 = new QLabel(i18n("Max clients:"), this);
	QLabel *l4 = new QLabel(i18n("Max request size:"), this);
	QLabel *l5 = new QLabel(i18n("Client timeout:"), this);
	QLabel *l6 = new QLabel(i18n("Listen to:"), this);

	QGridLayout	*m1 = new QGridLayout(this);
  m1->setMargin(7);
  m1->setSpacing(10);
	m1->setRowStretch(7, 1);
	m1->setColumnStretch(1, 1);
	m1->addWidget(l1, 0, 0, Qt::AlignRight);
	m1->addWidget(l2, 2, 0, Qt::AlignRight);
	m1->addWidget(l3, 3, 0, Qt::AlignRight);
	m1->addWidget(l4, 4, 0, Qt::AlignRight);
	m1->addWidget(l5, 5, 0, Qt::AlignRight);
	m1->addWidget(l6, 6, 0, Qt::AlignTop|Qt::AlignRight);
	m1->addWidget(keepalive_, 1, 1);
	m1->addWidget(hostnamelookup_, 0, 1);
	m1->addWidget(keepalivetimeout_, 2, 1);
	m1->addWidget(maxclients_, 3, 1);
	m1->addWidget(maxrequestsize_, 4, 1);
	m1->addWidget(clienttimeout_, 5, 1);
	m1->addWidget(listen_, 6, 1);

	connect(listen_, SIGNAL(add()), SLOT(slotAdd()));
	connect(listen_, SIGNAL(edit(int)), SLOT(slotEdit(int)));
	connect(listen_, SIGNAL(defaultList()), SLOT(slotDefaultList()));
	connect(keepalive_, SIGNAL(toggled(bool)), keepalivetimeout_, SLOT(setEnabled(bool)));
	keepalive_->setChecked(true);
}

bool CupsdNetworkPage::loadConfig(CupsdConf *conf, QString&)
{
	conf_ = conf;
	hostnamelookup_->setCurrentIndex(conf_->hostnamelookup_);
	keepalive_->setChecked(conf_->keepalive_);
	keepalivetimeout_->setValue(conf_->keepalivetimeout_);
	maxclients_->setValue(conf_->maxclients_);
	maxrequestsize_->setSizeString(conf_->maxrequestsize_);
	clienttimeout_->setValue(conf_->clienttimeout_);
	listen_->insertItems(conf_->listenaddresses_);

	return true;
}

bool CupsdNetworkPage::saveConfig(CupsdConf *conf, QString&)
{
	conf->hostnamelookup_ = hostnamelookup_->currentIndex();
	conf->keepalive_ = keepalive_->isChecked();
	conf->keepalivetimeout_ = keepalivetimeout_->value();
	conf->maxclients_ = maxclients_->value();
	conf->maxrequestsize_ = maxrequestsize_->sizeString();
	conf->clienttimeout_ = clienttimeout_->value();
	conf->listenaddresses_ = listen_->items();

	return true;
}

void CupsdNetworkPage::setInfos(CupsdConf *conf)
{
	hostnamelookup_->setWhatsThis(conf->comments_.toolTip("hostnamelookups"));
	keepalive_->setWhatsThis(conf->comments_.toolTip("keepalive"));
	keepalivetimeout_->setWhatsThis(conf->comments_.toolTip("keepalivetimeout"));
	maxclients_->setWhatsThis(conf->comments_.toolTip("maxclients"));
	maxrequestsize_->setWhatsThis(conf->comments_.toolTip("maxrequestsize"));
	clienttimeout_->setWhatsThis(conf->comments_.toolTip("timeout"));
	listen_->setWhatsThis(conf->comments_.toolTip("listen"));
}

void CupsdNetworkPage::slotAdd()
{
	QString	s = PortDialog::newListen(this, conf_);
	if (!s.isEmpty())
		listen_->insertItem(s);
}

void CupsdNetworkPage::slotEdit(int index)
{
	QString s = listen_->text(index);
	s = PortDialog::editListen(s, this, conf_);
	if (!s.isEmpty())
		listen_->setText(index, s);
}

void CupsdNetworkPage::slotDefaultList()
{
	listen_->clear();
	QStringList	l;
	l << "Listen *:631";
	listen_->insertItems(l);
}

#include "cupsdnetworkpage.moc"
