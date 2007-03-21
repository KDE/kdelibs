#include "knewstuff2/engine.h"

using namespace KNS;

XEngine::XEngine()
: DxsEngine()
{
}

XEngine::~XEngine()
{
}

KNS::Entry::List XEngine::downloadDialogModal()
{
	return KNS::Entry::List();
}

KNS::Entry *XEngine::uploadDialogModal()
{
	return NULL;
}

void XEngine::downloadDialog()
{
}

void XEngine::uploadDialog()
{
}

#include "engine.moc"
