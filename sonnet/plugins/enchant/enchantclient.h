#ifndef QSPELL_ENCHANTCLIENT_H
#define QSPELL_ENCHANTCLIENT_H

#include "speller.h"
#include "client.h"

#include <qobject.h>
#include <qset.h>
#include <enchant.h>

class QSpellEnchantClient : public KSpell2::Client
{
    Q_OBJECT
    Q_INTERFACES(QSpell::Client)
public:
    QSpellEnchantClient(QObject *parent=0);
    ~QSpellEnchantClient();

    virtual int reliability() const {
        return 30;
    }

    virtual QSpell::Speller *createSpeller(const QString &language);

    virtual QStringList languages() const;

    virtual QString name() const {
        return QString::fromLatin1("Enchant");
    }

    void addLanguage(const QString &lang);

private:
    EnchantBroker *m_broker;
    QSet<QString>  m_languages;
};

#endif
