#ifndef KPDQJOBLISTER_H
#define KPDQJOBLISTER_H

#include "kjoblister.h"

class KPdqJobLister : public KJobLister
{
public:
	KPdqJobLister();

	int actions();
	bool sendCommand(const QList<KJob>& jobs, int action, const QString& arg);

protected:
	bool listJobs();
};

#endif
