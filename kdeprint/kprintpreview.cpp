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

#include "kprintpreview.h"

#include <klibloader.h>
#include <ktrader.h>
#include <ktrader.h>
#include <kapp.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

KPrintPreview::KPrintPreview(QWidget *parent)
: KParts::MainWindow(parent,"KPrintPreview",WType_Modal|WType_TopLevel|WStyle_Dialog)
{
	kdDebug() << "kdeprint: creating preview dialog" << endl;
	setXMLFile(locate("config","ui/kprintpreviewui.rc"));
	setHelpMenuEnabled(false);

	new KAction(i18n("Print"),"fileprint",Qt::Key_Return,this,SLOT(accept()),actionCollection(),"continue_print");
	new KAction(i18n("Cancel"),"stop",Qt::Key_Escape,this,SLOT(reject()),actionCollection(),"stop_print");

	gvpart_ = 0;
	status_ = false;

	// ask the trader for service handling postscript
	kdDebug() << "kdeprint: querying trader for 'application/postscript' service" << endl;
	KTrader::OfferList	offers = KTrader::self()->query(QString::fromLatin1("application/postscript"),QString::fromLatin1("'KParts/ReadOnlyPart' in ServiceTypes"));
	for (KTrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it)
	{
		KService::Ptr	service = *it;
		KLibFactory	*factory = KLibLoader::self()->factory(service->library().latin1());
		if (factory)
		{
			gvpart_ = (KParts::ReadOnlyPart*)factory->create(this,"gvpart","KParts::ReadOnlyPart");
			break;
		}
	}
	if (!gvpart_)
	{
		// nothing has been found, try to load directly KGhostview part
		KLibFactory	*factory = KLibLoader::self()->factory("libkghostview");
		if (factory)
			gvpart_ = (KParts::ReadOnlyPart*)factory->create(this,"gvpart","KParts::ReadOnlyPart");
	}

	if (gvpart_)
	{
		setCentralWidget(gvpart_->widget());
		createGUI(gvpart_);
	}
}

KPrintPreview::~KPrintPreview()
{
	if (gvpart_) delete gvpart_;
}

void KPrintPreview::openFile(const QString& file)
{
	gvpart_->openURL(KURL(file));
	QSize	s(gvpart_->widget()->sizeHint());
	//resize(QSize(s.width()+40,s.height()+70));
	resize(QMAX(s.width(),760),QMAX(s.height(),750));

	setCaption(i18n("Print preview"));
}

void KPrintPreview::accept()
{
	done(true);
}

void KPrintPreview::reject()
{
	done(false);
}

void KPrintPreview::done(bool st)
{
	status_ = st;
	hide();
	kapp->exit_loop();
}

bool KPrintPreview::isValid() const
{
	return (gvpart_ != 0);
}

void KPrintPreview::exec(const QString& file)
{
	if (isValid())
	{
		show();
		if (!file.isNull()) openFile(file);
		kapp->enter_loop();
	}
	else
		status_ = (KMessageBox::warningYesNo(this,i18n("KDE was unable to locate an appropriate object for print previewing.\nDo you want to continue printing anyway ?")) == KMessageBox::Yes);
}

void KPrintPreview::closeEvent(QCloseEvent *e)
{
	e->accept();
	reject();
}

bool KPrintPreview::preview(const QString& file)
{
	KPrintPreview	dlg(0);
	dlg.exec(file);
	return dlg.status();
}
#include "kprintpreview.moc"
