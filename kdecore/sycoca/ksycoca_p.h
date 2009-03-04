#ifndef KSYCOCA_P_H
#define KSYCOCA_P_H

#include "ksycocafactory.h"
#include <QStringList>
class QFile;
class QDataStream;
class KSycocaAbstractDevice;

class KSycocaPrivate
{
public:
    KSycocaPrivate();
    bool checkVersion();
    bool openDatabase(bool openDummyIfNotFound=true);
    enum BehaviorIfNotFound {
        IfNotFoundDoNothing = 0,
        IfNotFoundOpenDummy = 1,
        IfNotFoundRecreate = 2
    };
    Q_DECLARE_FLAGS(BehaviorsIfNotFound, BehaviorIfNotFound)
    bool checkDatabase(BehaviorsIfNotFound ifNotFound);
    void closeDatabase();
    void setStrategyFromString(const QString& strategy);
    bool tryMmap();

    KSycocaAbstractDevice* device();
    QDataStream*& stream();

    enum {
        DatabaseNotOpen, // openDatabase must be called
        NoDatabase, // not found, so we opened a dummy one instead
        BadVersion, // it's opened, but it's not useable
        DatabaseOK } databaseStatus;
    bool readError;

    quint32 timeStamp;
    enum { StrategyMmap, StrategyMemFile, StrategyFile, StrategyDummyBuffer } m_sycocaStrategy;
    QString m_databasePath;
    QStringList changeList;
    QString language;
    quint32 updateSig;
    QStringList allResourceDirs;

    void addFactory(KSycocaFactory* factory) {
        m_factories.append(factory);
    }
    KSycocaFactoryList* factories() { return &m_factories; }

private:
    KSycocaFactoryList m_factories;
    size_t sycoca_size;
    const char *sycoca_mmap;
    QFile* m_mmapFile;
    KSycocaAbstractDevice* m_device;
};

#endif /* KSYCOCA_P_H */

