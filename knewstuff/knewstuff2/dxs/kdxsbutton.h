#ifndef KDXSBUTTON_H
#define KDXSBUTTON_H

#include <qtoolbutton.h>

#include <knewstuff/entry.h>

namespace KNS
{
	class Dxs;
	class Entry;
	class Category;
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
	void slotVersionsActivated(int id);
	void slotVersionsHighlighted(int id);
        void slotClicked();

	void slotInfo(QString provider, QString server, QString version);
	void slotCategories(QValueList<KNS::Category*> categories);
	void slotEntries(QValueList<KNS::Entry*> entries);
	void slotComments(QStringList comments);
	void slotHistory(QStringList entries);
	void slotChanges(QStringList entries);
	void slotRemoval(bool success);
	void slotSubscription(bool success);
	void slotComment(bool success);
	void slotRating(bool success);

	void slotFault();

private:
	bool authenticate();

	enum Items
	{
		install,
		deinstall,
		comments,
		changes,
		info,

		historysub,

		collabrating,
		collabcomment,
		collaboratesub,
		collabsubscribe,
		collabremoval,
		collabtranslation,

		contactsub,
		contactbymail,
		contactbyjabber
	};

	enum States
	{
		historyinactive,
		historydisabled,
		historyslots
	};

	KNS::Dxs *m_dxs;
	KPopupMenu *m_p, *m_history, *m_contact;
	KNS::Entry *m_entry;

	QString m_username;
	QString m_password;
};

#endif
