#ifndef KJOBLISTER_H
#define KJOBLISTER_H

#include <qlist.h>
#include <qstringlist.h>

#include <kdeprint/kjob.h>

void jobCopy(KJob *src, KJob *dest);

class KJobLister
{
public:
	KJobLister();
	virtual ~KJobLister();

	void addPrinter(const QString& pr);
	void removePrinter(const QString& pr);
	const QStringList& filter() const;

	KJob* findJob(int ID);
	bool sendCommand(int ID, int action, const QString& arg = QString::null);
	const QList<KJob>& jobList();

	virtual int actions();
	virtual bool sendCommand(const QList<KJob>& jobs, int action, const QString& arg = QString::null);

protected:
	void discardAllJobs();
	void removeDiscardedJobs();

protected:
	virtual bool listJobs();

protected:
	QList<KJob>	jobs_;
	QStringList	printers_;
};

inline void KJobLister::addPrinter(const QString& pr)
{ if (printers_.contains(pr) == 0) printers_.append(pr); }

inline void KJobLister::removePrinter(const QString& pr)
{ printers_.remove(pr); }

inline const QStringList& KJobLister::filter() const
{ return printers_; }

#endif
