#ifndef KMPRINTERPAGE_H
#define KMPRINTERPAGE_H

class KMPrinter;

class KMPrinterPage
{
public:
	virtual void setPrinter(KMPrinter*) = 0;
};

#endif
