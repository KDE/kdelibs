#ifndef KCUPSTEXTPAGE_H
#define KCUPSTEXTPAGE_H

#include "kcupstextpagebase.h"
#include "globalppd.h"
#include "ipprequest.h"

class QButtonGroup;

class KCupsTextPage : public KCupsTextPageBase
{
	Q_OBJECT
public:
	KCupsTextPage(global_ppd_file_t *ppd, IppRequest *req, QWidget *parent = 0, const char *name = 0);
	~KCupsTextPage();

	void setOptions(const OptionSet& options);
	void getOptions(OptionSet& options, bool incldef = false);
	QString pageTitle();

protected:
	void initialize();

protected slots:
	void updateMargins();
	void customMarginsClicked();
	void prettyChanged(int);
	void unitsChanged(int);

protected:
	void updateDefaultMargins();
	void updatePagesize();

private:
	global_ppd_file_t	*ppd_;
	IppRequest	*request_;
	int		currentunits_;
	ppd_size_t *pagesize_;
	int	orientation_;
};

#endif
