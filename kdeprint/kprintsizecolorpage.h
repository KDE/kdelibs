#ifndef KPRINTSIZECOLORPAGE_H
#define KPRINTSIZECOLORPAGE_H

#include "kprintsizecolorpagebase.h"

class KPrintSizeColorPage : public KPrintSizeColorPageBase
{
	Q_OBJECT
public:
	KPrintSizeColorPage(QWidget *parent = 0, const char *name = 0);
	~KPrintSizeColorPage();

	void setOptions(const OptionSet& opts);
	void getOptions(OptionSet& opts, bool incldef = false);
	QString pageTitle();

protected slots:
	void orientationChanged(int);
	void colorChanged(int);

protected:
	void initialize();
};

#endif
