#ifndef KMPROPMEMBERS_H
#define KMPROPMEMBERS_H

#include "kmpropwidget.h"

class QTextView;

class KMPropMembers : public KMPropWidget
{
public:
	KMPropMembers(QWidget *parent = 0, const char *name = 0);
	~KMPropMembers();

	void setPrinter(KMPrinter*);

protected:
	void configureWizard(KMWizard*);

private:
	QTextView	*m_members;
};

#endif
