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

#include <qfile.h>
#include <qlayout.h>
#include <qvbox.h>

#include <kparts/part.h>
#include <kaccel.h>
#include <kaction.h>
#include <klibloader.h>
#include <ktrader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kconfig.h>
#include <ktoolbar.h>

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

class KPrintPreview::KPrintPreviewPrivate
{
public:
	KPrintPreviewPrivate(KPrintPreview *dlg) : gvpart_(0)
	{
		mainwidget_ = new QWidget(dlg, "MainWidget");
		toolbar_ = new KToolBar(mainwidget_, "PreviewToolBar", true);
		actions_ = new KActionCollection(dlg);
		accel_ = new KAccel(dlg);
	}
	~KPrintPreviewPrivate()
	{
		if (gvpart_) delete gvpart_;
	}
	void plugAction(KAction *act)
	{
		act->plug(toolbar_);
		act->plugAccel(accel_);
	}

	KParts::ReadOnlyPart	*gvpart_;
	KToolBar		*toolbar_;
	KActionCollection	*actions_;
	QWidget			*mainwidget_;
	KAccel			*accel_;
};

//*******************************************************************************************

KPrintPreview::KPrintPreview(QWidget *parent, bool previewOnly)
: KDialogBase(parent, "PreviewDlg", true, i18n("Print Preview"), 0)
{
	kdDebug() << "kdeprint: creating preview dialog" << endl;
	d = new KPrintPreviewPrivate(this);

	// create main view and actions
	setMainWidget(d->mainwidget_);
	if (previewOnly)
		new KAction(i18n("Close"), "fileclose", Qt::Key_Return, this, SLOT(reject()), d->actions_, "close_print");
	else
	{
		new KAction(i18n("Print"), "fileprint", Qt::Key_Return, this, SLOT(accept()), d->actions_, "continue_print");
		new KAction(i18n("Cancel"), "stop", Qt::Key_Escape, this, SLOT(reject()), d->actions_, "stop_print");
	}

	// ask the trader for service handling postscript
	kdDebug() << "kdeprint: querying trader for 'application/postscript' service" << endl;
	KTrader::OfferList	offers = KTrader::self()->query(QString::fromLatin1("application/postscript"), QString::fromLatin1("'KParts/ReadOnlyPart' in ServiceTypes"));
	for (KTrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it)
	{
		KService::Ptr	service = *it;
		KLibFactory	*factory = KLibLoader::self()->factory(QFile::encodeName(service->library()));
		if (factory)
		{
			d->gvpart_ = (KParts::ReadOnlyPart*)factory->create(d->mainwidget_, "gvpart", "KParts::ReadOnlyPart");
			break;
		}
	}
	if (!d->gvpart_)
	{
		// nothing has been found, try to load directly KGhostview part
		KLibFactory	*factory = KLibLoader::self()->factory("libkghostview");
		if (factory)
			d->gvpart_ = (KParts::ReadOnlyPart*)factory->create(d->mainwidget_, "gvpart", "KParts::ReadOnlyPart");
	}

	// populate the toolbar
	KAction	*act;
	if (previewOnly)
		d->plugAction(d->actions_->action("close_print"));
	else
	{
		d->plugAction(d->actions_->action("continue_print"));
		d->plugAction(d->actions_->action("stop_print"));
	}
	if (d->gvpart_)
	{
		KAction	*act;
		d->toolbar_->insertLineSeparator();
		if ((act = d->gvpart_->action("zoomIn")) != 0)
			d->plugAction(act);
		if ((act = d->gvpart_->action("zoomOut")) != 0)
			d->plugAction(act);
		d->toolbar_->insertSeparator();
		if ((act = d->gvpart_->action("prevPage")) != 0)
			d->plugAction(act);
		if ((act = d->gvpart_->action("nextPage")) != 0)
			d->plugAction(act);
	}
	d->toolbar_->setIconText(KToolBar::IconTextRight);
	d->toolbar_->setBarPos(KToolBar::Top);
	d->toolbar_->setMovingEnabled(false);
	//static_cast<QWidget*>(d->toolbar_)->layout()->setMargin(1);
	d->toolbar_->adjustSize();

	// construct the layout
	QVBoxLayout	*l0 = new QVBoxLayout(d->mainwidget_, 0, 0);
	l0->addWidget(d->toolbar_, AlignTop);
	if (d->gvpart_)
		l0->addWidget(d->gvpart_->widget());
	
	resize(855, 500);
	setCaption(i18n("Print Preview"));
}

KPrintPreview::~KPrintPreview()
{
	delete d;
}

void KPrintPreview::openFile(const QString& file)
{
	d->gvpart_->openURL(KURL(file));
}

bool KPrintPreview::isValid() const
{
	return (d->gvpart_ != 0);
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
			KMessageBox::error(NULL, i18n("<p>The preview program <b>%1</b> cannot be found. "
						      "Check that the program is correctly installed and "
						      "located in a directory included in your <b>PATH</b> "
						      "environment variable.</p>").arg(exe));
			return false;

		}
		else
		{
			KPreviewProc	proc;
			proc << exe << file;
			if (!proc.startPreview())
			{
				KMessageBox::error(NULL, i18n("<p>Preview failed: unable to start program <b>%1</b>.</p>").arg(exe));
				return false;
			}
			else
				return !previewOnly;
		}
	}
	else
	{
		QWidget	*parentW = QWidget::find(parentId);
		KPrintPreview	dlg(parentW, previewOnly);

		if (dlg.isValid())
		{
			dlg.openFile(file);
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
