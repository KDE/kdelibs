#ifndef KPDRIVERPAGE_H
#define KPDRIVERPAGE_H

#include "kprintdialogpage.h"

class DriverView;

class KPDriverPage : public KPrintDialogPage
{
public:
	KPDriverPage(KMPrinter *p, DrMain *d = 0, QWidget *parent = 0, const char *name = 0);
	~KPDriverPage();

	bool isValid(QString&);
	void setOptions(const QMap<QString,QString>& opts);
	void getOptions(QMap<QString,QString>& opts, bool incldef = false);

protected:
	DriverView	*m_view;
};

#endif
