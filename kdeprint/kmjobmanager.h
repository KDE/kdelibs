#ifndef KMJOBMANAGER_H
#define KMJOBMANAGER_H

#include <qobject.h>
#include <qlist.h>
#include <qstringlist.h>

class KMJob;

class KMJobManager : public QObject
{
public:
	KMJobManager(QObject *parent = 0, const char *name = 0);
	virtual ~KMJobManager();

	void addPrinter(const QString& pr);
	void removePrinter(const QString& pr);
	const QStringList& filter() const;
	void clearFilter();

	KMJob* findJob(int ID);
	bool sendCommand(int ID, int action, const QString& arg = QString::null);
	const QList<KMJob>& jobList();

	virtual int actions();
	virtual bool sendCommand(const QList<KMJob>& jobs, int action, const QString& arg = QString::null);

protected:
	void discardAllJobs();
	void removeDiscardedJobs();

protected:
	virtual bool listJobs();

protected:
	QList<KMJob>	m_jobs;
	QStringList	m_printers;
};

inline void KMJobManager::addPrinter(const QString& pr)
{ if (m_printers.contains(pr) == 0) m_printers.append(pr); }

inline void KMJobManager::removePrinter(const QString& pr)
{ m_printers.remove(pr); }

inline const QStringList& KMJobManager::filter() const
{ return m_printers; }

inline void KMJobManager::clearFilter()
{ m_printers.clear(); }

#endif
