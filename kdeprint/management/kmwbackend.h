#ifndef KMWBACKEND_H
#define KMWBACKEND_H

#include "kmwizardpage.h"
#include <qmap.h>

class QButtonGroup;
class QVBoxLayout;

class KMWBackend : public KMWizardPage
{
public:
	KMWBackend(QWidget *parent = 0, const char *name = 0);

	bool isValid(QString&);
	void initPrinter(KMPrinter*);
	void updatePrinter(KMPrinter*);

	void addBackend(int ID = -1, const QString& txt = QString::null, bool on = true, int nextpage = -1);
	void enableBackend(int ID, bool on = true);

private:
	QButtonGroup	*m_buttons;
	QVBoxLayout	*m_layout;
	// keep a map between button ID and the real next page to switch to. This enables
	// to have different backends switching to the same page (like backends requiring
	// a password). If the next page is not given when adding the backend, the ID is
	// used by default.
	QMap<int,int>	m_map;
};

#endif
