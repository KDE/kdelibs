#ifndef KNEWSTUFF2_UI_KDXSBUTTON_H
#define KNEWSTUFF2_UI_KDXSBUTTON_H

#include <qtoolbutton.h>

class KMenu;
class QAction;
class KUrl;

namespace KNS
{

class Dxs;
class Entry;
class DxsEngine;
class Category;

class KDXSButton : public QToolButton
{
Q_OBJECT
public:
	KDXSButton(QWidget *parent = 0);
	~KDXSButton();

        void setEntry(KNS::Entry *e);
	void setEngine(KNS::DxsEngine *engine);

public slots:
	void slotTriggered(QAction *action);
	void slotVersionsActivated(int id);
	void slotVersionsHighlighted(int id);
        void slotClicked();

	void slotInfo(QString provider, QString server, QString version);
	void slotCategories(QList<KNS::Category*> categories);
	void slotEntries(QList<KNS::Entry*> entries);
	void slotComments(QStringList comments);
	void slotHistory(QStringList entries);
	void slotChanges(QStringList entries);
	void slotRemoval(bool success);
	void slotSubscription(bool success);
	void slotComment(bool success);
	void slotRating(bool success);

	void slotFault();
	void slotError();

	void slotPayloadLoaded(KUrl url);
	void slotPayloadFailed();

private:
	bool authenticate();

	// FIXME KDE4PORT item actions
	/*QAction *Items
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
	};*/

	QAction *action_install;
	QAction *action_deinstall;
	QAction *action_comments;
	QAction *action_changes;
	QAction *action_info;

	QAction *action_historysub;

	QAction *action_collabrating;
	QAction *action_collabcomment;
	QAction *action_collaboratesub;
	QAction *action_collabsubscribe;
	QAction *action_collabremoval;
	QAction *action_collabtranslation;

	QAction *action_contactsub;
	QAction *action_contactbymail;
	QAction *action_contactbyjabber;

	enum States
	{
		historyinactive,
		historydisabled,
		historyslots
	};

	KNS::Dxs *m_dxs;
	KMenu *m_p, *m_history, *m_contact;
	KNS::Entry *m_entry;
	KNS::DxsEngine *m_engine;

	QString m_username;
	QString m_password;
};

}

#endif
