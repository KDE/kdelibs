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
#include <kuserprofile.h>
#include <krun.h>
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
	m_bOk = false;
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
		return m_bOk;
	}
	else
		return false;
}

void KPreviewProc::slotProcessExited(KProcess* proc)
{
	kapp->exit_loop();
	if ( proc->normalExit() && proc->exitStatus() == 0 )
		m_bOk = true;
	else
		kdDebug(500) << "KPreviewProc::slotProcessExited: normalExit=" << proc->normalExit()
			<< " exitStatus=" << proc->exitStatus() << endl;
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
		previewonly_ = false;
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
	bool			previewonly_;
};

KLibFactory* componentFactory()
{
	kdDebug(500) << "kdeprint: querying trader for 'application/postscript' service" << endl;
	KLibFactory	*factory(0);
	KTrader::OfferList	offers = KTrader::self()->query(QString::fromLatin1("application/postscript"), QString::fromLatin1("'KParts/ReadOnlyPart' in ServiceTypes"));
	for (KTrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it)
	{
		KService::Ptr	service = *it;
		factory = KLibLoader::self()->factory(QFile::encodeName(service->library()));
		if (factory)
			break;
	}
	if (!factory)
	{
		// nothing has been found, try to load directly the KGhostview part
		factory = KLibLoader::self()->factory("libkghostviewpart");
	}
	return factory;
}

bool continuePrint(const QString& msg_, QWidget *parent, bool previewOnly)
{
	QString	msg(msg_);
	if (previewOnly)
	{
		KMessageBox::error(parent, msg);
		return false;
	}
	else
	{
		msg.append(" ").append(i18n("Do you want to continue printing anyway?"));
		return (KMessageBox::warningYesNo(parent, msg, QString::null, i18n("Print"), i18n("Cancel")) == KMessageBox::Yes);
	}
}

//*******************************************************************************************

KPrintPreview::KPrintPreview(QWidget *parent, bool previewOnly)
: KDialogBase(parent, "PreviewDlg", true, i18n("Print Preview"), 0)
{
	kdDebug(500) << "kdeprint: creating preview dialog" << endl;
	d = new KPrintPreviewPrivate(this);
	d->previewonly_ = previewOnly;

	// create main view and actions
	setMainWidget(d->mainwidget_);
	if (previewOnly)
		new KAction(i18n("Close"), "fileclose", Qt::Key_Return, this, SLOT(reject()), d->actions_, "close_print");
	else
	{
		new KAction(i18n("Print"), "fileprint", Qt::Key_Return, this, SLOT(accept()), d->actions_, "continue_print");
		new KAction(i18n("Cancel"), "stop", Qt::Key_Escape, this, SLOT(reject()), d->actions_, "stop_print");
	}

}

KPrintPreview::~KPrintPreview()
{
	delete d;
}

void KPrintPreview::initView(KLibFactory *factory)
{
	// load the component
	d->gvpart_ = (KParts::ReadOnlyPart*)factory->create(d->mainwidget_, "gvpart", "KParts::ReadOnlyPart");

	// populate the toolbar
	if (d->previewonly_)
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
	//d->adjustSize();

	// construct the layout
	QVBoxLayout	*l0 = new QVBoxLayout(d->mainwidget_, 0, 0);
	l0->addWidget(d->toolbar_, AlignTop);
	if (d->gvpart_)
		l0->addWidget(d->gvpart_->widget());

	resize(855, 500);
	setCaption(i18n("Print Preview"));
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
	KLibFactory	*factory(0);
	bool	externalPreview = conf->readBoolEntry("ExternalPreview", false);
	QWidget	*parentW = QWidget::find(parentId);
	QString	exe;
	if (!externalPreview && (factory = componentFactory()) != 0)
	{
		KPrintPreview	dlg(parentW, previewOnly);
		dlg.initView(factory);

		if (dlg.isValid())
		{
			dlg.openFile(file);
			return dlg.exec();
		}
		else
			// do nothing at that point: try to use the other way around by
			// using an external PS viewer if possible
			;
	}

	// Either the PS viewer component was not found, or an external
	// preview program has been specified
	KPreviewProc	proc;
	if (externalPreview)
	{
		exe = conf->readEntry("PreviewCommand", "gv");
		if (KStandardDirs::findExe(exe).isEmpty())
		{
			QString	msg = i18n("The preview program %1 cannot be found. "
						       "Check that the program is correctly installed and "
						       "located in a directory included in your PATH "
						       "environment variable.").arg(exe);
			return continuePrint(msg, parentW, previewOnly);
		}
		proc << exe << file;
	}
	else
	{
		KService::Ptr serv = KServiceTypeProfile::preferredService( "application/postscript", QString::null );
		if ( serv )
		{
			KURL url;
			url.setPath( file );
			QStringList args = KRun::processDesktopExec( *serv, url, false );
			proc << args;
			exe = serv->name();
		}
		else
		{
			// in that case, the PS viewer component could not be loaded and no service
			// could be found to view PS
			QString	msg = i18n("Preview failed: neither the internal KDE PostScript "
			                   "viewer (KGhostView) nor any other external PostScript "
			                   "viewer could be found.");
			return continuePrint(msg, parentW, previewOnly);
		}
	}

	// start the preview process
	if (!proc.startPreview())
	{
		QString	msg = i18n("Preview failed: unable to start program %1.").arg(exe);
		return continuePrint(msg, parentW, previewOnly);
	}
	else if (!previewOnly)
	{
		return (KMessageBox::questionYesNo(parentW, i18n("Do you want to continue printing?"), QString::null, i18n("Print"), i18n("Cancel"), "continuePrinting") == KMessageBox::Yes);
	}
	else
		return false;
}

#include "kprintpreview.moc"
