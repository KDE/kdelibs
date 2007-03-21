#include "knewstuff2/engine.h"

using namespace KNS;

Engine::Engine()
: DxsEngine()
{
}

Engine::~Engine()
{
}

KNS::Entry::List Engine::downloadDialogModal()
{
	return KNS::Entry::List();
}

KNS::Entry *Engine::uploadDialogModal()
{
	return NULL;
}

void Engine::downloadDialog()
{
}

void Engine::uploadDialog()
{
}

#include "engine.moc"
