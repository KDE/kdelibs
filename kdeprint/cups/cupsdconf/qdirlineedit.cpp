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

#include "qdirlineedit.h"

#include <qlineedit.h>
#include <qlayout.h>
#include <kpushbutton.h>
#include <kfiledialog.h>
#include <kiconloader.h>

QDirLineEdit::QDirLineEdit(QWidget *parent, const char *name)
	: QWidget(parent, name)
{
	edit_ = new QLineEdit(this);
	//button_ = new QPushButton("Browse...", this);
	button_ = new KPushButton(this);
	button_->setPixmap(SmallIcon("fileopen"));
	connect(button_,SIGNAL(clicked()),SLOT(buttonClicked()));

	QHBoxLayout	*main_ = new QHBoxLayout(this, 0, 10);
	main_->addWidget(edit_);
	main_->addWidget(button_);

	fileedit_ = false;
}

QDirLineEdit::~QDirLineEdit()
{
}

void QDirLineEdit::setText(const QString& txt)
{
	edit_->setText(txt);
}

void QDirLineEdit::setButtonText(const QString& txt)
{
	button_->setText(txt);
}

QString QDirLineEdit::text()
{
	return edit_->text();
}

QSize QDirLineEdit::sizeHint() const
{
	QSize	s1(edit_->sizeHint()), s2(button_->sizeHint());
	return QSize(s1.width()+s2.width()+10,s1.height());
}

void QDirLineEdit::buttonClicked()
{
	QString	dirname;
	if (!fileedit_) dirname = KFileDialog::getExistingDirectory(edit_->text(), this);
	else dirname = KFileDialog::getOpenFileName(edit_->text(), QString::null, this);
	if (!dirname.isEmpty()) edit_->setText(dirname);
}

void QDirLineEdit::setFileEdit(bool on)
{
	fileedit_ = on;
}
#include "qdirlineedit.moc"
