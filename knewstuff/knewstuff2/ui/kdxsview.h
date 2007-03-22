#ifndef KNEWSTUFF2_UI_KDXSVIEW_H
#define KNEWSTUFF2_UI_KDXSVIEW_H

#include <kdialog.h>

#include <knewstuff2/core/providerloader.h>

class QComboBox;

class KDE_EXPORT KDXSView : public KDialog
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
