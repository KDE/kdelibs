#ifndef KPQTPAGE_H
#define KPQTPAGE_H

#include "kprintdialogpage.h"

class QButtonGroup;
class QComboBox;
class QLabel;

class KPQtPage : public KPrintDialogPage
{
	Q_OBJECT
public:
	KPQtPage(QWidget *parent = 0, const char *name = 0);
	~KPQtPage();

	void setOptions(const QMap<QString,QString>& opts);
	void getOptions(QMap<QString,QString>& opts, bool incldef = false);

protected slots:
	void slotOrientationChanged(int);
	void slotColorModeChanged(int);

protected:
	QButtonGroup	*m_orientbox, *m_colorbox;
	QComboBox	*m_pagesize;
	QLabel		*m_orientpix, *m_colorpix;
};

#endif
