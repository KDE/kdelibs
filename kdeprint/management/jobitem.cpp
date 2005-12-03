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

#include "jobitem.h"
#include "kmjob.h"

#include <kiconloader.h>

#include <qpixmap.h>

JobItem::JobItem(Q3ListView *parent, KMJob *job)
: Q3ListViewItem(parent)
{
	m_job = new KMJob;
	init(job);
}

JobItem::~JobItem()
{
	delete m_job;
}

void JobItem::init(KMJob *job)
{
	m_job->copy(job ? *job : KMJob());

	setPixmap(0,SmallIcon(m_job->pixmap()));
	setText(0,QString::number(m_job->id()));
	//setText(6,m_job->printer());
	setText(2,m_job->name());
	setText(1,m_job->owner());
	setText(3,m_job->stateString());
	setText(4,QString::number(m_job->size()));
	setText(5,QString::number(m_job->processedPages()));
	m_ID = m_job->id();
	m_uri = m_job->uri();

	// additional attributes
	for (int i=0; i<m_job->attributeCount(); i++)
		setText(6+i, m_job->attribute(i));

	widthChanged();
}

int JobItem::compare(Q3ListViewItem *item, int col, bool asc) const
{
	switch (col)
	{
		case 0:
		case 4:
		case 5:
			{
				int	i1(this->text(col).toInt()), i2(item->text(col).toInt());
				return (i1 < i2 ? -1 : (i1 > i2 ? 1 : 0));
				break;
			}
		default:
			return Q3ListViewItem::compare(item, col, asc);
	}
}
