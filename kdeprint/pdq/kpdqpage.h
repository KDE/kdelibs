#ifndef KPDQPAGE_H
#define KPDQPAGE_H

#include "optionset.h"
#include "kprintdialogpage.h"

class KPdqView;
class KPdqOptionView;
class PDQPrinter;
class PDQDriver;
class PDQInterface;

class KPdqPage : public KPrintDialogPage
{
public:
	KPdqPage(PDQPrinter *pr, PDQDriver *dr, PDQInterface *in, QWidget *parent = 0, const char *name = 0);
	~KPdqPage();

	void setOptions(const OptionSet& opts);
	void getOptions(OptionSet& opts, bool incldef = false);
	QString pageTitle();

private:
	KPdqView	*view_;
	KPdqOptionView	*optview_;
	PDQPrinter	*printer_;
	PDQDriver	*driver_;
	PDQInterface	*interface_;
};

#endif
