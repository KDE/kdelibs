#include "kmdriverdialog.h"
#include "driverview.h"

#include <klocale.h>

KMDriverDialog::KMDriverDialog(QWidget *parent, const char *name)
: KDialogBase(KDialogBase::Swallow,i18n("Configure"),KDialogBase::Ok|KDialogBase::Cancel,KDialogBase::Ok,parent,name,true,false)
{
	m_view = new DriverView(0);
	setMainWidget(m_view);

	resize(400,450);
}

KMDriverDialog::~KMDriverDialog()
{
}

void KMDriverDialog::setDriver(DrMain *d)
{
	m_view->setDriver(d);
}
