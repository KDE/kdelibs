/* This file is part of the KDE libraries
   Copyright (c) 1997,2001 Stephan Kulow <coolo@kde.org>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1999-2002 Hans Petter Bieker <bieker@kde.org>
   Copyright (c) 2002 Lukas Tinkl <lukas@kde.org>
   Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "klocale.h"
#include "klocale_p.h"

#include <config.h>

#include <math.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_LANGINFO_H
#include <langinfo.h>
#endif

#include <QtCore/QTextCodec>
#include <QtCore/QFile>
#include <QtGui/QPrinter>
#include <QtCore/QFileInfo>
#include <QtCore/QRegExp>
#include <QtCore/QLocale>
#include <QtCore/QHash>
#include <QtCore/QMutexLocker>

#include "kcatalog_p.h"
#include "kglobal.h"
#include "kstandarddirs.h"
#include "kconfig.h"
#include "kcomponentdata.h"
#include "kdebug.h"
#include "kdatetime.h"
#include "kcalendarsystem.h"
#include "kcurrencycode.h"
#include "klocalizedstring.h"
#include "kconfiggroup.h"
#include "kcatalogname_p.h"
#include "common_helpers_p.h"

#ifdef Q_WS_WIN
#include <windows.h>
#endif

class KLocaleStaticData
{
public:

    KLocaleStaticData();

    QString maincatalog;

    // FIXME: Temporary until full language-sensitivity implemented.
    QHash<KLocale::DigitSet, QStringList> languagesUsingDigitSet;
};

KLocaleStaticData::KLocaleStaticData()
{
    // Languages using non-Western Arabic digit sets.
    // FIXME: Temporary until full language-sensitivity implemented.
    languagesUsingDigitSet.insert(KLocale::ArabicIndicDigits, QStringList() << "ar" << "ps");
    languagesUsingDigitSet.insert(KLocale::BengaliDigits, QStringList() << "bn" << "as" );
    languagesUsingDigitSet.insert(KLocale::DevenagariDigits, QStringList() << "hi" << "ne");
    languagesUsingDigitSet.insert(KLocale::EasternArabicIndicDigits, QStringList() << "fa" << "ur");
    languagesUsingDigitSet.insert(KLocale::GujaratiDigits, QStringList() << "gu" );
    languagesUsingDigitSet.insert(KLocale::GurmukhiDigits, QStringList() << "pa" );
    languagesUsingDigitSet.insert(KLocale::KannadaDigits, QStringList() << "kn" );
    languagesUsingDigitSet.insert(KLocale::KhmerDigits, QStringList() << "km" );
    languagesUsingDigitSet.insert(KLocale::MalayalamDigits, QStringList() << "ml" );
    languagesUsingDigitSet.insert(KLocale::OriyaDigits, QStringList() << "or" );
    languagesUsingDigitSet.insert(KLocale::TamilDigits, QStringList() << "ta" );
    languagesUsingDigitSet.insert(KLocale::TeluguDigits, QStringList() << "te" );
    languagesUsingDigitSet.insert(KLocale::ThaiDigits, QStringList() << "th");
}

K_GLOBAL_STATIC(KLocaleStaticData, staticData)


QDebug operator<<(QDebug debug, const KCatalogName &cn)
{
    return debug << cn.name << cn.loadCount;
}

class KLocalePrivate
{
public:
    KLocalePrivate(const QString& catalog, KConfig *config, const QString &language_ = QString(),
                   const QString &country_ = QString());
    /**
     * @internal Initializes the catalogs appname, kdelibs and kio for all chosen languages.
     */
    void initMainCatalogs();

    /**
     * @internal Initializes the list of valid languages from the user's point of view. This is the
     * list of languages that the user picks in kcontrol. The config object should be valid and
     * contain the global entries.
     *
     * @param config The configuration object used for init
     * @param useEnv True if we should use environment variables
     */
    void initLanguageList(KConfig *config, bool useEnv);

    /**
     * @internal Figures out which encoding the user prefers.
     */
    void initEncoding();

    /**
     * @internal Figures out which encoding the user prefers for filenames
     * and sets up the appropriate QFile encoding and decoding functions.
     */
    void initFileNameEncoding();

    /**
     * @internal A QFile filename encoding function (QFile::encodeFn).
     */
    static QByteArray encodeFileNameUTF8(const QString & fileName);

    /**
     * @internal QFile filename decoding function (QFile::decodeFn).
     */
    static QString decodeFileNameUTF8(const QByteArray & localFileName);

    /**
     * @internal Reads the format configuration from disk.
     */
    void initFormat(KConfig *config);

    /**
     * @internal Sets the Currency Code
     */
    void setCurrencyCode(const QString &newCurrencyCode);

    /**
     * @internal Initialises the Currency
     */
    void initCurrency();

    /**
     * @internal The worker of the same-name KLocale API function.
     */
    bool setLanguage(const QString & _language, KConfig *config);

    /**
     * @internal The worker of the same-name KLocale API function.
     */
    bool setLanguage(const QStringList & languages);

    /**
     * @internal The worker of the same-name KLocale API function.
     */
    bool setEncoding(int mibEnum);

    /**
     * @internal function used by the translate versions
     */
    void translate_priv(const char *catname, const char *msgctxt, const char *msgid, const char *msgid_plural = 0,
                        unsigned long n = 0, QString *language = 0, QString *translation = 0) const;

    /**
     * @internal function used to determine if we are using the en_US translation
     */
    bool useDefaultLanguage() const;

    /**
     * @internal evaluate the list of catalogs and check that all instances for all languages are
     * loaded and that they are sorted according to the catalog names
     *
     * Callers must lock the mutex first.
     */
    void updateCatalogs();

    /**
     * @internal Checks whether or not theFind catalog for given language and given catalog name.
     *
     * @param language language to check
     */
    bool isApplicationTranslatedInto(const QString & language);

    /**
     * @internal Formats a date/time according to specified format.
     */
    static QString formatDateTime(const KLocale *locale, const QDateTime &dateTime,
                                  KLocale::DateFormat, bool includeSeconds, int daysToNow,
                                  int secsToNow);

    /**
     * @internal
     * @return list of translated binary unit for @p dialect.
     */
    QList<QString> dialectUnitsList(KLocale::BinaryUnitDialect dialect);

    enum DurationType {
        DaysDurationType = 0,
        HoursDurationType,
        MinutesDurationType,
        SecondsDurationType
    };
    /**
     * @internal Formats a duration according to the given type and number
     */
    static QString formatSingleDuration(DurationType durationType, int n);

    // Numbers and money
    QString decimalSymbol;
    QString thousandsSeparator;
    QString currencySymbol;
    QString monetaryDecimalSymbol;
    QString monetaryThousandsSeparator;
    QString positiveSign;
    QString negativeSign;
    KLocale::DigitSet digitSet;
    int decimalPlaces;
    int monetaryDecimalPlaces;
    KLocale::SignPosition positiveMonetarySignPosition;
    KLocale::SignPosition negativeMonetarySignPosition;
    bool positivePrefixCurrencySymbol : 1;
    bool negativePrefixCurrencySymbol : 1;
    KLocale::DigitSet monetaryDigitSet;
    KLocale::BinaryUnitDialect binaryUnitDialect;

    // Date and time
    QString timeFormat;
    QString dateFormat;
    QString dateFormatShort;
    int weekStartDay;
    int workingWeekStartDay;
    int workingWeekEndDay;
    int weekDayOfPray;
    KLocale::DigitSet dateTimeDigitSet;

    // FIXME: Temporary until full language-sensitivity implemented.
    bool languageSensitiveDigits;

    // Locale
    QString language;
    QString country;

    // Currency
    KCurrencyCode *currency;
    QString currencyCode;
    QStringList currencyCodeList;

    // Handling of translation catalogs
    QStringList languageList;

    QList<KCatalogName> catalogNames; // list of all catalogs (regardless of language)
    QList<KCatalog> catalogs; // list of all found catalogs, one instance per catalog name and
                              // language
    int numberOfSysCatalogs; // number of catalogs that each app draws from
    bool useTranscript; // indicates if scripted messages are to be executed

    // Misc
    QString encoding;
    QTextCodec * codecForEncoding;
    //KSharedConfig::Ptr config;
    int pageSize;
    KLocale::MeasureSystem measureSystem;
    KConfig * languages;

    QString calendarType;
    KCalendarSystem * calendar;
    QString catalogName; // catalogName ("app name") used by this KLocale object
    bool nounDeclension: 1;
    bool dateMonthNamePossessive: 1;
    bool utf8FileEncoding: 1;
#ifdef Q_WS_WIN
    char win32SystemEncoding[3+7]; //"cp " + lang ID
#endif

    // Performance stuff for binary units.
    QList<QString> byteSizeFmt;
};

KLocalePrivate::KLocalePrivate(const QString& catalog, KConfig *config, const QString &language_,
                               const QString &country_)
               : language(language_),
                 country(country_),
                 currency(0),
                 useTranscript(false),
                 codecForEncoding(0),
                 languages(0),
                 calendar(0),
                 catalogName(catalog)
{
    initEncoding();
    initFileNameEncoding();

    if (config) {
        initLanguageList(config, false);
    } else {
        config = KGlobal::config().data();
        initLanguageList(config, true);
    }

    initMainCatalogs();

    initFormat(config);
}

KLocale::KLocale(const QString & catalog, KSharedConfig::Ptr config)
        : d(new KLocalePrivate(catalog, config.data()))
{
}

KLocale::KLocale(const QString& catalog, const QString &language, const QString &country,
                 KConfig *config)
        : d(new KLocalePrivate(catalog, config, language, country))
{
}

void KLocalePrivate::initMainCatalogs()
{
    KLocaleStaticData *s = staticData;
    QMutexLocker lock(kLocaleMutex());

    if (!s->maincatalog.isEmpty()) {
        // If setMainCatalog was called, then we use that
        // (e.g. korgac calls setMainCatalog("korganizer") to use korganizer.po)
        catalogName = s->maincatalog;
    }

    if (catalogName.isEmpty()) {
        kDebug(173) << "KLocale instance created called without valid "
                    << "catalog! Give an argument or call setMainCatalog "
                    << "before init" << endl;
    } else {
        // do not use insertCatalog here, that would already trigger updateCatalogs
        catalogNames.append(KCatalogName(catalogName));   // application catalog

        // catalogs from which each application can draw translations
        const int numberOfCatalogs = catalogNames.size();
        catalogNames.append(KCatalogName("libphonon"));
        catalogNames.append(KCatalogName("kio4"));
        catalogNames.append(KCatalogName("kdelibs4"));
        catalogNames.append(KCatalogName("kdeqt"));
        catalogNames.append(KCatalogName("solid_qt"));
        catalogNames.append(KCatalogName("kdecalendarsystems"));
        numberOfSysCatalogs = catalogNames.size() - numberOfCatalogs;

        updateCatalogs(); // evaluate this for all languages
    }
}

static void getLanguagesFromVariable(QStringList& list, const char* variable,
                                     bool isLanguageList = false)
{
    QByteArray var(qgetenv(variable));
    if (!var.isEmpty()) {
        QString value = QFile::decodeName(var);
        if (isLanguageList) {
            list += value.split(':');
        } else {
            // Process the value to create possible combinations.
            QString lang, ctry, modf, cset;
            KLocale::splitLocale(value, lang, ctry, modf, cset);

            if (!ctry.isEmpty() && !modf.isEmpty()) {
                list += lang + '_' + ctry + '@' + modf;
            }
            // NOTE: The priority is tricky in case both ctry and modf are present.
            // Should really lang@modf be of higher priority than lang_ctry?
            // For at least one case (Serbian language), it is better this way.
            if (!modf.isEmpty()) {
                list += lang + '@' + modf;
            }
            if (!ctry.isEmpty()) {
                list += lang + '_' + ctry;
            }
            list += lang;
        }
    }
}

void KLocalePrivate::initLanguageList(KConfig *config, bool useEnv)
{
    KConfigGroup cg(config, "Locale");

    // Set the country as specified by the KDE config or use default,
    // do not consider environment variables.
    if (country.isEmpty()) {
        country = cg.readEntry("Country");
    }
    if (country.isEmpty()) {
        country = KLocale::defaultCountry();
    }

    // Collect possible languages by decreasing priority.
    // The priority is as follows:
    // - the internally set language, if any
    // - KDE_LANG environment variable (can be a list)
    // - KDE configuration (can be a list)
    // - environment variables considered by gettext(3)
    // The environment variables are not considered if useEnv is false.
    QStringList list;
    if (!language.isEmpty()) {
        list += language;
    }
    if (useEnv) {
        // KDE_LANG contains list of language codes, not locale string.
        getLanguagesFromVariable(list, "KDE_LANG", true);
    }
    QString languages(cg.readEntry("Language", QString()));
    if (!languages.isEmpty()) {
        list += languages.split(':');
    }
    if (useEnv) {
        // Collect languages by same order of priority as for gettext(3).
        // LANGUAGE contains list of language codes, not locale string.
        getLanguagesFromVariable(list, "LANGUAGE", true);
        getLanguagesFromVariable(list, "LC_ALL");
        getLanguagesFromVariable(list, "LC_MESSAGES");
        getLanguagesFromVariable(list, "LANG");
    }
#ifdef Q_WS_WIN // how about Mac?
    // fall back to the system language
    getLanguagesFromVariable(list, QLocale::system().name().toLocal8Bit().data());
#endif

    // Send the list to filter for really present languages on the system.
    setLanguage(list);
}

void KLocalePrivate::initFormat(KConfig *config)
{
    Q_ASSERT(config);

    //kDebug(173) << "KLocalePrivate::KLocalePrivate";

    config->setLocale(language);

    KConfigGroup cg(config, "Locale");

    KConfig entryFile(KStandardDirs::locate("locale",
                                            QString::fromLatin1("l10n/%1/entry.desktop")
                                            .arg(country)));
    entryFile.setLocale(language);
    KConfigGroup entry(&entryFile, "KCM Locale");

    //One-time conversion in 4.4 from FracDigits to DecimalPlaces and MonetaryDecimalPlaces
    //If user has personal setting for FracDigits then use it for both Decimal Places
    //TODO: Possible to do with kconf_update
    if (cg.hasKey("FracDigits")) {
        QString fracDigits = cg.readEntry("FracDigits", "");
        if (!fracDigits.isEmpty()) {
            cg.writeEntry("DecimalPlaces", fracDigits);
            cg.writeEntry("MonetaryDecimalPlaces", fracDigits);
        }
        cg.deleteEntry("FracDigits");
        cg.config()->sync();
    }

    // Numeric
#define readConfigEntry(key, default, save) \
        save = entry.readEntry(key, default); \
        save = cg.readEntry(key, save);

#define readConfigNumEntry(key, default, save, type) \
        save = (type)entry.readEntry(key, int(default)); \
        save = (type)cg.readEntry(key, int(save));

    // Numeric formats
    readConfigNumEntry("DecimalPlaces", 2, decimalPlaces, int);

    readConfigEntry("DecimalSymbol", ".", decimalSymbol);
    readConfigEntry("ThousandsSeparator", ",", thousandsSeparator);
    thousandsSeparator.remove(QString::fromLatin1("$0"));

    readConfigEntry("PositiveSign", "", positiveSign);
    readConfigEntry("NegativeSign", "-", negativeSign);

    readConfigNumEntry("DigitSet", KLocale::ArabicDigits, digitSet, KLocale::DigitSet);
    // FIXME: Temporary until full language-sensitivity implemented.
    readConfigEntry("LanguageSensitiveDigits", true, languageSensitiveDigits);

    // Currency
    readConfigEntry("CurrencyCode", "USD", currencyCode);
    initCurrency();
    readConfigEntry("CurrencySymbol", currency->defaultSymbol(), currencySymbol);
    readConfigEntry("CurrencyCodesInUse", QStringList(currencyCode), currencyCodeList);

    // Monetary formats
    readConfigNumEntry("MonetaryDecimalPlaces", currency->decimalPlaces(),
                       monetaryDecimalPlaces, int);

    readConfigEntry("MonetaryDecimalSymbol", ".", monetaryDecimalSymbol);
    readConfigEntry("MonetaryThousandsSeparator", ",", monetaryThousandsSeparator);
    monetaryThousandsSeparator.remove(QString::fromLatin1("$0"));

    readConfigEntry("PositivePrefixCurrencySymbol", true, positivePrefixCurrencySymbol);
    readConfigEntry("NegativePrefixCurrencySymbol", true, negativePrefixCurrencySymbol);
    readConfigNumEntry("PositiveMonetarySignPosition", KLocale::BeforeQuantityMoney,
                       positiveMonetarySignPosition, KLocale::SignPosition);
    readConfigNumEntry("NegativeMonetarySignPosition", KLocale::ParensAround,
                       negativeMonetarySignPosition, KLocale::SignPosition);

    readConfigNumEntry("MonetaryDigitSet", KLocale::ArabicDigits,
                       monetaryDigitSet, KLocale::DigitSet);
    readConfigNumEntry("BinaryUnitDialect", KLocale::IECBinaryDialect,
                       binaryUnitDialect, KLocale::BinaryUnitDialect);

    // Date and time
    readConfigEntry("TimeFormat", "%H:%M:%S", timeFormat);
    readConfigEntry("DateFormat", "%A %d %B %Y", dateFormat);
    readConfigEntry("DateFormatShort", "%Y-%m-%d", dateFormatShort);
    readConfigNumEntry("WeekStartDay", 1, weekStartDay, int);                //default to Monday
    readConfigNumEntry("WorkingWeekStartDay", 1, workingWeekStartDay, int);  //default to Monday
    readConfigNumEntry("WorkingWeekEndDay", 5, workingWeekEndDay, int);      //default to Friday
    readConfigNumEntry("WeekDayOfPray", 7, weekDayOfPray, int);              //default to Sunday
    readConfigNumEntry("DateTimeDigitSet", KLocale::ArabicDigits,
                       dateTimeDigitSet, KLocale::DigitSet);

    // other
    readConfigNumEntry("PageSize", QPrinter::A4, pageSize, QPrinter::PageSize);
    readConfigNumEntry("MeasureSystem", KLocale::Metric, measureSystem, KLocale::MeasureSystem);
    readConfigEntry("CalendarSystem", "gregorian", calendarType);
    delete calendar;
    calendar = 0; // ### HPB Is this the correct place?

    readConfigEntry("Transcript", true, useTranscript);

    //Grammatical
    //Precedence here is l10n / i18n / config file
    KConfig langCfg(KStandardDirs::locate("locale",
                                          QString::fromLatin1("%1/entry.desktop").arg(language)));
    KConfigGroup lang(&langCfg, "KCM Locale");
#define read3ConfigBoolEntry(key, default, save) \
        save = entry.readEntry(key, default); \
        save = lang.readEntry(key, save); \
        save = cg.readEntry(key, save);

    read3ConfigBoolEntry("NounDeclension", false, nounDeclension);
    read3ConfigBoolEntry("DateMonthNamePossessive", false, dateMonthNamePossessive);
}

bool KLocale::setCountry(const QString & aCountry, KConfig *config)
{
    // Check if the file exists too??
    if (aCountry.isEmpty()) {
        return false;
    }

    d->country = aCountry;

    d->initFormat(config);

    return true;
}

bool KLocale::setLanguage(const QString & language, KConfig *config)
{
    return d->setLanguage(language, config);
}

bool KLocalePrivate::setLanguage(const QString & _language, KConfig *config)
{
    QMutexLocker lock(kLocaleMutex());
    languageList.removeAll(_language);
    languageList.prepend(_language);   // let us consider this language to be the most important one

    language = _language; // remember main language for shortcut evaluation

    // important when called from the outside and harmless when called before
    // populating the catalog name list
    updateCatalogs();

    initFormat(config);

    // Maybe the mo-files for this language are empty, but in principle we can speak all languages
    return true;
}

bool KLocale::setLanguage(const QStringList & languages)
{
    return d->setLanguage(languages);
}

bool KLocalePrivate::setLanguage(const QStringList & languages)
{
    QMutexLocker lock(kLocaleMutex());
    // This list might contain
    // 1) some empty strings that we have to eliminate
    // 2) duplicate entries like in de:fr:de, where we have to keep the first occurrence of a
    //    language in order to preserve the order of precenence of the user
    // 3) languages into which the application is not translated. For those languages we should not
    //    even load kdelibs.mo or kio.po. these languages have to be dropped. Otherwise we get
    //    strange side effects, e.g. with Hebrew: the right/left switch for languages that write
    //    from right to left (like Hebrew or Arabic) is set in kdelibs.mo. If you only have
    //    kdelibs.mo but nothing from appname.mo, you get a mostly English app with layout from
    //    right to left. That was considered to be a bug by the Hebrew translators.
    QStringList list;
    foreach(const QString &language, languages) {
        if (!language.isEmpty() && !list.contains(language) &&
            isApplicationTranslatedInto(language)) {
            list.append(language);
        }
    }

    if (!list.contains(KLocale::defaultLanguage())) {
        // English should always be added as final possibility; this is important
        // for proper initialization of message text post-processors which are
        // needed for English too, like semantic to visual formatting, etc.
        list.append(KLocale::defaultLanguage());
    }

    language = list.first(); // keep this for shortcut evaluations

    languageList = list; // keep this new list of languages to use

    // important when called from the outside and harmless when called before populating the
    // catalog name list
    updateCatalogs();

    return true; // we found something. Maybe it's only English, but we found something
}

void KLocalePrivate::initCurrency()
{
    if (currencyCode.isEmpty() || !KCurrencyCode::isValid(currencyCode)) {
        currencyCode = KLocale::defaultCurrencyCode();
    }

    if (!currency || currencyCode != currency->isoCurrencyCode() || !currency->isValid()) {
        delete currency;
        currency = new KCurrencyCode(currencyCode, language);
    }
}

void KLocalePrivate::setCurrencyCode(const QString &newCurrencyCode)
{
    if (!newCurrencyCode.isEmpty() && newCurrencyCode != currency->isoCurrencyCode() &&
        KCurrencyCode::isValid(newCurrencyCode)) {
        currencyCode = newCurrencyCode;
        initCurrency();
    }
}

void KLocale::setCurrencyCode(const QString &newCurrencyCode)
{
    d->setCurrencyCode(newCurrencyCode);
}

bool KLocale::isApplicationTranslatedInto(const QString & lang)
{
    return d->isApplicationTranslatedInto(lang);
}

bool KLocalePrivate::isApplicationTranslatedInto(const QString & lang)
{
    if (lang.isEmpty()) {
        return false;
    }

    if (lang == KLocale::defaultLanguage()) {
        // default language is always "installed"
        return true;
    }

    if (catalogName.isEmpty()) {
        kDebug() << "no appName!";
        return false;
    }

    if (!KCatalog::catalogLocaleDir(catalogName, lang).isEmpty()) {
        return true;
    }
    return false;
}

void KLocale::splitLocale(const QString &aLocale, QString &language, QString &country,
                          QString &modifier, QString &charset)
{
    QString locale = aLocale;

    language.clear();
    country.clear();
    modifier.clear();
    charset.clear();

    // In case there are several concatenated locale specifications,
    // truncate all but first.
    int f = locale.indexOf(':');
    if (f >= 0) {
        locale.truncate(f);
    }

    f = locale.indexOf('.');
    if (f >= 0) {
        charset = locale.mid(f + 1);
        locale.truncate(f);
    }

    f = locale.indexOf('@');
    if (f >= 0) {
        modifier = locale.mid(f + 1);
        locale.truncate(f);
    }

    f = locale.indexOf('_');
    if (f >= 0) {
        country = locale.mid(f + 1);
        locale.truncate(f);
    }

    language = locale;
}

QString KLocale::language() const
{
    return d->language;
}

QString KLocale::country() const
{
    return d->country;
}

KCurrencyCode *KLocale::currency() const
{
    if (!d->currency) {
        d->initCurrency();
    }
    return d->currency;
}

QString KLocale::currencyCode() const
{
    return d->currencyCode;
}

void KLocale::insertCatalog(const QString & catalog)
{
    QMutexLocker lock(kLocaleMutex());
    int pos = d->catalogNames.indexOf(KCatalogName(catalog));
    if (pos != -1) {
        ++d->catalogNames[pos].loadCount;
        return;
    }

    // Insert new catalog just before system catalogs, to preserve the
    // lowest priority of system catalogs.
    d->catalogNames.insert(d->catalogNames.size() - d->numberOfSysCatalogs, KCatalogName(catalog));
    d->updateCatalogs(); // evaluate the changed list and generate the necessary KCatalog objects
}

void KLocalePrivate::updateCatalogs()
{
    // some changes have occurred. Maybe we have learned or forgotten some languages.
    // Maybe the language precedence has changed.
    // Maybe we have learned or forgotten some catalog names.

    QList<KCatalog> newCatalogs;

    // now iterate over all languages and all wanted catalog names and append or create them in the
    // right order the sequence must be e.g. nds/appname nds/kdelibs nds/kio de/appname de/kdelibs
    // de/kio etc. and not nds/appname de/appname nds/kdelibs de/kdelibs etc. Otherwise we would be
    // in trouble with a language sequende nds,<default>,de. In this case <default> must hide
    // everything after itself in the language list.
    foreach(const QString &lang, languageList) {
        foreach(const KCatalogName &name, catalogNames) {
            // create and add catalog for this name and language if it exists
            if (! KCatalog::catalogLocaleDir(name.name, lang).isEmpty()) {
                newCatalogs.append(KCatalog(name.name, lang));
                //kDebug(173) << "Catalog: " << name << ":" << lang;
            }
        }
    }

    // notify KLocalizedString of catalog update.
    catalogs = newCatalogs;
    KLocalizedString::notifyCatalogsUpdated(languageList, catalogNames);
}

void KLocale::removeCatalog(const QString &catalog)
{
    QMutexLocker lock(kLocaleMutex());
    int pos = d->catalogNames.indexOf(KCatalogName(catalog));
    if (pos == -1) {
        return;
    }
    if (--d->catalogNames[pos].loadCount > 0) {
        return;
    }
    d->catalogNames.removeAt(pos);
    if (KGlobal::hasMainComponent()) {
        // walk through the KCatalog instances and weed out everything we no longer need
        d->updateCatalogs();
    }
}

void KLocale::setActiveCatalog(const QString &catalog)
{
    QMutexLocker lock(kLocaleMutex());
    int pos = d->catalogNames.indexOf(KCatalogName(catalog));
    if (pos == -1) {
        return;
    }
    d->catalogNames.move(pos, 0);
    // walk through the KCatalog instances and adapt to the new order
    d->updateCatalogs();
}

KLocale::~KLocale()
{
    delete d->currency;
    delete d->calendar;
    delete d->languages;
    delete d;
}

void KLocalePrivate::translate_priv(const char *catname, const char *msgctxt, const char *msgid,
                                    const char *msgid_plural, unsigned long n, QString *language,
                                    QString *translation) const
{
    if (!msgid || !msgid[0]) {
        kDebug(173) << "KLocale: trying to look up \"\" in catalog. "
        << "Fix the program" << endl;
        language->clear();
        translation->clear();
        return;
    }
    if (msgctxt && !msgctxt[0]) {
        kDebug(173) << "KLocale: trying to use \"\" as context to message. "
        << "Fix the program" << endl;
    }
    if (msgid_plural && !msgid_plural[0]) {
        kDebug(173) << "KLocale: trying to use \"\" as plural message. "
        << "Fix the program" << endl;
    }

    QMutexLocker locker(kLocaleMutex());
    // determine the fallback string
    QString fallback;
    if (msgid_plural == NULL) {
        fallback = QString::fromUtf8(msgid);
    } else {
        if (n == 1) {
            fallback = QString::fromUtf8(msgid);
        } else {
            fallback = QString::fromUtf8(msgid_plural);
        }
    }
    if (language) {
        *language = KLocale::defaultLanguage();
    }
    if (translation) {
        *translation = fallback;
    }

    // shortcut evaluation if default language is main language: do not consult the catalogs
    if (useDefaultLanguage()) {
        return;
    }

    const QList<KCatalog> catalogList = catalogs;
    QString catNameDecoded;
    if (catname != NULL) {
        catNameDecoded = QString::fromUtf8(catname);
    }
    for (QList<KCatalog>::ConstIterator it = catalogList.constBegin(); it != catalogList.constEnd();
         ++it) {
        // shortcut evaluation: once we have arrived at default language, we cannot consult
        // the catalog as it will not have an assiciated mo-file. For this default language we can
        // immediately pick the fallback string.
        if ((*it).language() == KLocale::defaultLanguage()) {
            return;
        }

        if (catNameDecoded.isEmpty() || catNameDecoded == (*it).name()) {
            QString text;
            if (msgctxt != NULL && msgid_plural != NULL) {
                text = (*it).translateStrict(msgctxt, msgid, msgid_plural, n);
            } else if (msgid_plural != NULL) {
                text = (*it).translateStrict(msgid, msgid_plural, n);
            } else if (msgctxt != NULL) {
                text = (*it).translateStrict(msgctxt, msgid);
            } else {
                text = (*it).translateStrict(msgid);
            }

            if (!text.isEmpty()) {
                // we found it
                if (language) {
                    *language = (*it).language();
                }
                if (translation) {
                    *translation = text;
                }
                return;
            }
        }
    }
}

void KLocale::translateRawFrom(const char* catname, const char* msg, QString *lang, QString *trans) const
{
    d->translate_priv(catname, 0, msg, 0, 0, lang, trans);
}

void KLocale::translateRaw(const char* msg, QString *lang, QString *trans) const
{
    d->translate_priv(0, 0, msg, 0, 0, lang, trans);
}

void KLocale::translateRawFrom(const char* catname, const char *ctxt, const char *msg, QString *lang, QString *trans) const
{
    d->translate_priv(catname, ctxt, msg, 0, 0, lang, trans);
}

void KLocale::translateRaw(const char *ctxt, const char *msg, QString *lang, QString *trans) const
{
    d->translate_priv(0, ctxt, msg, 0, 0, lang, trans);
}

void KLocale::translateRawFrom(const char* catname, const char *singular, const char *plural, unsigned long n, QString *lang,
                               QString *trans) const
{
    d->translate_priv(catname, 0, singular, plural, n, lang, trans);
}

void KLocale::translateRaw(const char *singular, const char *plural, unsigned long n, QString *lang,
                           QString *trans) const
{
    d->translate_priv(0, 0, singular, plural, n, lang, trans);
}

void KLocale::translateRawFrom(const char* catname, const char *ctxt, const char *singular, const char *plural,
                               unsigned long n, QString *lang, QString *trans) const
{
    d->translate_priv(catname, ctxt, singular, plural, n, lang, trans);
}

void KLocale::translateRaw(const char *ctxt, const char *singular, const char *plural,
                           unsigned long n, QString *lang, QString *trans) const
{
    d->translate_priv(0, ctxt, singular, plural, n, lang, trans);
}

QString KLocale::translateQt(const char *context, const char *sourceText, const char *comment) const
{
    // Qt's context is normally the name of the class of the method which makes
    // the tr(sourceText) call. However, it can also be manually supplied via
    // translate(context, sourceText) call.
    //
    // Qt's sourceText is the actual message displayed to the user.
    //
    // Qt's comment is an optional argument of tr() and translate(), like
    // tr(sourceText, comment) and translate(context, sourceText, comment).
    //
    // We handle this in the following way:
    //
    // If the comment is given, then it is considered gettext's msgctxt, so a
    // context call is made.
    //
    // If the comment is not given, but context is given, then we treat it as
    // msgctxt only if it was manually supplied (the one in translate()) -- but
    // we don't know this, so we first try a context call, and if translation
    // is not found, we fallback to ordinary call.
    //
    // If neither comment nor context are given, it's just an ordinary call
    // on sourceText.

    if (!sourceText || !sourceText[0]) {
        kDebug(173) << "KLocale: trying to look up \"\" in catalog. "
        << "Fix the program" << endl;
        return QString();
    }

    if (d->useDefaultLanguage()) {
        return QString();
    }

    QString translation;
    QString language;

    // NOTE: Condition (language != defaultLanguage()) means that translation
    // was found, otherwise we got the original string back as translation.

    if (comment && comment[0]) {
        // Comment given, go for context call.
        d->translate_priv(0, comment, sourceText, 0, 0, &language, &translation);
    } else {
        // Comment not given, go for try-fallback with context.
        if (context && context[0]) {
            d->translate_priv(0, context, sourceText, 0, 0, &language, &translation);
        }
        if (language.isEmpty() || language == defaultLanguage()) {
            d->translate_priv(0, 0, sourceText, 0, 0, &language, &translation);
        }
    }

    if (language != defaultLanguage()) {
        return translation;
    }

    // No proper translation found, return empty according to Qt's expectation.
    return QString();
}

QList<KLocale::DigitSet> KLocale::allDigitSetsList() const
{
    QList<DigitSet> digitSets;
    digitSets.append(ArabicDigits);
    digitSets.append(ArabicIndicDigits);
    digitSets.append(BengaliDigits);
    digitSets.append(DevenagariDigits);
    digitSets.append(EasternArabicIndicDigits);
    digitSets.append(GujaratiDigits);
    digitSets.append(GurmukhiDigits);
    digitSets.append(KannadaDigits);
    digitSets.append(KhmerDigits);
    digitSets.append(MalayalamDigits);
    digitSets.append(OriyaDigits);
    digitSets.append(TamilDigits);
    digitSets.append(TeluguDigits);
    digitSets.append(ThaiDigits);
    return digitSets;
}

static QString digitSetString(KLocale::DigitSet digitSet)
{
    switch (digitSet) {
    case KLocale::ArabicIndicDigits:
        return QString::fromUtf8("٠١٢٣٤٥٦٧٨٩");
    case KLocale::BengaliDigits:
        return QString::fromUtf8("০১২৩৪৫৬৭৮৯");
    case KLocale::DevenagariDigits:
        return QString::fromUtf8("०१२३४५६७८९");
    case KLocale::EasternArabicIndicDigits:
        return QString::fromUtf8("۰۱۲۳۴۵۶۷۸۹");
    case KLocale::GujaratiDigits:
        return QString::fromUtf8("૦૧૨૩૪૫૬૭૮૯");
    case KLocale::GurmukhiDigits:
        return QString::fromUtf8("੦੧੨੩੪੫੬੭੮੯");
    case KLocale::KannadaDigits:
        return QString::fromUtf8("೦೧೨೩೪೫೬೭೮೯");
    case KLocale::KhmerDigits:
        return QString::fromUtf8("០១២៣៤៥៦៧៨៩");
    case KLocale::MalayalamDigits:
        return QString::fromUtf8("൦൧൨൩൪൫൬൭൮൯");
    case KLocale::OriyaDigits:
        return QString::fromUtf8("୦୧୨୩୪୫୬୭୮୯");
    case KLocale::TamilDigits:
        return QString::fromUtf8("௦௧௨௩௪௫௬௭௮");
    case KLocale::TeluguDigits:
        return QString::fromUtf8("౦౧౨౩౪౫౬౭౯");
    case KLocale::ThaiDigits:
        return QString::fromUtf8("๐๑๒๓๔๕๖๗๘๙");
    default:
        return QString::fromUtf8("0123456789");
    }
}

QString KLocale::digitSetToName(DigitSet digitSet, bool withDigits) const
{
    QString name;
    switch (digitSet) {
    case KLocale::ArabicIndicDigits:
        name = i18nc("digit set", "Arabic-Indic");
        break;
    case KLocale::BengaliDigits:
        name = i18nc("digit set", "Bengali");
        break;
    case KLocale::DevenagariDigits:
        name = i18nc("digit set", "Devanagari");
        break;
    case KLocale::EasternArabicIndicDigits:
        name = i18nc("digit set", "Eastern Arabic-Indic");
        break;
    case KLocale::GujaratiDigits:
        name = i18nc("digit set", "Gujarati");
        break;
    case KLocale::GurmukhiDigits:
        name = i18nc("digit set", "Gurmukhi");
        break;
    case KLocale::KannadaDigits:
        name = i18nc("digit set", "Kannada");
        break;
    case KLocale::KhmerDigits:
        name = i18nc("digit set", "Khmer");
        break;
    case KLocale::MalayalamDigits:
        name = i18nc("digit set", "Malayalam");
        break;
    case KLocale::OriyaDigits:
        name = i18nc("digit set", "Oriya");
        break;
    case KLocale::TamilDigits:
        name = i18nc("digit set", "Tamil");
        break;
    case KLocale::TeluguDigits:
        name = i18nc("digit set", "Telugu");
        break;
    case KLocale::ThaiDigits:
        name = i18nc("digit set", "Thai");
        break;
    default:
        name = i18nc("digit set", "Arabic");
    }
    if (withDigits) {
        QString digits = digitSetString(digitSet);
        QString nameWithDigits = i18nc("name of digit set with digit string, "
                                       "e.g. 'Arabic (0123456789)'", "%1 (%2)", name, digits);
        return nameWithDigits;
    } else {
        return name;
    }
}

QString KLocale::convertDigits(const QString &str, DigitSet digitSet,
                               bool ignoreContext) const
{
    if (!ignoreContext) {
        // Fall back to Western Arabic digits if requested digit set
        // is not appropriate for current application language.
        // FIXME: Temporary until full language-sensitivity implemented.
        KLocaleStaticData *s = staticData;
        if (d->languageSensitiveDigits &&
            !s->languagesUsingDigitSet[digitSet].contains(d->language)) {
            digitSet = KLocale::ArabicDigits;
        }
    }

    QString nstr;
    QString digitDraw = digitSetString(digitSet);
    foreach(const QChar &c, str) {
        if (c.isDigit()) {
            nstr += digitDraw[c.digitValue()];
        } else {
            nstr += c;
        }
    }
    return nstr;
}

static QString toArabicDigits(const QString &str)
{
    QString nstr;
    foreach(const QChar &c, str) {
        if (c.isDigit()) {
            nstr += QChar('0' + c.digitValue());
        } else {
            nstr += c;
        }
    }
    return nstr;
}

bool KLocale::nounDeclension() const
{
    return d->nounDeclension;
}

bool KLocale::dateMonthNamePossessive() const
{
    return d->dateMonthNamePossessive;
}

int KLocale::weekStartDay() const
{
    return d->weekStartDay;
}

int KLocale::workingWeekStartDay() const
{
    return d->workingWeekStartDay;
}

int KLocale::workingWeekEndDay() const
{
    return d->workingWeekEndDay;
}

int KLocale::weekDayOfPray() const
{
    return d->weekDayOfPray;
}

int KLocale::decimalPlaces() const
{
    return d->decimalPlaces;
}

QString KLocale::decimalSymbol() const
{
    return d->decimalSymbol;
}

QString KLocale::thousandsSeparator() const
{
    return d->thousandsSeparator;
}

QString KLocale::currencySymbol() const
{
    return d->currencySymbol;
}

QString KLocale::monetaryDecimalSymbol() const
{
    return d->monetaryDecimalSymbol;
}

QString KLocale::monetaryThousandsSeparator() const
{
    return d->monetaryThousandsSeparator;
}

QString KLocale::positiveSign() const
{
    return d->positiveSign;
}

QString KLocale::negativeSign() const
{
    return d->negativeSign;
}

int KLocale::fracDigits() const
{
    return d->monetaryDecimalPlaces;
}

int KLocale::monetaryDecimalPlaces() const
{
    return d->monetaryDecimalPlaces;
}

bool KLocale::positivePrefixCurrencySymbol() const
{
    return d->positivePrefixCurrencySymbol;
}

bool KLocale::negativePrefixCurrencySymbol() const
{
    return d->negativePrefixCurrencySymbol;
}

KLocale::SignPosition KLocale::positiveMonetarySignPosition() const
{
    return d->positiveMonetarySignPosition;
}

KLocale::SignPosition KLocale::negativeMonetarySignPosition() const
{
    return d->negativeMonetarySignPosition;
}

static inline void put_it_in(QChar *buffer, int& index, const QString &s)
{
    for (int l = 0; l < s.length(); l++)
        buffer[index++] = s.at(l);
}

static inline void put_it_in(QChar *buffer, int& index, int number)
{
    buffer[index++] = number / 10 + '0';
    buffer[index++] = number % 10 + '0';
}

// insert (thousands)-"separator"s into the non-fractional part of str
static void _insertSeparator(QString &str, const QString &separator, const QString &decimalSymbol)
{
    // leave fractional part untouched
    const int decimalSymbolPos = str.indexOf(decimalSymbol);
    const int start = decimalSymbolPos == -1 ? str.length() : decimalSymbolPos;
    for (int pos = start - 3; pos > 0; pos -= 3) {
        str.insert(pos, separator);
    }
}

QString KLocale::formatMoney(double num, const QString & symbol, int precision) const
{
    // some defaults
    QString currencyString = symbol;
    if (symbol.isNull()) {
        currencyString = currencySymbol();
    }
    if (precision < 0) {
        precision = monetaryDecimalPlaces();
    }

    // the number itself
    bool neg = num < 0;
    QString res = QString::number(neg ? -num : num, 'f', precision);

    // Replace dot with locale decimal separator
    res.replace(QChar('.'), monetaryDecimalSymbol());

    // Insert the thousand separators
    _insertSeparator(res, monetaryThousandsSeparator(), monetaryDecimalSymbol());

    // set some variables we need later
    int signpos = neg
                  ? negativeMonetarySignPosition()
                  : positiveMonetarySignPosition();
    QString sign = neg
                   ? negativeSign()
                   : positiveSign();

    switch (signpos) {
    case ParensAround:
        res.prepend(QLatin1Char('('));
        res.append(QLatin1Char(')'));
        break;
    case BeforeQuantityMoney:
        res.prepend(sign);
        break;
    case AfterQuantityMoney:
        res.append(sign);
        break;
    case BeforeMoney:
        currencyString.prepend(sign);
        break;
    case AfterMoney:
        currencyString.append(sign);
        break;
    }

    if (neg ? negativePrefixCurrencySymbol() :
            positivePrefixCurrencySymbol()) {
        res.prepend(QLatin1Char(' '));
        res.prepend(currencyString);
    } else {
        res.append(QLatin1Char(' '));
        res.append(currencyString);
    }

    // Convert to target digit set.
    res = convertDigits(res, d->monetaryDigitSet);

    return res;
}


QString KLocale::formatNumber(double num, int precision) const
{
    if (precision < 0) {
        precision = decimalPlaces();
    }
    // no need to round since QString::number does this for us
    return formatNumber(QString::number(num, 'f', precision), false, 0);
}

QString KLocale::formatLong(long num) const
{
    return formatNumber((double)num, 0);
}


// increase the digit at 'position' by one
static void _inc_by_one(QString &str, int position)
{
    for (int i = position; i >= 0; i--) {
        char last_char = str[i].toLatin1();
        switch (last_char) {
        case '0':
            str[i] = '1';
            break;
        case '1':
            str[i] = '2';
            break;
        case '2':
            str[i] = '3';
            break;
        case '3':
            str[i] = '4';
            break;
        case '4':
            str[i] = '5';
            break;
        case '5':
            str[i] = '6';
            break;
        case '6':
            str[i] = '7';
            break;
        case '7':
            str[i] = '8';
            break;
        case '8':
            str[i] = '9';
            break;
        case '9':
            str[i] = '0';
            if (i == 0) str.prepend('1');
            continue;
        case '.':
            continue;
        }
        break;
    }
}

// Cut off if more digits in fractional part than 'precision'
static void _round(QString &str, int precision)
{
    int decimalSymbolPos = str.indexOf('.');

    if (decimalSymbolPos == -1) {
        if (precision == 0)  return;
        else if (precision > 0) { // add dot if missing (and needed)
            str.append('.');
            decimalSymbolPos = str.length() - 1;
        }
    }
    // fill up with more than enough zeroes (in case fractional part too short)
    str.reserve(str.length() + precision);
    for (int i = 0; i < precision; ++i)
        str.append('0');

    // Now decide whether to round up or down
    char last_char = str[decimalSymbolPos + precision + 1].toLatin1();
    switch (last_char) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
        // nothing to do, rounding down
        break;
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        _inc_by_one(str, decimalSymbolPos + precision);
        break;
    default:
        break;
    }

    decimalSymbolPos = str.indexOf('.');
    str.truncate(decimalSymbolPos + precision + 1);

    // if precision == 0 delete also '.'
    if (precision == 0) {
        str = str.left(decimalSymbolPos);
    }

    str.squeeze();
}

QString KLocale::formatNumber(const QString &numStr, bool round, int precision) const
{
    QString tmpString = numStr;

    if (precision < 0) {
        precision = decimalPlaces();
    }

    // Skip the sign (for now)
    const bool neg = (tmpString[0] == '-');
    if (neg || tmpString[0] == '+') {
        tmpString.remove(0, 1);
    }

    //kDebug(173)<<"tmpString:"<<tmpString;

    // Split off exponential part (including 'e'-symbol)
    const int expPos = tmpString.indexOf('e'); // -1 if not found
    QString mantString = tmpString.left(expPos); // entire string if no 'e' found
    QString expString;
    if (expPos > -1) {
        expString = tmpString.mid(expPos); // includes the 'e', or empty if no 'e'
        if (expString.length() == 1) {
            expString.clear();
        }
    }

    //kDebug(173)<<"mantString:"<<mantString;
    //kDebug(173)<<"expString:"<<expString;
    if (mantString.isEmpty() || !mantString[0].isDigit()) {// invalid number
        mantString = '0';
    }

    if (round) {
        _round(mantString, precision);
    }

    // Replace dot with locale decimal separator
    mantString.replace(QChar('.'), decimalSymbol());

    // Insert the thousand separators
    _insertSeparator(mantString, thousandsSeparator(), decimalSymbol());

    // How can we know where we should put the sign?
    mantString.prepend(neg ? negativeSign() : positiveSign());

    // Convert to target digit set.
    if (d->digitSet != KLocale::ArabicDigits) {
        mantString = convertDigits(mantString, d->digitSet);
        expString = convertDigits(expString, d->digitSet);
    }

    return mantString + expString;
}

// Returns a list of already translated units to use later in formatByteSize
// and friends.  Account for every unit in KLocale::BinarySizeUnits
QList<QString> KLocalePrivate::dialectUnitsList(KLocale::BinaryUnitDialect dialect)
{
    QList<QString> binaryUnits;
    QString s; // Used in CACHE_BYTE_FMT macro defined shortly

    // Adds a given translation to the binaryUnits list.
#define CACHE_BYTE_FMT(ctxt_text) \
        translate_priv(0, ctxt_text, 0, 0, 0, &s); \
        binaryUnits.append(s);

    // Do not remove i18n: comments below, they are used by the
    // translators.

    // This prefix is shared by all current dialects.
    // i18n: Dumb message, avoid any markup or scripting.
    CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("size in bytes", "%1 B"));

    switch (dialect) {
    case KLocale::MetricBinaryDialect:
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("size in 1000 bytes", "%1 kB"));
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("size in 10^6 bytes", "%1 MB"));
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("size in 10^9 bytes", "%1 GB"));
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("size in 10^12 bytes", "%1 TB"));
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("size in 10^15 bytes", "%1 PB"));
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("size in 10^18 bytes", "%1 EB"));
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("size in 10^21 bytes", "%1 ZB"));
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("size in 10^24 bytes", "%1 YB"));
        break;

    case KLocale::JEDECBinaryDialect:
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("memory size in 1024 bytes", "%1 KB"));
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("memory size in 2^20 bytes", "%1 MB"));
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("memory size in 2^30 bytes", "%1 GB"));
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("memory size in 2^40 bytes", "%1 TB"));
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("memory size in 2^50 bytes", "%1 PB"));
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("memory size in 2^60 bytes", "%1 EB"));
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("memory size in 2^70 bytes", "%1 ZB"));
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("memory size in 2^80 bytes", "%1 YB"));
        break;

    case KLocale::IECBinaryDialect:
    default:
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("size in 1024 bytes", "%1 KiB"));
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("size in 2^20 bytes", "%1 MiB"));
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("size in 2^30 bytes", "%1 GiB"));
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("size in 2^40 bytes", "%1 TiB"));
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("size in 2^50 bytes", "%1 PiB"));
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("size in 2^60 bytes", "%1 EiB"));
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("size in 2^70 bytes", "%1 ZiB"));
        // i18n: Dumb message, avoid any markup or scripting.
        CACHE_BYTE_FMT(I18N_NOOP2_NOSTRIP("size in 2^80 bytes", "%1 YiB"));
        break;
    }

    return binaryUnits;
}

QString KLocale::formatByteSize(double size, int precision, BinaryUnitDialect dialect,
                                BinarySizeUnits specificUnit) const
{
    // Error checking
    if (dialect <= DefaultBinaryDialect || dialect > LastBinaryDialect) {
        dialect = d->binaryUnitDialect;
    }

    if (specificUnit < DefaultBinaryUnits || specificUnit > UnitLastUnit) {
        specificUnit = DefaultBinaryUnits;
    }

    // Choose appropriate units.
    QList<QString> dialectUnits;
    if (dialect == d->binaryUnitDialect) {
        // Cache default units for speed
        if (d->byteSizeFmt.size() == 0) {
            QMutexLocker lock(kLocaleMutex());

            // We only cache the user's default dialect.
            d->byteSizeFmt = d->dialectUnitsList(d->binaryUnitDialect);
        }

        dialectUnits = d->byteSizeFmt;
    } else {
        dialectUnits = d->dialectUnitsList(dialect);
    }

    int unit = 0; // Selects what unit to use from cached list
    double multiplier = 1024.0;

    if (dialect == MetricBinaryDialect) {
        multiplier = 1000.0;
    }

    // If a specific unit conversion is given, use it directly.  Otherwise
    // search until the result is in [0, multiplier) (or out of our range).
    if (specificUnit == DefaultBinaryUnits) {
        while (size >= multiplier && unit < (int) UnitYottaByte) {
            size /= multiplier;
            unit++;
        }
    } else {
        // A specific unit is in use
        unit = static_cast<int>(specificUnit);
        if (unit > 0) {
            size /= pow(multiplier, unit);
        }
    }

    if (unit == 0) {
        // Bytes, no rounding
        return dialectUnits[unit].arg(formatNumber(size, 0));
    }

    return dialectUnits[unit].arg(formatNumber(size, precision));
}

QString KLocale::formatByteSize(double size) const
{
    return formatByteSize(size, 1);
}

KLocale::BinaryUnitDialect KLocale::binaryUnitDialect() const
{
    return d->binaryUnitDialect;
}

void KLocale::setBinaryUnitDialect(BinaryUnitDialect newDialect)
{
    QMutexLocker lock(kLocaleMutex());

    d->binaryUnitDialect = newDialect;
    d->byteSizeFmt.clear(); // Reset cached translations.
}

QString KLocale::formatDuration(unsigned long mSec) const
{
    if (mSec >= 24*3600000) {
        return i18nc("@item:intext %1 is a real number, e.g. 1.23 days", "%1 days",
                     formatNumber(mSec / (24 * 3600000.0), 2));
    } else if (mSec >= 3600000) {
        return i18nc("@item:intext %1 is a real number, e.g. 1.23 hours", "%1 hours",
                     formatNumber(mSec / 3600000.0, 2));
    } else if (mSec >= 60000) {
        return i18nc("@item:intext %1 is a real number, e.g. 1.23 minutes", "%1 minutes",
                     formatNumber(mSec / 60000.0, 2));
    } else if (mSec >= 1000) {
        return i18nc("@item:intext %1 is a real number, e.g. 1.23 seconds", "%1 seconds",
                     formatNumber(mSec / 1000.0, 2));
    }
    return i18ncp("@item:intext", "%1 millisecond", "%1 milliseconds", mSec);
}

QString KLocalePrivate::formatSingleDuration(DurationType durationType, int n)
{
    switch (durationType) {
    case DaysDurationType:
        return i18ncp("@item:intext", "1 day", "%1 days", n);
    case HoursDurationType:
        return i18ncp("@item:intext", "1 hour", "%1 hours", n);
    case MinutesDurationType:
        return i18ncp("@item:intext", "1 minute", "%1 minutes", n);
    case SecondsDurationType:
        return i18ncp("@item:intext", "1 second", "%1 seconds", n);
    }
    return QString();
}

QString KLocale::prettyFormatDuration(unsigned long mSec) const
{
    unsigned long ms = mSec;
    int days = ms / (24 * 3600000);
    ms = ms % (24 * 3600000);
    int hours = ms / 3600000;
    ms = ms % 3600000;
    int minutes = ms / 60000;
    ms = ms % 60000;
    int seconds = qRound(ms / 1000.0);

    // Handle correctly problematic case #1 (look at KLocaleTest::prettyFormatDuration()
    // at klocaletest.cpp)
    if (seconds == 60) {
        return prettyFormatDuration(mSec - ms + 60000);
    }

    if (days && hours) {
        return i18nc("@item:intext days and hours. This uses the previous item:intext messages. If this does not fit the grammar of your language please contact the i18n team to solve the problem",
                     "%1 and %2", d->formatSingleDuration(KLocalePrivate::DaysDurationType, days),
                     d->formatSingleDuration(KLocalePrivate::HoursDurationType, hours));
    } else if (days) {
        return d->formatSingleDuration(KLocalePrivate::DaysDurationType, days);
    } else if (hours && minutes) {
        return i18nc("@item:intext hours and minutes. This uses the previous item:intext messages. If this does not fit the grammar of your language please contact the i18n team to solve the problem",
                     "%1 and %2",
                     d->formatSingleDuration(KLocalePrivate::HoursDurationType, hours),
                     d->formatSingleDuration(KLocalePrivate::MinutesDurationType, minutes));
    } else if (hours) {
        return d->formatSingleDuration(KLocalePrivate::HoursDurationType, hours);
    } else if (minutes && seconds) {
        return i18nc("@item:intext minutes and seconds. This uses the previous item:intext messages. If this does not fit the grammar of your language please contact the i18n team to solve the problem",
                     "%1 and %2",
                     d->formatSingleDuration(KLocalePrivate::MinutesDurationType, minutes),
                     d->formatSingleDuration(KLocalePrivate::SecondsDurationType, seconds));
    } else if (minutes) {
        return d->formatSingleDuration(KLocalePrivate::MinutesDurationType, minutes);
    } else {
        return d->formatSingleDuration(KLocalePrivate::SecondsDurationType, seconds);
    }
}

QString KLocale::formatDate(const QDate &pDate, DateFormat format) const
{
    return calendar()->formatDate(pDate, format);
}

void KLocale::setMainCatalog(const char *catalog)
{
    KLocaleStaticData *s = staticData;
    s->maincatalog = QString::fromUtf8(catalog);
}

double KLocale::readNumber(const QString &_str, bool * ok) const
{
    QString str = _str.trimmed();
    bool neg = str.indexOf(negativeSign()) == 0;
    if (neg) {
        str.remove(0, negativeSign().length());
    }

    /* will hold the scientific notation portion of the number.
       Example, with 2.34E+23, exponentialPart == "E+23"
    */
    QString exponentialPart;
    int EPos;

    EPos = str.indexOf('E', 0, Qt::CaseInsensitive);

    if (EPos != -1) {
        exponentialPart = str.mid(EPos);
        str = str.left(EPos);
    }

    int pos = str.indexOf(decimalSymbol());
    QString major;
    QString minor;
    if (pos == -1) {
        major = str;
    } else {
        major = str.left(pos);
        minor = str.mid(pos + decimalSymbol().length());
    }

    // Remove thousand separators
    int thlen = thousandsSeparator().length();
    int lastpos = 0;
    while ((pos = major.indexOf(thousandsSeparator())) > 0) {
        // e.g. 12,,345,,678,,922 Acceptable positions (from the end) are 5, 10, 15...
        // i.e. (3+thlen)*N
        int fromEnd = major.length() - pos;
        if (fromEnd % (3 + thlen) != 0 || // Needs to be a multiple, otherwise it's an error
            pos - lastpos > 3 ||          // More than 3 digits between two separators -> error
            pos == 0 ||                   // Can't start with a separator
            (lastpos > 0 && pos - lastpos != 3)) { // Must have exactly 3 digits between 2 separators
            if (ok) {
                *ok = false;
            }
            return 0.0;
        }

        lastpos = pos;
        major.remove(pos, thlen);
    }
    // Must have exactly 3 digits after the last separator
    if (lastpos > 0 && major.length() - lastpos != 3) {
        if (ok) {
            *ok = false;
        }
        return 0.0;
    }

    QString tot;
    if (neg) {
        tot = '-';
    }

    tot += major + '.' + minor + exponentialPart;

    tot = toArabicDigits(tot);

    return tot.toDouble(ok);
}

double KLocale::readMoney(const QString &_str, bool * ok) const
{
    QString str = _str.trimmed();
    bool neg = false;
    bool currencyFound = false;
    QString symbol = currencySymbol();

    // First try removing currency symbol from either end
    int pos = str.indexOf(symbol);
    if (pos == 0 || pos == (int) str.length() - symbol.length()) {
        str.remove(pos, symbol.length());
        str = str.trimmed();
        currencyFound = true;
    }
    if (str.isEmpty()) {
        if (ok) {
            *ok = false;
        }
        return 0;
    }
    // Then try removing negative sign from either end
    // (with a special case for parenthesis)
    if (negativeMonetarySignPosition() == ParensAround) {
        if (str[0] == '(' && str[str.length()-1] == ')') {
            neg = true;
            str.remove(str.length() - 1, 1);
            str.remove(0, 1);
        }
    } else {
        int i1 = str.indexOf(negativeSign());
        if (i1 == 0 || i1 == (int) str.length() - 1) {
            neg = true;
            str.remove(i1, negativeSign().length());
        }
    }
    if (neg) str = str.trimmed();

    // Finally try again for the currency symbol, if we didn't find
    // it already (because of the negative sign being in the way).
    if (!currencyFound) {
        pos = str.indexOf(symbol);
        if (pos == 0 || pos == (int) str.length() - symbol.length()) {
            str.remove(pos, symbol.length());
            str = str.trimmed();
        }
    }

    // And parse the rest as a number
    pos = str.indexOf(monetaryDecimalSymbol());
    QString major;
    QString minior;
    if (pos == -1) {
        major = str;
    } else {
        major = str.left(pos);
        minior = str.mid(pos + monetaryDecimalSymbol().length());
    }

    // Remove thousand separators
    int thlen = monetaryThousandsSeparator().length();
    int lastpos = 0;
    while ((pos = major.indexOf(monetaryThousandsSeparator())) > 0) {
        // e.g. 12,,345,,678,,922 Acceptable positions (from the end) are 5, 10, 15...
        // i.e. (3+thlen)*N
        int fromEnd = major.length() - pos;
        if (fromEnd % (3 + thlen) != 0 || // Needs to be a multiple, otherwise it's an error
            pos - lastpos > 3 ||          // More than 3 digits between two separators -> error
            pos == 0 ||                   // Can't start with a separator
            (lastpos > 0 && pos - lastpos != 3)) { // Must have exactly 3 digits between two separators
            if (ok) {
                *ok = false;
            }
            return 0.0;
        }
        lastpos = pos;
        major.remove(pos, thlen);
    }
    // Must have exactly 3 digits after the last separator
    if (lastpos > 0 && major.length() - lastpos != 3) {
        if (ok) {
            *ok = false;
        }
        return 0.0;
    }

    QString tot;
    if (neg) {
        tot = '-';
    }
    tot += major + '.' + minior;
    tot = toArabicDigits(tot);
    return tot.toDouble(ok);
}

/**
 * helper function to read integers
 * @param str
 * @param pos the position to start at. It will be updated when we parse it.
 * @return the integer read in the string, or -1 if no string
 */
static int readInt(const QString &str, int &pos)
{
    if (!str.at(pos).isDigit()) {
        return -1;
    }
    int result = 0;
    for (; str.length() > pos && str.at(pos).isDigit(); ++pos) {
        result *= 10;
        result += str.at(pos).digitValue();
    }

    return result;
}

QDate KLocale::readDate(const QString &intstr, bool* ok) const
{
    return calendar()->readDate(intstr, ok);
}

QDate KLocale::readDate(const QString &intstr, ReadDateFlags flags, bool* ok) const
{
    return calendar()->readDate(intstr, flags, ok);
}

QDate KLocale::readDate(const QString &intstr, const QString &fmt, bool* ok) const
{
    return calendar()->readDate(intstr, fmt, ok);
}

QTime KLocale::readTime(const QString &intstr, bool *ok) const
{
    QTime _time;
    _time = readLocaleTime(intstr, ok, TimeDefault, ProcessStrict);
    if (_time.isValid()) return _time;
    return readLocaleTime(intstr, ok, TimeWithoutSeconds, ProcessStrict);
}

QTime KLocale::readTime(const QString &intstr, ReadTimeFlags flags, bool *ok) const
{
    return readLocaleTime(intstr, ok,
                          (flags == WithSeconds) ? TimeDefault : TimeWithoutSeconds, ProcessStrict);
}

// remove the first occurrence of the 2-character string
// strip2char from inout and if found, also remove one preceding
// punctuation character and arbitrary number of spaces.
static void stripStringAndPreceedingSeparator(QString &inout, const QLatin1String &strip2char)
{
    int remPos = inout.indexOf(strip2char);
    if (remPos == -1) {
        return;
    }
    int endPos = remPos + 2;
    int curPos = remPos - 1;
    while (curPos >= 0 && inout.at(curPos).isSpace()) {
        curPos--;
    }
    // remove the separator sign before the seconds
    // and assume that works everywhere
    if (curPos >= 0 && inout.at(curPos).isPunct() && inout.at(curPos) != '%') {
        curPos--;
    }
    while (curPos >= 0 && inout.at(curPos).isSpace()) {
        curPos--;
    }

    remPos = qMax(curPos + 1, 0);
    inout.remove(remPos, endPos - remPos);
}

// remove the first occurrence of "%p" from the inout.
static void stripAmPmFormat(QString &inout)
{
    // NOTE: this function assumes that %p - if it's present -
    //       is either the first or the last element of the format
    //       string. Either a succeeding or a preceding
    //       punctuation symbol is stripped.
    int length = inout.size();
    int ppos = inout.indexOf(QLatin1String("%p"));
    if (ppos == -1) {
        return;
    } else if (ppos == 0) {
        // first element, eat succeeding punctuation and spaces
        ppos = 2;
        while (ppos < length && (inout.at(ppos).isSpace() || inout.at(ppos).isPunct()) &&
                inout.at(ppos) != '%') {
            ppos++;
        }
        inout = inout.mid(ppos);
    } else {
        stripStringAndPreceedingSeparator(inout, QLatin1String("%p"));
    }
}

QTime KLocale::readLocaleTime(const QString &intstr, bool *ok, TimeFormatOptions options,
                              TimeProcessingOptions processing) const
{
    QString str(intstr.simplified().toLower());
    QString format(timeFormat().simplified());

    int hour = -1;
    int minute = -1;
    int second = -1;
    bool g_12h = false;
    bool pm = false;
    int strpos = 0;
    int formatpos = 0;
    bool error = false;

    bool excludeSecs = ((options & TimeWithoutSeconds) == TimeWithoutSeconds);
    bool isDuration = ((options & TimeDuration) == TimeDuration);
    bool noAmPm = ((options & TimeWithoutAmPm) == TimeWithoutAmPm);
    bool strict = ((processing & ProcessStrict) == ProcessStrict);

    // if seconds aren't needed, strip them from the timeFormat
    if (excludeSecs) {
        stripStringAndPreceedingSeparator(format, QLatin1String("%S"));
        second = 0; // seconds are always 0
    }

    // if am/pm isn't needed, strip it from the timeFormat
    if (noAmPm) {
        stripAmPmFormat(format);
    }

    while (!error && (format.length() > formatpos || str.length() > strpos)) {
        if (!(format.length() > formatpos && str.length() > strpos)) {
            error = true;
            break;
        }

        QChar c = format.at(formatpos++);
        if (c.isSpace()) {
            if (strict) { // strict processing: space is needed
                if (!str.at(strpos).isSpace()) {
                    error = true;
                    break;
                }
                strpos++;
            } else { // lax processing: space in str not needed
                // 1 space maximum as str is simplified
                if (str.at(strpos).isSpace()) {
                    strpos++;
                }
            }
            continue;
        }

        if (c != '%') {
            if (c != str.at(strpos++)) {
                error = true;
                break;
            }
            continue;
        }

        c = format.at(formatpos++);
        switch (c.unicode()) {

        case 'p': {
            QString s(i18n("pm").toLower());
            int len = s.length();
            if (str.mid(strpos, len) == s) {
                pm = true;
                strpos += len;
            } else {
                s = i18n("am").toLower();
                len = s.length();
                if (str.mid(strpos, len) == s) {
                    pm = false;
                    strpos += len;
                } else {
                    error = true;
                }
            }
        }
        break;

        case 'k':
        case 'H':
            g_12h = false;
            hour = readInt(str, strpos);
            if (hour < 0 || hour > 23) {
                error = true;
            }
            break;

        case 'l':
        case 'I':
            if (isDuration) {
                g_12h = false;
                hour = readInt(str, strpos);
                if (hour < 0 || hour > 23) {
                    error = true;
                }
            } else {
                g_12h = true;
                hour = readInt(str, strpos);
                if (hour < 1 || hour > 12) {
                    error = true;
                }
            }
            break;

        case 'M':
            minute = readInt(str, strpos);
            if (minute < 0 || minute > 59) {
                error = true;
            }
            break;

        case 'S':
            second = readInt(str, strpos);
            if (second < 0 || second > 59) {
                error = true;
            }
            break;
        }

        // NOTE: if anything is performed inside this loop, be sure to
        //       check for error!
    }

    if (!error) {
        if (g_12h) {
            hour %= 12;
            if (pm) {
                hour += 12;
            }
        }
        if (ok) {
            *ok = true;
        }
        return QTime(hour, minute, second);
    } else {
        if (ok) {
            *ok = false;
        }
        return QTime();
    }
}

QString KLocale::formatTime(const QTime &pTime, bool includeSecs, bool isDuration) const
{
    TimeFormatOptions options = TimeDefault;
    if (!includeSecs) {
        options |= TimeWithoutSeconds;
    }
    if (isDuration) {
        options |= TimeDuration;
    }
    return formatLocaleTime(pTime, options);
}

QString KLocale::formatLocaleTime(const QTime &pTime, TimeFormatOptions options) const
{
    QString rst(timeFormat());

    bool excludeSecs = ((options & TimeWithoutSeconds) == TimeWithoutSeconds);
    bool isDuration = ((options & TimeDuration) == TimeDuration);
    bool noAmPm = ((options & TimeWithoutAmPm) == TimeWithoutAmPm);

    // if seconds aren't needed, strip them from the timeFormat
    if (excludeSecs) {
        stripStringAndPreceedingSeparator(rst, QLatin1String("%S"));
    }

    // if am/pm isn't needed, strip it from the timeFormat
    if (noAmPm) {
        stripAmPmFormat(rst);
    }

    // only "pm/am" here can grow, the rest shrinks, but
    // I'm rather safe than sorry
    QChar *buffer = new QChar[rst.length() * 3 / 2 + 30];

    int index = 0;
    bool escape = false;
    int number = 0;

    for (int format_index = 0; format_index < rst.length(); format_index++) {
        if (!escape) {
            if (rst.at(format_index).unicode() == '%') {
                escape = true;
            } else {
                buffer[index++] = rst.at(format_index);
            }
        } else {
            switch (rst.at(format_index).unicode()) {
            case '%':
                buffer[index++] = '%';
                break;
            case 'H':
                put_it_in(buffer, index, pTime.hour());
                break;
            case 'I':
                if (isDuration) {
                    put_it_in(buffer, index, pTime.hour());
                } else {
                    put_it_in(buffer, index, (pTime.hour() + 11) % 12 + 1);
                }
                break;
            case 'M':
                put_it_in(buffer, index, pTime.minute());
                break;
            case 'S':
                put_it_in(buffer, index, pTime.second());
                break;
            case 'k':
                number = pTime.hour();
            case 'l':
                // to share the code
                if (rst.at(format_index).unicode() == 'l') {
                    number = isDuration ? pTime.hour() : (pTime.hour() + 11) % 12 + 1;
                }
                if (number / 10) {
                    buffer[index++] = number / 10 + '0';
                }
                buffer[index++] = number % 10 + '0';
                break;
            case 'p':
                if (pTime.hour() >= 12) {
                    put_it_in(buffer, index, i18n("pm"));
                } else {
                    put_it_in(buffer, index, i18n("am"));
                }
                break;
            default:
                buffer[index++] = rst.at(format_index);
                break;
            }
            escape = false;
        }
    }
    QString ret(buffer, index);
    delete [] buffer;
    ret = convertDigits(ret, d->dateTimeDigitSet);
    return ret.trimmed();
}

bool KLocale::use12Clock() const
{
    if ((timeFormat().contains(QString::fromLatin1("%I")) > 0) ||
        (timeFormat().contains(QString::fromLatin1("%l")) > 0)) {
        return true;
    } else {
        return false;
    }
}

QStringList KLocale::languageList() const
{
    return d->languageList;
}

QStringList KLocale::currencyCodeList() const
{
    return d->currencyCodeList;
}

QString KLocalePrivate::formatDateTime(const KLocale *locale, const QDateTime &dateTime,
                                       KLocale::DateFormat format, bool includeSeconds,
                                       int daysTo, int secsTo)
{
    // Have to do Fancy Date formatting here rather than using normal KCalendarSystem::formatDate()
    // as daysTo is relative to the time spec which formatDate doesn't know about.  Needs to be
    // kept in sync with Fancy Date code in KCalendarSystem::formatDate().  Fix in KDE5.

    // Only do Fancy if less than an hour into the future or less than a week in the past
    if ((daysTo == 0 && secsTo > 3600) ||  daysTo < 0 || daysTo > 6) {
        if (format == KLocale::FancyShortDate) {
            format = KLocale::ShortDate;
        } else if (format == KLocale::FancyLongDate) {
            format = KLocale::LongDate;
        }
    }

    QString dateStr;
    if (format == KLocale::FancyShortDate || format == KLocale::FancyLongDate) {
        switch (daysTo) {
        case 0:
            dateStr = i18n("Today");
            break;
        case 1:
            dateStr = i18n("Yesterday");
            break;
        default:
            dateStr = locale->calendar()->weekDayName(dateTime.date());
        }
    } else {
        dateStr = locale->formatDate(dateTime.date(), format);
    }

    KLocale::TimeFormatOption timeFormat;
    if (includeSeconds) {
        timeFormat = KLocale::TimeDefault;
    } else {
        timeFormat = KLocale::TimeWithoutSeconds;
    }

    return i18nc("concatenation of dates and time", "%1 %2", dateStr,
                 locale->formatLocaleTime(dateTime.time(), timeFormat));
}

QString KLocale::formatDateTime(const QDateTime &dateTime, DateFormat format,
                                bool includeSeconds) const
{
    QDateTime now = QDateTime::currentDateTime();
    int daysTo = dateTime.date().daysTo(now.date());
    int secsTo = now.secsTo(dateTime);
    return KLocalePrivate::formatDateTime(this, dateTime, format, includeSeconds, daysTo, secsTo);
}

QString KLocale::formatDateTime(const KDateTime &dateTime, DateFormat format,
                                DateTimeFormatOptions options) const
{
    QString dt;
    if (dateTime.isDateOnly())
        dt = formatDate(dateTime.date(), format);
    else {
        KDateTime now = KDateTime::currentDateTime(dateTime.timeSpec());
        int daysTo = dateTime.date().daysTo(now.date());
        int secsTo = now.secsTo(dateTime);
        dt = KLocalePrivate::formatDateTime(this, dateTime.dateTime(), format, (options & Seconds),
                                            daysTo, secsTo);
    }
    if (options & TimeZone) {
        QString tz;
        switch (dateTime.timeType()) {
        case KDateTime::OffsetFromUTC:
            tz = i18n(dateTime.toString("%z").toUtf8());
            break;
        case KDateTime::UTC:
        case KDateTime::TimeZone:
            tz = i18n(dateTime.toString((format == ShortDate) ? "%Z" : "%:Z").toUtf8());
            break;
        case KDateTime::ClockTime:
        default:
            break;
        }
        return i18nc("concatenation of date/time and time zone", "%1 %2", dt, tz);
    } else
        return dt;
}

QString KLocale::langLookup(const QString &fname, const char *rtype)
{
    QStringList search;

    // assemble the local search paths
    const QStringList localDoc = KGlobal::dirs()->resourceDirs(rtype);

    // look up the different languages
    for (int id = localDoc.count() - 1; id >= 0; --id) {
        QStringList langs = KGlobal::locale()->languageList();
        // FIXME: KDE 4.5, change such that English is not assumed.
        langs.replaceInStrings("en_US", "en");
        langs.append("en");
        Q_FOREACH(const QString &lang, langs)
        search.append(QString("%1%2/%3").arg(localDoc[id]).arg(lang).arg(fname));
    }

    // try to locate the file
    Q_FOREACH(const QString &file, search) {
        kDebug(173) << "Looking for help in: " << file;

        QFileInfo info(file);
        if (info.exists() && info.isFile() && info.isReadable())
            return file;
    }

    return QString();
}

bool KLocalePrivate::useDefaultLanguage() const
{
    return language == KLocale::defaultLanguage();
}

void KLocalePrivate::initEncoding()
{
    codecForEncoding = 0;

    // This all made more sense when we still had the EncodingEnum config key.
#if defined(HAVE_LANGINFO_H) && !defined(Q_OS_WIN)
    // Qt since 4.2 always returns 'System' as codecForLocale and KDE (for example
    // KEncodingFileDialog) expects real encoding name. So on systems that have langinfo.h use
    // nl_langinfo instead, just like Qt compiled without iconv does. Windows already has its own
    // workaround

    QByteArray systemLocale = nl_langinfo(CODESET);
#if defined(Q_OS_MAC)
    // Mac OX X is UTF-8, always.
    systemLocale = "UTF-8";
#endif //Q_OS_MAC

    if ((systemLocale == "ANSI_X3.4-1968") || (systemLocale == "US-ASCII")) {
        // means ascii, "C"; QTextCodec doesn't know, so avoid warning
        systemLocale = "ISO-8859-1";
    }

    QTextCodec* codec = QTextCodec::codecForName(systemLocale);
    if (codec) {
        setEncoding(codec->mibEnum());
    }
#else
    setEncoding(QTextCodec::codecForLocale()->mibEnum());
#endif

    if (!codecForEncoding) {
        kWarning() << "Cannot resolve system encoding, defaulting to ISO 8859-1.";
        const int mibDefault = 4; // ISO 8859-1
        setEncoding(mibDefault);
    }

    Q_ASSERT(codecForEncoding);
}

void KLocalePrivate::initFileNameEncoding()
{
    // If the following environment variable is set, assume all filenames
    // are in UTF-8 regardless of the current C locale.
    utf8FileEncoding = !qgetenv("KDE_UTF8_FILENAMES").isEmpty();
    if (utf8FileEncoding) {
        QFile::setEncodingFunction(KLocalePrivate::encodeFileNameUTF8);
        QFile::setDecodingFunction(KLocalePrivate::decodeFileNameUTF8);
    }
    // Otherwise, stay with QFile's default filename encoding functions
    // which, on Unix platforms, use the locale's codec.
}

QByteArray KLocalePrivate::encodeFileNameUTF8(const QString & fileName)
{
    return fileName.toUtf8();
}

QString KLocalePrivate::decodeFileNameUTF8(const QByteArray & localFileName)
{
    return QString::fromUtf8(localFileName);
}

void KLocale::setDateFormat(const QString & format)
{
    d->dateFormat = format.trimmed();
}

void KLocale::setDateFormatShort(const QString & format)
{
    d->dateFormatShort = format.trimmed();
}

void KLocale::setDateMonthNamePossessive(bool possessive)
{
    d->dateMonthNamePossessive = possessive;
}

void KLocale::setTimeFormat(const QString & format)
{
    d->timeFormat = format.trimmed();
}

void KLocale::setWeekStartDay(int day)
{
    if (day >= 1 && day <= calendar()->daysInWeek(QDate())) {
        d->weekStartDay = day;
    }
}

void KLocale::setWorkingWeekStartDay(int day)
{
    if (day >= 1 && day <= calendar()->daysInWeek(QDate())) {
        d->workingWeekStartDay = day;
    }
}

void KLocale::setWorkingWeekEndDay(int day)
{
    if (day >= 1 && day <= calendar()->daysInWeek(QDate())) {
        d->workingWeekEndDay = day;
    }
}

void KLocale::setWeekDayOfPray(int day)
{
    if (day >= 0 && day <= calendar()->daysInWeek(QDate())) { // 0 = None
        d->weekDayOfPray = day;
    }
}

QString KLocale::dateFormat() const
{
    return d->dateFormat;
}

QString KLocale::dateFormatShort() const
{
    return d->dateFormatShort;
}

QString KLocale::timeFormat() const
{
    return d->timeFormat;
}

void KLocale::setDecimalPlaces(int digits)
{
    d->decimalPlaces = digits;
}

void KLocale::setDecimalSymbol(const QString & symbol)
{
    d->decimalSymbol = symbol.trimmed();
}

void KLocale::setThousandsSeparator(const QString & separator)
{
    // allow spaces here
    d->thousandsSeparator = separator;
}

void KLocale::setPositiveSign(const QString & sign)
{
    d->positiveSign = sign.trimmed();
}

void KLocale::setNegativeSign(const QString & sign)
{
    d->negativeSign = sign.trimmed();
}

void KLocale::setPositiveMonetarySignPosition(SignPosition signpos)
{
    d->positiveMonetarySignPosition = signpos;
}

void KLocale::setNegativeMonetarySignPosition(SignPosition signpos)
{
    d->negativeMonetarySignPosition = signpos;
}

void KLocale::setPositivePrefixCurrencySymbol(bool prefix)
{
    d->positivePrefixCurrencySymbol = prefix;
}

void KLocale::setNegativePrefixCurrencySymbol(bool prefix)
{
    d->negativePrefixCurrencySymbol = prefix;
}

void KLocale::setFracDigits(int digits)
{
    setMonetaryDecimalPlaces(digits);
}

void KLocale::setMonetaryDecimalPlaces(int digits)
{
    d->monetaryDecimalPlaces = digits;
}

void KLocale::setMonetaryThousandsSeparator(const QString & separator)
{
    // allow spaces here
    d->monetaryThousandsSeparator = separator;
}

void KLocale::setMonetaryDecimalSymbol(const QString & symbol)
{
    d->monetaryDecimalSymbol = symbol.trimmed();
}

void KLocale::setCurrencySymbol(const QString & symbol)
{
    d->currencySymbol = symbol.trimmed();
}

int KLocale::pageSize() const
{
    return d->pageSize;
}

void KLocale::setPageSize(int size)
{
    // #### check if it's in range??
    d->pageSize = size;
}

KLocale::MeasureSystem KLocale::measureSystem() const
{
    return d->measureSystem;
}

void KLocale::setMeasureSystem(MeasureSystem value)
{
    d->measureSystem = value;
}

QString KLocale::defaultLanguage()
{
    static const QString en_US = QString::fromLatin1("en_US");
    return en_US;
}

QString KLocale::defaultCountry()
{
    return QString::fromLatin1("C");
}

QString KLocale::defaultCurrencyCode()
{
    return QString::fromLatin1("USD");
}

bool KLocale::useTranscript() const
{
    return d->useTranscript;
}

const QByteArray KLocale::encoding() const
{
#ifdef Q_WS_WIN
    if (0 == qstrcmp("System", codecForEncoding()->name())) {
        //win32 returns "System" codec name here but KDE apps expect a real name:
        LPWSTR buffer;
        strcpy(d->win32SystemEncoding, "cp ");
        if (GetLocaleInfoW(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT),
                           LOCALE_IDEFAULTANSICODEPAGE, buffer,
                           sizeof(buffer))) {
            QString localestr = QString::fromUtf16(buffer);
            QByteArray localechar = localestr.toAscii();
            strcpy(d->win32SystemEncoding, localechar.data() + 3);
            return d->win32SystemEncoding;
        }
    }
#endif
    return codecForEncoding()->name();
}

int KLocale::encodingMib() const
{
    return codecForEncoding()->mibEnum();
}

int KLocale::fileEncodingMib() const
{
    if (d->utf8FileEncoding) {
        return 106;
    }
    return codecForEncoding()->mibEnum();
}

QTextCodec * KLocale::codecForEncoding() const
{
    return d->codecForEncoding;
}

bool KLocale::setEncoding(int mibEnum)
{
    return d->setEncoding(mibEnum);
}

bool KLocalePrivate::setEncoding(int mibEnum)
{
    QTextCodec * codec = QTextCodec::codecForMib(mibEnum);
    if (codec) {
        codecForEncoding = codec;
    }

    return codec != 0;
}

QStringList KLocale::allLanguagesList() const
{
    if (!d->languages) {
        d->languages = new KConfig("all_languages", KConfig::NoGlobals, "locale");
    }

    return d->languages->groupList();
}

QString KLocale::languageCodeToName(const QString &language) const
{
    if (!d->languages) {
        d->languages = new KConfig("all_languages", KConfig::NoGlobals, "locale");
    }

    KConfigGroup cg(d->languages, language);
    return cg.readEntry("Name");
}

QStringList KLocale::allCountriesList() const
{
    QStringList countries;
    const QStringList paths = KGlobal::dirs()->findAllResources("locale", "l10n/*/entry.desktop");
    for (QStringList::ConstIterator it = paths.begin(); it != paths.end(); ++it) {
        QString code = (*it).mid((*it).length() - 16, 2);
        if (code != "/C") {
            countries.append(code);
        }
    }
    return countries;
}

QString KLocale::countryCodeToName(const QString &country) const
{
    QString countryName;
    QString entryFile = KStandardDirs::locate("locale",
                                              "l10n/" + country.toLower() + "/entry.desktop");
    if (!entryFile.isEmpty()) {
        KConfig cfg(entryFile);
        KConfigGroup cg(&cfg, "KCM Locale");
        countryName = cg.readEntry("Name");
    }
    return countryName;
}

void KLocale::setCalendar(const QString & calType)
{
    d->calendarType = calType;

    delete d->calendar;
    d->calendar = 0;
}

QString KLocale::calendarType() const
{
    return d->calendarType;
}

const KCalendarSystem * KLocale::calendar() const
{
    // Check if it's the correct calendar?!?
    if (!d->calendar) {
        d->calendar = KCalendarSystem::create(d->calendarType, this);
    }

    return d->calendar;
}

KLocale::KLocale(const KLocale & rhs) : d(new KLocalePrivate(*rhs.d))
{
    d->languages = 0; // Don't copy languages
    d->calendar = 0; // Don't copy the calendar
    d->currency = 0; // Don't copy the currency
}

KLocale & KLocale::operator=(const KLocale & rhs)
{
    // the assignment operator works here
    *d = *rhs.d;
    d->languages = 0; // Don't copy languages
    d->calendar = 0; // Don't copy the calendar
    d->currency = 0; // Don't copy the currency

    return *this;
}

void KLocale::copyCatalogsTo(KLocale *locale)
{
    QMutexLocker lock(kLocaleMutex());
    locale->d->catalogNames = d->catalogNames;
    locale->d->updateCatalogs();
}

QString KLocale::localizedFilePath(const QString &filePath) const
{
    // Stop here if the default language is primary.
    if (d->useDefaultLanguage()) {
        return filePath;
    }

    // Check if l10n sudir is present, stop if not.
    QFileInfo fileInfo(filePath);
    QString locDirPath = fileInfo.path() + "/l10n";
    QFileInfo locDirInfo(locDirPath);
    if (!locDirInfo.isDir()) {
        return filePath;
    }

    // Go through possible localized paths by priority of languages,
    // return first that exists.
    QString fileName = fileInfo.fileName();
    foreach(const QString &lang, d->languageList) {
        // Stop when the default language is reached.
        if (lang == KLocale::defaultLanguage()) {
            return filePath;
        }
        QString locFilePath = locDirPath + '/' + lang + '/' + fileName;
        QFileInfo locFileInfo(locFilePath);
        if (locFileInfo.isFile() && locFileInfo.isReadable()) {
            return locFilePath;
        }
    }

    return filePath;
}

QString KLocale::removeAcceleratorMarker(const QString &label) const
{
    return ::removeAcceleratorMarker(label);
}

void KLocale::setDigitSet(DigitSet digitSet)
{
    d->digitSet = digitSet;
}

KLocale::DigitSet KLocale::digitSet() const
{
    return d->digitSet;
}

void KLocale::setMonetaryDigitSet(DigitSet digitSet)
{
    d->monetaryDigitSet = digitSet;
}

KLocale::DigitSet KLocale::monetaryDigitSet() const
{
    return d->monetaryDigitSet;
}

void KLocale::setDateTimeDigitSet(DigitSet digitSet)
{
    d->dateTimeDigitSet = digitSet;
}

KLocale::DigitSet KLocale::dateTimeDigitSet() const
{
    return d->dateTimeDigitSet;
}

Q_GLOBAL_STATIC_WITH_ARGS(QMutex, s_kLocaleMutex, (QMutex::Recursive))

QMutex* kLocaleMutex()
{
    return s_kLocaleMutex();
}
