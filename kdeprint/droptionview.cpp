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

#include "droptionview.h"
#include "driver.h"
#include "driveritem.h"

#include <math.h>
#include <qlineedit.h>
#include <qslider.h>
#include <qlabel.h>
#include <klistbox.h>
#include <Q3VButtonGroup>
#include <qradiobutton.h>
#include <QStackedWidget>
#include <qlayout.h>
#include <qapplication.h>

#include <kcursor.h>
#include <kdialog.h>
#include <klocale.h>

OptionBaseView::OptionBaseView(QWidget *parent)
    : QWidget(parent)
{
	blockSS = false;
}

void OptionBaseView::setOption(DrBase*)
{
}

void OptionBaseView::setValue(const QString&)
{
}

//******************************************************************************************************

OptionNumericView::OptionNumericView(QWidget *parent)
    : OptionBaseView(parent)
{
	m_edit = new QLineEdit(this);
	m_slider = new QSlider(Qt::Horizontal,this);
	m_slider->setTickmarks(QSlider::TicksBelow);
	QLabel	*lab = new QLabel(i18n("Value:"),this);
	m_minval = new QLabel(this);
	m_maxval = new QLabel(this);

	m_integer = true;

	QVBoxLayout	*main_ = new QVBoxLayout(this, 0, 10);
	QHBoxLayout	*sub_ = new QHBoxLayout(0, 0, 10);
	QHBoxLayout	*sub2_ = new QHBoxLayout(0, 0, 5);
	main_->addStretch(1);
	main_->addLayout(sub_,0);
	main_->addLayout(sub2_,0);
	main_->addStretch(1);
	sub_->addWidget(lab,0);
	sub_->addWidget(m_edit,0);
	sub_->addStretch(1);
	sub2_->addWidget(m_minval,0);
	sub2_->addWidget(m_slider,1);
	sub2_->addWidget(m_maxval,0);

	connect(m_slider,SIGNAL(valueChanged(int)),SLOT(slotSliderChanged(int)));
	connect(m_edit,SIGNAL(textChanged(const QString&)),SLOT(slotEditChanged(const QString&)));
}

void OptionNumericView::setOption(DrBase *opt)
{
	if (opt->type() != DrBase::Integer && opt->type() != DrBase::Float)
		return;

	blockSS = true;
	if (opt->type() == DrBase::Integer)
	{
		m_integer = true;
		int	min_ = opt->get("minval").toInt();
		int	max_ = opt->get("maxval").toInt();
		m_slider->setRange(min_,max_);
		m_slider->setSteps(1,QMAX((max_-min_)/20,1));
		m_minval->setText(QString::number(min_));
		m_maxval->setText(QString::number(max_));
	}
	else
	{
		m_integer = false;
		int	min_ = (int)rint(opt->get("minval").toFloat()*1000);
		int	max_ = (int)rint(opt->get("maxval").toFloat()*1000);
		m_slider->setRange(min_,max_);
		m_slider->setSteps(1,QMAX((max_-min_)/20,1));
		m_minval->setText(opt->get("minval"));
		m_maxval->setText(opt->get("maxval"));
	}
	m_slider->update();
	blockSS = false;

	setValue(opt->valueText());
}

void OptionNumericView::setValue(const QString& val)
{
	m_edit->setText(val);
}

void OptionNumericView::slotSliderChanged(int value)
{
	if (blockSS) return;

	QString	txt;
	if (m_integer)
		txt = QString::number(value);
	else
		txt = QString::number(float(value)/1000.0,'f',3);
	blockSS = true;
	m_edit->setText(txt);
	blockSS = false;
	emit valueChanged(txt);
}

void OptionNumericView::slotEditChanged(const QString& txt)
{
	if (blockSS) return;

	bool	ok(false);
	int	val(0);
	if (m_integer)
		val = txt.toInt(&ok);
	else
		val = (int)rint(txt.toFloat(&ok)*1000);
	if (ok)
	{
		blockSS = true;
		m_slider->setValue(val);
		blockSS = false;
		emit valueChanged(txt);
	}
	else
	{
		m_edit->selectAll();
		QApplication::beep();
	}
}

//******************************************************************************************************

OptionStringView::OptionStringView(QWidget *parent)
    : OptionBaseView(parent)
{
	m_edit = new QLineEdit(this);
	QLabel	*lab = new QLabel(i18n("String value:"),this);

	QVBoxLayout	*main_ = new QVBoxLayout(this, 0, 5);
	main_->addStretch(1);
	main_->addWidget(lab,0);
	main_->addWidget(m_edit,0);
	main_->addStretch(1);

	connect(m_edit,SIGNAL(textChanged(const QString&)),SIGNAL(valueChanged(const QString&)));
}

void OptionStringView::setOption(DrBase *opt)
{
	if (opt->type() == DrBase::String)
		m_edit->setText(opt->valueText());
}

void OptionStringView::setValue(const QString& val)
{
	m_edit->setText(val);
}

//******************************************************************************************************

OptionListView::OptionListView(QWidget *parent)
    : OptionBaseView(parent)
{
	m_list = new KListBox(this);

	QVBoxLayout	*main_ = new QVBoxLayout(this, 0, 10);
	main_->addWidget(m_list);

	connect(m_list,SIGNAL(selectionChanged()),SLOT(slotSelectionChanged()));
}

void OptionListView::setOption(DrBase *opt)
{
	if (opt->type() == DrBase::List)
	{
		blockSS = true;
		m_list->clear();
		m_choices.clear();
                foreach (DrBase* choice, static_cast<DrListOption*>(opt)->choices())
		{
			m_list->insertItem(choice->get("text"));
			m_choices.append(choice->name());
		}
		blockSS = false;
		setValue(opt->valueText());
	}
}

void OptionListView::setValue(const QString& val)
{
	m_list->setCurrentItem(m_choices.findIndex(val));
}

void OptionListView::slotSelectionChanged()
{
	if (blockSS) return;

	QString	s = m_choices[m_list->currentItem()];
	emit valueChanged(s);
}

//******************************************************************************************************

OptionBooleanView::OptionBooleanView(QWidget *parent)
    : OptionBaseView(parent)
{
	m_group = new Q3VButtonGroup(this);
	//m_group->setFrameStyle(QFrame::NoFrame);

	QRadioButton	*btn = new QRadioButton(m_group);
	btn->setCursor(KCursor::handCursor());
	btn = new QRadioButton(m_group);
	btn->setCursor(KCursor::handCursor());

	QVBoxLayout	*main_ = new QVBoxLayout(this, 0, 10);
	main_->addWidget(m_group);

	connect(m_group,SIGNAL(clicked(int)),SLOT(slotSelected(int)));
}

void OptionBooleanView::setOption(DrBase *opt)
{
	if (opt->type() == DrBase::Boolean)
	{
                const QList<DrBase*>& choices = static_cast<DrBooleanOption*>(opt)->choices();
		m_choices.clear();
		m_group->find(0)->setText(choices.first()->get("text"));
		m_choices.append(choices.first()->name());
		m_group->find(1)->setText(choices.last()->get("text"));
		m_choices.append(choices.last()->name());
		setValue(opt->valueText());
	}
}

void OptionBooleanView::setValue(const QString& val)
{
	int	ID = m_choices.findIndex(val);
	m_group->setButton(ID);
}

void OptionBooleanView::slotSelected(int ID)
{
	QString	s = m_choices[ID];
	emit valueChanged(s);
}

//******************************************************************************************************

DrOptionView::DrOptionView(QWidget *parent, const char *name)
: Q3GroupBox(parent,name)
{
	m_stack = new QStackedWidget(this);

	OptionBaseView	*w = new OptionListView(m_stack);
	connect(w,SIGNAL(valueChanged(const QString&)),SLOT(slotValueChanged(const QString&)));
	m_stack->insertWidget(DrBase::List,w);

	w = new OptionStringView(m_stack);
	connect(w,SIGNAL(valueChanged(const QString&)),SLOT(slotValueChanged(const QString&)));
	m_stack->insertWidget(DrBase::String,w);

	w = new OptionNumericView(m_stack);
	connect(w,SIGNAL(valueChanged(const QString&)),SLOT(slotValueChanged(const QString&)));
	m_stack->insertWidget(DrBase::Integer,w);

	w = new OptionBooleanView(m_stack);
	connect(w,SIGNAL(valueChanged(const QString&)),SLOT(slotValueChanged(const QString&)));
	m_stack->insertWidget(DrBase::Boolean,w);

	w = new OptionBaseView(m_stack);
	connect(w,SIGNAL(valueChanged(const QString&)),SLOT(slotValueChanged(const QString&)));
	m_stack->insertWidget(0,w);	// empty widget

	m_stack->setCurrentWidget(w);
	setTitle(i18n("No Option Selected"));

	setColumnLayout(0, Qt::Vertical );
	layout()->setSpacing( KDialog::spacingHint() );
	layout()->setMargin( KDialog::marginHint() );
	QVBoxLayout	*main_ = new QVBoxLayout(layout(), KDialog::marginHint());
	main_->addWidget(m_stack);

	m_item = 0;
	m_block = false;
	m_allowfixed = true;
}

void DrOptionView::slotItemSelected(Q3ListViewItem *i)
{
	m_item = (DriverItem*)i;
	if (m_item && !m_item->drItem()->isOption())
		m_item = 0;
	int	ID(0);
	if (m_item)
		if (m_item->drItem()->type() == DrBase::Float) ID = DrBase::Integer;
		else ID = m_item->drItem()->type();

	OptionBaseView	*w = (OptionBaseView*)m_stack->widget(ID);
	if (w)
	{
		m_block = true;
		bool 	enabled(true);
		if (m_item)
		{
			w->setOption((m_item ? m_item->drItem() : 0));
			setTitle(m_item->drItem()->get("text"));
			enabled = ((m_item->drItem()->get("fixed") != "1") || m_allowfixed);
		}
		else
			setTitle(i18n("No Option Selected"));
		m_stack->setCurrentWidget(w);
		w->setEnabled(enabled);
		m_block = false;
	}
}

void DrOptionView::slotValueChanged(const QString& val)
{
	if (m_item && m_item->drItem() && !m_block)
	{
		m_item->drItem()->setValueText(val);
		m_item->updateText();
		emit changed();
	}
}

#include "droptionview.moc"
