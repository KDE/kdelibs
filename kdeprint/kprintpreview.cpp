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
#include "kmfactory.h"

#include <klibloader.h>
#include <ktrader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kprocess.h>
#include <kdialogbase.h>
#include <qlayout.h>
#include <qvbox.h>

KPreviewProc::KPreviewProc()
: KProcess()
{
	connect(this, SIGNAL(processExited(KProcess*)), SLOT(slotProcessExited(KProcess*)));
}

KPreviewProc::~KPreviewProc()
{
}

bool KPreviewProc::startPreview()
{
	if (start())
	{
		kapp->enter_loop();
		return true;
	}
	else
		return false;
}

void KPreviewProc::slotProcessExited(KProcess*)
{
	kapp->exit_loop();
}

//*******************************************************************************************

class PreviewDlg : public KDialogBase
{
public:
	PreviewDlg(QWidget *parent = 0, bool previewOnly = false);
	KPrintPreview* view()	{ return view_; }
private:
	KPrintPreview	*view_;
};

PreviewDlg::PreviewDlg(QWidget *parent, bool previewOnly)
: KDialogBase(Swallow, i18n("Print preview"), 0, (KDialogBase::ButtonCode)0, parent, "PreviewDlg", true, false, QString::null)
{
	if (layout())
		layout()->setMargin(0);

	view_ = new KPrintPreview(this, previewOnly);
	setMainWidget(view_);

	resize(560, 600);

	connect(view_, SIGNAL(continuePrint()), this, SLOT(accept()));
	connect(view_, SIGNAL(cancelPrint()), this, SLOT(reject()));
}

//*******************************************************************************************

KPrintPreview::KPrintPreview(QWidget *parent, bool previewOnly)
: KParts::MainWindow(parent, "PreviewDlg")
{
	kdDebug() << "kdeprint: creating preview dialog" << endl;
	setXMLFile(locate("config","ui/kprintpreviewui.rc"));
	setHelpMenuEnabled(false);

	if (previewOnly)
		new KAction(i18n("Close"),"fileclose",Qt::Key_Return,this,SIGNAL(cancelPrint()),actionCollection(),"close_print");
	else
	{
		new KAction(i18n("Print"),"fileprint",Qt::Key_Return,this,SIGNAL(continuePrint()),actionCollection(),"continue_print");
		new KAction(i18n("Cancel"),"stop",Qt::Key_Escape,this,SIGNAL(cancelPrint()),actionCollection(),"stop_print");
	}

	gvpart_ = 0;

	// ask the trader for service handling postscript
	kdDebug() << "kdeprint: querying trader for 'application/postscript' service" << endl;
	KTrader::OfferList	offers = KTrader::self()->query(QString::fromLatin1("application/postscript"), QString::fromLatin1("'KParts/ReadOnlyPart' in ServiceTypes"));
	for (KTrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it)
	{
		KService::Ptr	service = *it;
		KLibFactory	*factory = KLibLoader::self()->factory(service->library().latin1());
		if (factory)
		{
			gvpart_ = (KParts::ReadOnlyPart*)factory->create(this, "gvpart", "KParts::ReadOnlyPart");
			break;
		}
	}
	if (!gvpart_)
	{
		// nothing has been found, try to load directly KGhostview part
		KLibFactory	*factory = KLibLoader::self()->factory("libkghostview");
		if (factory)
			gvpart_ = (KParts::ReadOnlyPart*)factory->create(this, "gvpart", "KParts::ReadOnlyPart");
	}

	if (gvpart_)
	{
		setCentralWidget(gvpart_->widget());
		createGUI(gvpart_);
	}

	resize(560, 600);
}

KPrintPreview::~KPrintPreview()
{
	if (gvpart_) delete gvpart_;
}

void KPrintPreview::openFile(const QString& file)
{
	gvpart_->openURL(KURL(file));
}

bool KPrintPreview::isValid() const
{
	return (gvpart_ != 0);
}

bool KPrintPreview::preview(const QString& file, bool previewOnly, WId parentId)
{
	KConfig	*conf = KMFactory::self()->printConfig();
	conf->setGroup("General");
	if (conf->readBoolEntry("ExternalPreview", false))
	{
		QString	exe = conf->readEntry("PreviewCommand", "gv");
		if (KStandardDirs::findExe(exe).isEmpty())
		{
			KMessageBox::error(NULL, i18n("The preview program <b>%1</b> cannot be found. "
						      "Check that the program is correctly installed and "
						      "located in a directory included in your <b>PATH</b> "
						      "environment variable.").arg(exe));
			return false;

		}
		else
		{
			KPreviewProc	proc;
			proc << exe << file;
			if (!proc.startPreview())
			{
				KMessageBox::error(NULL, i18n("Preview failed: unable to start program <b>%1</b>.").arg(exe));
				return false;
			}
			else
				return !previewOnly;
		}
	}
	else
	{
		QWidget	*parentW = QWidget::find(parentId);
		PreviewDlg	dlg(parentW, previewOnly);

		if (dlg.view()->isValid())
		{
			dlg.view()->openFile(file);
			return dlg.exec();
		}
		else
			return (KMessageBox::warningYesNo(parentW,
				i18n("<p>KDE was unable to locate an appropriate object "
				     "for print previewing. Do you want to continue "
					 "printing anyway?</p>"),
				i18n("Warning"), i18n("Print"), i18n("Cancel")) == KMessageBox::Yes);
	}
}

#include "kprintpreview.moc"
