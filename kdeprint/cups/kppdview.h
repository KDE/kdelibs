#ifndef KPPDVIEW_H
#define	KPPDVIEW_H

#include <qlistview.h>
#include <qdict.h>

#include <cups/cups.h>
#include <cups/ppd.h>
#include "globalppd.h"

#include "optionset.h"

class KPPDBaseItem;

class KPPDView : public QListView
{
	Q_OBJECT;
public:
	KPPDView(global_ppd_file_t *ppd = 0, QWidget *parent = 0, const char *name = 0);
	~KPPDView();

	void initialize(global_ppd_file_t *ppd);
	void markChoices(global_ppd_file_t *ppd);
	void updateChoices(global_ppd_file_t *ppd);
	void getOptions(OptionSet& opts, bool incldef = false);
	void setOptions(const OptionSet& opts);
	bool hasConflict();
	void checkConflict();

public slots:
	void checkConstraints();

protected:
	void resizeEvent(QResizeEvent *);

private:
	global_ppd_file_t	*ppd_;
	KPPDBaseItem	*rootitem_;
	QDict<KPPDBaseItem>	options_;
	bool		conflict_;
};

#endif
