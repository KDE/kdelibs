#ifndef KMWINFOBASE_H
#define KMWINFOBASE_H

#include "kmwizardpage.h"
#include <qlist.h>

class QLabel;
class QLineEdit;

class KMWInfoBase : public KMWizardPage
{
public:
	KMWInfoBase(int n = 1, QWidget *parent = 0, const char *name = 0);

	void setInfo(const QString&);
	void setLabel(int, const QString&);
	void setText(int, const QString&);
	void setCurrent(int);

	QString text(int);

private:
	QList<QLabel>		m_labels;
	QList<QLineEdit>	m_edits;
	QLabel			*m_info;
	int			m_nlines;
};

#endif
