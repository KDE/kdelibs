#ifndef KCUPSHPGL2PAGE_H
#define KCUPSHPGL2PAGE_H

#include "kcupshpgl2pagebase.h"
#include "globalppd.h"
#include "ipprequest.h"

/**
  *@author Michael Goffioul
  */

class KCupsHPGL2Page : public KCupsHPGL2PageBase
{
public:
	KCupsHPGL2Page(global_ppd_file_t *ppd, IppRequest *req, QWidget *parent=0, const char *name=0);
	~KCupsHPGL2Page();

	void setOptions(const OptionSet& opts);
	void getOptions(OptionSet& opts, bool incldef = false);
	QString pageTitle();

protected:
	void initialize();
};

#endif
