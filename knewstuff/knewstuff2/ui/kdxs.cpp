#include "kdxs.h"

#include "kdxsview.h"

#include <kglobal.h>
#include <klocale.h>

KDXS::KDXS()
: KMainWindow()
{
	KNS::KDXSView *view = new KNS::KDXSView(this);
	setCentralWidget(view);
}

KDXS::~KDXS()
{
}

#include "kdxs.moc"
