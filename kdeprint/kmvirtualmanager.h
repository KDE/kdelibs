#ifndef KMVIRTUALMANAGER_H
#define KMVIRTUALMANAGER_H

#include <qobject.h>
#include <qlist.h>
#include <qdatetime.h>

class KMPrinter;
class KMManager;

class KMVirtualManager : public QObject
{
public:
	KMVirtualManager(QObject *parent = 0, const char *name = 0);
	~KMVirtualManager();

        void refresh();
	void reset();
	void virtualList(QList<KMPrinter>& list, const QString& prname);
	void triggerSave();

	KMPrinter* findPrinter(const QString& name);
	KMPrinter* findInstance(KMPrinter *p, const QString& name);
	void setDefault(KMPrinter *p, bool save = true);
	QString defaultPrinterName();
	bool isDefault(KMPrinter *p, const QString& name);

	void create(KMPrinter *p, const QString& name);
	void remove(KMPrinter *p, const QString& name);
	void copy(KMPrinter *p, const QString& src, const QString& name);
	void setAsDefault(KMPrinter *p, const QString& name);

protected:
	void loadFile(const QString& filename);
	void saveFile(const QString& filename);
	void addPrinter(KMPrinter *p);
	void checkPrinter(KMPrinter*);

private:
	QDateTime		m_checktime;
        KMManager               *m_manager;
        QString                 m_defaultprinter;
};

inline void KMVirtualManager::reset()
{ m_checktime = QDateTime(); }

#endif
