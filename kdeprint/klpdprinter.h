#ifndef KLPDPRINTER_H
#define KLPDPRINTER_H

#include "kprinter.h"

class KLpdPrinter : public KPrinter
{
public:
	KLpdPrinter();
	~KLpdPrinter();

	virtual bool printFiles(const QStringList& files);
	int copyFlags() const;

protected:
	virtual void translateOptions(const OptionSet& opts);
};

#endif
