#ifndef KCUPSPRINTER_H
#define KCUPSPRINTER_H

#include "kprinter.h"

class KCupsPrinter : public KPrinter
{
public:
	KCupsPrinter();
	~KCupsPrinter();

	virtual QSize margins() const;
	virtual bool printFiles(const QStringList& filename);

protected:
	virtual void translateOptions(const OptionSet& opts);
	virtual int metric(int f) const;
	virtual bool supportPrinterOptions() const;

private:
	QSize	pagesize_;
	QSize	margins_;
};

#endif
