#ifndef KPCOPIESPAGE_H
#define KPCOPIESPAGE_H

#include "kprintdialogpage.h"

class QRadioButton;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QLabel;

class KPCopiesPage : public KPrintDialogPage
{
	Q_OBJECT
public:
	KPCopiesPage(QWidget *parent = 0, const char *name = 0);
	~KPCopiesPage();

	void setFlags(int f);

	void setOptions(const QMap<QString,QString>& opts);
	void getOptions(QMap<QString,QString>& opts, bool incldef = false);

protected slots:
	void slotRangeEntered();
	void slotCollateClicked();

protected:
	QRadioButton	*m_all, *m_current, *m_range;
	QLineEdit	*m_rangeedit;
	QComboBox	*m_pageset;
	QCheckBox	*m_collate, *m_order;
	QSpinBox	*m_copies;
	QLabel		*m_collatepix;
};

#endif
