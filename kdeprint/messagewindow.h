/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2002 Michael Goffioul <kdeprint@swing.be>
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

#ifndef MESSAGEWINDOW_H
#define MESSAGEWINDOW_H

#include <qwidget.h>
#include <qptrdict.h>

#include <kdelibs_export.h>

class QLabel;

class KDEPRINT_EXPORT MessageWindow : public QWidget
{
	Q_OBJECT

public:
	~MessageWindow();

	static void add( QWidget *parent, const QString& txt, int delay = 500 );
	static void change( QWidget *parent, const QString& txt );
	static void remove( QWidget *parent );
	static void removeAll();

protected slots:
	void slotTimer();

protected:
	MessageWindow( const QString& txt, int delay = 500, QWidget *parent = 0, const char *name = 0 );
	void setText( const QString& txt );
	QString text() const;

private:
	QLabel *m_text;
	static QPtrDict<MessageWindow> m_windows;
};

#endif
