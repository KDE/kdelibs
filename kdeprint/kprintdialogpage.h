#ifndef KPRINTDIALOGPAGE_H
#define KPRINTDIALOGPAGE_H

#include <qwidget.h>
#include <qstring.h>

#include "optionset.h"

class KPrintDialogPage : public QWidget
{
public:
	KPrintDialogPage(QWidget *parent = 0, const char *name = 0) : QWidget(parent,name) {}
	~KPrintDialogPage() {}

	virtual void setOptions(const OptionSet& options) = 0;
	virtual void getOptions(OptionSet& options, bool incldef = false) = 0;
	virtual QString pageTitle() = 0;
	virtual bool isValid();
};

#endif
