#ifndef KCUPSJOBLISTER_H
#define KCUPSJOBLISTER_H

#include "kjoblister.h"

class KCupsJobLister : public KJobLister
{
public:
	KCupsJobLister();
	virtual ~KCupsJobLister();

	int actions();
	bool sendCommand(const QList<KJob>& jobs, int action, const QString& arg);

protected:
	bool listJobs();
};

#endif
