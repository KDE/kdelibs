#include "kdxsview.h"

#include "newstuff.h"

#include "knewstuff2/dxs/dxs.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qmessagebox.h>

using namespace KNS;

KDXSView::KDXSView(QWidget *parent)
: KDialog(parent)
{
	setCaption(i18n("GHNS Providers"));
	setButtons(KDialog::Close);

	QWidget *root = new QWidget(this);
	setMainWidget(root);

	QPushButton *button = new QPushButton(i18n("Get Hot New Stuff!"), root);
	button->setIcon(SmallIcon("knewstuff"));

	QLabel *dxslabel = new QLabel(i18n(
		"Please select the <b>Get Hot New Stuff</b> "
		"provider and method for exploring it. "
		"Feel the power of the <b>Desktop eXchange Service</b>!"), root);

	m_url = new QComboBox(root);
	m_url->setEditable(true);
	m_url->addItem("http://localhost/provider.xml");
	m_url->addItem("http://new.kstuff.org/provider-kdeedu.xml");
	m_url->addItem("http://download.kde.org/khotnewstuff/wallpaper-providers.xml");

	m_type = new QComboBox(root);
	m_type->addItem(i18n("Automatic choice"), access_auto);
	m_type->addItem(i18n("Traditional HTTP download"), access_http);
	m_type->addItem(i18n("Web service interaction (DXS)"), access_webservice);

	QLabel *typelabel = new QLabel(i18n("GHNS access method"), root);
	QLabel *urllabel = new QLabel(i18n("URL of GHNS provider"), root);

	QHBoxLayout *top_layout = new QHBoxLayout(root);
	top_layout->addStretch(1);
	QVBoxLayout *v_layout = new QVBoxLayout();
	v_layout->addStretch(1);
	v_layout->addWidget(dxslabel);
	v_layout->addStretch(1);
	v_layout->addWidget(typelabel);
	v_layout->addWidget(m_type);
	v_layout->addWidget(urllabel);
	v_layout->addWidget(m_url);
	v_layout->addWidget(button);
	v_layout->addStretch(1);
	top_layout->addLayout(v_layout);
	top_layout->addStretch(1);

	connect(button, SIGNAL(clicked()), SLOT(slotRun()));

	show();
	root->setMinimumSize(root->size());
	//disableResize(); // FIXME KDE4PORT
}

void KDXSView::slotRun()
{
	ProviderLoader *pl = new ProviderLoader(/*this*/);
	connect(pl,
		SIGNAL(signalProvidersLoaded(KNS::Provider::List*)),
		SLOT(slotProvidersLoaded(KNS::Provider::List*)));
	connect(pl,
		SIGNAL(signalProvidersFailed()),
		SLOT(slotProvidersFailed()));
	pl->load(m_url->currentText());
}

void KDXSView::slotProvidersLoaded(KNS::Provider::List *providers)
{
	kDebug() << "(providers loaded)" << endl;

	if(m_type->currentIndex() != access_webservice)
	{
		QMessageBox::warning(this,
			i18n("KDXSView"),
			i18n("The access method is not supported yet (only DXS is!)"));
		return;
		// FIXME: remove this restriction after GHNS/DXS merge
		// FIXME: unless we want to keep this a DXS-only test program
	}

	KUrl endpoint;
	for(Provider::List::Iterator it = providers->begin(); it != providers->end(); it++)
	{
		Provider *provider = (*it);
		if(provider->webService().isValid())
		{
			endpoint = provider->webService();
		}
	}

	if(!endpoint.isValid())
	{
		QMessageBox::critical(this,
			i18n("KDXSView"),
			i18n("No provider offered DXS access."));
		return;
	}

	Dxs *dxs = new Dxs();
	dxs->setEndpoint(endpoint);
	//dxs->setMethod(m_type->currentItem());

	DownloadDialog *d = new DownloadDialog(this);
	//d->setEngine(dxs);
	// FIXME: use DxsEngine object here
	d->show();

	// FIXME: provide queueing/update mechanism for dialog
	dxs->call_categories();
}

void KDXSView::slotProvidersFailed()
{
	QMessageBox::critical(this,
		i18n("KDXSView"),
		i18n("It was not possible to load the providers URL."));
}

#include "kdxsview.moc"
