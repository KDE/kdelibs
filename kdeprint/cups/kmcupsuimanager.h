#ifndef KMCUPSUIMANAGER_H
#define KMCUPSUIMANAGER_H

#include "kmuimanager.h"

class KMCupsUiManager : public KMUiManager
{
public:
	KMCupsUiManager(QObject *parent = 0, const char *name = 0);
	~KMCupsUiManager();

	void addPropertyPages(KMPropertyPage*);
	void setupWizard(KMWizard*);
	void setupPrinterPropertyDialog(KPrinterPropertyDialog*);
};

#endif
