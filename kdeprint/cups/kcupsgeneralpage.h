#ifndef KCUPSGENERALPAGE_H
#define KCUPSGENERALPAGE_H

#include "kcupsgeneralpagebase.h"
#include "globalppd.h"
#include "ipprequest.h"

class KCupsGeneralPage : public KCupsGeneralPageBase
{
	Q_OBJECT
public:
	KCupsGeneralPage(global_ppd_file_t *ppd, IppRequest *req, QWidget *parent = 0, const char *name = 0);
	~KCupsGeneralPage();

	void setOptions(const OptionSet& options);
	void getOptions(OptionSet& options, bool incldef = false);
	QString pageTitle();

protected:
	void initialize();

protected slots:
	void orientationChanged(int);
	void nupChanged(int);
	void duplexChanged(int);

private:
	global_ppd_file_t	*ppd_;
	IppRequest	*request_;
};

#endif
