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

#include <config.h>

#include "kpmarginpage.h"
#include "kprinter.h"
#include "driver.h"
#include "marginwidget.h"

#include <qgroupbox.h>
#include <qlayout.h>
#include <qprinter.h>
#include <qpaintdevicemetrics.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>

KPMarginPage::KPMarginPage(KPrinter *prt, DrMain *driver, QWidget *parent, const char *name)
: KPrintDialogPage(0, driver, parent, name)
{
	m_printer = prt;
	setTitle(i18n("Margins"));
	m_usedriver = true;

	QGroupBox	*box = new QGroupBox(1, Qt::Vertical, i18n("Margins"), this);
	m_margin = new MarginWidget(box, "MarginWidget", (m_printer != 0));
	//m_margin->setSymetricMargins(true);
	if (m_printer)
		m_margin->setResolution(m_printer->resolution());

	QVBoxLayout	*l0 = new QVBoxLayout(this, 0, 10);
	l0->addWidget(box);
	l0->addStretch(1);
}

KPMarginPage::~KPMarginPage()
{
}

void KPMarginPage::initPageSize(const QString& ps, bool landscape)
{
	QSize	sz(-1, -1);
	unsigned int mt( 36 ), mb( mt ), ml( 24 ), mr( ml );
	QString	m_currentps(ps);
	if (driver() && m_usedriver )
	{
		if (m_currentps.isEmpty())
		{
			DrListOption	*o = (DrListOption*)driver()->findOption("PageSize");
			if (o)
				m_currentps = o->get("default");
		}
		if (!m_currentps.isEmpty())
		{
			DrPageSize	*ps = driver()->findPageSize(m_currentps);
			if (ps)
			{
				sz = ps->pageSize();
				QRect r = ps->pageRect();
				mt = r.top();
				ml = r.left();
				mb = sz.height() - r.bottom();
				mr = sz.width() - r.right();
			}
		}
	}
	else
	{
		// no driver, use the Qt values for page size and margins
		QPrinter	prt(QPrinter::PrinterResolution);
		prt.setFullPage(true);
		prt.setPageSize((QPrinter::PageSize)(m_currentps.isEmpty() ? KGlobal::locale()->pageSize() : ps.toInt()));
		QPaintDeviceMetrics	metrics(&prt);
		sz = QSize(metrics.width(), metrics.height());
#ifdef KDEPRINT_USE_MARGINS
		prt.margins( &mt, &ml, &mb, &mr );
#else
		QSize mg = prt.margins();
		mt = mb = mg.height();
		ml = mr = mg.width();
#endif
	}
	m_margin->setPageSize(sz.width(), sz.height());
	m_margin->setOrientation(landscape ? KPrinter::Landscape : KPrinter::Portrait);
	m_margin->setDefaultMargins( mt, mb, ml, mr );
	m_margin->setCustomEnabled(false);
}

void KPMarginPage::setOptions(const QMap<QString,QString>& opts)
{
	QString	orient = opts["orientation-requested"];
	bool 	land = (orient.isEmpty()? opts["kde-orientation"] == "Landscape" : orient == "4" || orient == "5");
	QString ps = opts[ "kde-printsize" ];
	if ( ps.isEmpty() )
	{
		m_usedriver = true;
		ps = opts[ "PageSize" ];
		if (ps.isEmpty())
			ps = opts["kde-pagesize"];
	}
	else
		m_usedriver = false;
	initPageSize(ps, land);

	bool	marginset(false);
	QString	value;
	if (!(value=opts["kde-margin-top"]).isEmpty() && value.toInt() != m_margin->top())
	{
		marginset = true;
		m_margin->setTop(value.toInt());
	}
	if (!(value=opts["kde-margin-left"]).isEmpty() && value.toInt() != m_margin->left())
	{
		marginset = true;
		m_margin->setLeft(value.toInt());
	}
	if (!(value=opts["kde-margin-bottom"]).isEmpty() && value.toInt() != m_margin->bottom())
	{
		marginset = true;
		m_margin->setBottom(value.toInt());
	}
	if (!(value=opts["kde-margin-right"]).isEmpty() && value.toInt() != m_margin->right())
	{
		marginset = true;
		m_margin->setRight(value.toInt());
	}
	m_margin->setCustomEnabled(marginset);
}

void KPMarginPage::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	if (m_margin->isCustomEnabled() /*|| incldef*/)
	{
		opts["kde-margin-top"] = QString::number(m_margin->top());
		opts["kde-margin-left"] = QString::number(m_margin->left());
		opts["kde-margin-bottom"] = QString::number(m_margin->bottom());
		opts["kde-margin-right"] = QString::number(m_margin->right());
	}
	else
	{
		opts.remove("kde-margin-top");
		opts.remove("kde-margin-left");
		opts.remove("kde-margin-bottom");
		opts.remove("kde-margin-right");
	}
}
