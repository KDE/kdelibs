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

#include "portdialog.h"
#include "cupsdconf.h"

#include <qlineedit.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>

#include <klocale.h>

PortDialog::PortDialog(QWidget *parent, const char *name)
	: KDialogBase(parent, name, true, QString::null, Ok|Cancel, Ok, true)
{
	QWidget	*dummy = new QWidget(this);
	setMainWidget(dummy);
	address_ = new QLineEdit(dummy);
	port_ = new QSpinBox(0, 9999, 1, dummy);
	port_->setValue(631);
	usessl_ = new QCheckBox(i18n("Use SSL encryption"), dummy);

	QLabel	*l1 = new QLabel(i18n("Address:"), dummy);
	QLabel	*l2 = new QLabel(i18n("Port:"), dummy);

	QVBoxLayout	*m1 = new QVBoxLayout(dummy, 0, 10);
	QGridLayout	*m2 = new QGridLayout(0, 3, 2, 0, 5);
	m1->addLayout(m2);
	m2->addWidget(l1, 0, 0, Qt::AlignRight);
	m2->addWidget(l2, 1, 0, Qt::AlignRight);
	m2->addMultiCellWidget(usessl_, 2, 2, 0, 1);
	m2->addWidget(address_, 0, 1);
	m2->addWidget(port_, 1, 1);

	setCaption(i18n("Listen To"));
	resize(250, 100);
}

QString PortDialog::listenString()
{
	QString s;
	if (usessl_->isChecked())
		s.append("SSLListen ");
	else
		s.append("Listen ");
	if (!address_->text().isEmpty())
		s.append(address_->text());
	else
		s.append("*");
	s.append(":").append(port_->text());
	return s;
}

void PortDialog::setInfos(CupsdConf *conf)
{
	QWhatsThis::add(address_, conf->comments_.toolTip("address"));
	QWhatsThis::add(port_, conf->comments_.toolTip("port"));
	QWhatsThis::add(usessl_, conf->comments_.toolTip("usessl"));
}

QString PortDialog::newListen(QWidget *parent, CupsdConf *conf)
{
	PortDialog	dlg(parent);
	dlg.setInfos(conf);
	if (dlg.exec())
	{
		return dlg.listenString();
	}
	return QString::null;
}

QString PortDialog::editListen(const QString& s, QWidget *parent, CupsdConf *conf)
{
	PortDialog	dlg(parent);
	dlg.setInfos(conf);
	int	p = s.find(' ');
	if (p != -1)
	{
		dlg.usessl_->setChecked(s.left(p).startsWith("SSL"));
		QString	addr = s.mid(p+1).stripWhiteSpace();
		int p1 = addr.find(':');
		if (p1 == -1)
		{
			dlg.address_->setText(addr);
			dlg.port_->setValue(631);
		}
		else
		{
			dlg.address_->setText(addr.left(p1));
			dlg.port_->setValue(addr.mid(p1+1).toInt());
		}
	}
	if (dlg.exec())
	{
		return dlg.listenString();
	}
	return QString::null;
}
