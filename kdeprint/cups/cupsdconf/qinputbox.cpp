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

#include "qinputbox.h"

#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qregexp.h>
#include <klocale.h>

QInputBox::QInputBox(QWidget *parent,const char *name)
	: QDialog(parent, name, true)
{
	count_ = 0;
	initialized_ = false;
	edits_.setAutoDelete(false);
	labels_.setAutoDelete(false);
	init(1);
}

QInputBox::QInputBox(int numlines, QWidget *parent,const char *name)
	: QDialog(parent, name, true)
{
	count_ = 0;
	initialized_ = false;
	edits_.setAutoDelete(false);
	labels_.setAutoDelete(false);
	init(numlines);
}

QInputBox::~QInputBox()
{
}

QSize QInputBox::sizeHint() const
{
	QSize	s1(count_ > 0 ? labels_.getFirst()->sizeHint() : QSize(0,0)), s2(count_ > 0 ? edits_.getFirst()->sizeHint() : QSize(0,0)), s3(okbtn_->sizeHint()),s4(cancelbtn_->sizeHint());
	return QSize(QMAX(s1.width(),s3.width()+s4.width())+120, count_*(s1.height()+s2.height()+13)+s3.height()+20);
}

QString QInputBox::text(int index)
{
	if (index >= 0 && index < count_)
	{
		return edits_.at(index)->text();
	}
	else return QString("");
}

void QInputBox::setText(const QString& txt, int index)
{
	if (index >= 0 && index < count_)
	{
		edits_.at(index)->setText(txt);
		edits_.at(index)->selectAll();
	}
}

void QInputBox::setMessage(const QString& msg, int index)
{
	if (index >= 0 && index < count_)
	{
		labels_.at(index)->setText(msg);
		resize(sizeHint());
	}
}

QString QInputBox::inputBox(QWidget *parent, const QString& caption, const QString& msg, const QString& txt, bool *ok)
{
	QInputBox	dlg(parent);
	dlg.setMessage(msg);
	dlg.setCaption(caption);
	dlg.setText(txt);
	QString	result("");
	if (ok) *ok = false;
	if (dlg.exec())
	{
		result = dlg.text();
		if (ok) *ok = true;
	}
	return result;
}

QStringList QInputBox::inputBox(QWidget *parent, const QString& caption, const QStringList& msgs, int numlines, bool *ok)
{
	if (numlines <= 0) return QStringList();

	QInputBox	dlg(numlines, parent);
	QStringList::ConstIterator	it;
	int	i = 0;
	for (it=msgs.begin(); it!=msgs.end() && i<numlines; i++, ++it)
		dlg.setMessage(*it, i);
	dlg.setCaption(caption);
	QStringList	res;
	if (ok) *ok = false;
	if (dlg.exec())
	{
		for (i=0;i<numlines;i++)
			res.append(dlg.text(i));
		if (ok) *ok = true;
	}
	return res;
}

void QInputBox::init(int numlines)
{
	if (initialized_ || numlines <= 0) return;
	initialized_ = true;

	QVBoxLayout	*main_ = new QVBoxLayout(this, 10, 0);
	count_ = numlines;
	for (int i=0;i<numlines;i++)
	{
		QLineEdit	*edit_ = new QLineEdit(this);
		edits_.append(edit_);
		QLabel		*label_ = new QLabel(i18n("Input value:"), this);
		labels_.append(label_);

		main_->addWidget(label_);
		main_->addSpacing(3);
		main_->addWidget(edit_);

		main_->addSpacing(10);
	}

	okbtn_ = new QPushButton(i18n("OK"), this);
	connect(okbtn_, SIGNAL(clicked()), SLOT(accept()));
	okbtn_->setDefault(true);

	cancelbtn_ = new QPushButton(i18n("Cancel"), this);
	connect(cancelbtn_, SIGNAL(clicked()), SLOT(reject()));

	QHBoxLayout	*btnlayout_ = new QHBoxLayout(0, 0, 10);

	main_->addLayout(btnlayout_);
	btnlayout_->addStretch(1);
	btnlayout_->addWidget(okbtn_);
	btnlayout_->addWidget(cancelbtn_);

	edits_.first()->setFocus();

	resize(sizeHint());
}
