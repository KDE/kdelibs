#ifndef KPGENERALPAGE_H
#define KPGENERALPAGE_H

#include "kprintdialogpage.h"

class QComboBox;
class QButtonGroup;
class QGroupBox;
class QLabel;

class KPGeneralPage : public KPrintDialogPage
{
	Q_OBJECT
public:
	KPGeneralPage(KMPrinter *pr, DrMain *dr, QWidget *parent = 0, const char *name = 0);
	~KPGeneralPage();

	void setOptions(const QMap<QString,QString>& opts);
	void getOptions(QMap<QString,QString>& opts, bool incldef = false);

protected:
	void initialize();

protected slots:
	void slotOrientationChanged(int);
	void slotDuplexChanged(int);
	void slotNupChanged(int);

protected:
	QComboBox	*m_pagesize, *m_papertype, *m_inputslot;
	QComboBox	*m_startbanner, *m_endbanner;
	QButtonGroup	*m_orientbox, *m_duplexbox, *m_nupbox;
	QGroupBox	*m_bannerbox;
	QLabel		*m_orientpix, *m_duplexpix, *m_nuppix;
};

#endif
