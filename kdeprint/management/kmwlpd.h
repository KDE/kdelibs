#ifndef KMWLPD_H
#define KMWLPD_H

#include "kmwinfobase.h"

class KMWLpd : public KMWInfoBase
{
public:
	KMWLpd(QWidget *parent = 0, const char *name = 0);

	bool isValid(QString&);
	void updatePrinter(KMPrinter*);
};

#endif
