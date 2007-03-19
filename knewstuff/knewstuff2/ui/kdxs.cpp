#include "kdxs.h"

#include "kdxsview.h"

#include <kglobal.h>
#include <klocale.h>

KDXS::KDXS()
: KMainWindow()
{
	KDXSView *view = new KDXSView(this);
	setCentralWidget(view);
}

KDXS::~KDXS()
{
}

#include "kdxs.moc"
