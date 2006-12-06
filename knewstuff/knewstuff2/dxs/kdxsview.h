#ifndef KDXSVIEW_H
#define KDXSVIEW_H

#include <kdialogbase.h>

#include <knewstuff2/providerloader.h>

class QComboBox;

class KDXSView : public KDialogBase
{
Q_OBJECT
public:
	KDXSView(QWidget *parent = NULL);
private slots:
	void slotRun();
	void slotProvidersLoaded(KNS::Provider::List *providers);
	void slotProvidersFailed();
private:
	enum Access
	{
		access_auto,
		access_http,
		access_webservice
	};
	QComboBox *m_url;
	QComboBox *m_type;
};

#endif
