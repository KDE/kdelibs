#ifndef KMPROPWIDGET_H
#define KMPROPWIDGET_H

#include <qwidget.h>

class KMPrinter;
class KMWizard;

class KMPropWidget : public QWidget
{
	Q_OBJECT
public:
	KMPropWidget(QWidget *parent = 0, const char *name = 0);
	virtual ~KMPropWidget();

	virtual void setPrinter(KMPrinter*);
	void setPrinterBase(KMPrinter*);
	QString pixmap() const	{ return m_pixmap; }
	QString title() const	{ return m_title; }
	QString header() const	{ return m_header; }

signals:
	void enable(bool);
	void enableChange(bool);

public slots:
	void slotChange();

protected:
	virtual bool requestChange();
	virtual void configureWizard(KMWizard*);

protected:
	QString		m_pixmap;
	QString		m_title;
	QString		m_header;
	KMPrinter	*m_printer;
};

#endif
