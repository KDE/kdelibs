#ifndef KNEWSTUFF_DXS_H
#define KNEWSTUFF_DXS_H

#include <qobject.h>

class QDomNode;

namespace KNS
{

class Soap;
class Entry;

class Category
{
public:
	Category(){}
	QString categoryname;
	QString name;
	QString icon;
	QString description;
};

class Dxs : public QObject
{
Q_OBJECT
public:
	Dxs();
	~Dxs();
	void setEndpoint(QString endpoint);

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
	void signalCategories(QValueList<KNS::Category*> categories);
	void signalEntries(QValueList<KNS::Entry*> entries);
	void signalComments(QStringList comments);
	void signalChanges(QStringList comments);
	void signalHistory(QStringList entries);
	void signalRemoval(bool success);
	void signalSubscription(bool success);
	void signalComment(bool success);
	void signalRating(bool success);
	void signalFault();

private slots:
	void slotResult(QDomNode node);

private:
	Soap *m_soap;
	QString m_endpoint;
};

}

#endif
