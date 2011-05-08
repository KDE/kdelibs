/*  This file is part of the KDE libraries
 *  Copyright 2005, 2008 Jaroslaw Staniek <staniek@kde.org>
 *  Copyright 2010 John Layt <john@layt.net>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "klocale_win_p.h"

#include <QtCore/QLocale>
#include <QtCore/QTextCodec>

KLocaleWindowsPrivate::KLocaleWindowsPrivate(KLocale *q_ptr, const QString &catalogName, KSharedConfig::Ptr config)
                      :KLocalePrivate(q_ptr)
{
    // Lock in the current Windows Locale ID
    // Can we also lock in the actual settings like we do for Mac?
    m_winLocaleId = GetUserDefaultLCID();
    init(catalogName, QString(), QString(), config, 0);
}

KLocaleWindowsPrivate::KLocaleWindowsPrivate(KLocale *q_ptr, const QString& catalogName,
                                             const QString &language, const QString &country, KConfig *config)
                      :KLocalePrivate(q_ptr)
{
    // Lock in the current Windows Locale ID
    // Can we also lock in the actual settings like we do for Mac?
    m_winLocaleId = GetUserDefaultLCID();
    init(catalogName, language, country, KSharedConfig::Ptr(), config);
}

KLocaleWindowsPrivate::KLocaleWindowsPrivate( const KLocaleWindowsPrivate &rhs )
                      :KLocalePrivate( rhs )
{
    KLocalePrivate::copy( rhs );
    m_winLocaleId = rhs.m_winLocaleId;
    strcpy( m_win32SystemEncoding, rhs.m_win32SystemEncoding );
}

KLocaleWindowsPrivate &KLocaleWindowsPrivate::operator=( const KLocaleWindowsPrivate &rhs )
{
    KLocalePrivate::copy( rhs );
    m_winLocaleId = rhs.m_winLocaleId;
    strcpy( m_win32SystemEncoding, rhs.m_win32SystemEncoding );
    return *this;
}

KLocaleWindowsPrivate::~KLocaleWindowsPrivate()
{
}

QString KLocaleWindowsPrivate::windowsLocaleValue( LCTYPE key ) const
{
    // Find out how big the buffer needs to be
    int size = GetLocaleInfoW( m_winLocaleId, key, 0, 0 );

    QString result;
    if ( size ) {
        wchar_t* buffer = new wchar_t[size];
        if ( GetLocaleInfoW( m_winLocaleId, key, buffer, size ) )
            result = QString::fromWCharArray( buffer );
        delete[] buffer;
    }
    return result;
}

QString KLocaleWindowsPrivate::systemCountry() const
{
    return windowsLocaleValue( LOCALE_SISO3166CTRYNAME );
}

QStringList KLocaleWindowsPrivate::systemLanguageList() const
{
    QStringList list;
    getLanguagesFromVariable( list, QLocale::system().name().toLocal8Bit().data() );
    return list;
}

QByteArray KLocaleWindowsPrivate::systemCodeset() const
{
    return QByteArray();
}

const QByteArray KLocaleWindowsPrivate::encoding()
{
    if ( qstrcmp( codecForEncoding()->name(), "System" ) == 0 ) {
        //win32 returns "System" codec name here but KDE apps expect a real name:
        strcpy( m_win32SystemEncoding, "cp " );
        // MSDN says the returned string for LOCALE_IDEFAULTANSICODEPAGE is max 6 char including '\0'
        char buffer[6];
        if ( GetLocaleInfoA( MAKELCID( MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT ), SORT_DEFAULT ),
                             LOCALE_IDEFAULTANSICODEPAGE, buffer, sizeof( buffer ) ) ) {
            strcpy( m_win32SystemEncoding + 3, buffer );
            return m_win32SystemEncoding;
        }
    }
    return KLocalePrivate::encoding();
}

/*
These functions are commented out for now until all required Date/Time functions are implemented
to ensure consistent behaviour, i.e. all KDE format or all Windows format, not some invalid mixture

KConfig KLocaleWindowsPrivate::systemFormats()
{
    KConfig systemConfig("systemrc", KConfig::SimpleConfig);
    systemConfig.setLocale(m_language);
    KConfigGroup cg(&systemConfig, "Locale");

    // Setting enums obtained from http://msdn.microsoft.com/en-us/library/dd464799%28v=VS.85%29.aspx
    // and http://msdn.microsoft.com/en-us/library/bb507201%28v=vs.85%29.aspx

    // Country settings
    // "Country"                                - see initCountry() & systemCountry()
    // cg.writeEntry("CountryDivisionCode", );  - Use KDE

    // LOCALE_SABBREVCTRYNAME gives short country name, or do we keep KDE translations?
    // LOCALE_SCOUNTRY gives country name, or do we keep KDE translations?

    // Language settings

    // "Language"             - see initLanguageList() & systemLanguageList()

    // LOCALE_SABBREVLANGNAME gives short language "name" but really a code, or do we keep KDE translations?
    // LOCALE_SLANGDISPLAYNAME langauge display name
    // LOCALE_SLANGUAGE localized language name

    // LOCALE_IDIGITSUBSTITUTION this needs work, see http://msdn.microsoft.com/en-us/library/dd373769%28v=VS.85%29.aspx
    // cg.writeEntry("LanguageSensitiveDigits", );

    // cg.writeEntry("NounDeclension", );

    // Win7+ LOCALE_IREADINGLAYOUT text direction

    // Calendar settings

    // See http://msdn.microsoft.com/en-us/library/dd317732%28v=vs.85%29.aspx
    // This conversion isn't perfect, may be better to have a KCalendarSystemWindows instead
    // See also LOCALE_IOPTIONALCALENDAR
    CALID calendarSystem = windowsLocaleValue(LOCALE_ICALENDARTYPE).toInt();
    switch (calendarSystem) {
    case CAL_JAPAN:                   //  3 Japanese Emperor Era
        cg.writeEntry("CalendarSystem", "japanese");
        break;
    case CAL_TAIWAN:                  //  4 Taiwan calendar
        cg.writeEntry("CalendarSystem", "minguo");
        break;
    case CAL_HIJRI:                   //  6 Hijri (Arabic Lunar)
    case CAL_UMALQURA:                // 23 Windows Vista and later: Um Al Qura (Arabic lunar) calendar
        cg.writeEntry("CalendarSystem", "hijri");
        break;
    case CAL_THAI:                    //  7 Thai
        cg.writeEntry("CalendarSystem", "thai");
        break;
    case CAL_HEBREW:                  //  8 Hebrew (Lunar)
        cg.writeEntry("CalendarSystem", "hebrew");
        break;
    case CAL_GREGORIAN:               //  1 Gregorian (localized)
    case CAL_GREGORIAN_US:            //  2 Gregorian (English strings always)
    case CAL_KOREA:                   //  5 Korean Tangun Era
    case CAL_GREGORIAN_ME_FRENCH:     //  9 Gregorian Middle East French
    case CAL_GREGORIAN_ARABIC:        // 10 Gregorian Arabic
    case CAL_GREGORIAN_XLIT_ENGLISH:  // 11 Gregorian transliterated English
    case CAL_GREGORIAN_XLIT_FRENCH:   // 12 Gregorian transliterated French
    default:
        cg.writeEntry("CalendarSystem", "gregorian");
        break;
    }

    // cg.writeEntry("WeekNumberSystem", );  use LOCALE_IFIRSTWEEKOFYEAR
    cg.writeEntry("WeekStartDay", windowsLocaleValue(LOCALE_IFIRSTDAYOFWEEK).toInt() + 1); // 0 = Monday
    // cg.writeEntry("WorkingWeekStartDay", );
    // cg.writeEntry("WorkingWeekEndDay", );
    // cg.writeEntry("WeekDayOfPray", );

    // LOCALE_SDAYNAME* gives day name, or do we keep KDE translations?
    // LOCALE_SABBREVDAYNAME* gives short day name, or do we keep KDE translations?
    // LOCALE_SSHORTESTDAYNAME* gives shortest day name, or do we keep KDE translations?
    // LOCALE_SMONTHNAME* gives month name, or do we keep KDE translations?
    // LOCALE_SABBREVMONTHNAME* gives short month name, or do we keep KDE translations?

    // Date/Time formats

    // cg.writeEntry("DateFormat", ); LOCALE_SLONGDATE - convert to posix format?
    // cg.writeEntry("DateFormatShort", ); LOCALE_SSHORTDATE - convert to posix format?
    // cg.writeEntry("TimeFormat", );  LOCALE_STIMEFORMAT - convert to posix format?
    // cg.writeEntry("DateTimeDigitSet",);
    // cg.writeEntry("DateMonthNamePossessive", );
    // "DayPeriodN" - see initDayPeriod()
    // LOCALE_ICENTURY for "UseShortYear" in calendar?
    // LOCALE_IDATE and LOCALE_ILDATE for future "DateComponantOrder"
    // LOCALE_IDAYLZERO for leading 0's in days
    // LOCALE_IMONLZERO for leading 0's in months
    // LOCALE_ITLZERO for leading 0's in hours
    // LOCALE_ITIME for 12 or 24 hour format
    // LOCALE_ITIMEMARKPOSN for prefix/postfix am/pm
    // LOCALE_SDATE for date componant separator
    // LOCALE_STIME for time componant separator
    // LOCALE_SDURATION for time duration format
    // LOCALE_SYEARMONTH <=win2k year month format
    // LOCALE_SMONTHDAY Win7+ format for month/day only
    // LOCALE_SSHORTTIME Win7+ short time format

    // Numeric formats

    cg.writeEntry("DecimalPlaces", windowsLocaleValue(LOCALE_IDIGITS).toInt());
    cg.writeEntry("DecimalSymbol", windowsLocaleValue(LOCALE_SDECIMAL));
    cg.writeEntry("ThousandsSeparator", windowsLocaleValue(LOCALE_STHOUSAND));
    cg.writeEntry("PositiveSign", windowsLocaleValue(LOCALE_SPOSITIVESIGN));
    cg.writeEntry("NegativeSign", windowsLocaleValue(LOCALE_SNEGATIVESIGN));
    // cg.writeEntry("DigitSet", );
    // LOCALE_ILZERO for leading 0's                  - not supported by KDE
    // LOCALE_INEGNUMBER for numeric SignPosition     - not supported by KDE
    // LOCALE_SGROUPING for grouping format, same as POSIX? - not supported by KDE (yet)
    // LOCALE_SNAN "Not A Number" symbol - not supported in KDE
    // LOCALE_SPOSINFINITY LOCALE_SNEGINFINITY vista+ infinity strings - not supported in KDE

    // Win7+: LOCALE_IPOSITIVEPERCENT LOCALE_INEGATIVEPERCENT LOCALE_SPERCENT format for percentages - not supported by KDE
    // LOCALE_SPERMILLE Win7+ permille ‰ (U+2030) symbol

    // Currency settings

    // cg.writeEntry("CurrencyCode", );
    cg.writeEntry("CurrencySymbol", windowsLocaleValue(LOCALE_SCURRENCY));
    // cg.writeEntry("CurrencyCodesInUse", ); - use KDE for Windows CurrencyCode
    // LOCALE_SINTLSYMBOL ISO currency code plus separator char, i.e. symbol to use before amount

    // Monetary formats

    cg.writeEntry("MonetaryDecimalSymbol", windowsLocaleValue(LOCALE_SMONDECIMALSEP));

    cg.writeEntry("MonetaryThousandsSeparator", windowsLocaleValue(LOCALE_SMONTHOUSANDSEP));

    cg.writeEntry("MonetaryDecimalPlaces", windowsLocaleValue(LOCALE_ICURRDIGITS).toInt());

    switch (windowsLocaleValue(LOCALE_IPOSSIGNPOSN).toInt()) {
    case 1:
        cg.writeEntry("PositiveMonetarySignPosition", KLocale::BeforeQuantityMoney);
    case 2:
        cg.writeEntry("PositiveMonetarySignPosition", KLocale::AfterQuantityMoney);
    case 3:
        cg.writeEntry("PositiveMonetarySignPosition", KLocale::BeforeMoney);
    case 4:
        cg.writeEntry("PositiveMonetarySignPosition", KLocale::AfterMoney);
    }

    // Enum value matches exactly!
    switch (windowsLocaleValue(LOCALE_INEGSIGNPOSN).toInt()) {
    case 0:
        cg.writeEntry("NegativeMonetarySignPosition", KLocale::ParensAround);
    case 1:
        cg.writeEntry("NegativeMonetarySignPosition", KLocale::BeforeQuantityMoney);
    case 2:
        cg.writeEntry("NegativeMonetarySignPosition", KLocale::AfterQuantityMoney);
    case 3:
        cg.writeEntry("NegativeMonetarySignPosition", KLocale::BeforeMoney);
    case 4:
        cg.writeEntry("NegativeMonetarySignPosition", KLocale::AfterMoney);
    }

    int positiveCurrencySymbolPosition =  windowsLocaleValue(LOCALE_IPOSSYMPRECEDES).toInt();
    if (positiveCurrencySymbolPosition == 0) {
        cg.writeEntry("PositivePrefixCurrencySymbol", false);
    } else if (positiveCurrencySymbolPosition == 1) {
        cg.writeEntry("PositivePrefixCurrencySymbol", true);
    }

    int negativeCurrencySymbolPosition =  windowsLocaleValue(LOCALE_INEGSYMPRECEDES).toInt();
    if (negativeCurrencySymbolPosition == 0) {
        cg.writeEntry("NegativePrefixCurrencySymbol", false);
    } else if (negativeCurrencySymbolPosition == 1) {
        cg.writeEntry("NegativePrefixCurrencySymbol", true);
    }

    // LOCALE_SMONGROUPING for grouping format, same as POSIX? - not supported by KDE (yet)
    // LOCALE_IPOSSEPBYSPACE and LOCALE_INEGSEPBYSPACE if space between sign and amount - not supported by KDE
    // LOCALE_ICURRENCY combines LOCALE_IPOSSIGNPOSN, LOCALE_IPOSSYMPRECEDES and LOCALE_IPOSSEPBYSPACE
    // LOCALE_INEGCURR combines LOCALE_INEGSIGNPOSN, LOCALE_INEGSYMPRECEDES and LOCALE_INEGSEPBYSPACE

    //cg.writeEntry("MonetaryDigitSet", );

    // Units settings

    //cg.writeEntry("BinaryUnitDialect", );    - Use KDE

#ifndef QT_NO_PRINTER
    // Windows paper sizes from http://msdn.microsoft.com/en-us/library/dd319099%28v=vs.85%29.aspx
    // QT paper sizes from http://doc.qt.nokia.com/latest/qprinter.html#PaperSize-enum
    switch (windowsLocaleValue(LOCALE_IPAPERSIZE).toInt()) {
    case DMPAPER_LETTER:                          // 1   US Letter 8 1/2 x 11 in
        cg.writeEntry("PageSize", QPrinter::Letter);
    case DMPAPER_TABLOID:                         // 3   US Tabloid 11 x 17 in
        cg.writeEntry("PageSize", QPrinter::Tabloid);
    case DMPAPER_LEDGER:                          // 4   US Ledger 17 x 11 in
        cg.writeEntry("PageSize", QPrinter::Ledger);
    case DMPAPER_LEGAL:                           // 5   US Legal 8 1/2 x 14 in
        cg.writeEntry("PageSize", QPrinter::Legal);
    case DMPAPER_EXECUTIVE:                       // 7   US Executive 7 1/4 x 10 1/2 in
        cg.writeEntry("PageSize", QPrinter::Executive);
    case DMPAPER_A3:                              // 8   A3 297 x 420 mm
        cg.writeEntry("PageSize", QPrinter::A3);
    case DMPAPER_A4:                              // 9   A4 210 x 297 mm
        cg.writeEntry("PageSize", QPrinter::A4);
    case DMPAPER_A5:                              // 11  A5 148 x 210 mm
        cg.writeEntry("PageSize", QPrinter::A5);
    case DMPAPER_ISO_B4:                          // 42  B4 (ISO) 250 x 353 mm
        cg.writeEntry("PageSize", QPrinter::B4);
    case DMPAPER_FOLIO:                           // 14  Folio 8 1/2 x 13 in
        cg.writeEntry("PageSize", QPrinter::Folio);
    case DMPAPER_ENV_10:                          // 20  US Envelope #10 4 1/8 x 9 1/2
        cg.writeEntry("PageSize", QPrinter::Comm10E);
    case DMPAPER_ENV_DL:                          // 27  Envelope DL 110 x 220mm
        cg.writeEntry("PageSize", QPrinter::DLE);
    case DMPAPER_ENV_C5:                          // 28  Envelope C5 162 x 229 mm
        cg.writeEntry("PageSize", QPrinter::C5E);
    case DMPAPER_A2:                              // 66  A2 420 x 594 mm
        cg.writeEntry("PageSize", QPrinter::A2);
    case DMPAPER_A6:                              // 70  A6 105 x 148 mm
        cg.writeEntry("PageSize", QPrinter::A6);
    // No direct match in QPrinter for these, although some of these are just rotated by 90 degrees treat as custom
    // Be very careful re-assigning these, don't rely on names matching you need to check the physical dimensions!
    case DMPAPER_LETTERSMALL:                     // 2   US Letter Small 8 1/2 x 11 in
    case DMPAPER_STATEMENT:                       // 6   US Statement 5 1/2 x 8 1/2 in
    case DMPAPER_A4SMALL:                         // 10  A4 Small 210 x 297 mm
    case DMPAPER_B4:                              // 12  B4 (JIS) 257 x 364 mm
    case DMPAPER_B5:                              // 13  B5 (JIS) 182 x 257 mm
    case DMPAPER_QUARTO:                          // 15  Quarto 215 x 275 mm
    case DMPAPER_10X14:                           // 16  10 x 14 in
    case DMPAPER_11X17:                           // 17  11 x 17 in
    case DMPAPER_NOTE:                            // 18  US Note 8 1/2 x 11 in
    case DMPAPER_ENV_9:                           // 19  US Envelope #9 3 7/8 x 8 7/8
    case DMPAPER_ENV_11:                          // 21  US Envelope #11 4 1/2 x 10 3/8
    case DMPAPER_ENV_12:                          // 22  US Envelope #12 4 3/4 x 11 in
    case DMPAPER_ENV_14:                          // 23  US Envelope #14 5 x 11 1/2
    case DMPAPER_CSHEET:                          // 24  C size sheet
    case DMPAPER_DSHEET:                          // 25  D size sheet
    case DMPAPER_ESHEET:                          // 26  E size sheet
    case DMPAPER_ENV_C3:                          // 29  Envelope C3 324 x 458 mm
    case DMPAPER_ENV_C4:                          // 30  Envelope C4 229 x 324 mm
    case DMPAPER_ENV_C6:                          // 31  Envelope C6 114 x 162 mm
    case DMPAPER_ENV_C65:                         // 32  Envelope C65 114 x 229 mm
    case DMPAPER_ENV_B4:                          // 33  Envelope B4 250 x 353 mm
    case DMPAPER_ENV_B5:                          // 34  Envelope B5 176 x 250 mm
    case DMPAPER_ENV_B6:                          // 35  Envelope B6 176 x 125 mm
    case DMPAPER_ENV_ITALY:                       // 36  Envelope 110 x 230 mm
    case DMPAPER_ENV_MONARCH:                     // 37  US Envelope Monarch 3.875 x 7.5 in
    case DMPAPER_ENV_PERSONAL:                    // 38  6 3/4 US Envelope 3 5/8 x 6 1/2 in
    case DMPAPER_FANFOLD_US:                      // 39  US Std Fanfold 14 7/8 x 11 in
    case DMPAPER_FANFOLD_STD_GERMAN:              // 40  German Std Fanfold 8 1/2 x 12 in
    case DMPAPER_FANFOLD_LGL_GERMAN:              // 41  German Legal Fanfold 8 1/2 x 13 in
    case DMPAPER_JAPANESE_POSTCARD:               // 43  Japanese Postcard 100 x 148 mm
    case DMPAPER_9X11:                            // 44  9 x 11 in
    case DMPAPER_10X11:                           // 45  10 x 11 in
    case DMPAPER_15X11:                           // 46  15 x 11 in
    case DMPAPER_ENV_INVITE:                      // 47  Envelope Invite 220 x 220 mm
    case DMPAPER_RESERVED_48:                     // 48  RESERVED--DO NOT USE
    case DMPAPER_RESERVED_49:                     // 49  RESERVED--DO NOT USE
    case DMPAPER_LETTER_EXTRA:                    // 50  US Letter Extra 9 1/2 x 12 in
    case DMPAPER_LEGAL_EXTRA:                     // 51  US Legal Extra 9 1/2 x 15 in
    case DMPAPER_TABLOID_EXTRA:                   // 52  US Tabloid Extra 11.69 x 18 in
    case DMPAPER_A4_EXTRA:                        // 53  A4 Extra 9.27 x 12.69 in
    case DMPAPER_LETTER_TRANSVERSE:               // 54  Letter Transverse 8 1/2 x 11 in
    case DMPAPER_A4_TRANSVERSE:                   // 55  A4 Transverse 210 x 297 mm
    case DMPAPER_LETTER_EXTRA_TRANSVERSE:         // 56  Letter Extra Transverse 9 1/2 x 12 in
    case DMPAPER_A_PLUS:                          // 57  SuperA/SuperA/A4 227 x 356 mm
    case DMPAPER_B_PLUS:                          // 58  SuperB/SuperB/A3 305 x 487 mm
    case DMPAPER_LETTER_PLUS:                     // 59  US Letter Plus 8.5 x 12.69 in
    case DMPAPER_A4_PLUS:                         // 60  A4 Plus 210 x 330 mm
    case DMPAPER_A5_TRANSVERSE:                   // 61  A5 Transverse 148 x 210 mm
    case DMPAPER_B5_TRANSVERSE:                   // 62  B5 (JIS) Transverse 182 x 257 mm
    case DMPAPER_A3_EXTRA:                        // 63  A3 Extra 322 x 445 mm
    case DMPAPER_A5_EXTRA:                        // 64  A5 Extra 174 x 235 mm
    case DMPAPER_B5_EXTRA:                        // 65  B5 (ISO) Extra 201 x 276 mm
    case DMPAPER_A3_TRANSVERSE:                   // 67  A3 Transverse 297 x 420 mm
    case DMPAPER_A3_EXTRA_TRANSVERSE:             // 68  A3 Extra Transverse 322 x 445 mm
    case DMPAPER_DBL_JAPANESE_POSTCARD:           // 69  Japanese Double Postcard 200 x 148 mm
    case DMPAPER_JENV_KAKU2:                      // 71  Japanese Envelope Kaku #2
    case DMPAPER_JENV_KAKU3:                      // 72  Japanese Envelope Kaku #3
    case DMPAPER_JENV_CHOU3:                      // 73  Japanese Envelope Chou #3
    case DMPAPER_JENV_CHOU4:                      // 74  Japanese Envelope Chou #4
    case DMPAPER_LETTER_ROTATED:                  // 75  Letter Rotated 11 x 8 1/2 11 in
    case DMPAPER_A3_ROTATED:                      // 76  A3 Rotated 420 x 297 mm
    case DMPAPER_A4_ROTATED:                      // 77  A4 Rotated 297 x 210 mm
    case DMPAPER_A5_ROTATED:                      // 78  A5 Rotated 210 x 148 mm
    case DMPAPER_B4_JIS_ROTATED:                  // 79  B4 (JIS) Rotated 364 x 257 mm
    case DMPAPER_B5_JIS_ROTATED:                  // 80  B5 (JIS) Rotated 257 x 182 mm
    case DMPAPER_JAPANESE_POSTCARD_ROTATED:       // 81  Japanese Postcard Rotated 148 x 100 mm
    case DMPAPER_DBL_JAPANESE_POSTCARD_ROTATED:   // 82  Double Japanese Postcard Rotated 148 x 200 mm
    case DMPAPER_A6_ROTATED:                      // 83  A6 Rotated 148 x 105 mm
    case DMPAPER_JENV_KAKU2_ROTATED:              // 84  Japanese Envelope Kaku #2 Rotated
    case DMPAPER_JENV_KAKU3_ROTATED:              // 85  Japanese Envelope Kaku #3 Rotated
    case DMPAPER_JENV_CHOU3_ROTATED:              // 86  Japanese Envelope Chou #3 Rotated
    case DMPAPER_JENV_CHOU4_ROTATED:              // 87  Japanese Envelope Chou #4 Rotated
    case DMPAPER_B6_JIS:                          // 88  B6 (JIS) 128 x 182 mm
    case DMPAPER_B6_JIS_ROTATED:                  // 89  B6 (JIS) Rotated 182 x 128 mm
    case DMPAPER_12X11:                           // 90  12 x 11 in
    case DMPAPER_JENV_YOU4:                       // 91  Japanese Envelope You #4
    case DMPAPER_JENV_YOU4_ROTATED:               // 92  Japanese Envelope You #4 Rotated
    case DMPAPER_P16K:                            // 93  PRC 16K 146 x 215 mm
    case DMPAPER_P32K:                            // 94  PRC 32K 97 x 151 mm
    case DMPAPER_P32KBIG:                         // 95  PRC 32K(Big) 97 x 151 mm
    case DMPAPER_PENV_1:                          // 96  PRC Envelope #1 102 x 165 mm
    case DMPAPER_PENV_2:                          // 97  PRC Envelope #2 102 x 176 mm
    case DMPAPER_PENV_3:                          // 98  PRC Envelope #3 125 x 176 mm
    case DMPAPER_PENV_4:                          // 99  PRC Envelope #4 110 x 208 mm
    case DMPAPER_PENV_5:                          // 100 PRC Envelope #5 110 x 220 mm
    case DMPAPER_PENV_6:                          // 101 PRC Envelope #6 120 x 230 mm
    case DMPAPER_PENV_7:                          // 102 PRC Envelope #7 160 x 230 mm
    case DMPAPER_PENV_8:                          // 103 PRC Envelope #8 120 x 309 mm
    case DMPAPER_PENV_9:                          // 104 PRC Envelope #9 229 x 324 mm
    case DMPAPER_PENV_10:                         // 105 PRC Envelope #10 324 x 458 mm
    case DMPAPER_P16K_ROTATED:                    // 106 PRC 16K Rotated
    case DMPAPER_P32K_ROTATED:                    // 107 PRC 32K Rotated
    case DMPAPER_P32KBIG_ROTATED:                 // 108 PRC 32K(Big) Rotated
    case DMPAPER_PENV_1_ROTATED:                  // 109 PRC Envelope #1 Rotated 165 x 102 mm
    case DMPAPER_PENV_2_ROTATED:                  // 110 PRC Envelope #2 Rotated 176 x 102 mm
    case DMPAPER_PENV_3_ROTATED:                  // 111 PRC Envelope #3 Rotated 176 x 125 mm
    case DMPAPER_PENV_4_ROTATED:                  // 112 PRC Envelope #4 Rotated 208 x 110 mm
    case DMPAPER_PENV_5_ROTATED:                  // 113 PRC Envelope #5 Rotated 220 x 110 mm
    case DMPAPER_PENV_6_ROTATED:                  // 114 PRC Envelope #6 Rotated 230 x 120 mm
    case DMPAPER_PENV_7_ROTATED:                  // 115 PRC Envelope #7 Rotated 230 x 160 mm
    case DMPAPER_PENV_8_ROTATED:                  // 116 PRC Envelope #8 Rotated 309 x 120 mm
    case DMPAPER_PENV_9_ROTATED:                  // 117 PRC Envelope #9 Rotated 324 x 229 mm
    case DMPAPER_PENV_10_ROTATED:                 // 118 PRC Envelope #10 Rotated 458 x 324 mm
    default:
        cg.writeEntry("PageSize", QPrinter::Custom);
    }

    // The following QPrinter values do not appear to have Windows equivalents:
    // QPrinter::A0
    // QPrinter::A1
    // QPrinter::A7
    // QPrinter::A8
    // QPrinter::A9
    // QPrinter::B0
    // QPrinter::B1
    // QPrinter::B2
    // QPrinter::B3
    // QPrinter::B5
    // QPrinter::B6
    // QPrinter::B7
    // QPrinter::B8
    // QPrinter::B9
    // QPrinter::B10
#endif

    int measureSystem = windowsLocaleValue(LOCALE_IMEASURE).toInt();
    if ( measureSystem == 0 ) {
        cg.writeEntry("MeasureSystem", KLocale::Metric);
    } else if ( measureSystem == 1 ) {
        cg.writeEntry("MeasureSystem", KLocale::Imperial);
    }

    // LOCALE_SLIST list seperator - not supported by KDE

    return systemConfig;
}

void KLocaleWindowsPrivate::initDayPeriods(const KConfigGroup &cg)
{
    QString amText = windowsLocaleValue(LOCALE_S1159);
    QString pmText = windowsLocaleValue(LOCALE_S2359);

    m_dayPeriods.clear();
    m_dayPeriods.append(KDayPeriod("am", amText, amText, amText
                                   QTime(0, 0, 0), QTime(11, 59, 59, 999), 0, 12));
    m_dayPeriods.append(KDayPeriod("pm", pmText, pmText, pmText,
                                   QTime(12, 0, 0), QTime(23, 59, 59, 999), 0, 12));
}

*/
