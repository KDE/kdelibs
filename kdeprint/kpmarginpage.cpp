/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
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

#include <config.h>

#include "kpmarginpage.h"
#include "kprinter.h"
#include "driver.h"
#include "marginwidget.h"

#include <q3groupbox.h>
#include <qlayout.h>
#include <qprinter.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>

KPMarginPage::KPMarginPage(KPrinter *prt, DrMain *driver, QWidget *parent)
    : KPrintDialogPage(0, driver, parent)
{
	m_printer = prt;
	setTitle(i18n("Margins"));
	m_usedriver = true;

	Q3GroupBox	*box = new Q3GroupBox(1, Qt::Vertical, i18n("Margins"), this);
	m_margin = new MarginWidget(box, (m_printer != 0));
        m_margin->setObjectName( "MarginWidget" );
	//m_margin->setSymetricMargins(true);
	//if (m_printer)
	//	m_margin->setResolution(m_printer->resolution());

	QVBoxLayout	*l0 = new QVBoxLayout(this);
  l0->setMargin(0);
  l0->setSpacing(10);
	l0->addWidget(box);
	l0->addStretch(1);
}

KPMarginPage::~KPMarginPage()
{
}

void KPMarginPage::initPageSize(const QString& ps, bool landscape)
{
	// first retrieve the Qt values for page size and margins
	QPrinter	prt(QPrinter::PrinterResolution);
	prt.setFullPage(true);
	prt.setPageSize((QPrinter::PageSize)(ps.isEmpty() ? KGlobal::locale()->pageSize() : ps.toInt()));
	float	w = prt.width();
	float	h = prt.height();
	unsigned int	it, il, ib, ir;
	prt.margins( &it, &il, &ib, &ir );
	float	mt = it;
	float	ml = il;
	float	mb = ib;
	float	mr = ir;

	if (driver() && m_usedriver )
	{
		QString	pageSize(ps);

		if (pageSize.isEmpty())
		{
			DrListOption	*o = (DrListOption*)driver()->findOption("PageSize");
			if (o)
				pageSize = o->get("default");
		}
		if (!pageSize.isEmpty())
		{
			DrPageSize	*dps = driver()->findPageSize(pageSize);
			if (dps)
			{
				w = dps->pageWidth();
				h = dps->pageHeight();
				mt = qMax( mt, dps->topMargin() );
				ml = qMax( ml, dps->leftMargin() );
				mb = qMax( mb, dps->bottomMargin() );
				mr = qMax( mr, dps->rightMargin() );
			}
		}
	}
	m_margin->setPageSize(w, h);
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
	if (!(value=opts["kde-margin-top"]).isEmpty() && value.toFloat() != m_margin->top())
	{
		marginset = true;
		m_margin->setTop(value.toFloat());
	}
	if (!(value=opts["kde-margin-left"]).isEmpty() && value.toFloat() != m_margin->left())
	{
		marginset = true;
		m_margin->setLeft(value.toFloat());
	}
	if (!(value=opts["kde-margin-bottom"]).isEmpty() && value.toFloat() != m_margin->bottom())
	{
		marginset = true;
		m_margin->setBottom(value.toFloat());
	}
	if (!(value=opts["kde-margin-right"]).isEmpty() && value.toFloat() != m_margin->right())
	{
		marginset = true;
		m_margin->setRight(value.toFloat());
	}
	m_margin->setCustomEnabled(marginset);
}

void KPMarginPage::getOptions(QMap<QString,QString>& opts, bool /* incldef */)
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
