#ifndef KPRINTCOPIESPAGE_H
#define KPRINTCOPIESPAGE_H

#include "kprintcopiespagebase.h"
#include "optionset.h"

class KPrintCopiesPage : public KPrintCopiesPageBase
{
	Q_OBJECT
public:
	KPrintCopiesPage(QWidget *parent = 0, const char *name = 0);
	~KPrintCopiesPage();

	void setOptions(const OptionSet& options);
	void getOptions(OptionSet& options, bool incldef = false);
	QString pageTitle();

protected slots:
	void collateClicked();
	void rangeEntered();
};

#endif
