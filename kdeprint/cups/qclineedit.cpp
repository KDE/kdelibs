/***************************************************************************
                          qclineedit.cpp  -  description
                             -------------------
    begin                : Mon Jan 22 2001
    copyright            : (C) 2001 by Michael Goffioul
    email                : goffioul@imec.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qclineedit.h"

QCLineEdit::QCLineEdit(QWidget *parent, const char *name)
: QLineEdit(parent,name)
{
}

QCLineEdit::~QCLineEdit()
{
}

void QCLineEdit::focusOutEvent(QFocusEvent *e)
{
	QLineEdit::focusOutEvent(e);
	emit returnPressed();
}

void QCLineEdit::focusInEvent(QFocusEvent *e)
{
	QLineEdit::focusInEvent(e);
	selectAll();
}
