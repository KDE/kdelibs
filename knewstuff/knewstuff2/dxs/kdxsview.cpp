#include "kdxsview.h"

#include "newstuff.h"

#include <klocale.h>
#include <kiconloader.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qmessagebox.h>

KDXSView::KDXSView(QWidget *parent)
: KDialogBase(parent, "view", true, i18n("GHNS Providers"),
	KDialogBase::Close, KDialogBase::Close, true)
{
	QWidget *root = new QWidget(this);
	setMainWidget(root);

	QPushButton *button = new QPushButton(i18n("Get Hot New Stuff!"), root);

	KIconLoader *il = KGlobal::iconLoader();
	QPixmap pix = il->loadIcon("knewstuff", KIcon::Small);
	button->setIconSet(pix);

	QLabel *dxslabel = new QLabel(i18n(
		"Please select the <b>Get Hot New Stuff</b> "
		"provider and method for exploring it. "
		"Feel the power of the <b>Desktop eXchange Service</b>!"), root);

	m_url = new QComboBox(root);
	m_url->setEditable(true);
	// FIXME: first two are DXS urls, only third is GHNS providers file
	m_url->insertItem("http://localhost/cgi-bin/hotstuff-dxs.pl");
	m_url->insertItem("http://new.kstuff.org/cgi-bin/hotstuff-dxs.pl");
	m_url->insertItem("http://download.kde.org/khotnewstuff/wallpaper-providers.xml");
	//m_url->insertItem("http://download.kde.org/khotnewstuff/wallpaper/wallpaper.xml");

	m_type = new QComboBox(root);
	m_type->insertItem(i18n("Automatic choice"), access_auto);
	m_type->insertItem(i18n("Traditional HTTP download"), access_http);
	m_type->insertItem(i18n("Web service interaction (DXS)"), access_webservice);

	QLabel *typelabel = new QLabel(i18n("GHNS access method"), root);
	QLabel *urllabel = new QLabel(i18n("URL of GHNS repository"), root);

	QHBoxLayout *top_layout = new QHBoxLayout(root, spacingHint());
	top_layout->addStretch(1);
	QVBoxLayout *v_layout = new QVBoxLayout(top_layout, spacingHint());
	v_layout->addStretch(1);
	v_layout->add(dxslabel);
	v_layout->addStretch(1);
	v_layout->add(typelabel);
	v_layout->add(m_type);
	v_layout->add(urllabel);
	v_layout->add(m_url);
	v_layout->add(button);
	v_layout->addStretch(1);
	top_layout->addStretch(1);

	connect(button, SIGNAL(clicked()), SLOT(slotRun()));

	root->resize(500, 300);
}

void KDXSView::slotRun()
{
	if(m_type->currentItem() != access_webservice)
	{
		QMessageBox::warning(this,
			i18n("GHNS access"),
			i18n("The access method is not supported yet (only DXS is!)"));
		return;
	}

	Dxs *dxs = new Dxs();
	dxs->setEndpoint(m_url->currentText());
	//dxs->setMethod(m_type->currentItem());

	NewStuffDialog *d = new NewStuffDialog(this);
	d->setEngine(dxs);
	d->show();

	// FIXME: provide queueing/update mechanism for dialog
	dxs->call_categories();
}

#include "kdxsview.moc"
