/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id$
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

#ifndef KPRINTPREVIEW_H
#define KPRINTPREVIEW_H

#include <kparts/part.h>
#include <kparts/mainwindow.h>
#include <qstring.h>
#include <kprocess.h>

class KPrintPreview : public KParts::MainWindow
{
	Q_OBJECT
public:
	KPrintPreview(QWidget *parent = 0, bool previewOnly = false);
	~KPrintPreview();

	void openFile(const QString& file);
	bool status() const;
	void exec(const QString& file = QString::null);
	bool isValid() const;

	static bool preview(const QString& file, bool previewOnly = false);

protected slots:
	void accept();
	void reject();

protected:
	void done(bool st);
	void closeEvent(QCloseEvent *e);

private:
	KParts::ReadOnlyPart	*gvpart_;
	bool			status_;
};

class KPreviewProc : public KProcess
{
	Q_OBJECT
public:
	KPreviewProc();
	virtual ~KPreviewProc();

	bool startPreview();

protected slots:
	void slotProcessExited(KProcess*);
};

inline bool KPrintPreview::status() const
{ return status_; }

#endif
