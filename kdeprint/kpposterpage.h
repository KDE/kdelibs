/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2002 Michael Goffioul <goffioul@imec.be>
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

#ifndef KPPOSTERPAGE_H
#define KPPOSTERPAGE_H

#include "kprintdialogpage.h"

class QComboBox;
class QCheckBox;
class QLabel;
class PosterPreview;
class KIntNumInput;
class QPushButton;
class QLineEdit;

class KPPosterPage : public KPrintDialogPage
{
	Q_OBJECT
public:
	KPPosterPage( QWidget *parent = 0, const char *name = 0 );
	~KPPosterPage();

	void setOptions( const QMap<QString,QString>& opts );
	void getOptions( QMap<QString,QString>& opts, bool incldef = false );
	bool isValid();

protected slots:
	void slotPosterSizeChanged( int );
	void slotMarginChanged( int );
	void slotLockToggled( bool );

private:
	QComboBox *m_postersize;
	QComboBox *m_printsize;
	PosterPreview *m_preview;
	QCheckBox *m_postercheck;
	QLabel *m_mediasize;
	KIntNumInput *m_cutmargin;
	QPushButton *m_lockbtn;
	QLineEdit *m_selection;
};

#endif /* KPPOSTERPAGE_H */
