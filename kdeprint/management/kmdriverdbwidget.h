#ifndef KMDRIVERDBWIDGET_H
#define KMDRIVERDBWIDGET_H

#include <qwidget.h>
#include "kmdbentry.h"

class QListBox;
class QCheckBox;
class QPushButton;

class KMDriverDbWidget : public QWidget
{
	Q_OBJECT;
public:
	KMDriverDbWidget(QWidget *parent = 0, const char *name = 0);
	~KMDriverDbWidget();

	void init();
	void setHaveRaw(bool on);
	void setHaveOther(bool on);
	void setDriver(const QString& manu, const QString& model);

	QString manufacturer();
	QString model();
	QString description()		{ return m_desc; }
	KMDBEntryList* drivers();
	QString driverFile();
	bool isRaw();
	bool isExternal();

protected slots:
	void slotDbLoaded(bool reloaded);
	void slotManufacturerSelected(const QString& name);
	void slotPostscriptToggled(bool);
	void slotOtherClicked();

private:
	QListBox	*m_manu;
	QListBox	*m_model;
	QCheckBox	*m_postscript;
	QCheckBox	*m_raw;
	QPushButton	*m_other;
	QString		m_external;
	QString		m_desc;
};

inline QString KMDriverDbWidget::driverFile()
{ return m_external; }

inline bool KMDriverDbWidget::isExternal()
{ return !(m_external.isEmpty()); }

#endif
