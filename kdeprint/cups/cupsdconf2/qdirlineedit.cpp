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

#include "qdirlineedit.h"

#include <qlineedit.h>
#include <qlayout.h>
#include <kpushbutton.h>
#include <kfiledialog.h>
#include <kiconloader.h>

QDirLineEdit::QDirLineEdit(bool file, QWidget *parent)
	: QWidget(parent)
{
	edit_ = new QLineEdit(this);
	button_ = new KPushButton(this);
	button_->setIcon(SmallIcon("fileopen"));
	connect(button_,SIGNAL(clicked()),SLOT(buttonClicked()));

	QHBoxLayout	*main_ = new QHBoxLayout(this);
  main_->setMargin(0);
  main_->setSpacing(3);
	main_->addWidget(edit_);
	main_->addWidget(button_);

	fileedit_ = file;
}

QDirLineEdit::~QDirLineEdit()
{
}

void QDirLineEdit::setURL(const QString& txt)
{
	edit_->setText(txt);
}

QString QDirLineEdit::url()
{
	return edit_->text();
}

void QDirLineEdit::buttonClicked()
{
	QString	dirname;
	if (!fileedit_)
		dirname = KFileDialog::getExistingDirectory(edit_->text(), this);
	else
		dirname = KFileDialog::getOpenFileName(edit_->text(), QString(), this);
	if (!dirname.isEmpty())
		edit_->setText(dirname);
}

void QDirLineEdit::setFileEdit(bool on)
{
	fileedit_ = on;
}

#include "qdirlineedit.moc"
