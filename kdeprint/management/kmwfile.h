#ifndef KMWFILE_H
#define KMWFILE_H

#include "kmwizardpage.h"

class QLineEdit;

class KMWFile : public KMWizardPage
{
	Q_OBJECT
public:
	KMWFile(QWidget *parent = 0, const char *name = 0);

	bool isValid(QString&);
	void updatePrinter(KMPrinter*);

protected slots:
	void slotBrowse();

private:
	QLineEdit	*m_edit;
};

#endif
