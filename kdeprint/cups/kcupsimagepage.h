#ifndef KCUPSIMAGEPAGE_H
#define KCUPSIMAGEPAGE_H

#include "kcupsimagepagebase.h"
#include "globalppd.h"
#include "ipprequest.h"

class QButtonGroup;

class KCupsImagePage : public KCupsImagePageBase
{
	Q_OBJECT
public:
	KCupsImagePage(global_ppd_file_t *ppd, IppRequest *req, QWidget *parent = 0, const char *name = 0);
	~KCupsImagePage();

	void setOptions(const OptionSet& options);
	void getOptions(OptionSet& options, bool incldef = false);
	QString pageTitle();

protected:
	void initialize();

protected slots:
	void colorSettingsChanged(int);
	void defaultClicked();
	void ppiChanged(int);
	void ppiCheckClicked();
	void scalingChanged(int);
	void scalingCheckClicked();
	void horizChanged(int);
	void vertChanged(int);

private:
	global_ppd_file_t	*ppd_;
	IppRequest	*request_;
	int		vertpos_, horizpos_;
	QButtonGroup	*horizgrp_, *vertgrp_;
};

#endif
