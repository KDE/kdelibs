#ifndef KMFACTORY_H
#define KMFACTORY_H

class KMManager;
class KMJobManager;
class KMUiManager;
class KMVirtualManager;
class KPrinterImpl;
class KLibFactory;

class KMFactory
{
public:
	static KMFactory* self();
	static void release();

	KMFactory();
	~KMFactory();

	KMManager* manager();
	KMJobManager* jobManager();
	KMUiManager* uiManager();
	KMVirtualManager* virtualManager();
	KPrinterImpl* printerImplementation();

private:
	void createManager();
	void createJobManager();
	void createUiManager();
	void createPrinterImpl();
	void loadFactory();

private:
	static KMFactory	*m_self;

	KMManager		*m_manager;
	KMJobManager		*m_jobmanager;
	KMUiManager		*m_uimanager;
	KMVirtualManager	*m_virtualmanager;
	KPrinterImpl		*m_implementation;
	KLibFactory		*m_factory;
};

#endif
