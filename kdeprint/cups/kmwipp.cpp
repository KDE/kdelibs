#include "kmwipp.h"
#include "kmwizard.h"
#include "kmprinter.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <klocale.h>

#include <cups/http.h>

KMWIpp::KMWIpp(QWidget *parent, const char *name)
: KMWInfoBase(2,parent,name)
{
	m_ID = KMWizard::IPP;
	m_title = i18n("Remote IPP server");
	m_nextpage = KMWizard::IPPSelect;

	setInfo(i18n("<p>Enter the informations concerning the remote IPP server "
		     "owning the targeted printer. This wizard will poll the server "
		     " before continuing.</p>"));
	setLabel(0,i18n("Host:"));
	setLabel(1,i18n("Port:"));
}

bool KMWIpp::isValid(QString& msg)
{
	// check informations
	if (text(0).isEmpty())
	{
		msg = i18n("Empty server name !");
		return false;
	}
	bool	ok(false);
	int	p = text(1).toInt(&ok);
	if (!ok)
	{
		msg = i18n("Incorrect port number !");
		return false;
	}

	// check server
	http_t	*HTTP = httpConnect(text(0).latin1(),p);
	if (HTTP)
	{
		httpClose(HTTP);
		return true;
	}
	else
	{
		msg = i18n("<nobr>Unable to connect to <b>%1</b> on port <b>%2</b> !</nobr>").arg(text(0)).arg(p);
		return false;
	}
}

void KMWIpp::updatePrinter(KMPrinter *p)
{
	KURL	url;
	url.setProtocol("ipp");
	url.setHost(text(0));
	url.setPort(text(1).toInt());
	if (!p->option("kde-login").isEmpty()) url.setUser(p->option("kde-login"));
	if (!p->option("kde-password").isEmpty()) url.setPass(p->option("kde-password"));
	p->setDevice(url);
}
