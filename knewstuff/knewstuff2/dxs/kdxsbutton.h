#ifndef KDXSBUTTON_H
#define KDXSBUTTON_H

#include <qtoolbutton.h>

#include <knewstuff/entry.h>

namespace KNS
{
	class Dxs;
	class Entry;
};

class KPopupMenu;

class KDXSButton : public QToolButton
{
Q_OBJECT
public:
	KDXSButton(QWidget *parent);
	~KDXSButton();

        void setEntry(KNS::Entry *e);

public slots:
	void slotActivated(int id);
	void slotHighlighted(int id);
        void slotClicked();

	void slotInfo(QString provider, QString server, QString version);
	void slotCategories(QStringList categories);
	void slotHistory(QStringList categories);
	void slotRemoval(bool success);
	void slotSubscription(bool success);
	void slotComment(bool success);
	void slotRating(bool success);

	void slotFault();

private:
	enum Items
	{
		install,
		deinstall,
		addrating,
		addcomment,
		subscribe,
		info,

		historysub,

		collaboratesub,
		collabremoval,
		collabtranslation,

		contactsub,
		contactbymail,
		contactbyjabber
	};

	enum States
	{
		historydisabled
	};

	KNS::Dxs *m_dxs;
	KPopupMenu *m_p, *m_history, *m_contact;
        KNS::Entry *m_entry;
};

#endif
