#ifndef KMWNAME_H
#define KMWNAME_H

#include "kmwinfobase.h"

class KMWName : public KMWInfoBase
{
public:
	KMWName(QWidget *parent = 0, const char *name = 0);

	bool isValid(QString&);
	void initPrinter(KMPrinter*);
	void updatePrinter(KMPrinter*);
};

#endif
