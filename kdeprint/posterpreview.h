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

#ifndef POSTERPREVIEW_H
#define POSTERPREVIEW_H

#include <qframe.h>
#include <qvaluelist.h>

class KProcess;

class PosterPreview : public QFrame
{
	Q_OBJECT
public:
	PosterPreview( QWidget *parent = 0, const char *name = 0 );
	PosterPreview( const QString& postersize, const QString& mediasize, QWidget *parent = 0, const char *name = 0 );
	~PosterPreview();

public slots:
	void setPosterSize( int );
	void setPosterSize( const QString& );
	void setMediaSize( int );
	void setMediaSize( const QString& );
	void setCutMargin( int );
	void updatePoster();
	void setSelectedPages( const QString& );

signals:
	void selectionChanged( const QString& );

protected:
	void parseBuffer();
	void drawContents( QPainter* );
	void init();
	void setDirty();
	void mouseMoveEvent( QMouseEvent* );
	void mousePressEvent( QMouseEvent* );
	void emitSelectedPages();

protected slots:
	void slotProcessStderr( KProcess*, char*, int );
	void slotProcessExited( KProcess* );

private:
	int m_rows, m_cols;
	int m_pw, m_ph; // page size
	int m_mw, m_mh; // cur margins
	QRect m_posterbb; // poster bounding box (without any margin)
	KProcess *m_process;
	QString m_buffer;
	QString m_postersize, m_mediasize;
	int m_cutmargin;
	bool m_dirty;
	QRect m_boundingrect;
	QValueList<int> m_selectedpages;
};

#endif /* POSTERPREVIEW_H */
