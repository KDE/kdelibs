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

#include "cupsdserverpage.h"
#include "cupsdconf.h"

#include <qlineedit.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qwhatsthis.h>

#include <klocale.h>

int findComboItem(QComboBox *cb, const QString& str)
{
	for (int i=0; i<cb->count(); i++)
		if (cb->text(i) == str)
			return i;
	return (-1);
}

CupsdServerPage::CupsdServerPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	setPageLabel(i18n("Server"));
	setHeader(i18n("Server Settings"));
	setPixmap("gear");

	servername_ = new QLineEdit(this);
	serveradmin_ = new QLineEdit(this);
	otherclassname_ = new QLineEdit(this);
	language_ = new QLineEdit(this);
	printcap_ = new QLineEdit(this);
	classification_ = new QComboBox(this);
	charset_ = new QComboBox(this);
	printcapformat_ = new QComboBox(this);
	classoverride_ = new QCheckBox(i18n("Allow overrides"), this);

	classification_->insertItem(i18n("None"));
	classification_->insertItem(i18n("Classified"));
	classification_->insertItem(i18n("Confidential"));
	classification_->insertItem(i18n("Secret"));
	classification_->insertItem(i18n("Top Secret"));
	classification_->insertItem(i18n("Unclassified"));
	classification_->insertItem(i18n("Other"));

	charset_->insertItem("UTF-8");
	charset_->insertItem("ISO-8859-1");
	charset_->insertItem("ISO-8859-2");
	charset_->insertItem("ISO-8859-3");
	charset_->insertItem("ISO-8859-4");
	charset_->insertItem("ISO-8859-5");
	charset_->insertItem("ISO-8859-6");
	charset_->insertItem("ISO-8859-7");
	charset_->insertItem("ISO-8859-8");
	charset_->insertItem("ISO-8859-9");
	charset_->insertItem("ISO-8859-10");
	charset_->insertItem("ISO-8859-13");
	charset_->insertItem("ISO-8859-14");
	charset_->insertItem("ISO-8859-15");

	printcapformat_->insertItem("BSD");
	printcapformat_->insertItem("SOLARIS");

	QLabel *l1 = new QLabel(i18n("Server name:"), this);
	QLabel *l2 = new QLabel(i18n("Server administrator:"), this);
	QLabel *l3 = new QLabel(i18n("Classification:"), this);
	QLabel *l4 = new QLabel(i18n("Default character set:"), this);
	QLabel *l5 = new QLabel(i18n("Default language:"), this);
	QLabel *l6 = new QLabel(i18n("Printcap file:"), this);
	QLabel *l7 = new QLabel(i18n("Printcap format:"), this);

	connect(classification_, SIGNAL(activated(int)), SLOT(classChanged(int)));
	classification_->setCurrentItem(0);
	charset_->setCurrentItem(0);
	printcapformat_->setCurrentItem(0);
	classChanged(0);

	QGridLayout	*m1 = new QGridLayout(this, 9, 2, 10, 7);
	m1->setRowStretch(8, 1);
	m1->setColStretch(1, 1);
	m1->addWidget(l1, 0, 0, Qt::AlignRight);
	m1->addWidget(l2, 1, 0, Qt::AlignRight);
	m1->addWidget(l3, 2, 0, Qt::AlignRight);
	m1->addWidget(l4, 4, 0, Qt::AlignRight);
	m1->addWidget(l5, 5, 0, Qt::AlignRight);
	m1->addWidget(l6, 6, 0, Qt::AlignRight);
	m1->addWidget(l7, 7, 0, Qt::AlignRight);
	m1->addWidget(servername_, 0, 1);
	m1->addWidget(serveradmin_, 1, 1);
	m1->addWidget(charset_, 4, 1);
	m1->addWidget(language_, 5, 1);
	m1->addWidget(printcap_, 6, 1);
	m1->addWidget(printcapformat_, 7, 1);
	QHBoxLayout	*m2 = new QHBoxLayout(0, 0, 5);
	m1->addLayout(m2, 2, 1);
	m2->addWidget(classification_);
	m2->addWidget(otherclassname_);
	QWidget	*w = new QWidget(this);
	w->setFixedWidth(20);
	QHBoxLayout	*m3 = new QHBoxLayout(0, 0, 0);
	m1->addLayout(m3, 3, 1);
	m3->addWidget(w);
	m3->addWidget(classoverride_);
}

bool CupsdServerPage::loadConfig(CupsdConf *conf, QString&)
{
	conf_ = conf;
	servername_->setText(conf_->servername_);
	serveradmin_->setText(conf_->serveradmin_);
	classification_->setCurrentItem(conf_->classification_);
	classChanged(conf_->classification_);
	if (conf->classification_ != CLASS_NONE)
		classoverride_->setChecked(conf_->classoverride_);
	if (conf->classification_ == CLASS_OTHER)
		otherclassname_->setText(conf_->otherclassname_);
	int index = findComboItem(charset_, conf_->charset_.upper());
	if (index != -1)
		charset_->setCurrentItem(index);
	language_->setText(conf_->language_);
	printcap_->setText(conf_->printcap_);
	printcapformat_->setCurrentItem(conf_->printcapformat_);

	return true;
}

bool CupsdServerPage::saveConfig(CupsdConf *conf, QString&)
{
	conf->servername_ = servername_->text();
	conf->serveradmin_ = serveradmin_->text();
	conf->classification_ = classification_->currentItem();
	if (conf->classification_ != CLASS_NONE)
		conf->classoverride_ = classoverride_->isChecked();
	if (conf->classification_ == CLASS_OTHER)
		conf->otherclassname_ = otherclassname_->text();
	conf->charset_ = charset_->currentText();
	conf->language_ = language_->text();
	conf->printcap_ = printcap_->text();
	conf->printcapformat_ = printcapformat_->currentItem();

	return true;
}

void CupsdServerPage::setInfos(CupsdConf *conf)
{
	QWhatsThis::add(servername_, conf->comments_.toolTip("servername"));
	QWhatsThis::add(serveradmin_, conf->comments_.toolTip("serveradmin"));
	QWhatsThis::add(classification_, conf->comments_.toolTip("classification"));
	QWhatsThis::add(classoverride_, conf->comments_.toolTip("classifyoverride"));
	QWhatsThis::add(charset_, conf->comments_.toolTip("defaultcharset"));
	QWhatsThis::add(language_, conf->comments_.toolTip("defaultlanguage"));
	QWhatsThis::add(printcap_, conf->comments_.toolTip("printcap"));
	QWhatsThis::add(printcapformat_, conf->comments_.toolTip("printcapformat"));
}

void CupsdServerPage::classChanged(int index)
{
	classoverride_->setEnabled(index != 0);
	otherclassname_->setEnabled(index == CLASS_OTHER);
}

#include "cupsdserverpage.moc"
