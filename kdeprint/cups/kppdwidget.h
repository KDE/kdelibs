#ifndef KPPDWIDGET_H
#define	KPPDWIDGET_H

#include <qwidget.h>
#include <cups/ppd.h>
#include <cups/cups.h>

#include <kdeprint/cups/globalppd.h>
#include <kdeprint/optionset.h>

class KPPDView;
class KPPDOptionView;

class KPPDWidget : public QWidget
{
public:
	KPPDWidget(QWidget *parent = 0, const char *name = 0);
	~KPPDWidget();

	void initialize(global_ppd_file_t *ppd);
	void markChoices(global_ppd_file_t *ppd);
	void updateChoices(global_ppd_file_t *ppd);
	void getOptions(OptionSet& opts, bool incldef = false);
	void setOptions(const OptionSet& opts);
	void setAllowNonChangeable(bool on = true);
	bool allowNonChangeable() const;
	bool hasConflict();

private:
	KPPDView	*ppdview_;
	KPPDOptionView	*ppdoptview_;
};

#endif
