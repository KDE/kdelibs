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

#include "ippreportdlg.h"
#include "ipprequest.h"
#include "kprinter.h"

#include <klocale.h>
#include <kguiitem.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <ktextedit.h>
#include <qsimplerichtext.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>

IppReportDlg::IppReportDlg(QWidget *parent, const char *name)
: KDialogBase(parent, name, true, i18n("IPP Report"), Close|User1, Close, false, KGuiItem(i18n("&Print"), "fileprint"))
{
	m_edit = new KTextEdit(this);
	m_edit->setReadOnly(true);
	setMainWidget(m_edit);
	resize(540, 500);
	setFocusProxy(m_edit);
	setButtonText(User1, i18n("&Print"));
}

void IppReportDlg::slotUser1()
{
	KPrinter	printer;
	printer.setFullPage(true);
	printer.setDocName(caption());
	if (printer.setup(this))
	{
		QPainter	painter(&printer);
		QPaintDeviceMetrics	metrics(&printer);

		// report is printed using QSimpleRichText
		QSimpleRichText	rich(m_edit->text(), font());
		rich.setWidth(&painter, metrics.width());
		int	margin = (int)(1.5 / 2.54 * metrics.logicalDpiY());	// 1.5 cm
		QRect	r(margin, margin, metrics.width()-2*margin, metrics.height()-2*margin);
		int	hh = rich.height(), page(1);
		while (1)
		{
			rich.draw(&painter, margin, margin, r, colorGroup());
			QString	s = caption() + ": " + QString::number(page);
			QRect	br = painter.fontMetrics().boundingRect(s);
			painter.drawText(r.right()-br.width()-5, r.top()-br.height()-4, br.width()+5, br.height()+4, Qt::AlignRight|Qt::AlignTop, s);
			r.moveBy(0, r.height()-10);
			painter.translate(0, -(r.height()-10));
			if (r.top() < hh)
			{
				printer.newPage();
				page++;
			}
			else
				break;
		}
	}
}

void IppReportDlg::report(IppRequest *req, int group, const QString& caption)
{
	QString	str_report;
	QTextStream	t(&str_report, IO_WriteOnly);

	if (req->htmlReport(group, t))
	{
		IppReportDlg	dlg;
		if (!caption.isEmpty())
			dlg.setCaption(caption);
		dlg.m_edit->setText(str_report);
		dlg.exec();
	}
	else
		KMessageBox::error(0, i18n("Internal error: unable to generate HTML report."));
}

#include "ippreportdlg.moc"
