#ifndef KPDQPRINTER_H
#define KPDQPRINTER_H

#include "kprinter.h"

class KPdqPrinter : public KPrinter
{
public:
	KPdqPrinter();
	~KPdqPrinter();

	virtual bool printFiles(const QStringList& files);
	int copyFlags() const;

protected:
	virtual void translateOptions(const OptionSet& opts);
};

#endif
