#ifndef KPDQVIEW_H
#define KPDQVIEW_H

#include "optionset.h"

#include <qlistview.h>

class KPdqBaseItem;
class PDQPrinter;
class PDQDriver;
class PDQInterface;

class PdqToolTip;
class PdqWhatsThis;

class KPdqView : public QListView
{
public:
	KPdqView(QWidget *parent = 0, const char *name = 0);
	~KPdqView();

	void init(PDQPrinter *pr, PDQDriver *dr, PDQInterface *in);

	void setOptions(const OptionSet& opts);
	void getOptions(OptionSet& opts, bool incldef = false);

protected:
	void resizeEvent(QResizeEvent*);

private:
	KPdqBaseItem	*root_;
	PDQPrinter	*printer_;
	PDQDriver	*driver_;
	PDQInterface	*interface_;

	PdqToolTip	*tip_;
	PdqWhatsThis	*whatsthis_;
};

#endif
