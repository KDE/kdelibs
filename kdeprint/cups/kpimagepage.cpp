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

#include "kpimagepage.h"
#include "imageposition.h"
#include "imagepreview.h"
#include "driver.h"

#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <klocale.h>
#include <knuminput.h>
#include <kseparator.h>
#include <kstandarddirs.h>

KPImagePage::KPImagePage(DrMain *driver, QWidget *parent, const char *name)
: KPrintDialogPage(parent, name)
{
	setTitle(i18n("Image"));

	QGroupBox	*colorbox = new QGroupBox(0, Qt::Vertical, i18n("Color Settings"), this);
	QGroupBox	*sizebox = new QGroupBox(0, Qt::Vertical, i18n("Image Size"), this);
	QGroupBox	*positionbox = new QGroupBox(0, Qt::Vertical, i18n("Image Position"), this);

	m_brightness = new KIntNumInput(100, colorbox);
	m_brightness->setLabel(i18n("&Brightness"));
	m_brightness->setRange(0, 200, 20, true);
	m_hue = new KIntNumInput(m_brightness, 0, colorbox);
	m_hue->setLabel(i18n("&Hue (Color Rotation)"));
	m_hue->setRange(-360, 360, 36, true);
	m_saturation = new KIntNumInput(m_brightness, 100, colorbox);
	m_saturation->setLabel(i18n("&Saturation"));
	m_saturation->setRange(0, 200, 20, true);
	m_gamma = new KIntNumInput(m_saturation, 1000, colorbox);
	m_gamma->setLabel(i18n("&Gamma (Color Correction)"));
	m_gamma->setRange(1, 3000, 100, true);
	connect(m_brightness, SIGNAL(valueChanged(int)), SLOT(slotImageSettingsChanged()));
	connect(m_hue, SIGNAL(valueChanged(int)), SLOT(slotImageSettingsChanged()));
	connect(m_saturation, SIGNAL(valueChanged(int)), SLOT(slotImageSettingsChanged()));
	//connect(m_gamma, SIGNAL(valueChanged(int)), SLOT(slotImageSettingsChanged()));

	m_preview = new ImagePreview(colorbox);
	bool	useColor = (driver ? driver->get("colordevice") == "1" : true);
	m_preview->setBlackAndWhite(!useColor);
	m_hue->setEnabled(useColor);
	m_saturation->setEnabled(useColor);
	QImage	img(locate("data", "kdeprint/preview.png"));
	m_preview->setImage(img);
	KSeparator	*sep = new KSeparator(Qt::Horizontal, colorbox);

	QPushButton	*defbtn = new QPushButton(i18n("&Default Settings"), colorbox);
	connect(defbtn, SIGNAL(clicked()), SLOT(slotDefaultClicked()));
	slotDefaultClicked();

	m_sizetype = new QComboBox(sizebox);
	m_sizetype->insertItem(i18n("Natural Image Size"));
	m_sizetype->insertItem(i18n("Resolution (ppi)"));
	// xgettext:no-c-format
	m_sizetype->insertItem(i18n("% of Page"));
	// xgettext:no-c-format
	m_sizetype->insertItem(i18n("% of Natural Image Size"));
	m_size = new KIntNumInput(sizebox);
	m_size->setRange(1, 1200, 20, true);
	m_size->setValue(72);
	connect(m_sizetype, SIGNAL(activated(int)), SLOT(slotSizeTypeChanged(int)));
	m_sizetype->setCurrentItem(0);
	slotSizeTypeChanged(0);
	QLabel	*lab = new QLabel(i18n("&Image Size Type"), sizebox);
	lab->setBuddy(m_sizetype);

	m_position = new ImagePosition(positionbox);
	QRadioButton	*bottom = new QRadioButton(positionbox);
	QRadioButton	*top = new QRadioButton(positionbox);
	QRadioButton	*vcenter = new QRadioButton(positionbox);
	QRadioButton	*left = new QRadioButton(positionbox);
	QRadioButton	*right = new QRadioButton(positionbox);
	QRadioButton	*hcenter = new QRadioButton(positionbox);
	QSize	sz = bottom->sizeHint();
	bottom->setFixedSize(sz);
	vcenter->setFixedSize(sz);
	top->setFixedSize(sz);
	left->setFixedSize(sz);
	hcenter->setFixedSize(sz);
	right->setFixedSize(sz);
	m_vertgrp = new QButtonGroup(positionbox);
	m_vertgrp->hide();
	m_horizgrp = new QButtonGroup(positionbox);
	m_horizgrp->hide();
	m_vertgrp->insert(top, 0);
	m_vertgrp->insert(vcenter, 1);
	m_vertgrp->insert(bottom, 2);
	m_horizgrp->insert(left, 0);
	m_horizgrp->insert(hcenter, 1);
	m_horizgrp->insert(right, 2);
	connect(m_vertgrp, SIGNAL(clicked(int)), SLOT(slotPositionChanged()));
	connect(m_horizgrp, SIGNAL(clicked(int)), SLOT(slotPositionChanged()));
	m_vertgrp->setButton(1);
	m_horizgrp->setButton(1);
	slotPositionChanged();

	QGridLayout	*l0 = new QGridLayout(this, 2, 2, 0, 10);
	l0->addMultiCellWidget(colorbox, 0, 0, 0, 1);
	l0->addWidget(sizebox, 1, 0);
	l0->addWidget(positionbox, 1, 1);
	l0->setColStretch(0, 1);
	QGridLayout	*l1 = new QGridLayout(colorbox->layout(), 5, 2, 10);
	l1->addWidget(m_brightness, 0, 0);
	l1->addWidget(m_hue, 1, 0);
	l1->addWidget(m_saturation, 2, 0);
	l1->addWidget(sep, 3, 0);
	l1->addWidget(m_gamma, 4, 0);
	l1->addMultiCellWidget(m_preview, 0, 3, 1, 1);
	l1->addWidget(defbtn, 4, 1);
	QVBoxLayout	*l2 = new QVBoxLayout(sizebox->layout(), 3);
	l2->addStretch(1);
	l2->addWidget(lab);
	l2->addWidget(m_sizetype);
	l2->addSpacing(10);
	l2->addWidget(m_size);
	l2->addStretch(1);
	QGridLayout	*l3 = new QGridLayout(positionbox->layout(), 2, 2, 10);
	QHBoxLayout	*l4 = new QHBoxLayout(0, 0, 10);
	QVBoxLayout	*l5 = new QVBoxLayout(0, 0, 10);
	l3->addLayout(l4, 0, 1);
	l3->addLayout(l5, 1, 0);
	l3->addWidget(m_position, 1, 1);
	l4->addWidget(left, Qt::AlignLeft);
	l4->addWidget(hcenter, Qt::AlignCenter);
	l4->addWidget(right, Qt::AlignRight);
	l5->addWidget(top, Qt::AlignTop);
	l5->addWidget(vcenter, Qt::AlignVCenter);
	l5->addWidget(bottom, Qt::AlignBottom);
}

KPImagePage::~KPImagePage()
{
}

void KPImagePage::setOptions(const QMap<QString,QString>& opts)
{
	QString	value;
	if (!(value=opts["brightness"]).isEmpty())
		m_brightness->setValue(value.toInt());
	if (!(value=opts["hue"]).isEmpty())
		m_hue->setValue(value.toInt());
	if (!(value=opts["saturation"]).isEmpty())
		m_saturation->setValue(value.toInt());
	if (!(value=opts["gamma"]).isEmpty())
		m_gamma->setValue(value.toInt());

	int	type = 0;
	int	ival(0);
	if ((ival = opts["ppi"].toInt()) != 0)
		type = 1;
	else if ((ival = opts["scaling"].toInt()) != 0)
		type = 2;
	else if (!opts["natural-scaling"].isEmpty() && (ival = opts["natural-scaling"].toInt()) != 1)
		type = 3;
	m_sizetype->setCurrentItem(type);
	slotSizeTypeChanged(type);
	if (type != 0)
		m_size->setValue(ival);

	if (!(value=opts["position"]).isEmpty())
	{
		m_position->setPosition(value.latin1());
		int	pos = m_position->position();
		m_vertgrp->setButton(pos/3);
		m_horizgrp->setButton(pos%3);
	}
}

void KPImagePage::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	if (incldef || m_brightness->value() != 100)
		opts["brightness"] = QString::number(m_brightness->value());
	if (m_hue->isEnabled())
	{
		if (incldef || m_hue->value() != 0)
			opts["hue"] = QString::number(m_hue->value());
		if (incldef || m_saturation->value() != 100)
			opts["saturation"] = QString::number(m_saturation->value());
	}
	if (incldef || m_gamma->value() != 1000)
		opts["gamma"] = QString::number(m_gamma->value());

	QString	name;
	if (incldef)
	{
		opts["ppi"] = "0";
		opts["scaling"] = "0";
		opts["natural-scaling"] = "1";
	}
	switch (m_sizetype->currentItem())
	{
		case 0: break;
		case 1: name = "ppi"; break;
		case 2: name = "scaling"; break;
		case 3: name = "natural-scaling"; break;
	}
	if (!name.isEmpty())
		opts[name] = QString::number(m_size->value());

	if (incldef || m_position->position() != ImagePosition::Center)
		opts["position"] = m_position->positionString();
}

void KPImagePage::slotSizeTypeChanged(int t)
{
	m_size->setEnabled(t > 0);
	if (t > 0)
	{
		int	minval, maxval, defval;
		if (t == 1)
		{
			minval = 1;
			maxval = 1200;
			defval = 72;
		}
		else
		{
			minval = 1;
			maxval = 800;
			defval = 100;
		}
		m_size->setRange(minval, maxval);
		m_size->setValue(defval);
	}
}

void KPImagePage::slotPositionChanged()
{
	int	h = m_horizgrp->id(m_horizgrp->selected()), v = m_vertgrp->id(m_vertgrp->selected());
	m_position->setPosition(h, v);
}

void KPImagePage::slotImageSettingsChanged()
{
	int	b = m_brightness->value(), h = m_hue->value(), s = m_saturation->value();
	//int g = m_gamma->value();
	int g = 1000;
	m_preview->setParameters(b, h, s, g);
}

void KPImagePage::slotDefaultClicked()
{
	m_brightness->setValue(100);
	m_hue->setValue(0);
	m_saturation->setValue(100);
	m_gamma->setValue(1000);
}

#include "kpimagepage.moc"
