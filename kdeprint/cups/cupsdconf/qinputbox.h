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

#ifndef	QINPUTBOX_H
#define	QINPUTBOX_H

#include <qdialog.h>
#include <qstring.h>
#include <qptrlist.h>

class QLineEdit;
class QLabel;
class QPushButton;

class QInputBox : public QDialog
{
public:
	QInputBox(QWidget *parent = 0, const char *name = 0);
	QInputBox(int numlines, QWidget *parent = 0, const char *name = 0);
	~QInputBox();

	void setMessage(const QString& msg, int index = 0);
	QString text(int index = 0);
	void setText(const QString& txt, int index = 0);

	static QStringList inputBox(QWidget *parent, const QString& caption, const QStringList& msgs, int numlines = 1, bool *ok = 0);
	static QString inputBox(QWidget *parent, const QString& caption, const QString& msg, const QString& txt = QString::null, bool *ok = 0);

	QSize sizeHint() const;

protected:
	void init(int numlines);

private:
	QPtrList<QLineEdit>	edits_;
	QPtrList<QLabel>		labels_;
	int			count_;
	bool			initialized_;
	QPushButton	*okbtn_, *cancelbtn_;
};

#endif
