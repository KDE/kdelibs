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

#include "cupsdoption.h"

#include <qcheckbox.h>
#include <qlayout.h>
#include <qframe.h>
#include <klocale.h>
#include <qtooltip.h>

CupsdOption::CupsdOption(QWidget *parent, const char *name)
	: QWidget(parent, name)
{
	cb_ = new QCheckBox(this);
	cb_->setChecked(true);
	connect(cb_,SIGNAL(clicked()),SLOT(checkClicked()));
	QToolTip::add(cb_, i18n("Toggle default value"));

	int	w = (fontMetrics().width(i18n("Default")) - cb_->sizeHint().width()) / 2;

	fr_ = new QFrame(this);
	fr_->setFrameStyle(QFrame::VLine|QFrame::Sunken);
	fr_->setLineWidth(1);

	layout_ = new QHBoxLayout(this, 0, 10);
	layout_->addSpacing(5);
	layout_->addWidget(fr_, 0);
	layout_->addSpacing(w);
	layout_->addWidget(cb_, 0);
	layout_->addSpacing(w);

	widget_ = 0;
}

CupsdOption::~CupsdOption()
{
}

QSize CupsdOption::sizeHint() const
{
	QSize	s1(cb_->sizeHint()), s2(widget_ ? widget_->sizeHint() : QSize(0,0));
	int	w = fontMetrics().width(i18n("Default"));
	return QSize(s2.width()+w+fr_->width()+10, QMAX(s1.height(), s2.height()));
}

void CupsdOption::setDefault(bool on)
{
	cb_->setChecked(on);
	checkClicked();
}

bool CupsdOption::isDefault() const
{
	return (cb_->isChecked());
}

void CupsdOption::checkClicked()
{
	if (widget_) widget_->setEnabled(!(cb_->isChecked()));
}

void CupsdOption::childEvent(QChildEvent *ev)
{
	QObject::childEvent(ev);
	if (ev->inserted())
	{
		if (ev->child() != layout_ && ev->child() != cb_ && ev->child() != fr_ && ev->child()->isWidgetType())
		{
			widget_ = (QWidget*)(ev->child());
			layout_->insertWidget(0, widget_, 1);
			checkClicked();
		}
	}
}
#include "cupsdoption.moc"
