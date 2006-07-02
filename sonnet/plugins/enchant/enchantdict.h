#ifndef QSPELL_ENCHANTDICT_H
#define QSPELL_ENCHANTDICT_H

#include "speller.h"

#include <enchant.h>

class QSpellEnchantDict : public KSpell2::Speller
{
public:
    ~QSpellEnchantDict();
    virtual bool isCorrect(const QString &word) const;

    virtual QStringList suggest(const QString &word) const;

    virtual bool storeReplacement(const QString &bad,
                                  const QString &good);

    virtual bool addToPersonal(const QString &word);
    virtual bool addToSession(const QString &word);
protected:
    friend class QSpellEnchantClient;
    QSpellEnchantDict(EnchantBroker *broker,
                      EnchantDict *dict,
                      const QString &language);

private:
    EnchantBroker *m_broker;
    EnchantDict   *m_dict;
};

#endif
