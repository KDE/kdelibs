#ifndef KMINSTANCEPAGE_H
#define KMINSTANCEPAGE_H

#include <kmainwindow.h>
#include "kmprinterpage.h"

class KListBox;
class KMPrinter;
class KActionCollection;
class KToolBar;

class KMInstancePage : public QWidget, public KMPrinterPage
{
	Q_OBJECT
public:
	KMInstancePage(QWidget *parent = 0, const char *name = 0);
	~KMInstancePage();

	void setPrinter(KMPrinter*);

protected slots:
	void slotNew();
	void slotCopy();
	void slotRemove();
	void slotDefault();
	void slotTest();
	void slotSettings();

protected:
	void initActions();

private:
	KListBox		*m_view;
	KActionCollection	*m_actions;
	KToolBar		*m_toolbar;
	KMPrinter		*m_printer;
};

#endif
