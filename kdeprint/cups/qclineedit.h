/***************************************************************************
                          qclineedit.h  -  description
                             -------------------
    begin                : Sat Jan 20 2001
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

#ifndef QCLINEEDIT_H
#define QCLINEEDIT_H

#include <qlineedit.h>

/* reimplementation of QLineEdit to emit "returnPressed()" signal
   on focusOut event, and select all on focusIn event.
*/
class QCLineEdit : public QLineEdit
{
public:
	QCLineEdit(QWidget *parent = 0, const char *name = 0);
	~QCLineEdit();
protected:
	void focusOutEvent(QFocusEvent *e);
	void focusInEvent(QFocusEvent *e);
};

#endif
