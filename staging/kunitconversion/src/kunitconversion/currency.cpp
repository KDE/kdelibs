/*
 *   Copyright (C) 2009 Petri Damstén <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "currency.h"

#include "converter.h"
#include <QLocale>
#include <QtCore/QFileInfo>
#include <QtCore/QDateTime>
#include <QtCore/QMutex>
#include <QtCore/QProcess>
#include <QtXml/QDomDocument>
#include <QDebug>
#include <klocalizedstring.h>

#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <kcurrencycode.h>
#include <qstandardpaths.h>

using namespace KUnitConversion;

static const char URL[] = "http://www.ecb.europa.eu/stats/eurofxref/eurofxref-daily.xml";

Currency::Currency() : UnitCategory(CurrencyCategory)
{
    setName(i18n("Currency"));
    setDescription(i18n("From ECB"));
    setUrl(QUrl("http://www.ecb.int/stats/exchange/eurofxref/html/index.en.html"));
    setSymbolStringFormat(ki18nc("%1 value, %2 unit symbol (currency)", "%1 %2"));

    // Static rates
    setDefaultUnit(UP(Eur, 1,
      "EUR",
      KCurrencyCode::currencyCodeToName("EUR"),
      i18nc("EUR Euro - unit synonyms for matching user input", "euro;euros") +
          ";EUR;" + QString::fromUtf8("€") + ';' + KCurrencyCode::currencyCodeToName("EUR"),
      ki18nc("amount in units (real)", "%1 euros"),
      ki18ncp("amount in units (integer)", "%1 euro", "%1 euros")
    ));
    U(Ats, 0.0726728,
      "ATS",
      KCurrencyCode::currencyCodeToName("ATS"),
      i18nc("ATS Austrian Schilling - unit synonyms for matching user input",
          "schilling;schillings") + ";ATS;" + QLocale::countryToString(QLocale::Austria) +';' +
          KCurrencyCode::currencyCodeToName("ATS"),
      ki18nc("amount in units (real)", "%1 schillings"),
      ki18ncp("amount in units (integer)", "%1 schilling", "%1 schillings")
    );
    U(Bef, 0.0247894,
      "BEF",
      KCurrencyCode::currencyCodeToName("BEF"),
      i18nc("BEF Belgian Franc - unit synonyms for matching user input", "franc;francs") +
          ";BEF;" + QLocale::countryToString(QLocale::Belgium) + ';' +
          KCurrencyCode::currencyCodeToName("BEF"),
      ki18nc("amount in units (real)", "%1 Belgian francs"),
      ki18ncp("amount in units (integer)", "%1 Belgian franc", "%1 Belgian francs")
    );
    U(Nlg, 0.45378,
      "NLG",
      KCurrencyCode::currencyCodeToName("NLG"),
      i18nc("NLG Netherlands Guilder - unit synonyms for matching user input", "guilder;guilders") +
          ";NLG;" + QLocale::countryToString(QLocale::Netherlands) + ';' +
          KCurrencyCode::currencyCodeToName("NLG"),
      ki18nc("amount in units (real)", "%1 guilders"),
      ki18ncp("amount in units (integer)", "%1 guilder", "%1 guilders")
    );
    U(Fim, 0.168188,
      "FIM",
      KCurrencyCode::currencyCodeToName("FIM"),
      i18nc("FIM Finnish Markka - unit synonyms for matching user input",
            "markka;markkas;markkaa") + ";FIM;" +
            QLocale::countryToString(QLocale::Finland) + ';' +
            KCurrencyCode::currencyCodeToName("FIM"),
      ki18nc("amount in units (real)", "%1 markkas"),
      ki18ncp("amount in units (integer)", "%1 markka", "%1 markkas")  // Alternative = markkaa
    );
    U(Frf, 0.152449,
      "FRF",
      KCurrencyCode::currencyCodeToName("FRF"),
      i18nc("FRF French Franc - unit synonyms for matching user input", "franc;francs") + ";FRF;" +
          QLocale::countryToString(QLocale::France) + ';' +
          KCurrencyCode::currencyCodeToName("FRF"),
      ki18nc("amount in units (real)", "%1 French francs"),
      ki18ncp("amount in units (integer)", "%1 French franc", "%1 French francs")
    );
    U(Dem, 0.511292,
      "DEM",
      KCurrencyCode::currencyCodeToName("DEM"),
      i18nc("DEM German Mark - unit synonyms for matching user input", "mark;marks") + ";DEM;" +
          QLocale::countryToString(QLocale::Germany) + ';' +
          KCurrencyCode::currencyCodeToName("DEM"),
      ki18nc("amount in units (real)", "%1 marks"),
      ki18ncp("amount in units (integer)", "%1 mark", "%1 marks")
    );
    U(Iep, 1.26974,
      "IEP",
      KCurrencyCode::currencyCodeToName("IEP"),
      i18nc("IEP Irish Pound - unit synonyms for matching user input", "Irish pound;Irish pounds") +
          ";IEP;" + QLocale::countryToString(QLocale::Ireland) + ';' +
          KCurrencyCode::currencyCodeToName("IEP"),
      ki18nc("amount in units (real)", "%1 Irish pounds"),
      ki18ncp("amount in units (integer)", "%1 Irish pound", "%1 Irish pounds")
    );
    U(Itl, 0.000516457,
      "ITL",
      KCurrencyCode::currencyCodeToName("ITL"),
      i18nc("ITL Italian Lira - unit synonyms for matching user input", "lira;liras") + ";ITL;" +
          QLocale::countryToString(QLocale::Italy) + ';' +
          KCurrencyCode::currencyCodeToName("ITL"),
      ki18nc("amount in units (real)", "%1 Italian lira"),
      ki18ncp("amount in units (integer)", "%1 Italian lira", "%1 Italian lira")
    );
    U(Luf, 0.0247894,
      "LUF",
      KCurrencyCode::currencyCodeToName("LUF"),
      i18nc("LUF Luxembourgish Franc - unit synonyms for matching user input", "franc;francs") +
          ";LUF;" + QLocale::countryToString(QLocale::Luxembourg) + ';' +
          KCurrencyCode::currencyCodeToName("LUF"),
      ki18nc("amount in units (real)", "%1 Luxembourgish francs"),
      ki18ncp("amount in units (integer)", "%1 Luxembourgish franc", "%1 Luxembourgish francs")
    );
    U(Pte, 0.00498798,
      "PTE",
      KCurrencyCode::currencyCodeToName("PTE"),
      i18nc("PTE Portugeuse Escudo - unit synonyms for matching user input", "escudo;escudos") +
          ";PTE;" + QLocale::countryToString(QLocale::Portugal) + ';' +
          KCurrencyCode::currencyCodeToName("PTE"),
      ki18nc("amount in units (real)", "%1 escudos"),
      ki18ncp("amount in units (integer)", "%1 escudo", "%1 escudos")
    );
    U(Esp, 0.00601012,
      "ESP",
      KCurrencyCode::currencyCodeToName("ESP"),
      i18nc("ESP Spanish Pesetas - unit synonyms for matching user input", "peseta;pesetas") +
          ";ESP;" + QLocale::countryToString(QLocale::Spain) + ';' +
          KCurrencyCode::currencyCodeToName("ESP"),
      ki18nc("amount in units (real)", "%1 pesetas"),
      ki18ncp("amount in units (integer)", "%1 peseta", "%1 pesetas")
    );
    U(Grd, 0.0029347,
      "GRD",
      KCurrencyCode::currencyCodeToName("GRD"),
      i18nc("GRD Greek Drachma - unit synonyms for matching user input", "drachma;drachmas") +
          ";GRD;" + QLocale::countryToString(QLocale::Greece) + ';' +
          KCurrencyCode::currencyCodeToName("GRD"),
      ki18nc("amount in units (real)", "%1 drachmas"),
      ki18ncp("amount in units (integer)", "%1 drachma", "%1 drachmas")
    );
    U(Sit, 0.00417293,
      "SIT",
      KCurrencyCode::currencyCodeToName("SIT"),
      i18nc("SIT Slovenian Tolar - unit synonyms for matching user input", "tolar;tolars;tolarjev") + ";SIT;" +
      QLocale::countryToString(QLocale::Slovenia) + ';' + KCurrencyCode::currencyCodeToName("SIT"),
      ki18nc("amount in units (real)", "%1 tolars"),
      ki18ncp("amount in units (integer)", "%1 tolar", "%1 tolars")  //Alt: tolarjev/tolarja/tolarji
    );
    U(Cyp, 1.7086,
      "CYP",
      KCurrencyCode::currencyCodeToName("CYP"),
      i18nc("CYP Cypriot Pound - unit synonyms for matching user input",
          "Cypriot pound;Cypriot pounds") + ";CYP;" +
          QLocale::countryToString(QLocale::Cyprus) + ';' +
          KCurrencyCode::currencyCodeToName("CYP"),
      ki18nc("amount in units (real)", "%1 Cypriot pounds"),
      ki18ncp("amount in units (integer)", "%1 Cypriot pound", "%1 Cypriot pounds")
    );
    U(Mtl, 2.32937,
      "MTL",
      KCurrencyCode::currencyCodeToName("MTL"),
      i18nc("MTL Maltese Lira - unit synonyms for matching user input", "Maltese lira") + ";MTL;" +
      QLocale::countryToString(QLocale::Malta) + ';' + KCurrencyCode::currencyCodeToName("MTL"),
      ki18nc("amount in units (real)", "%1 Maltese lira"),
      ki18ncp("amount in units (integer)", "%1 Maltese lira", "%1 Maltese lira")
    );
    U(Skk, 0.0331939,
      "SKK",
      KCurrencyCode::currencyCodeToName("SKK"),
      i18nc("SKK Slovak Koruna - unit synonyms for matching user input",
          "koruna;korunas;koruny;korun") + ";SKK;" +
          QLocale::countryToString(QLocale::Slovakia) + ';' +
          KCurrencyCode::currencyCodeToName("SKK"),
      ki18nc("amount in units (real)", "%1 Slovak korunas"),
      ki18ncp("amount in units (integer)", "%1 Slovak koruna", "%1 Slovak korunas") // Alt: koruny, korun
    );

    // From ECB
    U(Usd, 1e+99,
      "USD",
      KCurrencyCode::currencyCodeToName("USD"),
      i18nc("USD United States Dollars - unit synonyms for matching user input", "dollar;dollars") +
          ";USD;$;" + QLocale::countryToString(QLocale::UnitedStates) + ';' +
          KCurrencyCode::currencyCodeToName("USD"),
      ki18nc("amount in units (real)", "%1 United States dollars"),
      ki18ncp("amount in units (integer)", "%1 United States dollar", "%1 United States dollars")
    );
    U(Jpy, 1e+99,
      "JPY",
      KCurrencyCode::currencyCodeToName("JPY"),
      i18nc("JPY Japanese Yen - unit synonyms for matching user input", "yen") +
      ";JPY;" + QString::fromUtf8("¥") + ';' +
      QLocale::countryToString(QLocale::Japan) + ';' + KCurrencyCode::currencyCodeToName("JPY"),
      ki18nc("amount in units (real)", "%1 yen"),
      ki18ncp("amount in units (integer)", "%1 yen", "%1 yen")
    );
    U(Bgn, 1e+99,
      "BGN",
      KCurrencyCode::currencyCodeToName("BGN"),
      i18nc("BGN Bulgarian Lev - unit synonyms for matching user input", "lev;leva") + ";BGN;" +
      QLocale::countryToString(QLocale::Bulgaria) + ';' + KCurrencyCode::currencyCodeToName("BGN"),
      ki18nc("amount in units (real)", "%1 leva"),
      ki18ncp("amount in units (integer)", "%1 lev", "%1 leva")
    );
    U(Czk, 1e+99,
      "CZK",
      KCurrencyCode::currencyCodeToName("CZK"),
      i18nc("CZK Czech Koruna - unit synonyms for matching user input", "koruna;korunas") +
          ";CZK;" + QLocale::countryToString(QLocale::CzechRepublic) + ';' +
          KCurrencyCode::currencyCodeToName("CZK"),
      ki18nc("amount in units (real)", "%1 Czech korunas"),
      // Alt: koruny, korun
      ki18ncp("amount in units (integer)", "%1 Czech koruna", "%1 Czech korunas")
    );
    U(Dkk, 1e+99,
      "DKK",
      KCurrencyCode::currencyCodeToName("DKK"),
      i18nc("DKK Danish Krone - unit synonyms for matching user input",
          "Danish krone;Danish kroner") + ";DKK;" +
          QLocale::countryToString(QLocale::Denmark) + ';' +
          KCurrencyCode::currencyCodeToName("DKK"),
      ki18nc("amount in units (real)", "%1 Danish kroner"),
      ki18ncp("amount in units (integer)", "%1 Danish krone", "%1 Danish kroner")
    );
    U(Eek, 1e+99,
      "EEK",
      KCurrencyCode::currencyCodeToName("EEK"),
      i18nc("EEK Estonian Kroon - unit synonyms for matching user input", "kroon;kroons;krooni") +
          ";EEK;" + QLocale::countryToString(QLocale::Estonia) + ';' +
          KCurrencyCode::currencyCodeToName("EEK"),
      ki18nc("amount in units (real)", "%1 kroons"),
      ki18ncp("amount in units (integer)", "%1 kroon", "%1 kroons") // Alt: krooni
    );
    U(Gbp, 1e+99,
      "GBP",
      KCurrencyCode::currencyCodeToName("GBP"),
      i18nc("GBP British Pound - unit synonyms for matching user input",
          "pound;pounds;pound sterling;pounds sterling") +
          ";GBP;" + QString::fromUtf8("£") + ';' +
          QLocale::countryToString(QLocale::UnitedKingdom) + ';' +
          KCurrencyCode::currencyCodeToName("GBP"),
      ki18nc("amount in units (real)", "%1 pounds sterling"),
      ki18ncp("amount in units (integer)", "%1 pound sterling", "%1 pounds sterling")
    );
    U(Huf, 1e+99,
      "HUF",
      KCurrencyCode::currencyCodeToName("HUF"),
      i18nc("HUF hungarian Forint - unit synonyms for matching user input", "forint") + ";HUF;" +
          QLocale::countryToString(QLocale::Hungary) + ';' +
          KCurrencyCode::currencyCodeToName("HUF"),
      ki18nc("amount in units (real)", "%1 forint"),
      ki18ncp("amount in units (integer)", "%1 forint", "%1 forint")
    );
    U(Ltl, 1e+99,
      "LTL",
      KCurrencyCode::currencyCodeToName("LTL"),
      i18nc("LTL Lithuanian Litas - unit synonyms for matching user input", "litas;litai;litu") +
          ";LTL;" + QLocale::countryToString(QLocale::Lithuania) + ';' +
          KCurrencyCode::currencyCodeToName("LTL"),
      ki18nc("amount in units (real)", "%1 litas"),
      ki18ncp("amount in units (integer)", "%1 litas", "%1 litai") // Alt: litu
    );
    U(Lvl, 1e+99,
      "LVL",
      KCurrencyCode::currencyCodeToName("LVL"),
      i18nc("LVL Latvian Lats - unit synonyms for matching user input", "lats;lati") + ";LVL;" +
      QLocale::countryToString(QLocale::Latvia) + ';' + KCurrencyCode::currencyCodeToName("LVL"),
      ki18nc("amount in units (real)", "%1 lati"),
      ki18ncp("amount in units (integer)", "%1 lats", "%1 lati")
    );
    U(Pln, 1e+99,
      "PLN",
      KCurrencyCode::currencyCodeToName("PLN"),
      i18nc("PLN Polish Zloty - unit synonyms for matching user input", "zloty;zlotys;zloties") +
          ";PLN;" + QLocale::countryToString(QLocale::Poland) + ';' +
          KCurrencyCode::currencyCodeToName("PLN"),
      ki18nc("amount in units (real)", "%1 zlotys"),
      ki18ncp("amount in units (integer)", "%1 zloty", "%1 zlotys") // Alt: zloty, zlote, zlotych
    );
    U(Ron, 1e+99,
      "RON",
      KCurrencyCode::currencyCodeToName("RON"),
      i18nc("RON Roumanian Leu - unit synonyms for matching user input", "leu;lei") + ";RON;" +
          QLocale::countryToString(QLocale::Romania) + ';' +
          KCurrencyCode::currencyCodeToName("RON"),
      ki18nc("amount in units (real)", "%1 lei"),
      ki18ncp("amount in units (integer)", "%1 leu", "%1 lei")
    );
    U(Sek, 1e+99,
      "SEK",
      KCurrencyCode::currencyCodeToName("SEK"),
      i18nc("SEK Swedish Krona - unit synonyms for matching user input", "krona;kronor") + ";SEK;" +
          QLocale::countryToString(QLocale::Sweden) + ';' +
          KCurrencyCode::currencyCodeToName("SEK"),
      ki18nc("amount in units (real)", "%1 kronor"),
      ki18ncp("amount in units (integer)", "%1 krona", "%1 kronor")
    );
    U(Chf, 1e+99,
      "CHF",
      KCurrencyCode::currencyCodeToName("CHF"),
      i18nc("CHF Swiss Francs - unit synonyms for matching user input", "franc;francs") + ";CHF;" +
          QLocale::countryToString(QLocale::Switzerland) + ';' +
          KCurrencyCode::currencyCodeToName("CHF"),
      ki18nc("amount in units (real)", "%1 Swiss francs"),
      ki18ncp("amount in units (integer)", "%1 Swiss franc", "%1 Swiss francs")
    );
    U(Nok, 1e+99,
      "NOK",
      KCurrencyCode::currencyCodeToName("NOK"),
      i18nc("Norwegian Krone - unit synonyms for matching user input",
          "Norwegian krone;Norwegian kroner") + ";NOK;" +
          QLocale::countryToString(QLocale::Norway) + ';' +
          KCurrencyCode::currencyCodeToName("NOK"),
      ki18nc("amount in units (real)", "%1 Norwegian kroner"),
      ki18ncp("amount in units (integer)", "%1 Norwegian krone", "%1 Norwegian kroner")
    );
    U(Hrk, 1e+99,
      "HRK",
      KCurrencyCode::currencyCodeToName("HRK"),
      i18nc("HRK Croatian Kuna - unit synonyms for matching user input", "kuna;kune") + ";HRK;" +
          QLocale::countryToString(QLocale::Croatia) + ';' +
          KCurrencyCode::currencyCodeToName("HRK"),
      ki18nc("amount in units (real)", "%1 kune"),
      ki18ncp("amount in units (integer)", "%1 kuna", "%1 kune")
    );
    U(Rub, 1e+99,
       "RUB",
      KCurrencyCode::currencyCodeToName("RUB"),
      i18nc("RUB Russsian Ruble - unit synonyms for matching user input",
          "ruble;rubles;rouble;roubles") + ";RUB;" +
          QLocale::countryToString(QLocale::Russia) + ';' +
          KCurrencyCode::currencyCodeToName("RUB"),
      ki18nc("amount in units (real)", "%1 rubles"),
      ki18ncp("amount in units (integer)", "%1 ruble", "%1 rubles") //Alt: rouble/roubles
    );
    U(Try, 1e+99,
      "TRY",
      KCurrencyCode::currencyCodeToName("TRY"),
      i18nc("TRY Turkish Lira - unit synonyms for matching user input", "lira") + ";TRY;" +
          QLocale::countryToString(QLocale::Turkey) + ';' +
          KCurrencyCode::currencyCodeToName("TRY"),
      ki18nc("amount in units (real)", "%1 Turkish lira"),
      ki18ncp("amount in units (integer)", "%1 Turkish lira", "%1 Turkish lira")
    );
    U(Aud, 1e+99,
      "AUD",
      KCurrencyCode::currencyCodeToName("AUD"),
      i18nc("AUD Australian Dollar - unit synonyms for matching user input",
          "Australian dollar;Australian dollars") + ";AUD;" +
          QLocale::countryToString(QLocale::Australia) + ';' +
          KCurrencyCode::currencyCodeToName("AUD"),
      ki18nc("amount in units (real)", "%1 Australian dollars"),
      ki18ncp("amount in units (integer)", "%1 Australian dollar", "%1 Australian dollars")
    );
    U(Brl, 1e+99,
      "BRL",
      KCurrencyCode::currencyCodeToName("BRL"),
      i18nc("BRL Brazillian Real - unit synonyms for matching user input", "real;reais") +
          ";BRL;" + QLocale::countryToString(QLocale::Brazil) + ';' +
          KCurrencyCode::currencyCodeToName("BRL"),
      ki18nc("amount in units (real)", "%1 reais"),
      ki18ncp("amount in units (integer)", "%1 real", "%1 reais")
    );
    U(Cad, 1e+99,
      "CAD",
      KCurrencyCode::currencyCodeToName("CAD"),
      i18nc("Canadian Dollar - unit synonyms for matching user input",
          "Canadian dollar;Canadian dollars") + ";CAD;" +
          QLocale::countryToString(QLocale::Canada) + ';' +
          KCurrencyCode::currencyCodeToName("CAD"),
      ki18nc("amount in units (real)", "%1 Canadian dollars"),
      ki18ncp("amount in units (integer)", "%1 Canadian dollar", "%1 Canadian dollars")
    );
    U(Cny, 1e+99,
      "CNY",
      KCurrencyCode::currencyCodeToName("CNY"),
      i18nc("Chinese Yuan - unit synonyms for matching user input", "yuan") + ";CNY;" +
          QLocale::countryToString(QLocale::China) + ';' +
          KCurrencyCode::currencyCodeToName("CNY"),
      ki18nc("amount in units (real)", "%1 yuan"),
      ki18ncp("amount in units (integer)", "%1 yuan", "%1 yuan")
    );
    U(Hkd, 1e+99,
      "HKD",
      KCurrencyCode::currencyCodeToName("HKD"),
      i18nc("Hong Kong Dollar - unit synonyms for matching user input",
          "Hong Kong dollar;Hong Kong dollars") + ";HKD;" +
          QLocale::countryToString(QLocale::HongKong) + ';' +
          KCurrencyCode::currencyCodeToName("HKD"),
      ki18nc("amount in units (real)", "%1 Hong Kong dollars"),
      ki18ncp("amount in units (integer)", "%1 Hong Kong dollar", "%1 Hong Kong dollars")
    );
    U(Idr, 1e+99,
      "IDR",
      KCurrencyCode::currencyCodeToName("IDR"),
      i18nc("IDR Indonesian Rupiah - unit synonyms for matching user input", "rupiah;rupiahs") +
          ";IDR;" + QLocale::countryToString(QLocale::Indonesia) + ';' +
          KCurrencyCode::currencyCodeToName("IDR"),
      ki18nc("amount in units (real)", "%1 rupiahs"),
      ki18ncp("amount in units (integer)", "%1 rupiah", "%1 rupiahs") // Alt: rupiah
    );
    U(Inr, 1e+99,
      "INR",
      KCurrencyCode::currencyCodeToName("INR"),
      i18nc("INR Indian Rupee - unit synonyms for matching user input", "rupee;rupees") + ";INR;" +
          QLocale::countryToString(QLocale::India) + ';' +
          KCurrencyCode::currencyCodeToName("INR"),
      ki18nc("amount in units (real)", "%1 rupees"),
      ki18ncp("amount in units (integer)", "%1 rupee", "%1 rupees") // Alt: rupee
    );
    U(Krw, 1e+99,
      "KRW",
      KCurrencyCode::currencyCodeToName("KRW"),
      i18nc("KRW Korean Won - unit synonyms for matching user input", "won") + ";KRW;" +
          QLocale::countryToString(QLocale::SouthKorea) + ';' +
          KCurrencyCode::currencyCodeToName("KRW"),
      ki18nc("amount in units (real)", "%1 won"),
      ki18ncp("amount in units (integer)", "%1 won", "%1 won")
    );
    U(Mxn, 1e+99,
      "MXN",
      KCurrencyCode::currencyCodeToName("MXN"),
      i18nc("MXN Mexican Peso - unit synonyms for matching user input",
          "Mexican peso;Mexican pesos") + ";MXN;" +
          QLocale::countryToString(QLocale::Mexico) + ';' +
          KCurrencyCode::currencyCodeToName("MXN"),
      ki18nc("amount in units (real)", "%1 Mexican pesos"),
      ki18ncp("amount in units (integer)", "%1 Mexican peso", "%1 Mexican pesos")
    );
    U(Myr, 1e+99,
      "MYR",
      KCurrencyCode::currencyCodeToName("MYR"),
      i18nc("MYR Malasian Ringgit - unit synonyms for matching user input", "ringgit;ringgits") +
          ";MYR;" + QLocale::countryToString(QLocale::Malaysia) + ';' +
          KCurrencyCode::currencyCodeToName("MYR"),
      ki18nc("amount in units (real)", "%1 ringgit"),
      ki18ncp("amount in units (integer)", "%1 ringgit", "%1 ringgit") // Alt: ringgits
    );
    U(Nzd, 1e+99,
      "NZD",
      KCurrencyCode::currencyCodeToName("NZD"),
      i18nc("NZD New Zealand Dollar - unit synonyms for matching user input",
          "New Zealand dollar;New Zealand dollars") + ";NZD;" +
          QLocale::countryToString(QLocale::NewZealand) + ';' +
          KCurrencyCode::currencyCodeToName("NZD"),
      ki18nc("amount in units (real)", "%1 New Zealand dollars"),
      ki18ncp("amount in units (integer)", "%1 New Zealand dollar", "%1 New Zealand dollars")
    );
    U(Php, 1e+99,
      "PHP",
      KCurrencyCode::currencyCodeToName("PHP"),
      i18nc("PHP Philippine Peso - unit synonyms for matching user input",
          "Philippine peso;Philippine pesos") + ";PHP;" +
          QLocale::countryToString(QLocale::Philippines) + ';' +
          KCurrencyCode::currencyCodeToName("PHP"),
      ki18nc("amount in units (real)", "%1 Philippine pesos"),
      ki18ncp("amount in units (integer)", "%1 Philippine peso", "%1 Philippine pesos")
    );
    U(Sgd, 1e+99,
      "SGD",
      KCurrencyCode::currencyCodeToName("SGD"),
      i18nc("SGD Singapore Dollar - unit synonyms for matching user input",
          "Singapore dollar;Singapore dollars") + ";SGD;" +
          QLocale::countryToString(QLocale::Singapore) + ';' +
          KCurrencyCode::currencyCodeToName("SGD"),
      ki18nc("amount in units (real)", "%1 Singapore dollars"),
      ki18ncp("amount in units (integer)", "%1 Singapore dollar", "%1 Singapore dollars")
    );
    U(Thb, 1e+99,
      "THB",
      KCurrencyCode::currencyCodeToName("THB"),
      i18nc("THB Thai Baht - unit synonyms for matching user input", "baht") + ";THB;" +
          QLocale::countryToString(QLocale::Thailand) + ';' +
          KCurrencyCode::currencyCodeToName("THB"),
      ki18nc("amount in units (real)", "%1 baht"),
      ki18ncp("amount in units (integer)", "%1 baht", "%1 baht")
    );
    U(Zar, 1e+99,
      "ZAR",
      KCurrencyCode::currencyCodeToName("ZAR"),
      i18nc("South African Rand - unit synonyms for matching user input", "rand") + ";ZAR;" +
          QLocale::countryToString(QLocale::SouthAfrica) + ';' +
          KCurrencyCode::currencyCodeToName("ZAR"),
      ki18nc("amount in units (real)", "%1 rand"),
      ki18ncp("amount in units (integer)", "%1 rand", "%1 rand")
    );

    setMostCommonUnits(QList<int>() << Eur << Usd << Jpy << Gbp << Cad);

    m_cache = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/libkunitconversion/currency.xml");
    m_update = true;
}

static bool isConnected()
{
    bool ret = false;
    foreach (const QNetworkInterface& net, QNetworkInterface::allInterfaces()) {
        if (net.flags().testFlag(QNetworkInterface::IsUp) && !net.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            ret = true;
            break;
        }
    }
    return ret;
}

Value Currency::convert(const Value& value, UnitPtr to)
{
    static QMutex mutex;

    mutex.lock();
    QFileInfo info(m_cache);
    if (!info.exists() || info.lastModified().secsTo(QDateTime::currentDateTime()) > 86400) {
        if (isConnected()) {
            //qDebug() << "Getting currency info from net:" << URL;
            // TODO: This crashes in runner. Threading issues??
            /*
            KIO::Job* job = KIO::file_copy(QUrl(URL), QUrl::fromLocalFile(m_cache), -1,
                                           KIO::Overwrite | KIO::HideProgressInfo);
            job->setUiDelegate(0);
            if (job->exec()) {
                m_update = true;
            }
            */
            //qDebug() << "Removed previous cache:" << QFile::remove(m_cache);

            QNetworkAccessManager manager;
            QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(URL)));
            QFile cacheFile(m_cache);
            cacheFile.open(QFile::WriteOnly);
            while (!reply->error() && !reply->atEnd()) {
                if (reply->bytesAvailable()>0
                 || reply->waitForReadyRead(500)) {
                    cacheFile.write(reply->readAll());
                }
            }
            cacheFile.close();
        }
    }
    mutex.unlock();

    if (m_update) {
        QFile file(m_cache);
        if (file.open(QIODevice::ReadOnly)) {
            QDomDocument doc;
            if (doc.setContent(&file, false) ) {
                QDomNodeList list = doc.elementsByTagName("Cube");
                for (int i = 0; i < list.count(); ++i) {
                    const QDomElement e = list.item(i).toElement();
                    if (e.hasAttribute("currency")) {
                        UnitPtr u = unit(e.attribute("currency"));
                        if (u) {
                            u->setMultiplier(1.0 / e.attribute("rate").toDouble());
                        }
                    }
                }
                m_update = false;
            }
        }
    }
    Value v = UnitCategory::convert(value, to);
    return v;
}
