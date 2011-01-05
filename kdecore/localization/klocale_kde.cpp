/* This file is part of the KDE libraries
   Copyright (c) 1997,2001 Stephan Kulow <coolo@kde.org>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1999-2002 Hans Petter Bieker <bieker@kde.org>
   Copyright (c) 2002 Lukas Tinkl <lukas@kde.org>
   Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>
   Copyright (C) 2009, 2010 John Layt <john@layt.net>

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
#include "kdayperiod_p.h"

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
    languagesUsingDigitSet.insert(KLocale::ArabicIndicDigits, QStringList() << QString::fromLatin1("ar") << QString::fromLatin1("ps"));
    languagesUsingDigitSet.insert(KLocale::BengaliDigits, QStringList() << QString::fromLatin1("bn") << QString::fromLatin1("as") );
    languagesUsingDigitSet.insert(KLocale::DevenagariDigits, QStringList() << QString::fromLatin1("hi") << QString::fromLatin1("ne"));
    languagesUsingDigitSet.insert(KLocale::EasternArabicIndicDigits, QStringList() << QString::fromLatin1("fa") << QString::fromLatin1("ur"));
    languagesUsingDigitSet.insert(KLocale::GujaratiDigits, QStringList() << QString::fromLatin1("gu") );
    languagesUsingDigitSet.insert(KLocale::GurmukhiDigits, QStringList() << QString::fromLatin1("pa") );
    languagesUsingDigitSet.insert(KLocale::KannadaDigits, QStringList() << QString::fromLatin1("kn") );
    languagesUsingDigitSet.insert(KLocale::KhmerDigits, QStringList() << QString::fromLatin1("km") );
    languagesUsingDigitSet.insert(KLocale::MalayalamDigits, QStringList() << QString::fromLatin1("ml") );
    languagesUsingDigitSet.insert(KLocale::OriyaDigits, QStringList() << QString::fromLatin1("or") );
    languagesUsingDigitSet.insert(KLocale::TamilDigits, QStringList() << QString::fromLatin1("ta") );
    languagesUsingDigitSet.insert(KLocale::TeluguDigits, QStringList() << QString::fromLatin1("te") );
    languagesUsingDigitSet.insert(KLocale::ThaiDigits, QStringList() << QString::fromLatin1("th"));
}

K_GLOBAL_STATIC(KLocaleStaticData, staticData)


QDebug operator<<(QDebug debug, const KCatalogName &cn)
{
    return debug << cn.name << cn.loadCount;
}

KLocalePrivate::KLocalePrivate(KLocale *q_ptr)
               : q(q_ptr),
                 m_config(KSharedConfig::Ptr()),
                 m_country(QString()),
                 m_language(QString()),
                 m_languages(0),
                 m_catalogName(QString()),
                 m_calendar(0),
                 m_currency(0),
                 m_codecForEncoding(0)
{
}

KLocalePrivate::KLocalePrivate(const KLocalePrivate &rhs)
{
    copy(rhs);
}

KLocalePrivate &KLocalePrivate::operator=(const KLocalePrivate &rhs)
{
    copy(rhs);
    return *this;
}

KConfig *KLocalePrivate::config()
{
    if (m_config != KSharedConfig::Ptr()) {
        return m_config.data();
    } else {
        return KGlobal::config().data();
    }
}

void KLocalePrivate::copy(const KLocalePrivate &rhs)
{
    // Parent KLocale
    q = 0;

    // Config
    m_config = rhs.m_config;

    // Country settings
    m_country = rhs.m_country;
    m_countryDivisionCode = rhs.m_countryDivisionCode;

    // Language settings
    m_language = rhs.m_language;
    m_languages = 0;
    m_languageList = rhs.m_languageList;
    m_languageSensitiveDigits = rhs.m_languageSensitiveDigits;
    m_nounDeclension = rhs.m_nounDeclension;

    // Catalog settings
    m_catalogName = rhs.m_catalogName;
    m_catalogNames = rhs.m_catalogNames;
    m_catalogs = rhs.m_catalogs;
    m_numberOfSysCatalogs = rhs.m_numberOfSysCatalogs;
    m_useTranscript = rhs.m_useTranscript;

    // Calendar settings
    m_calendarSystem = rhs.m_calendarSystem;
    m_calendar = 0;
    m_weekStartDay = rhs.m_weekStartDay;
    m_workingWeekStartDay = rhs.m_workingWeekStartDay;
    m_workingWeekEndDay = rhs.m_workingWeekEndDay;
    m_weekDayOfPray = rhs.m_weekDayOfPray;

    // Date/Time settings
    m_dateFormat = rhs.m_dateFormat;
    m_dateFormatShort = rhs.m_dateFormatShort;
    m_timeFormat = rhs.m_timeFormat;
    m_dateTimeDigitSet = rhs.m_dateTimeDigitSet;
    m_dateMonthNamePossessive = rhs.m_dateMonthNamePossessive;
    m_dayPeriods = rhs.m_dayPeriods;

    // Number settings
    m_decimalPlaces = rhs.m_decimalPlaces;
    m_decimalSymbol = rhs.m_decimalSymbol;
    m_thousandsSeparator = rhs.m_thousandsSeparator;
    m_positiveSign = rhs.m_positiveSign;
    m_negativeSign = rhs.m_negativeSign;
    m_digitSet = rhs.m_digitSet;

    // Currency settings
    m_currencyCode = rhs.m_currencyCode;
    m_currency = 0;
    m_currencyCodeList = rhs.m_currencyCodeList;

    // Money settings
    m_currencySymbol = rhs.m_currencySymbol;
    m_monetaryDecimalSymbol = rhs.m_monetaryDecimalSymbol;
    m_monetaryThousandsSeparator = rhs.m_monetaryThousandsSeparator;
    m_monetaryDecimalPlaces = rhs.m_monetaryDecimalPlaces;
    m_positiveMonetarySignPosition = rhs.m_positiveMonetarySignPosition;
    m_negativeMonetarySignPosition = rhs.m_negativeMonetarySignPosition;
    m_positivePrefixCurrencySymbol = rhs.m_positivePrefixCurrencySymbol;
    m_negativePrefixCurrencySymbol = rhs.m_negativePrefixCurrencySymbol;
    m_monetaryDigitSet = rhs.m_monetaryDigitSet;

    // Units settings
    m_binaryUnitDialect = rhs.m_binaryUnitDialect;
    m_byteSizeFmt = rhs.m_byteSizeFmt;
    m_pageSize = rhs.m_pageSize;
    m_measureSystem = rhs.m_measureSystem;

    // Encoding settings
    m_encoding = rhs.m_encoding;
    m_codecForEncoding = rhs.m_codecForEncoding;
    m_utf8FileEncoding = rhs.m_utf8FileEncoding;
}

KLocalePrivate::~KLocalePrivate()
{
    delete m_currency;
    delete m_calendar;
    delete m_languages;
}

// init only called from platform specific constructor, so set everything up
// Will be given a persistantConfig or a tempConfig or neither, but never both
void KLocalePrivate::init(const QString& catalogName, const QString &language, const QString &country,
                          KSharedConfig::Ptr persistantConfig, KConfig *tempConfig)
{
    m_catalogName = catalogName;

    // Only keep the persistant config if it is not the global
    if (persistantConfig != KSharedConfig::Ptr() && persistantConfig != KGlobal::config()) {
        m_config = persistantConfig;
    }

    KConfigGroup cg;
    bool useEnvironmentVariables;

    // We can't read the formats from the config until we know what locale to read in, but we need
    // to read the config to find out the locale.  The Country and Language settings should never
    // be localized in the config, so we can read a temp copy of them to get us started.

    // If no config given, use the global config and include envvars, otherwise use only the config.
    if (m_config != KSharedConfig::Ptr()) {
        cg = m_config->group(QLatin1String("Locale"));
        useEnvironmentVariables = false;
    } else if (tempConfig == 0 || tempConfig == KGlobal::config().data()) {
        cg = KGlobal::config()->group(QLatin1String("Locale"));
        useEnvironmentVariables = true;
    } else {
        cg = tempConfig->group(QLatin1String("Locale"));
        useEnvironmentVariables = false;
    }

    initEncoding();
    initFileNameEncoding();
    initCountry(country, cg.readEntry(QLatin1String("Country")));
    initLanguageList(language, cg.readEntry(QLatin1String("Language")), useEnvironmentVariables);
    // Now that we have a language, we can set up the config which uses it to setLocale()
    initConfig(tempConfig);
    initMainCatalogs();
    initFormat();
}

// Init the config, this is called during construction and by later setCountry/setLanguage calls.
// You _must_ have the m_language set to a valid language or en_US before calling this so a
// setLocale can be applied to the config
void KLocalePrivate::initConfig(KConfig *config)
{
    // * If we were constructed with a KSharedConfig it means the user gave it to us
    //   to use for the life of the KLocale, so just keep using it after a setLocale
    // * If passed in KConfig is null or the global config then use the global, but
    //   do the setLocale first.
    // * If we have a KConfig we need to use that, but due to keeping old behaviour
    //   of not requiring access to it for life we can't keep a reference so instead
    //   take a copy and use that, but do setLocale first.

    if (m_config != KSharedConfig::Ptr()) {
        m_config->setLocale(m_language);
    } else {
        // If no config given then use the global
        if (config == 0 || config == KGlobal::config().data()) {
            KGlobal::config()->setLocale(m_language);
        } else {
            config->setLocale(m_language);
            m_config = KSharedConfig::openConfig();
            config->copyTo(QString(), m_config.data());
            m_config->markAsClean();
        }
    }
}

void KLocalePrivate::initMainCatalogs()
{
    KLocaleStaticData *s = staticData;
    QMutexLocker lock(kLocaleMutex());

    if (!s->maincatalog.isEmpty()) {
        // If setMainCatalog was called, then we use that
        // (e.g. korgac calls setMainCatalog("korganizer") to use korganizer.po)
        m_catalogName = s->maincatalog;
    }

    if (m_catalogName.isEmpty()) {
        kDebug(173) << "KLocale instance created called without valid "
                    << "catalog! Give an argument or call setMainCatalog "
                    << "before init" << endl;
    } else {
        // do not use insertCatalog here, that would already trigger updateCatalogs
        m_catalogNames.append(KCatalogName(m_catalogName));   // application catalog

        // catalogs from which each application can draw translations
        const int numberOfCatalogs = m_catalogNames.size();
        m_catalogNames.append(KCatalogName(QString::fromLatin1("libphonon")));
        m_catalogNames.append(KCatalogName(QString::fromLatin1("kio4")));
        m_catalogNames.append(KCatalogName(QString::fromLatin1("kdelibs4")));
        m_catalogNames.append(KCatalogName(QString::fromLatin1("kdeqt")));
        m_catalogNames.append(KCatalogName(QString::fromLatin1("solid_qt")));
        m_catalogNames.append(KCatalogName(QString::fromLatin1("kdecalendarsystems")));
        m_numberOfSysCatalogs = m_catalogNames.size() - numberOfCatalogs;

        updateCatalogs(); // evaluate this for all languages
    }
}

void KLocalePrivate::getLanguagesFromVariable(QStringList &list, const char *variable, bool isLanguageList)
{
    QByteArray var(qgetenv(variable));
    if (!var.isEmpty()) {
        QString value = QFile::decodeName(var);
        if (isLanguageList) {
            list += value.split(QLatin1Char(':'));
        } else {
            // Process the value to create possible combinations.
            QString lang, ctry, modf, cset;
            KLocale::splitLocale(value, lang, ctry, modf, cset);

            if (!ctry.isEmpty() && !modf.isEmpty()) {
                list += lang + QLatin1Char('_') + ctry + QLatin1Char('@') + modf;
            }
            // NOTE: The priority is tricky in case both ctry and modf are present.
            // Should really lang@modf be of higher priority than lang_ctry?
            // For at least one case (Serbian language), it is better this way.
            if (!modf.isEmpty()) {
                list += lang + QLatin1Char('@') + modf;
            }
            if (!ctry.isEmpty()) {
                list += lang + QLatin1Char('_') + ctry;
            }
            list += lang;
        }
    }
}

// init the country at construction only, will ensure we always have a country set
void KLocalePrivate::initCountry(const QString &country, const QString &configCountry)
{
    // Cache the valid countries list and add the default C as it is valid to use
    QStringList validCountries = allCountriesList();
    validCountries.append( defaultCountry() );

    // First check if the constructor passed in a value and if so if it is valid
    QString putativeCountry = country;

    if ( putativeCountry.isEmpty() || !validCountries.contains( putativeCountry, Qt::CaseInsensitive ) ) {

        // If the requested country is not valid, try the country as set in the config:
        putativeCountry = configCountry;

        if ( putativeCountry.isEmpty() || !validCountries.contains( putativeCountry, Qt::CaseInsensitive ) ) {

            // If the config country is not valid try the current host system country
            putativeCountry = systemCountry();

            if ( putativeCountry.isEmpty() || !validCountries.contains( putativeCountry, Qt::CaseInsensitive ) ) {
                // Only if no other option, resort to the default C
                putativeCountry = defaultCountry();
            }
        }
    }

    // Always save as lowercase, unless it's C when we want it uppercase
    if ( putativeCountry.toLower() == defaultCountry().toLower() ) {
        m_country = defaultCountry();
    } else {
        m_country = putativeCountry.toLower();
    }
}

QString KLocalePrivate::systemCountry() const
{
    // Use QLocale for now as it supposedly provides a sensible default most times,
    // e.g. if locale is only "de" it is assumed to mean country of "DE"
    QString systemCountry, s1, s2, s3;
    splitLocale( QLocale::system().name(), s1, systemCountry, s2, s3 );
    return systemCountry.toLower();
}

void KLocalePrivate::initLanguageList(const QString &language, const QString &configLanguages,
                                      bool useEnvironmentVariables)
{
    m_language = language;

    // Collect possible languages by decreasing priority.
    // The priority is as follows:
    // - the internally set language, if any
    // - KDE_LANG environment variable (can be a list)
    // - KDE configuration (can be a list)
    // - environment variables considered by gettext(3)
    // The environment variables are not considered if useEnvironmentVariables is false.
    QStringList list;
    if (!m_language.isEmpty()) {
        list += m_language;
    }

    // If the Locale object was created with a specific config file, then do not use the
    // environmental variables.  If the locale object was created with the global config, then
    // do use the environmental variables.
    if (useEnvironmentVariables) {
        // KDE_LANG contains list of language codes, not locale string.
        getLanguagesFromVariable(list, "KDE_LANG", true);
    }

    if (!configLanguages.isEmpty()) {
        list += configLanguages.split(QLatin1Char(':'));
    }

    if (useEnvironmentVariables) {
        // Collect languages by same order of priority as for gettext(3).
        // LANGUAGE contains list of language codes, not locale string.
        getLanguagesFromVariable(list, "LANGUAGE", true);
        getLanguagesFromVariable(list, "LC_ALL");
        getLanguagesFromVariable(list, "LC_MESSAGES");
        getLanguagesFromVariable(list, "LANG");
    }

    // fall back to the system language
    list += systemLanguageList();

    // Send the list to filter for really present languages on the system.
    setLanguage(list);
}

QStringList KLocalePrivate::systemLanguageList() const
{
    return QStringList();
}

void KLocalePrivate::initFormat()
{
    KConfigGroup cg(config(), "Locale");

    KConfig entryFile(KStandardDirs::locate("locale", QString::fromLatin1("l10n/%1/entry.desktop").arg(m_country)));
    entryFile.setLocale(m_language);
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

    // Country settings
    readConfigEntry("CountryDivisionCode", QString(), m_countryDivisionCode);

    // Numeric formats
    readConfigNumEntry("DecimalPlaces", 2, m_decimalPlaces, int);

    readConfigEntry("DecimalSymbol", ".", m_decimalSymbol);
    readConfigEntry("ThousandsSeparator", ",", m_thousandsSeparator);
    m_thousandsSeparator.remove(QString::fromLatin1("$0"));

    readConfigEntry("PositiveSign", "", m_positiveSign);
    readConfigEntry("NegativeSign", "-", m_negativeSign);

    readConfigNumEntry("DigitSet", KLocale::ArabicDigits, m_digitSet, KLocale::DigitSet);
    // FIXME: Temporary until full language-sensitivity implemented.
    readConfigEntry("LanguageSensitiveDigits", true, m_languageSensitiveDigits);

    // Currency
    readConfigEntry("CurrencyCode", "USD", m_currencyCode);
    initCurrency();
    readConfigEntry("CurrencySymbol", m_currency->defaultSymbol(), m_currencySymbol);
    readConfigEntry("CurrencyCodesInUse", QStringList(m_currencyCode), m_currencyCodeList);

    // Monetary formats
    readConfigNumEntry("MonetaryDecimalPlaces", m_currency->decimalPlaces(), m_monetaryDecimalPlaces, int);

    readConfigEntry("MonetaryDecimalSymbol", ".", m_monetaryDecimalSymbol);
    readConfigEntry("MonetaryThousandsSeparator", ",", m_monetaryThousandsSeparator);
    m_monetaryThousandsSeparator.remove(QString::fromLatin1("$0"));

    readConfigEntry("PositivePrefixCurrencySymbol", true, m_positivePrefixCurrencySymbol);
    readConfigEntry("NegativePrefixCurrencySymbol", true, m_negativePrefixCurrencySymbol);
    readConfigNumEntry("PositiveMonetarySignPosition", KLocale::BeforeQuantityMoney,
                       m_positiveMonetarySignPosition, KLocale::SignPosition);
    readConfigNumEntry("NegativeMonetarySignPosition", KLocale::ParensAround,
                       m_negativeMonetarySignPosition, KLocale::SignPosition);

    readConfigNumEntry("MonetaryDigitSet", KLocale::ArabicDigits,
                       m_monetaryDigitSet, KLocale::DigitSet);
    readConfigNumEntry("BinaryUnitDialect", KLocale::IECBinaryDialect,
                       m_binaryUnitDialect, KLocale::BinaryUnitDialect);

    // Date and time
    readConfigEntry("TimeFormat", "%H:%M:%S", m_timeFormat);
    readConfigEntry("DateFormat", "%A %d %B %Y", m_dateFormat);
    readConfigEntry("DateFormatShort", "%Y-%m-%d", m_dateFormatShort);
    readConfigNumEntry("WeekStartDay", 1, m_weekStartDay, int);                //default to Monday
    readConfigNumEntry("WorkingWeekStartDay", 1, m_workingWeekStartDay, int);  //default to Monday
    readConfigNumEntry("WorkingWeekEndDay", 5, m_workingWeekEndDay, int);      //default to Friday
    readConfigNumEntry("WeekDayOfPray", 7, m_weekDayOfPray, int);              //default to Sunday
    readConfigNumEntry("DateTimeDigitSet", KLocale::ArabicDigits,
                       m_dateTimeDigitSet, KLocale::DigitSet);

    // other
#ifndef QT_NO_PRINTER
    readConfigNumEntry("PageSize", QPrinter::A4, m_pageSize, QPrinter::PageSize);
#endif
    readConfigNumEntry("MeasureSystem", KLocale::Metric, m_measureSystem, KLocale::MeasureSystem);
    QString calendarType;
    readConfigEntry("CalendarSystem", "gregorian", calendarType);
    setCalendar(calendarType);

    readConfigEntry("Transcript", true, m_useTranscript);

    //Grammatical
    //Precedence here is l10n / i18n / config file
    KConfig langCfg(KStandardDirs::locate("locale", QString::fromLatin1("%1/entry.desktop").arg(m_language)));
    KConfigGroup lang(&langCfg, "KCM Locale");
#define read3ConfigBoolEntry(key, default, save) \
        save = entry.readEntry(key, default); \
        save = lang.readEntry(key, save); \
        save = cg.readEntry(key, save);

    read3ConfigBoolEntry("NounDeclension", false, m_nounDeclension);
    read3ConfigBoolEntry("DateMonthNamePossessive", false, m_dateMonthNamePossessive);

    initDayPeriods(cg);
}

void KLocalePrivate::initDayPeriods(const KConfigGroup &cg)
{
    // Prefer any l10n file value for country/language,
    // otherwise default to language only value which will be filled in later when i18n available

    //Day Period are stored in config as one QStringList entry per Day Period
    //PeriodCode,LongName,ShortName,NarrowName,StartTime,EndTime,Offset,OffsetIfZero
    //where start and end time are in the format HH:MM:SS.MMM

    m_dayPeriods.clear();
    QString periodKey = QString::fromLatin1("DayPeriod1");
    int i = 1;
    while (cg.hasKey(periodKey)) {
        QStringList period = cg.readEntry(periodKey, QStringList());
        if (period.count() == 8) {
            m_dayPeriods.append(KDayPeriod(period[0], period[1], period[2], period[3],
                                           QTime::fromString(period[4], QString::fromLatin1("HH:mm:ss.zzz")),
                                           QTime::fromString(period[5], QString::fromLatin1("HH:mm:ss.zzz")),
                                           period[6].toInt(), period[7].toInt()));
        }
        i = i + 1;
        periodKey = QString::fromLatin1("DayPeriod%1").arg(i);
    }
}

bool KLocalePrivate::setCountry(const QString &country, KConfig *newConfig)
{
    // Cache the valid countries list and add the default C as it is valid to use
    QStringList validCountries = allCountriesList();
    validCountries.append(defaultCountry());

    QString putativeCountry = country;

    if (putativeCountry.isEmpty()) {
        // An empty string means to use the system country
        putativeCountry = systemCountry();
        if (putativeCountry.isEmpty() || !validCountries.contains(putativeCountry, Qt::CaseInsensitive)) {
            // If the system country is not valid, use the default
            putativeCountry = defaultCountry();
        }
    } else if (!validCountries.contains(putativeCountry, Qt::CaseInsensitive)) {
        return false;
    }

    // Always save as lowercase, unless it's C when we want it uppercase
    if (putativeCountry.toLower() == defaultCountry().toLower()) {
        m_country = defaultCountry();
    } else {
        m_country = putativeCountry.toLower();
    }

    // Get rid of the old config, start again with the new
    m_config = KSharedConfig::Ptr();
    initConfig(newConfig);

    // Init all the settings
    initFormat();

    return true;
}

bool KLocalePrivate::setCountryDivisionCode(const QString &countryDivisionCode)
{
    m_countryDivisionCode = countryDivisionCode;
    return true;
}

bool KLocalePrivate::setLanguage(const QString &language, KConfig *config)
{
    QMutexLocker lock(kLocaleMutex());
    m_languageList.removeAll(language);
    m_languageList.prepend(language);   // let us consider this language to be the most important one

    m_language = language; // remember main language for shortcut evaluation

    // important when called from the outside and harmless when called before
    // populating the catalog name list
    updateCatalogs();

    // Get rid of the old config, start again with the new
    m_config = KSharedConfig::Ptr();
    initConfig(config);

    // Init the new format settings
    initFormat();

    // Maybe the mo-files for this language are empty, but in principle we can speak all languages
    return true;
}

// KDE5 Unlike the other setLanguage call this does not reparse the config so the localized config
// settings for the new primary language will _not_ be loaded.  In KDE5 always keep the original
// config so this can be reparsed when required.
bool KLocalePrivate::setLanguage(const QStringList &languages)
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
        if (!language.isEmpty() && !list.contains(language) && isApplicationTranslatedInto(language)) {
            list.append(language);
        }
    }

    if (!list.contains(KLocale::defaultLanguage())) {
        // English should always be added as final possibility; this is important
        // for proper initialization of message text post-processors which are
        // needed for English too, like semantic to visual formatting, etc.
        list.append(KLocale::defaultLanguage());
    }

    m_language = list.first(); // keep this for shortcut evaluations

    m_languageList = list; // keep this new list of languages to use

    // important when called from the outside and harmless when called before populating the
    // catalog name list
    updateCatalogs();

    return true; // we found something. Maybe it's only English, but we found something
}

void KLocalePrivate::initCurrency()
{
    if (m_currencyCode.isEmpty() || !KCurrencyCode::isValid(m_currencyCode)) {
        m_currencyCode = KLocale::defaultCurrencyCode();
    }

    if (!m_currency || m_currencyCode != m_currency->isoCurrencyCode() || !m_currency->isValid()) {
        delete m_currency;
        m_currency = new KCurrencyCode(m_currencyCode, m_language);
    }
}

void KLocalePrivate::setCurrencyCode(const QString &newCurrencyCode)
{
    if (!newCurrencyCode.isEmpty() && newCurrencyCode != m_currency->isoCurrencyCode() &&
        KCurrencyCode::isValid(newCurrencyCode)) {
        m_currencyCode = newCurrencyCode;
        initCurrency();
    }
}

bool KLocalePrivate::isApplicationTranslatedInto(const QString &lang)
{
    if (lang.isEmpty()) {
        return false;
    }

    if (lang == KLocale::defaultLanguage()) {
        // default language is always "installed"
        return true;
    }

    if (m_catalogName.isEmpty()) {
        kDebug() << "no appName!";
        return false;
    }

    if (!KCatalog::catalogLocaleDir(m_catalogName, lang).isEmpty()) {
        return true;
    }
    return false;
}

void KLocalePrivate::splitLocale(const QString &aLocale, QString &language, QString &country,
                                 QString &modifier, QString &charset)
{
    QString locale = aLocale;

    language.clear();
    country.clear();
    modifier.clear();
    charset.clear();

    // In case there are several concatenated locale specifications,
    // truncate all but first.
    int f = locale.indexOf(QLatin1Char(':'));
    if (f >= 0) {
        locale.truncate(f);
    }

    f = locale.indexOf(QLatin1Char('.'));
    if (f >= 0) {
        charset = locale.mid(f + 1);
        locale.truncate(f);
    }

    f = locale.indexOf(QLatin1Char('@'));
    if (f >= 0) {
        modifier = locale.mid(f + 1);
        locale.truncate(f);
    }

    f = locale.indexOf(QLatin1Char('_'));
    if (f >= 0) {
        country = locale.mid(f + 1);
        locale.truncate(f);
    }

    language = locale;
}

QString KLocalePrivate::language() const
{
    return m_language;
}

QString KLocalePrivate::country() const
{
    return m_country;
}

QString KLocalePrivate::countryDivisionCode() const
{
    if (m_countryDivisionCode.isEmpty()) {
        return country().toUpper();
    } else {
        return m_countryDivisionCode;
    }
}

KCurrencyCode *KLocalePrivate::currency()
{
    if (!m_currency) {
        initCurrency();
    }
    return m_currency;
}

QString KLocalePrivate::currencyCode() const
{
    return m_currencyCode;
}

void KLocalePrivate::insertCatalog(const QString &catalog)
{
    QMutexLocker lock(kLocaleMutex());
    int pos = m_catalogNames.indexOf(KCatalogName(catalog));
    if (pos != -1) {
        ++m_catalogNames[pos].loadCount;
        return;
    }

    // Insert new catalog just before system catalogs, to preserve the
    // lowest priority of system catalogs.
    m_catalogNames.insert(m_catalogNames.size() - m_numberOfSysCatalogs, KCatalogName(catalog));
    updateCatalogs(); // evaluate the changed list and generate the necessary KCatalog objects
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
    foreach(const QString &lang, m_languageList) {
        if (lang == KLocale::defaultLanguage()) {
            // Default language has no catalogs (messages from the code),
            // so loading catalogs for languages below the default
            // would later confuse the fallback resolution.
            break;
        }
        foreach(const KCatalogName &name, m_catalogNames) {
            // create and add catalog for this name and language if it exists
            if (! KCatalog::catalogLocaleDir(name.name, lang).isEmpty()) {
                newCatalogs.append(KCatalog(name.name, lang));
                //kDebug(173) << "Catalog: " << name << ":" << lang;
            }
        }
    }

    // notify KLocalizedString of catalog update.
    m_catalogs = newCatalogs;
    KLocalizedString::notifyCatalogsUpdated(m_languageList, m_catalogNames);
}

void KLocalePrivate::removeCatalog(const QString &catalog)
{
    QMutexLocker lock(kLocaleMutex());
    int pos = m_catalogNames.indexOf(KCatalogName(catalog));
    if (pos == -1) {
        return;
    }
    if (--m_catalogNames[pos].loadCount > 0) {
        return;
    }
    m_catalogNames.removeAt(pos);
    if (KGlobal::hasMainComponent()) {
        // walk through the KCatalog instances and weed out everything we no longer need
        updateCatalogs();
    }
}

void KLocalePrivate::setActiveCatalog(const QString &catalog)
{
    QMutexLocker lock(kLocaleMutex());
    int pos = m_catalogNames.indexOf(KCatalogName(catalog));
    if (pos == -1) {
        return;
    }
    m_catalogNames.move(pos, 0);
    // walk through the KCatalog instances and adapt to the new order
    updateCatalogs();
}

void KLocalePrivate::translateRawFrom(const char *catname, const char *msgctxt, const char *msgid, const char *msgid_plural,
                                      unsigned long n, QString *language, QString *translation) const
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

    const QList<KCatalog> catalogList = m_catalogs;
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

QString KLocalePrivate::translateQt(const char *context, const char *sourceText, const char *comment) const
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

    if (useDefaultLanguage()) {
        return QString();
    }

    QString translation;
    QString language;

    // NOTE: Condition (language != defaultLanguage()) means that translation
    // was found, otherwise we got the original string back as translation.

    if (comment && comment[0]) {
        // Comment given, go for context call.
        translateRawFrom(0, comment, sourceText, 0, 0, &language, &translation);
    } else {
        // Comment not given, go for try-fallback with context.
        if (context && context[0]) {
            translateRawFrom(0, context, sourceText, 0, 0, &language, &translation);
        }
        if (language.isEmpty() || language == defaultLanguage()) {
            translateRawFrom(0, 0, sourceText, 0, 0, &language, &translation);
        }
    }

    if (language != defaultLanguage()) {
        return translation;
    }

    // No proper translation found, return empty according to Qt's expectation.
    return QString();
}

QList<KLocale::DigitSet> KLocalePrivate::allDigitSetsList() const
{
    QList<KLocale::DigitSet> digitSets;
    digitSets.append(KLocale::ArabicDigits);
    digitSets.append(KLocale::ArabicIndicDigits);
    digitSets.append(KLocale::BengaliDigits);
    digitSets.append(KLocale::DevenagariDigits);
    digitSets.append(KLocale::EasternArabicIndicDigits);
    digitSets.append(KLocale::GujaratiDigits);
    digitSets.append(KLocale::GurmukhiDigits);
    digitSets.append(KLocale::KannadaDigits);
    digitSets.append(KLocale::KhmerDigits);
    digitSets.append(KLocale::MalayalamDigits);
    digitSets.append(KLocale::OriyaDigits);
    digitSets.append(KLocale::TamilDigits);
    digitSets.append(KLocale::TeluguDigits);
    digitSets.append(KLocale::ThaiDigits);
    qSort(digitSets);
    return digitSets;
}

QString KLocalePrivate::digitSetString(KLocale::DigitSet digitSet)
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

QString KLocalePrivate::digitSetToName(KLocale::DigitSet digitSet, bool withDigits) const
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

QString KLocalePrivate::convertDigits(const QString &str, KLocale::DigitSet digitSet, bool ignoreContext) const
{
    if (!ignoreContext) {
        // Fall back to Western Arabic digits if requested digit set
        // is not appropriate for current application language.
        // FIXME: Temporary until full language-sensitivity implemented.
        KLocaleStaticData *s = staticData;
        if (m_languageSensitiveDigits && !s->languagesUsingDigitSet[digitSet].contains(m_language)) {
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

QString KLocalePrivate::toArabicDigits(const QString &str)
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

bool KLocalePrivate::nounDeclension() const
{
    return m_nounDeclension;
}

bool KLocalePrivate::dateMonthNamePossessive() const
{
    return m_dateMonthNamePossessive;
}

int KLocalePrivate::weekStartDay() const
{
    return m_weekStartDay;
}

int KLocalePrivate::workingWeekStartDay() const
{
    return m_workingWeekStartDay;
}

int KLocalePrivate::workingWeekEndDay() const
{
    return m_workingWeekEndDay;
}

int KLocalePrivate::weekDayOfPray() const
{
    return m_weekDayOfPray;
}

int KLocalePrivate::decimalPlaces() const
{
    return m_decimalPlaces;
}

QString KLocalePrivate::decimalSymbol() const
{
    return m_decimalSymbol;
}

QString KLocalePrivate::thousandsSeparator() const
{
    return m_thousandsSeparator;
}

QString KLocalePrivate::currencySymbol() const
{
    return m_currencySymbol;
}

QString KLocalePrivate::monetaryDecimalSymbol() const
{
    return m_monetaryDecimalSymbol;
}

QString KLocalePrivate::monetaryThousandsSeparator() const
{
    return m_monetaryThousandsSeparator;
}

QString KLocalePrivate::positiveSign() const
{
    return m_positiveSign;
}

QString KLocalePrivate::negativeSign() const
{
    return m_negativeSign;
}

/* Just copy to keep the diff looking clean, delete later
int KLocale::fracDigits() const
{
    return monetaryDecimalPlaces();
}
*/

int KLocalePrivate::monetaryDecimalPlaces() const
{
    return m_monetaryDecimalPlaces;
}

bool KLocalePrivate::positivePrefixCurrencySymbol() const
{
    return m_positivePrefixCurrencySymbol;
}

bool KLocalePrivate::negativePrefixCurrencySymbol() const
{
    return m_negativePrefixCurrencySymbol;
}

KLocale::SignPosition KLocalePrivate::positiveMonetarySignPosition() const
{
    return m_positiveMonetarySignPosition;
}

KLocale::SignPosition KLocalePrivate::negativeMonetarySignPosition() const
{
    return m_negativeMonetarySignPosition;
}

static inline void put_it_in(QChar *buffer, int &index, const QString &s)
{
    for (int l = 0; l < s.length(); l++) {
        buffer[index++] = s.at(l);
    }
}

static inline void put_it_in(QChar *buffer, int &index, int number)
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

QString KLocalePrivate::formatMoney(double num, const QString &symbol, int precision) const
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
    res.replace(QLatin1Char('.'), monetaryDecimalSymbol());

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
    case KLocale::ParensAround:
        res.prepend(QLatin1Char('('));
        res.append(QLatin1Char(')'));
        break;
    case KLocale::BeforeQuantityMoney:
        res.prepend(sign);
        break;
    case KLocale::AfterQuantityMoney:
        res.append(sign);
        break;
    case KLocale::BeforeMoney:
        currencyString.prepend(sign);
        break;
    case KLocale::AfterMoney:
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
    res = convertDigits(res, m_monetaryDigitSet);

    return res;
}


QString KLocalePrivate::formatNumber(double num, int precision) const
{
    if (precision < 0) {
        precision = decimalPlaces();
    }
    // no need to round since QString::number does this for us
    return formatNumber(QString::number(num, 'f', precision), false, 0);
}

QString KLocalePrivate::formatLong(long num) const
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
            if (i == 0) str.prepend(QLatin1Char('1'));
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
    int decimalSymbolPos = str.indexOf(QLatin1Char('.'));

    if (decimalSymbolPos == -1) {
        if (precision == 0)  return;
        else if (precision > 0) { // add dot if missing (and needed)
            str.append(QLatin1Char('.'));
            decimalSymbolPos = str.length() - 1;
        }
    }
    // fill up with more than enough zeroes (in case fractional part too short)
    str.reserve(str.length() + precision);
    for (int i = 0; i < precision; ++i)
        str.append(QLatin1Char('0'));

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

    decimalSymbolPos = str.indexOf(QLatin1Char('.'));
    str.truncate(decimalSymbolPos + precision + 1);

    // if precision == 0 delete also '.'
    if (precision == 0) {
        str = str.left(decimalSymbolPos);
    }

    str.squeeze();
}

QString KLocalePrivate::formatNumber(const QString &numStr, bool round, int precision) const
{
    QString tmpString = numStr;

    if (precision < 0) {
        precision = decimalPlaces();
    }

    // Skip the sign (for now)
    const bool neg = (tmpString[0] == QLatin1Char('-'));
    if (neg || tmpString[0] == QLatin1Char('+')) {
        tmpString.remove(0, 1);
    }

    //kDebug(173)<<"tmpString:"<<tmpString;

    // Split off exponential part (including 'e'-symbol)
    const int expPos = tmpString.indexOf(QLatin1Char('e')); // -1 if not found
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
        mantString = QLatin1Char('0');
    }

    if (round) {
        _round(mantString, precision);
    }

    // Replace dot with locale decimal separator
    mantString.replace(QLatin1Char('.'), decimalSymbol());

    // Insert the thousand separators
    _insertSeparator(mantString, thousandsSeparator(), decimalSymbol());

    // How can we know where we should put the sign?
    mantString.prepend(neg ? negativeSign() : positiveSign());

    // Convert to target digit set.
    if (digitSet() != KLocale::ArabicDigits) {
        mantString = convertDigits(mantString, digitSet());
        expString = convertDigits(expString, digitSet());
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
        translateRawFrom(0, ctxt_text, 0, 0, 0, &s); \
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

QString KLocalePrivate::formatByteSize(double size, int precision, KLocale::BinaryUnitDialect dialect,
                                       KLocale::BinarySizeUnits specificUnit)
{
    // Error checking
    if (dialect <= KLocale::DefaultBinaryDialect || dialect > KLocale::LastBinaryDialect) {
        dialect = m_binaryUnitDialect;
    }

    if (specificUnit < KLocale::DefaultBinaryUnits || specificUnit > KLocale::UnitLastUnit) {
        specificUnit = KLocale::DefaultBinaryUnits;
    }

    // Choose appropriate units.
    QList<QString> dialectUnits;
    if (dialect == m_binaryUnitDialect) {
        // Cache default units for speed
        if (m_byteSizeFmt.size() == 0) {
            QMutexLocker lock(kLocaleMutex());

            // We only cache the user's default dialect.
            m_byteSizeFmt = dialectUnitsList(m_binaryUnitDialect);
        }

        dialectUnits = m_byteSizeFmt;
    } else {
        dialectUnits = dialectUnitsList(dialect);
    }

    int unit = 0; // Selects what unit to use from cached list
    double multiplier = 1024.0;

    if (dialect == KLocale::MetricBinaryDialect) {
        multiplier = 1000.0;
    }

    // If a specific unit conversion is given, use it directly.  Otherwise
    // search until the result is in [0, multiplier) (or out of our range).
    if (specificUnit == KLocale::DefaultBinaryUnits) {
        while (size >= multiplier && unit < (int) KLocale::UnitYottaByte) {
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

QString KLocalePrivate::formatByteSize(double size)
{
    return formatByteSize(size, 1);
}

KLocale::BinaryUnitDialect KLocalePrivate::binaryUnitDialect() const
{
    return m_binaryUnitDialect;
}

void KLocalePrivate::setBinaryUnitDialect(KLocale::BinaryUnitDialect newDialect)
{
    if (newDialect > KLocale::DefaultBinaryDialect && newDialect <= KLocale::LastBinaryDialect) {
        QMutexLocker lock(kLocaleMutex());
        m_binaryUnitDialect = newDialect;
        m_byteSizeFmt.clear(); // Reset cached translations.
    }
}

QString KLocalePrivate::formatDuration(unsigned long mSec) const
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

QString KLocalePrivate::formatSingleDuration(KLocalePrivate::DurationType durationType, int n)
{
    switch (durationType) {
    case KLocalePrivate::DaysDurationType:
        return i18ncp("@item:intext", "1 day", "%1 days", n);
    case KLocalePrivate::HoursDurationType:
        return i18ncp("@item:intext", "1 hour", "%1 hours", n);
    case KLocalePrivate::MinutesDurationType:
        return i18ncp("@item:intext", "1 minute", "%1 minutes", n);
    case KLocalePrivate::SecondsDurationType:
        return i18ncp("@item:intext", "1 second", "%1 seconds", n);
    }
    return QString();
}

QString KLocalePrivate::prettyFormatDuration(unsigned long mSec) const
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
                     "%1 and %2", formatSingleDuration(KLocalePrivate::DaysDurationType, days),
                     formatSingleDuration(KLocalePrivate::HoursDurationType, hours));
    } else if (days) {
        return formatSingleDuration(KLocalePrivate::DaysDurationType, days);
    } else if (hours && minutes) {
        return i18nc("@item:intext hours and minutes. This uses the previous item:intext messages. If this does not fit the grammar of your language please contact the i18n team to solve the problem",
                     "%1 and %2",
                     formatSingleDuration(KLocalePrivate::HoursDurationType, hours),
                     formatSingleDuration(KLocalePrivate::MinutesDurationType, minutes));
    } else if (hours) {
        return formatSingleDuration(KLocalePrivate::HoursDurationType, hours);
    } else if (minutes && seconds) {
        return i18nc("@item:intext minutes and seconds. This uses the previous item:intext messages. If this does not fit the grammar of your language please contact the i18n team to solve the problem",
                     "%1 and %2",
                     formatSingleDuration(KLocalePrivate::MinutesDurationType, minutes),
                     formatSingleDuration(KLocalePrivate::SecondsDurationType, seconds));
    } else if (minutes) {
        return formatSingleDuration(KLocalePrivate::MinutesDurationType, minutes);
    } else {
        return formatSingleDuration(KLocalePrivate::SecondsDurationType, seconds);
    }
}

QString KLocalePrivate::formatDate(const QDate &date, KLocale::DateFormat format)
{
    return calendar()->formatDate(date, format);
}

void KLocalePrivate::setMainCatalog(const char *catalog)
{
    KLocaleStaticData *s = staticData;
    s->maincatalog = QString::fromUtf8(catalog);
}

double KLocalePrivate::readNumber(const QString &_str, bool * ok) const
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

    EPos = str.indexOf(QLatin1Char('E'), 0, Qt::CaseInsensitive);

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
        tot = QLatin1Char('-');
    }

    tot += major + QLatin1Char('.') + minor + exponentialPart;

    tot = toArabicDigits(tot);

    return tot.toDouble(ok);
}

double KLocalePrivate::readMoney(const QString &_str, bool *ok) const
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
    if (negativeMonetarySignPosition() == KLocale::ParensAround) {
        if (str[0] == QLatin1Char('(') && str[str.length()-1] == QLatin1Char(')')) {
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
        tot = QLatin1Char('-');
    }
    tot += major + QLatin1Char('.') + minior;
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

QDate KLocalePrivate::readDate(const QString &intstr, bool *ok)
{
    return calendar()->readDate(intstr, ok);
}

QDate KLocalePrivate::readDate(const QString &intstr, KLocale::ReadDateFlags flags, bool *ok)
{
    return calendar()->readDate(intstr, flags, ok);
}

QDate KLocalePrivate::readDate(const QString &intstr, const QString &fmt, bool *ok)
{
    return calendar()->readDate(intstr, fmt, ok);
}

QTime KLocalePrivate::readTime(const QString &intstr, bool *ok) const
{
    QTime time = readLocaleTime(intstr, ok, KLocale::TimeDefault, KLocale::ProcessStrict);
    if (time.isValid()) {
        return time;
    }
    return readLocaleTime(intstr, ok, KLocale::TimeWithoutSeconds, KLocale::ProcessStrict);
}

QTime KLocalePrivate::readTime(const QString &intstr, KLocale::ReadTimeFlags flags, bool *ok) const
{
    return readLocaleTime(intstr, ok, (flags == KLocale::WithSeconds) ? KLocale::TimeDefault : KLocale::TimeWithoutSeconds,
                          KLocale::ProcessStrict);
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
    if (curPos >= 0 && inout.at(curPos).isPunct() && inout.at(curPos) != QLatin1Char('%')) {
        curPos--;
    }
    while (curPos >= 0 && inout.at(curPos).isSpace()) {
        curPos--;
    }

    remPos = qMax(curPos + 1, 0);
    inout.remove(remPos, endPos - remPos);
}

// remove the first occurrence of the 2-character string
// strip2char from inout and if found, also remove one
// succeeding punctuation character and arbitrary number of spaces.
static void stripStringAndSucceedingSeparator(QString &inout, const QLatin1String &strip2char)
{
    int remPos = inout.indexOf(strip2char);
    if (remPos == -1) {
        return;
    }
    int curPos = remPos + 2;
    while (curPos < inout.size() &&
           (inout.at(curPos).isSpace() ||
            (inout.at(curPos).isPunct() && inout.at(curPos) != QLatin1Char('%')))) {
        curPos++;
    }
    inout.remove(remPos, curPos - remPos);
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
                inout.at(ppos) != QLatin1Char('%')) {
            ppos++;
        }
        inout = inout.mid(ppos);
    } else {
        stripStringAndPreceedingSeparator(inout, QLatin1String("%p"));
    }
}

QTime KLocalePrivate::readLocaleTime(const QString &intstr, bool *ok, KLocale::TimeFormatOptions options,
                                     KLocale::TimeProcessingOptions processing) const
{
    QString str(intstr.simplified().toLower());
    QString format(timeFormat().simplified());

    int hour = -1;
    int minute = -1;
    int second = -1;
    bool useDayPeriod = false;
    KDayPeriod dayPeriod = dayPeriodForTime(QTime(0,0,0));
    int strpos = 0;
    int formatpos = 0;
    bool error = false;

    bool excludeSecs = ((options & KLocale::TimeWithoutSeconds) == KLocale::TimeWithoutSeconds);
    bool isDuration = ((options & KLocale::TimeDuration) == KLocale::TimeDuration);
    bool noAmPm = ((options & KLocale::TimeWithoutAmPm) == KLocale::TimeWithoutAmPm);
    bool foldHours = ((options & KLocale::TimeFoldHours) == KLocale::TimeFoldHours);
    bool strict = ((processing & KLocale::ProcessStrict) == KLocale::ProcessStrict);

    // if seconds aren't needed, strip them from the timeFormat
    if (excludeSecs) {
        stripStringAndPreceedingSeparator(format, QLatin1String("%S"));
        second = 0; // seconds are always 0
    }

    // if hours are folded, strip them from the timeFormat
    if (foldHours) {
        stripStringAndSucceedingSeparator(format, QLatin1String("%H"));
        stripStringAndSucceedingSeparator(format, QLatin1String("%k"));
        stripStringAndSucceedingSeparator(format, QLatin1String("%I"));
        stripStringAndSucceedingSeparator(format, QLatin1String("%l"));
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

        if (c != QLatin1Char('%')) {
            if (c != str.at(strpos++)) {
                error = true;
                break;
            }
            continue;
        }

        c = format.at(formatpos++);
        switch (c.unicode()) {

        case 'p': // Day Period, normally AM/PM
        case 'P': // Lowercase Day Period, normally am/pm
        {
            error = true;
            foreach (const KDayPeriod &testDayPeriod, dayPeriods()) {
                QString dayPeriodText = testDayPeriod.periodName(KLocale::ShortName);
                int len = dayPeriodText.length();
                if (str.mid(strpos, len) == dayPeriodText.toLower()) {
                    dayPeriod = testDayPeriod;
                    strpos += len;
                    error = false;
                    useDayPeriod = true;
                    break;
                }
            }
            break;
        }

        case 'k':  // 24h Hours Short Number
        case 'H':  // 24h Hours Long Number
            useDayPeriod = false;
            hour = readInt(str, strpos);
            break;

        case 'l': // 12h Hours Short Number
        case 'I': // 12h Hours Long Number
            useDayPeriod = !isDuration;
            hour = readInt(str, strpos);
            break;

        case 'M':
            minute = readInt(str, strpos);
            // minutes can be bigger than 59 if hours are folded
            if (foldHours) {
                // if hours are folded, make sure minutes doesn't get bigger than 59.
                hour = minute / 60;
                minute = minute % 60;
            }
            break;

        case 'S':
            second = readInt(str, strpos);
            break;
        }

        // NOTE: if anything is performed inside this loop, be sure to
        //       check for error!
    }

    QTime returnTime;
    if (!error) {
        if (useDayPeriod) {
            returnTime = dayPeriod.time(hour, minute, second);
        } else {
            returnTime = QTime(hour, minute, second);
        }
    }
    if (ok) {
        *ok = returnTime.isValid();
    }
    return returnTime;
}

QString KLocalePrivate::formatTime(const QTime &time, bool includeSecs, bool isDuration) const
{
    KLocale::TimeFormatOptions options = KLocale::TimeDefault;
    if (!includeSecs) {
        options |= KLocale::TimeWithoutSeconds;
    }
    if (isDuration) {
        options |= KLocale::TimeDuration;
    }
    return formatLocaleTime(time, options);
}

QString KLocalePrivate::formatLocaleTime(const QTime &time, KLocale::TimeFormatOptions options) const
{
    QString rst(timeFormat());

    bool excludeSecs = ((options & KLocale::TimeWithoutSeconds) == KLocale::TimeWithoutSeconds);
    bool isDuration = ((options & KLocale::TimeDuration) == KLocale::TimeDuration);
    bool noAmPm = ((options & KLocale::TimeWithoutAmPm) == KLocale::TimeWithoutAmPm);
    bool foldHours = ((options & KLocale::TimeFoldHours) == KLocale::TimeFoldHours);

    // if seconds aren't needed, strip them from the timeFormat
    if (excludeSecs) {
        stripStringAndPreceedingSeparator(rst, QLatin1String("%S"));
    }

    // if hours should be folded, strip all hour symbols from the timeFormat
    if (foldHours) {
        stripStringAndSucceedingSeparator(rst, QLatin1String("%H"));
        stripStringAndSucceedingSeparator(rst, QLatin1String("%k"));
        stripStringAndSucceedingSeparator(rst, QLatin1String("%I"));
        stripStringAndSucceedingSeparator(rst, QLatin1String("%l"));
    }

    // if am/pm isn't needed, strip it from the timeFormat
    if (noAmPm) {
        stripAmPmFormat(rst);
    }

    // only "pm/am" and %M here can grow, the rest shrinks, but
    // I'm rather safe than sorry
    QChar *buffer = new QChar[rst.length() * 3 / 2 + 32];

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
                buffer[index++] = QLatin1Char('%');
                break;
            case 'H':
                put_it_in(buffer, index, time.hour());
                break;
            case 'I':
                if (isDuration) {
                    put_it_in(buffer, index, time.hour());
                } else {
                    put_it_in(buffer, index, dayPeriodForTime(time).hourInPeriod(time));
                }
                break;
            case 'M':
                if (foldHours) {
                    put_it_in(buffer, index, QString::number(time.hour() * 60 + time.minute()));
                } else {
                    put_it_in(buffer, index, time.minute());
                }
                break;
            case 'S':
                put_it_in(buffer, index, time.second());
                break;
            case 'k':
            case 'l':
                // to share the code
                if (!isDuration && rst.at(format_index).unicode() == 'l') {
                    number = dayPeriodForTime(time).hourInPeriod(time);
                } else {
                    number = time.hour();
                }
                if (number / 10) {
                    buffer[index++] = number / 10 + '0';
                }
                buffer[index++] = number % 10 + '0';
                break;
            case 'p':
            {
                put_it_in(buffer, index, dayPeriodForTime(time).periodName(KLocale::ShortName));
                break;
            }
            default:
                buffer[index++] = rst.at(format_index);
                break;
            }
            escape = false;
        }
    }
    QString ret(buffer, index);
    delete [] buffer;
    ret = convertDigits(ret, dateTimeDigitSet());
    return ret.trimmed();
}

bool KLocalePrivate::use12Clock() const
{
    if ((timeFormat().contains(QString::fromLatin1("%I")) > 0) ||
        (timeFormat().contains(QString::fromLatin1("%l")) > 0)) {
        return true;
    } else {
        return false;
    }
}

void KLocalePrivate::setDayPeriods(const QList<KDayPeriod> &dayPeriods)
{
    if (dayPeriods.count() > 0) {
        foreach (const KDayPeriod &dayPeriod, dayPeriods) {
            if (!dayPeriod.isValid()) {
                return;
            }
        }
        m_dayPeriods = dayPeriods;
    }
}

QList<KDayPeriod> KLocalePrivate::dayPeriods() const
{
    // If no Day Periods currently loaded then it means there were no country specific ones defined
    // in the country l10n file, so default to standard AM/PM translations for the users language.
    // Note we couldn't do this in initDayPeriods() as i18n isn't available until we have a
    // valid loacle constructed.
    if (m_dayPeriods.isEmpty()) {
        m_dayPeriods.append(KDayPeriod(QString::fromLatin1("am"),
                                       i18nc( "Before Noon KLocale::LongName", "Ante Meridiem" ),
                                       i18nc( "Before Noon KLocale::ShortName", "AM" ),
                                       i18nc( "Before Noon KLocale::NarrowName", "A" ),
                                       QTime( 0, 0, 0 ), QTime( 11, 59, 59, 999 ), 0, 12 ));
        m_dayPeriods.append(KDayPeriod(QString::fromLatin1("pm"),
                                       i18nc( "After Noon KLocale::LongName", "Post Meridiem" ),
                                       i18nc( "After Noon KLocale::ShortName", "PM" ),
                                       i18nc( "After Noon KLocale::NarrowName", "P" ),
                                       QTime( 12, 0, 0 ), QTime( 23, 59, 59, 999 ), 0, 12 ));
    }
    return m_dayPeriods;
}

KDayPeriod KLocalePrivate::dayPeriodForTime(const QTime &time) const
{
    if (time.isValid()) {
        foreach (const KDayPeriod &dayPeriod, dayPeriods()) {
            if (dayPeriod.isValid(time)) {
                return dayPeriod;
            }
        }
    }
    return KDayPeriod();
}

QStringList KLocalePrivate::languageList() const
{
    return m_languageList;
}

QStringList KLocalePrivate::currencyCodeList() const
{
    return m_currencyCodeList;
}

QString KLocalePrivate::formatDateTime(const KLocale *locale, const QDateTime &dateTime, KLocale::DateFormat format,
                                       bool includeSeconds, int daysTo, int secsTo)
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

QString KLocalePrivate::formatDateTime(const QDateTime &dateTime, KLocale::DateFormat format, bool includeSeconds) const
{
    QDateTime now = QDateTime::currentDateTime();
    int daysTo = dateTime.date().daysTo(now.date());
    int secsTo = now.secsTo(dateTime);
    return KLocalePrivate::formatDateTime(q, dateTime, format, includeSeconds, daysTo, secsTo);
}

QString KLocalePrivate::formatDateTime(const KDateTime &dateTime, KLocale::DateFormat format,
                                       KLocale::DateTimeFormatOptions options)
{
    QString dt;

    if (dateTime.isDateOnly()) {
        dt = formatDate(dateTime.date(), format);
    } else {
        KDateTime now = KDateTime::currentDateTime(dateTime.timeSpec());
        int daysTo = dateTime.date().daysTo(now.date());
        int secsTo = now.secsTo(dateTime);
        dt = KLocalePrivate::formatDateTime(q, dateTime.dateTime(), format, (options & KLocale::Seconds), daysTo, secsTo);
    }

    if (options & KLocale::TimeZone) {
        QString tz;
        switch (dateTime.timeType()) {
        case KDateTime::OffsetFromUTC:
            tz = i18n(dateTime.toString(QString::fromLatin1("%z")).toUtf8());
            break;
        case KDateTime::UTC:
        case KDateTime::TimeZone:
            tz = i18n(dateTime.toString(QString::fromLatin1((format == KLocale::ShortDate) ? "%Z" : "%:Z")).toUtf8());
            break;
        case KDateTime::ClockTime:
        default:
            break;
        }
        return i18nc("concatenation of date/time and time zone", "%1 %2", dt, tz);
    }

    return dt;
}

QString KLocalePrivate::langLookup(const QString &fname, const char *rtype)
{
    QStringList search;

    // assemble the local search paths
    const QStringList localDoc = KGlobal::dirs()->resourceDirs(rtype);

    // look up the different languages
    for (int id = localDoc.count() - 1; id >= 0; --id) {
        QStringList langs = KGlobal::locale()->languageList();
        // FIXME: KDE 4.5, change such that English is not assumed.
        langs.replaceInStrings(QLatin1String("en_US"), QLatin1String("en"));
        langs.append(QLatin1String("en"));
        Q_FOREACH(const QString &lang, langs)
            search.append(QString::fromLatin1("%1%2/%3").arg(localDoc[id]).arg(lang).arg(fname));
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
    return language() == KLocale::defaultLanguage();
}

void KLocalePrivate::initEncoding()
{
    m_codecForEncoding = 0;

    // This all made more sense when we still had the EncodingEnum config key.

    QByteArray codeset = systemCodeset();

    if (!codeset.isEmpty()) {
        QTextCodec* codec = QTextCodec::codecForName(codeset);
        if (codec) {
            setEncoding(codec->mibEnum());
        }
    } else {
        setEncoding(QTextCodec::codecForLocale()->mibEnum());
    }

    if (!m_codecForEncoding) {
        kWarning() << "Cannot resolve system encoding, defaulting to ISO 8859-1.";
        const int mibDefault = 4; // ISO 8859-1
        setEncoding(mibDefault);
    }

    Q_ASSERT(m_codecForEncoding);
}

QByteArray KLocalePrivate::systemCodeset() const
{
    QByteArray codeset;
#if defined(HAVE_LANGINFO_H)
    // Qt since 4.2 always returns 'System' as codecForLocale and KDE (for example
    // KEncodingFileDialog) expects real encoding name. So on systems that have langinfo.h use
    // nl_langinfo instead, just like Qt compiled without iconv does. Windows already has its own
    // workaround

    codeset = nl_langinfo(CODESET);

    if ((codeset == "ANSI_X3.4-1968") || (codeset == "US-ASCII")) {
        // means ascii, "C"; QTextCodec doesn't know, so avoid warning
        codeset = "ISO-8859-1";
    }
#endif
    return codeset;
}

void KLocalePrivate::initFileNameEncoding()
{
    // If the following environment variable is set, assume all filenames
    // are in UTF-8 regardless of the current C locale.
    m_utf8FileEncoding = !qgetenv("KDE_UTF8_FILENAMES").isEmpty();
    if (m_utf8FileEncoding) {
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

QString KLocalePrivate::decodeFileNameUTF8(const QByteArray &localFileName)
{
    return QString::fromUtf8(localFileName);
}

void KLocalePrivate::setDateFormat(const QString &format)
{
    m_dateFormat = format.trimmed();
}

void KLocalePrivate::setDateFormatShort(const QString &format)
{
    m_dateFormatShort = format.trimmed();
}

void KLocalePrivate::setDateMonthNamePossessive(bool possessive)
{
    m_dateMonthNamePossessive = possessive;
}

void KLocalePrivate::setTimeFormat(const QString &format)
{
    m_timeFormat = format.trimmed();
}

void KLocalePrivate::setWeekStartDay(int day)
{
    if (day >= 1 && day <= calendar()->daysInWeek(QDate())) {
        m_weekStartDay = day;
    }
}

void KLocalePrivate::setWorkingWeekStartDay(int day)
{
    if (day >= 1 && day <= calendar()->daysInWeek(QDate())) {
        m_workingWeekStartDay = day;
    }
}

void KLocalePrivate::setWorkingWeekEndDay(int day)
{
    if (day >= 1 && day <= calendar()->daysInWeek(QDate())) {
        m_workingWeekEndDay = day;
    }
}

void KLocalePrivate::setWeekDayOfPray(int day)
{
    if (day >= 0 && day <= calendar()->daysInWeek(QDate())) { // 0 = None
        m_weekDayOfPray = day;
    }
}

QString KLocalePrivate::dateFormat() const
{
    return m_dateFormat;
}

QString KLocalePrivate::dateFormatShort() const
{
    return m_dateFormatShort;
}

QString KLocalePrivate::timeFormat() const
{
    return m_timeFormat;
}

void KLocalePrivate::setDecimalPlaces(int digits)
{
    m_decimalPlaces = digits;
}

void KLocalePrivate::setDecimalSymbol(const QString &symbol)
{
    m_decimalSymbol = symbol.trimmed();
}

void KLocalePrivate::setThousandsSeparator(const QString &separator)
{
    // allow spaces here
    m_thousandsSeparator = separator;
}

void KLocalePrivate::setPositiveSign(const QString &sign)
{
    m_positiveSign = sign.trimmed();
}

void KLocalePrivate::setNegativeSign(const QString &sign)
{
    m_negativeSign = sign.trimmed();
}

void KLocalePrivate::setPositiveMonetarySignPosition(KLocale::SignPosition signpos)
{
    m_positiveMonetarySignPosition = signpos;
}

void KLocalePrivate::setNegativeMonetarySignPosition(KLocale::SignPosition signpos)
{
    m_negativeMonetarySignPosition = signpos;
}

void KLocalePrivate::setPositivePrefixCurrencySymbol(bool prefix)
{
    m_positivePrefixCurrencySymbol = prefix;
}

void KLocalePrivate::setNegativePrefixCurrencySymbol(bool prefix)
{
    m_negativePrefixCurrencySymbol = prefix;
}

void KLocalePrivate::setMonetaryDecimalPlaces(int digits)
{
    m_monetaryDecimalPlaces = digits;
}

void KLocalePrivate::setMonetaryThousandsSeparator(const QString &separator)
{
    // allow spaces here
    m_monetaryThousandsSeparator = separator;
}

void KLocalePrivate::setMonetaryDecimalSymbol(const QString &symbol)
{
    m_monetaryDecimalSymbol = symbol.trimmed();
}

void KLocalePrivate::setCurrencySymbol(const QString & symbol)
{
    m_currencySymbol = symbol.trimmed();
}

int KLocalePrivate::pageSize() const
{
    return m_pageSize;
}

void KLocalePrivate::setPageSize(int size)
{
    // #### check if it's in range??
    m_pageSize = size;
}

KLocale::MeasureSystem KLocalePrivate::measureSystem() const
{
    return m_measureSystem;
}

void KLocalePrivate::setMeasureSystem(KLocale::MeasureSystem value)
{
    m_measureSystem = value;
}

QString KLocalePrivate::defaultLanguage()
{
    static const QString en_US = QString::fromLatin1("en_US");
    return en_US;
}

QString KLocalePrivate::defaultCountry()
{
    return QString::fromLatin1("C");
}

QString KLocalePrivate::defaultCurrencyCode()
{
    return QString::fromLatin1("USD");
}

bool KLocalePrivate::useTranscript() const
{
    return m_useTranscript;
}

const QByteArray KLocalePrivate::encoding()
{
    return codecForEncoding()->name();
}

int KLocalePrivate::encodingMib() const
{
    return codecForEncoding()->mibEnum();
}

int KLocalePrivate::fileEncodingMib() const
{
    if (m_utf8FileEncoding) {
        return 106;
    }
    return codecForEncoding()->mibEnum();
}

QTextCodec *KLocalePrivate::codecForEncoding() const
{
    return m_codecForEncoding;
}

bool KLocalePrivate::setEncoding(int mibEnum)
{
    QTextCodec * codec = QTextCodec::codecForMib(mibEnum);
    if (codec) {
        m_codecForEncoding = codec;
    }

    return codec != 0;
}

QStringList KLocalePrivate::allLanguagesList()
{
    if (!m_languages) {
        m_languages = new KConfig(QLatin1String("all_languages"), KConfig::NoGlobals, "locale");
    }
    return m_languages->groupList();
}

QStringList KLocalePrivate::installedLanguages()
{
    QStringList languages;
    QStringList paths = KGlobal::dirs()->findAllResources("locale", QLatin1String("*/entry.desktop"));
    foreach (const QString &path, paths) {
        QString part = path.left(path.length() - 14);
        languages.append(part.mid(part.lastIndexOf(QLatin1Char('/')) + 1));
    }
    languages.sort();
    return languages;
}

QString KLocalePrivate::languageCodeToName(const QString &language)
{
    if (!m_languages) {
        m_languages = new KConfig(QLatin1String("all_languages"), KConfig::NoGlobals, "locale");
    }

    KConfigGroup cg(m_languages, language);
    return cg.readEntry("Name");
}

QStringList KLocalePrivate::allCountriesList() const
{
    QStringList countries;
    const QStringList paths = KGlobal::dirs()->findAllResources("locale", QLatin1String("l10n/*/entry.desktop"));
    for (QStringList::ConstIterator it = paths.begin(); it != paths.end(); ++it) {
        QString code = (*it).mid((*it).length() - 16, 2);
        if (code != QLatin1String("/C")) {
            countries.append(code);
        }
    }
    return countries;
}

QString KLocalePrivate::countryCodeToName(const QString &country) const
{
    QString countryName;
    QString entryFile = KStandardDirs::locate("locale", QString::fromLatin1("l10n/") + country.toLower() + QLatin1String("/entry.desktop"));
    if (!entryFile.isEmpty()) {
        KConfig cfg(entryFile);
        KConfigGroup cg(&cfg, "KCM Locale");
        countryName = cg.readEntry("Name");
    }
    return countryName;
}

KLocale::CalendarSystem KLocalePrivate::calendarTypeToCalendarSystem(const QString &calendarType) const
{
    if (calendarType == QLatin1String("coptic")) {
        return KLocale::CopticCalendar;
    } else if (calendarType == QLatin1String("ethiopian")) {
        return KLocale::EthiopianCalendar;
    } else if (calendarType == QLatin1String("gregorian")) {
        return KLocale::QDateCalendar;
    } else if (calendarType == QLatin1String("gregorian-proleptic")) {
        return KLocale::GregorianCalendar;
    } else if (calendarType == QLatin1String("hebrew")) {
        return KLocale::HebrewCalendar;
    } else if (calendarType == QLatin1String("hijri")) {
        return KLocale::IslamicCivilCalendar;
    } else if (calendarType == QLatin1String("indian-national")) {
        return KLocale::IndianNationalCalendar;
    } else if (calendarType == QLatin1String("jalali")) {
        return KLocale::JalaliCalendar;
    } else if (calendarType == QLatin1String("japanese")) {
        return KLocale::JapaneseCalendar;
    } else if (calendarType == QLatin1String("julian")) {
        return KLocale::JulianCalendar;
    } else if (calendarType == QLatin1String("minguo")) {
        return KLocale::MinguoCalendar;
    } else if (calendarType == QLatin1String("thai")) {
        return KLocale::ThaiCalendar;
    } else {
        return KLocale::QDateCalendar;
    }
}

QString KLocalePrivate::calendarSystemToCalendarType(KLocale::CalendarSystem calendarSystem) const
{
    switch (calendarSystem) {
    case KLocale::QDateCalendar:
        return QLatin1String("gregorian");
    case KLocale::CopticCalendar:
        return QLatin1String("coptic");
    case KLocale::EthiopianCalendar:
        return QLatin1String("ethiopian");
    case KLocale::GregorianCalendar:
        return QLatin1String("gregorian-proleptic");
    case KLocale::HebrewCalendar:
        return QLatin1String("hebrew");
    case KLocale::IslamicCivilCalendar:
        return QLatin1String("hijri");
    case KLocale::IndianNationalCalendar:
        return QLatin1String("indian-national");
    case KLocale::JalaliCalendar:
        return QLatin1String("jalali");
    case KLocale::JapaneseCalendar:
        return QLatin1String("japanese");
    case KLocale::JulianCalendar:
        return QLatin1String("julian");
    case KLocale::MinguoCalendar:
        return QLatin1String("minguo");
    case KLocale::ThaiCalendar:
        return QLatin1String("thai");
    default:
        return QLatin1String("gregorian");
    }
}

void KLocalePrivate::setCalendar(const QString &calendarType)
{
    setCalendarSystem(calendarTypeToCalendarSystem(calendarType));
}

void KLocalePrivate::setCalendarSystem(KLocale::CalendarSystem calendarSystem)
{
    m_calendarSystem = calendarSystem;
    delete m_calendar;
    m_calendar = 0;
}

QString KLocalePrivate::calendarType() const
{
    return calendarSystemToCalendarType(m_calendarSystem);
}

KLocale::CalendarSystem KLocalePrivate::calendarSystem() const
{
    return m_calendarSystem;
}

const KCalendarSystem * KLocalePrivate::calendar()
{
    if (!m_calendar) {
        m_calendar = KCalendarSystem::create(m_calendarSystem, m_config, q);
    }

    return m_calendar;
}

void KLocalePrivate::setWeekNumberSystem(KLocale::WeekNumberSystem weekNumberSystem)
{
    m_weekNumberSystem = weekNumberSystem;
}

KLocale::WeekNumberSystem KLocalePrivate::weekNumberSystem()
{
    return m_weekNumberSystem;
}

void KLocalePrivate::copyCatalogsTo(KLocale *locale)
{
    QMutexLocker lock(kLocaleMutex());
    locale->d->m_catalogNames = m_catalogNames;
    locale->d->updateCatalogs();
}

QString KLocalePrivate::localizedFilePath(const QString &filePath) const
{
    // Stop here if the default language is primary.
    if (useDefaultLanguage()) {
        return filePath;
    }

    // Check if l10n sudir is present, stop if not.
    QFileInfo fileInfo(filePath);
    QString locDirPath = fileInfo.path() + QLatin1String("/l10n");
    QFileInfo locDirInfo(locDirPath);
    if (!locDirInfo.isDir()) {
        return filePath;
    }

    // Go through possible localized paths by priority of languages,
    // return first that exists.
    QString fileName = fileInfo.fileName();
    foreach(const QString &lang, languageList()) {
        // Stop when the default language is reached.
        if (lang == KLocale::defaultLanguage()) {
            return filePath;
        }
        QString locFilePath = locDirPath + QLatin1Char('/') + lang + QLatin1Char('/') + fileName;
        QFileInfo locFileInfo(locFilePath);
        if (locFileInfo.isFile() && locFileInfo.isReadable()) {
            return locFilePath;
        }
    }

    return filePath;
}

QString KLocalePrivate::removeAcceleratorMarker(const QString &label) const
{
    return ::removeAcceleratorMarker(label);
}

void KLocalePrivate::setDigitSet(KLocale::DigitSet digitSet)
{
    m_digitSet = digitSet;
}

KLocale::DigitSet KLocalePrivate::digitSet() const
{
    return m_digitSet;
}

void KLocalePrivate::setMonetaryDigitSet(KLocale::DigitSet digitSet)
{
    m_monetaryDigitSet = digitSet;
}

KLocale::DigitSet KLocalePrivate::monetaryDigitSet() const
{
    return m_monetaryDigitSet;
}

void KLocalePrivate::setDateTimeDigitSet(KLocale::DigitSet digitSet)
{
    m_dateTimeDigitSet = digitSet;
}

KLocale::DigitSet KLocalePrivate::dateTimeDigitSet() const
{
    return m_dateTimeDigitSet;
}

Q_GLOBAL_STATIC_WITH_ARGS(QMutex, s_kLocaleMutex, (QMutex::Recursive))

QMutex *kLocaleMutex()
{
    return s_kLocaleMutex();
}
