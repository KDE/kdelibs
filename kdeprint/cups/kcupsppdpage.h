#ifndef KCUPSPPDPAGE_H
#define KCUPSPPDPAGE_H

#include "kprintdialogpage.h"
#include "globalppd.h"
#include "ipprequest.h"

class KPPDWidget;

class KCupsPPDPage : public KPrintDialogPage
{
public:
	KCupsPPDPage(global_ppd_file_t *ppd, IppRequest *req, QWidget *parent = 0, const char *name = 0);
	~KCupsPPDPage();

	void setOptions(const OptionSet& options);
	void getOptions(OptionSet& options, bool incldef = false);
	QString pageTitle();
	bool isValid();

protected:
	void initialize();

private:
	global_ppd_file_t	*ppd_;
	IppRequest		*request_;
	KPPDWidget		*widget_;
};

#endif
