#ifndef KDXSVIEW_H
#define KDXSVIEW_H

#include <kdialogbase.h>

//class QLineEdit;
class QComboBox;

class KDXSView : public KDialogBase
{
Q_OBJECT
public:
	KDXSView(QWidget *parent);
private slots:
	void slotRun();
private:
	enum Access
	{
		access_auto,
		access_http,
		access_webservice
	};
	//QLineEdit *m_url;
	QComboBox *m_url;
	QComboBox *m_type;
};

#endif
