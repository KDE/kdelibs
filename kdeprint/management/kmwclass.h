#ifndef KMWCLASS_H
#define KMWCLASS_H

#include "kmwizardpage.h"

class KListBox;

class KMWClass : public KMWizardPage
{
	Q_OBJECT
public:
	KMWClass(QWidget *parent = 0, const char *name = 0);
	~KMWClass();

	bool isValid(QString&);
	void initPrinter(KMPrinter*);
	void updatePrinter(KMPrinter*);

protected slots:
	void slotAdd();
	void slotRemove();

private:
	KListBox	*m_list1, *m_list2;
};

#endif
