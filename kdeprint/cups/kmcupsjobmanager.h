#ifndef KMCUPSJOBMANAGER_H
#define KMCUPSJOBMANAGER_H

#include "kmjobmanager.h"

class KMCupsJobManager : public KMJobManager
{
public:
	KMCupsJobManager(QObject *parent = 0, const char *name = 0);
	virtual ~KMCupsJobManager();

	int actions();
	bool sendCommand(const QList<KMJob>& jobs, int action, const QString& arg = QString::null);

protected:
	bool listJobs();
};

#endif
