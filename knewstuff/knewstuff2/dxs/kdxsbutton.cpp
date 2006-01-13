#include "kdxsbutton.h"

#include "dxs.h"

#include "kns-translation.h"
#include "newstuff.h"
#include "kdxsrating.h"
#include "kdxscomment.h"

#include <qlayout.h>
#include <qdom.h>

#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcursor.h>

#include <ktoolbarbutton.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <klocale.h>
#include <kprocess.h>

KDXSButton::KDXSButton(QWidget *parent)
: QToolButton(parent)
{
	m_entry = 0;

	setBackgroundColor(QColor(255, 255, 255));

	KIconLoader *il = KGlobal::iconLoader();

	m_p = new KPopupMenu(this);
	m_p->insertItem(il->loadIcon("knewstuff", KIcon::Small),
		i18n("Deinstall"), install);
	m_p->insertItem(il->loadIcon("wizard", KIcon::Small),
		i18n("Add Rating"), addrating);
	m_p->insertItem(il->loadIcon("add", KIcon::Small),
		i18n("Add Comment"), addcomment);
	m_p->insertItem(il->loadIcon("bookmark_add", KIcon::Small),
		i18n("Subscribe"), subscribe);

	m_history = new KPopupMenu(this);

	m_p->insertItem(il->loadIcon("kmultiple", KIcon::Small),
		i18n("Switch version"), m_history, historysub);

	m_p->insertSeparator();
	m_p->insertItem(il->loadIcon("info", KIcon::Small),
		i18n("Provider information"), info);

	m_contact = new KPopupMenu(this);

	KPopupMenu *pcollab = new KPopupMenu(this);
	pcollab->insertItem(il->loadIcon("translate", KIcon::Small),
		i18n("Translate"), collabtranslation);
	pcollab->insertItem(il->loadIcon("remove", KIcon::Small),
		i18n("Report bad entry"), collabremoval);
	pcollab->insertItem(il->loadIcon("mail_new", KIcon::Small),
		i18n("Contact author"), m_contact, contactsub);

	m_p->insertSeparator();
	m_p->insertItem(il->loadIcon("gear", KIcon::Small),
		i18n("Collaboration"), pcollab, collaboratesub);

        connect(this, SIGNAL(clicked()), SLOT(slotClicked()));

	connect(m_p, SIGNAL(activated(int)), SLOT(slotActivated(int)));
	connect(m_p, SIGNAL(highlighted(int)), SLOT(slotHighlighted(int)));

	connect(m_contact, SIGNAL(activated(int)), SLOT(slotActivated(int)));
	connect(pcollab, SIGNAL(activated(int)), SLOT(slotActivated(int)));

	connect(m_history, SIGNAL(activated(int)), SLOT(slotActivated(int)));

	m_dxs = new KNS::Dxs();
	//m_dxs->setEndpoint("http://localhost:8080/cgi-bin/run.sh");
	m_dxs->setEndpoint("http://localhost/cgi-bin/kstuff-ws.pl");

	connect(m_dxs,
		SIGNAL(signalInfo(QString, QString, QString)),
		SLOT(slotInfo(QString, QString, QString)));
	connect(m_dxs,
		SIGNAL(signalCategories(QStringList)),
		SLOT(slotCategories(QStringList)));
	connect(m_dxs,
		SIGNAL(signalHistory(QStringList)),
		SLOT(slotHistory(QStringList)));
	connect(m_dxs,
		SIGNAL(signalRemoval(bool)),
		SLOT(slotRemoval(bool)));
	connect(m_dxs,
		SIGNAL(signalSubscription(bool)),
		SLOT(slotSubscription(bool)));
	connect(m_dxs,
		SIGNAL(signalComment(bool)),
		SLOT(slotComment(bool)));
	connect(m_dxs,
		SIGNAL(signalRating(bool)),
		SLOT(slotRating(bool)));
	connect(m_dxs,
		SIGNAL(signalFault()),
		SLOT(slotFault()));

	QPixmap pix = il->loadIcon("knewstuff", KIcon::Small);
	setIconSet(pix);
        setTextLabel(i18n("Install"), false);
        setUsesTextLabel(true);
        setUsesBigPixmap(false);
        setTextPosition(QToolButton::BesideIcon);
	setPopup(m_p);
	show();
}

KDXSButton::~KDXSButton()
{
}

void KDXSButton::setEntry(Entry *e)
{
//	m_dxs->setEntry(e);
	m_entry = e;

	KIconLoader *il = KGlobal::iconLoader();

// XXX ???
// extend Entry class to contain author contact information
	m_contact->insertItem(il->loadIcon("mail_send", KIcon::Small),
		i18n("Send Mail"), contactbymail);
	m_contact->insertItem(il->loadIcon("idea", KIcon::Small),
		i18n("Contact on Jabber"), contactbyjabber);
}

void KDXSButton::slotInfo(QString provider, QString server, QString version)
{
	QString infostring = i18n("Server: %1").arg(server);
	infostring += "\n" + i18n("Provider: %1").arg(provider);
	infostring += "\n" + i18n("Version: %1").arg(version);

	KMessageBox::information(this,
		i18n(infostring),
		i18n("Provider information"));
}

void KDXSButton::slotCategories(QStringList categories)
{
	for(QStringList::Iterator it = categories.begin(); it != categories.end(); it++)
	{
		kdDebug() << (*it) << endl;
	}
}

void KDXSButton::slotHistory(QStringList entries)
{
	KIconLoader *il = KGlobal::iconLoader();

	for(QStringList::Iterator it = entries.begin(); it != entries.end(); it++)
	{
		kdDebug() << (*it) << endl;

		//pversions->insertItem(il->loadIcon("", KIcon::Small),
		//	i18n("0.1 (19.08.2005)"));
		m_history->insertItem(il->loadIcon("history", KIcon::Small),
			i18n((*it)));
	}

	if(entries.size() == 0)
	{
		m_history->insertItem(i18n("(No history found)"), historydisabled);
		m_history->setItemEnabled(historydisabled, false);
	}

	m_p->setCursor(Qt::ArrowCursor);
}

void KDXSButton::slotRemoval(bool success)
{
	if(success)
	{
		KMessageBox::information(this,
			i18n("The removal request was successfully registered."),
			i18n("Removal of entry"));
	}
	else
	{
		KMessageBox::error(this,
			i18n("The removal request failed."),
			i18n("Removal of entry"));
	}
}

void KDXSButton::slotSubscription(bool success)
{
	if(success)
	{
		KMessageBox::information(this,
			i18n("The subscription was successfully completed."),
			i18n("Subscription to entry"));
	}
	else
	{
		KMessageBox::error(this,
			i18n("The subscription request failed."),
			i18n("Subscription to entry"));
	}
}

void KDXSButton::slotRating(bool success)
{
	if(success)
	{
		KMessageBox::information(this,
			i18n("The rating was submitted successfully."),
			i18n("Rating for entry"));
	}
	else
	{
		KMessageBox::error(this,
			i18n("The rating could not be submitted."),
			i18n("Rating for entry"));
	}
}

void KDXSButton::slotComment(bool success)
{
	if(success)
	{
		KMessageBox::information(this,
			i18n("The comment was submitted successfully."),
			i18n("Comment on entry"));
	}
	else
	{
		KMessageBox::error(this,
			i18n("The comment could not be submitted."),
			i18n("Comment on entry"));
	}
}

void KDXSButton::slotFault()
{
	KMessageBox::error(this,
		i18n("A protocol fault has occurred. The request has failed."),
		i18n("Desktop Exchange Service"));
}

void KDXSButton::slotActivated(int id)
{
	if(id == info)
	{
		m_dxs->call_info();
	}
	//if(id == subscribe)
	//{
	//	// TEST
	//	m_dxs->call_categories();
	//}
	if(id == contactbymail)
	{
		QString address = "spillner@kde.org";
		kapp->invokeMailer(address, i18n("KNewStuff contributions"), "");
	}
	if(id == contactbyjabber)
	{
		QString address = "josef@jabber.org";
		KProcess proc;
		proc << "kopete";
		proc << "--autoconnect";
		proc << address;
		proc.start(KProcess::DontCare);
	}
	if(id == collabtranslation)
	{
		Form1 *f = new Form1();
		f->show();
	}
	if(id == collabremoval)
	{
		m_dxs->call_removal(0);
	}
	if(id == subscribe)
	{
		m_dxs->call_subscription(0, true);
	}
	if((id == deinstall) || (id == install))
	{
		NewStuffDialog *d = new NewStuffDialog(this);
		d->show();
	}
	if(id == addcomment)
	{
		KDXSComment comment(this);
		comment.exec();
		QString s = comment.comment();
		if(!s.isEmpty())
		{
			m_dxs->call_comment(0, s);
		}
	}
	if(id == addrating)
	{
		KDXSRating rating(this);
		rating.exec();
		int r = rating.rating();
		if(r >= 0)
		{
			m_dxs->call_rating(0, r);
		}
	}
}

void KDXSButton::slotHighlighted(int id)
{
	kdDebug() << "highlighted!" << endl;

	if(id == historysub)
	{
		m_history->clear();

		m_p->setCursor(KCursor::workingCursor());
		kdDebug() << "hourglass!" << endl;

		m_dxs->call_history();
		// .....
	}
}

void KDXSButton::slotClicked()
{
	slotActivated(install);
}

#include "kdxsbutton.moc"
