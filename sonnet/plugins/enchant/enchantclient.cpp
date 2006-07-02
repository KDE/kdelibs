#include "enchantclient.h"
#include "enchantdict.h"

#include <kgenericfactory.h>
#include <qdebug.h>

typedef KGenericFactory<QSpellEnchantClient> EnchantClientFactory;
K_EXPORT_COMPONENT_FACTORY(kspell_enchant, EnchantClientFactory("kspell_enchant"))

static void enchantDictDescribeFn(const char * const lang_tag,
                                  const char * const provider_name,
                                  const char * const provider_desc,
                                  const char * const provider_file,
                                  void * user_data)
{
    QSpellEnchantClient *client =
        reinterpret_cast<QSpellEnchantClient*>(user_data);
    //qDebug()<<lang_tag<<provider_name<<provider_desc<<provider_file;
    Q_UNUSED(provider_name);
    Q_UNUSED(provider_desc);
    Q_UNUSED(provider_file);
    client->addLanguage(QString::fromLatin1(lang_tag));

}

QSpellEnchantClient::QSpellEnchantClient(QObject *parent)
    : QSpell::Client(parent)
{
    m_broker = enchant_broker_init();
    enchant_broker_list_dicts(m_broker,
                              enchantDictDescribeFn,
                              this);
}

QSpellEnchantClient::~QSpellEnchantClient()
{
    enchant_broker_free(m_broker);
}

QSpell::Speller *QSpellEnchantClient::createSpeller(const QString &language)
{
    EnchantDict *dict = enchant_broker_request_dict(m_broker,
                                                    language.toUtf8());

    if (!dict) {
        const char *err = enchant_broker_get_error(m_broker);
#ifndef NDEBUG
        qDebug()<<"Couldn't create speller for"<<language<<": "<<err;
#endif
        return 0;
    } else {
        return new QSpellEnchantDict(m_broker, dict, language);
    }
}

QStringList QSpellEnchantClient::languages() const
{
    return m_languages.toList();
}

void QSpellEnchantClient::addLanguage(const QString &lang)
{
    m_languages.insert(lang);
}

#include "enchantclient.moc"
