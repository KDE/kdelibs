/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
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
#include <qwhatsthis.h>

#include <klocale.h>
#include <knuminput.h>

CupsdNetworkPage::CupsdNetworkPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	setPageLabel(i18n("Network"));
	setHeader(i18n("Network settings"));
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

	hostnamelookup_->insertItem(i18n("Off"));
	hostnamelookup_->insertItem(i18n("On"));
	hostnamelookup_->insertItem(i18n("Double"));

	QLabel *l1 = new QLabel(i18n("Hostname lookups:"), this);
	QLabel *l2 = new QLabel(i18n("Keep-alive timeout:"), this);
	QLabel *l3 = new QLabel(i18n("Max clients:"), this);
	QLabel *l4 = new QLabel(i18n("Max request size:"), this);
	QLabel *l5 = new QLabel(i18n("Client timeout:"), this);
	QLabel *l6 = new QLabel(i18n("Listen to:"), this);

	QGridLayout	*m1 = new QGridLayout(this, 8, 2, 10, 7);
	m1->setRowStretch(7, 1);
	m1->setColStretch(1, 1);
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
	hostnamelookup_->setCurrentItem(conf_->hostnamelookup_);
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
	conf->hostnamelookup_ = hostnamelookup_->currentItem();
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
	QWhatsThis::add(hostnamelookup_, conf->comments_.toolTip("hostnamelookups"));
	QWhatsThis::add(keepalive_, conf->comments_.toolTip("keepalive"));
	QWhatsThis::add(keepalivetimeout_, conf->comments_.toolTip("keepalivetimeout"));
	QWhatsThis::add(maxclients_, conf->comments_.toolTip("maxclients"));
	QWhatsThis::add(maxrequestsize_, conf->comments_.toolTip("maxrequestsize"));
	QWhatsThis::add(clienttimeout_, conf->comments_.toolTip("timeout"));
	QWhatsThis::add(listen_, conf->comments_.toolTip("listen"));
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
