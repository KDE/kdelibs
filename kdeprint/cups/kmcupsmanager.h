#ifndef KMCUPSMANAGER_H
#define KMCUPSMANAGER_H

#include "kmmanager.h"

class IppRequest;

class KMCupsManager : public KMManager
{
public:
	KMCupsManager(QObject *parent = 0, const char *name = 0);
	virtual ~KMCupsManager();

	// printer management functions
	bool createPrinter(KMPrinter *p);
	bool removePrinter(KMPrinter *p);
	bool enablePrinter(KMPrinter *p);
	bool disablePrinter(KMPrinter *p);
	bool completePrinter(KMPrinter *p);
	bool completePrinterShort(KMPrinter *p);
	bool setDefaultPrinter(KMPrinter *p);
	bool testPrinter(KMPrinter *p);

	// printer listing functions
	// driver DB functions
	QString driverDbCreationProgram();
	QString driverDirectory();

	DrMain* loadPrinterDriver(KMPrinter *p, bool config = false);
	DrMain* loadFileDriver(const QString& filename);
	bool savePrinterDriver(KMPrinter *p, DrMain *d);

	bool configure(QWidget *parent = 0);

protected:
	// the real printer listing job is done here
	void listPrinters();
	void loadServerPrinters();
	void processRequest(IppRequest*);
	bool setPrinterState(KMPrinter *p, int st);
	DrMain* loadDriverFile(const QString& filename);
	void saveDriverFile(DrMain *driver, const QString& filename);
	void reportIppError(IppRequest*);
};

#endif
