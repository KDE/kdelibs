/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "qprogressindicator.h"

#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QLayout>

#include <kiconloader.h>

QProgressIndicator::QProgressIndicator(QWidget *parent)
: QFrame(parent)
{
	m_pb = new QProgressBar();
	m_pb->setMinimum(0);
	m_pb->setMaximum(100);

	m_pbdetails = new QPushButton();
	m_pbdetails->setFixedWidth(32); // FIXME: I want a squared button
	m_pbdetails->setIcon(SmallIcon("go-up"));
	m_pbdetails->setEnabled(false);

	QHBoxLayout *hbox = new QHBoxLayout(this);
	hbox->setSpacing(0);
	hbox->addWidget(m_pbdetails);
	hbox->addWidget(m_pb);

	m_details = new QWidget(this);
	m_details->setWindowFlags(Qt::Popup);
	m_details->hide();

	m_detailsvbox = new QVBoxLayout(m_details);

	connect(m_pbdetails, SIGNAL(clicked()), SLOT(slotClicked()));
}

void QProgressIndicator::slotClicked()
{
	QPoint indicatorpos = mapToGlobal(pos());
	m_details->move(indicatorpos.x(), indicatorpos.y());

	if(m_details->isVisible())
		m_details->hide();
	else
		m_details->show();
}

void QProgressIndicator::addProgress(KUrl url, int percentage)
{
	QProgressBar *pb;

	m_progress[url] = percentage;

	if(!m_progresswidgets.contains(url))
	{
		QWidget *pbcontainer = new QWidget();

		pb = new QProgressBar(pbcontainer);
		pb->setMinimum(0);
		pb->setMaximum(100);
		m_progresswidgets.insert(url, pb);

		QPushButton *pbcancel = new QPushButton();
		pbcancel->setFixedWidth(32); // FIXME: I want a squared button
		pbcancel->setIcon(SmallIcon("dialog-cancel"));

		QHBoxLayout *hbox = new QHBoxLayout(pbcontainer);
		hbox->addWidget(pbcancel);
		hbox->addWidget(pb);

		pbcontainer->show();

		m_detailsvbox->addWidget(pbcontainer);
	}
	else
	{
		pb = m_progresswidgets[url];
	}

	pb->setValue(percentage);

	if(m_progress.count() > 0)
		m_pbdetails->setEnabled(true);

	if(percentage == 100)
		removeProgress(url);

	calculateAverage();
}

void QProgressIndicator::removeProgress(KUrl url)
{
	m_progress.remove(url);

	if (m_progresswidgets[url])
		delete m_progresswidgets[url]->parentWidget();

	if(m_progress.count() == 0)
	{
		m_pbdetails->setEnabled(false);
		m_details->hide();
	}

	calculateAverage();
}

void QProgressIndicator::calculateAverage()
{
	if(m_progress.count() == 0)
	{
		m_pb->reset();
		return;
	}

	int average = 0;
	QHashIterator<KUrl, int> it(m_progress);
	while(it.hasNext())
	{
		it.next();
		average += it.value();
	}
	average = (average / m_progress.count());

	m_pb->setValue(average);
}

#include "qprogressindicator.moc"
