#ifndef KMWIPP_H
#define KMWIPP_H

#include "kmwinfobase.h"

class KMWIpp : public KMWInfoBase
{
public:
	KMWIpp(QWidget *parent = 0, const char *name = 0);

	bool isValid(QString&);
	void updatePrinter(KMPrinter*);
};

#endif
