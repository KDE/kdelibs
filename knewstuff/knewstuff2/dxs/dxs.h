#ifndef KNEWSTUFF2_DXS_DXS_H
#define KNEWSTUFF2_DXS_DXS_H

#include <qobject.h>

#include <kurl.h>

class QDomNode;

namespace KNS
{

class Soap;
class Entry;
class Category;

class KDE_EXPORT Dxs : public QObject
{
Q_OBJECT
public:
	Dxs();
	~Dxs();
	void setEndpoint(KUrl endpoint);

	void call_info();
	void call_categories();
	void call_entries(QString category, QString feed);
	void call_comments(int id);
	void call_changes(int id);
	void call_history(int id);
	void call_removal(int id);
	void call_subscription(int id, bool subscribe);
	void call_comment(int id, QString comment);
	void call_rating(int id, int rating);

signals:
	void signalInfo(QString provider, QString server, QString version);
	void signalCategories(QList<KNS::Category*> categories);
	void signalEntries(QList<KNS::Entry*> entries);
	void signalComments(QStringList comments);
	void signalChanges(QStringList comments);
	void signalHistory(QStringList entries);
	void signalRemoval(bool success);
	void signalSubscription(bool success);
	void signalComment(bool success);
	void signalRating(bool success);
	void signalFault();
	void signalError();

private slots:
	void slotResult(QDomNode node);
	void slotError();

private:
	Soap *m_soap;
	KUrl m_endpoint;
};

}

#endif
