/* This file is part of the KDE libraries

   Copyright (C) 2007 Daniel Laidig <d.laidig@gmx.de>

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


#include <QStringList>

#include <QDebug>
#include <QTime>

#include "kcharselectdata_p.h"
#include "kcharselect_unicodedata.h"

/* constants for hangul (de)composition, see UAX #15 */
#define SBase 0xAC00
#define LBase 0x1100
#define VBase 0x1161
#define TBase 0x11A7
#define LCount 19
#define VCount 21
#define TCount 28
#define NCount (VCount * TCount)
#define SCount (LCount * NCount)

static const char JAMO_L_TABLE[][4] =
    {
        "G", "GG", "N", "D", "DD", "R", "M", "B", "BB",
        "S", "SS", "", "J", "JJ", "C", "K", "T", "P", "H"
    };

static const char JAMO_V_TABLE[][4] =
    {
        "A", "AE", "YA", "YAE", "EO", "E", "YEO", "YE", "O",
        "WA", "WAE", "OE", "YO", "U", "WEO", "WE", "WI",
        "YU", "EU", "YI", "I"
    };

static const char JAMO_T_TABLE[][4] =
    {
        "", "G", "GG", "GS", "N", "NJ", "NH", "D", "L", "LG", "LM",
        "LB", "LS", "LT", "LP", "LH", "M", "B", "BS",
        "S", "SS", "NG", "J", "C", "K", "T", "P", "H"
    };

static const NamesList* getNamesList(const QChar& c)
{
    int min = 0;
    int mid;
    int max = sizeof(names_list) / sizeof(NamesList) - 1;
    int unicode = c.unicode();

    static ushort most_recent_searched;
    static const NamesList *most_recent_result;

    if (unicode < names_list[0].index || unicode > names_list[max].index)
        return 0;

    if (unicode == most_recent_searched)
        return most_recent_result;

    most_recent_searched = unicode;

    while (max >= min) {

        mid = (min + max) / 2;
        if (unicode > names_list[mid].index)
            min = mid + 1;
        else if (unicode < names_list[mid].index)
            max = mid - 1;
        else {
            most_recent_result = names_list + mid;
            return names_list + mid;
        }
    }

    most_recent_result = 0;
    return 0;
}

QString KCharSelectData::formatCode(ushort code, int length, const QString& prefix, int base)
{
    QString s = QString::number(code, base).toUpper();
    while (s.size() < length)
        s.prepend('0');
    s.prepend(prefix);
    return s;
}

QList<QChar> KCharSelectData::blockContents(int block)
{
    QList<QChar> res;
    int size = sizeof(unicode_blocks) / sizeof(UnicodeBlock);
    if (block >= size)
        return res;

    UnicodeBlock b = unicode_blocks[block];

    ushort c = b.start;
    res.append(c);

    while (c != b.end) {
        ++c;
        res.append(c);
    }

    return res;
}

QList<int> KCharSelectData::sectionContents(int section)
{
    QList<int> res;
    int size = sizeof(unicode_sections) / sizeof(UnicodeSection);
    if (section >= size)
        return res;

    for (int i = 0; i < size; i++) {
        if (unicode_sections[i].section_index != section)
            continue;

        res.append(unicode_sections[i].block_index);
    }

    return res;
}

QStringList KCharSelectData::blockList()
{
    QStringList list;
    int blocks_size = sizeof(unicode_blocks) / sizeof(UnicodeBlock);
    for (int i = 0; i < blocks_size; i++) {
        list.append(unicode_blocks[i].block_name);
    }
    return list;
}

QStringList KCharSelectData::sectionList()
{
    QStringList list;
    int sections_size = sizeof(unicode_section_list) / sizeof(unicode_section_list[0]);
    for (int i = 0; i < sections_size; i++) {
        list.append(unicode_section_list[i]);
    }
    return list;
}

QString KCharSelectData::block(const QChar& c)
{
    ushort unicode = c.unicode();
    int max = sizeof(unicode_blocks) / sizeof(UnicodeBlock) - 1;
    int i = 0;

    while (unicode > unicode_blocks[i].end && i < max)
        i++;

    return unicode_blocks[i].block_name;
}

QString KCharSelectData::name(const QChar& c)
{
    ushort unicode = c.unicode();
    if ((unicode >= 0x3400 && unicode <= 0x4DB5)
            || (unicode >= 0x4e00 && unicode <= 0x9fa5)) {
        // || (unicode >= 0x20000 && unicode <= 0x2A6D6) // useless, since limited to 16 bit
        return "CJK UNIFIED IDEOGRAPH-" + QString::number(unicode, 16);
    } else if (c >= 0xac00 && c <= 0xd7af) {
        /* compute hangul syllable name as per UAX #15 */
        int SIndex = c.unicode() - SBase;
        int LIndex, VIndex, TIndex;

        if (SIndex < 0 || SIndex >= SCount)
            return QString();

        LIndex = SIndex / NCount;
        VIndex = (SIndex % NCount) / TCount;
        TIndex = SIndex % TCount;

        return QString("HANGUL SYLLABLE ") + JAMO_L_TABLE[LIndex] + JAMO_V_TABLE[VIndex] + JAMO_T_TABLE[TIndex];
    } else if (unicode >= 0xD800 && unicode <= 0xDB7F)
        return i18n("<Non Private Use High Surrogate>");
    else if (unicode >= 0xDB80 && unicode <= 0xDBFF)
        return i18n("<Private Use High Surrogate>");
    else if (unicode >= 0xDC00 && unicode <= 0xDFFF)
        return i18n("<Low Surrogate>");
    else if (unicode >= 0xE000 && unicode <= 0xF8FF)
        return i18n("<Private Use>");
//  else if (unicode >= 0xF0000 && unicode <= 0xFFFFD) // 16 bit!
//   return i18n("<Plane 15 Private Use>");
//  else if (unicode >= 0x100000 && unicode <= 0x10FFFD)
//   return i18n("<Plane 16 Private Use>");
    else {
        int min = 0;
        int mid;
        int max = (sizeof(unicode_names) / sizeof(unicode_names[0])) - 1;
        QString s;

        if (unicode < unicode_names[0].index || unicode > unicode_names[max].index)
            return QString();

        while (max >= min) {
            mid = (min + max) / 2;
            if (unicode > unicode_names[mid].index)
                min = mid + 1;
            else if (unicode < unicode_names[mid].index)
                max = mid - 1;
            else {
                s = unicode_names_strings + unicode_names[mid].name_offset;
                break;
            }
        }

        if (s.isNull())
            return i18n("<not assigned>");
        else
            return s;
    }
}

int KCharSelectData::blockIndex(const QChar& c)
{
    ushort unicode = c.unicode();
    int max = sizeof(unicode_blocks) / sizeof(UnicodeBlock) - 1;
    int i = 0;

    while (unicode > unicode_blocks[i].end && i < max)
        i++;

    return i;
}

int KCharSelectData::sectionIndex(int block)
{
    int max = sizeof(unicode_sections) / sizeof(UnicodeSection) - 1;
    int i = 0;

    while (block != unicode_sections[i].block_index && i < max)
        i++;

    return unicode_sections[i].section_index;
}

QString KCharSelectData::blockName(int index)
{
    int max = sizeof(unicode_blocks) / sizeof(UnicodeBlock) - 1;
    if (index > max)
        return QString();
    return i18n(unicode_blocks[index].block_name);
}

QStringList KCharSelectData::aliases(const QChar& c)
{
    const NamesList* namesList = getNamesList(c);
    int count;
    ushort unicode = c.unicode();

    QStringList aliases;

    if (namesList == 0 || namesList->equals_index == -1)
        return QStringList();

    for (count = 0;  names_list_equals[namesList->equals_index + count].index == unicode;  count++);

    for (int i = 0;  i < count;  i++)
        aliases.append(QString::fromUtf8(names_list_equals[namesList->equals_index + i].value));
    return aliases;
}

QStringList KCharSelectData::notes(const QChar& c)
{
    const NamesList* namesList = getNamesList(c);
    int count;
    ushort unicode = c.unicode();

    QStringList notes;

    if (namesList == 0 || namesList->stars_index == -1)
        return QStringList();

    count = 0;

    for (count = 0;  names_list_stars[namesList->stars_index + count].index == unicode;  count++);
    for (int i = 0;  i < count;  i++)
        notes.append(QString::fromUtf8(names_list_stars[namesList->stars_index + i].value));
    return notes;
}

QList<QChar> KCharSelectData::seeAlso(const QChar& c)
{
    const NamesList* namesList = getNamesList(c);
    int count;
    ushort unicode = c.unicode();

    QList<QChar> seeAlso;

    if (namesList == 0 || namesList->exes_index == -1)
        return QList<QChar>();

    count = 0;

    for (count = 0;  names_list_exes[namesList->exes_index + count].index == unicode;  count++);
    for (int i = 0;  i < count;  i++)
        seeAlso.append(names_list_exes[namesList->exes_index + i].value);
    return seeAlso;
}

QStringList KCharSelectData::equivalents(const QChar& c)
{
    const NamesList* namesList = getNamesList(c);
    int count;
    ushort unicode = c.unicode();

    QStringList equivalents;

    if (namesList == 0 || namesList->colons_index == -1)
        return QStringList();

    count = 0;

    for (count = 0;  names_list_colons[namesList->colons_index + count].index == unicode;  count++);
    for (int i = 0;  i < count;  i++)
        equivalents.append(QString::fromUtf8(names_list_colons[namesList->colons_index + i].value));
    return equivalents;
}

QStringList KCharSelectData::approximateEquivalents(const QChar& c)
{
    const NamesList* namesList = getNamesList(c);
    int count;
    ushort unicode = c.unicode();

    QStringList approxEquivalents;

    if (namesList == 0 || namesList->pounds_index == -1)
        return QStringList();

    count = 0;

    for (count = 0;  names_list_pounds[namesList->pounds_index + count].index == unicode;  count++);
    for (int i = 0;  i < count;  i++)
        approxEquivalents.append(QString::fromUtf8(names_list_pounds[namesList->pounds_index + i].value));
    return approxEquivalents;
}

QStringList KCharSelectData::unihanInfo(const QChar& c)
{
    int min = 0;
    int mid;
    int max = sizeof(unihan) / sizeof(Unihan) - 1;
    int unicode = c.unicode();

    if (unicode < unihan[0].index || unicode > unihan[max].index)
        return QStringList();


    while (max >= min) {

        mid = (min + max) / 2;
        if (unicode > unihan[mid].index)
            min = mid + 1;
        else if (unicode < unihan[mid].index)
            max = mid - 1;
        else {
            QStringList res;
            if (unihan[mid].kDefinition != -1) {
                res.append(QString::fromUtf8(&unihan_strings[unihan[mid].kDefinition]));
            } else {
                res.append(QString());
            }
            if (unihan[mid].kCantonese != -1) {
                res.append(QString::fromUtf8(&unihan_strings[unihan[mid].kCantonese]));
            } else {
                res.append(QString());
            }
            if (unihan[mid].kMandarin != -1) {
                res.append(QString::fromUtf8(&unihan_strings[unihan[mid].kMandarin]));
            } else {
                res.append(QString());
            }
            if (unihan[mid].kTang != -1) {
                res.append(QString::fromUtf8(&unihan_strings[unihan[mid].kTang]));
            } else {
                res.append(QString());
            }
            if (unihan[mid].kKorean != -1) {
                res.append(QString::fromUtf8(&unihan_strings[unihan[mid].kKorean]));
            } else {
                res.append(QString());
            }
            if (unihan[mid].kJapaneseKun != -1) {
                res.append(QString::fromUtf8(&unihan_strings[unihan[mid].kJapaneseKun]));
            } else {
                res.append(QString());
            }
            if (unihan[mid].kJapaneseOn != -1) {
                res.append(QString::fromUtf8(&unihan_strings[unihan[mid].kJapaneseOn]));
            } else {
                res.append(QString());
            }
            return res;
        }
    }

    return QStringList();
}

QString KCharSelectData::categoryText(QChar::Category category)
{
    switch (category) {
    case QChar::Other_Control: return i18n("Other, Control");
    case QChar::Other_Format: return i18n("Other, Format");
    case QChar::Other_NotAssigned: return i18n("Other, Not Assigned");
    case QChar::Other_PrivateUse: return i18n("Other, Private Use");
    case QChar::Other_Surrogate: return i18n("Other, Surrogate");
    case QChar::Letter_Lowercase: return i18n("Letter, Lowercase");
    case QChar::Letter_Modifier: return i18n("Letter, Modifier");
    case QChar::Letter_Other: return i18n("Letter, Other");
    case QChar::Letter_Titlecase: return i18n("Letter, Titlecase");
    case QChar::Letter_Uppercase: return i18n("Letter, Uppercase");
    case QChar::Mark_SpacingCombining: return i18n("Mark, Spacing Combining");
    case QChar::Mark_Enclosing: return i18n("Mark, Enclosing");
    case QChar::Mark_NonSpacing: return i18n("Mark, Non-Spacing");
    case QChar::Number_DecimalDigit: return i18n("Number, Decimal Digit");
    case QChar::Number_Letter: return i18n("Number, Letter");
    case QChar::Number_Other: return i18n("Number, Other");
    case QChar::Punctuation_Connector: return i18n("Punctuation, Connector");
    case QChar::Punctuation_Dash: return i18n("Punctuation, Dash");
    case QChar::Punctuation_Close: return i18n("Punctuation, Close");
    case QChar::Punctuation_FinalQuote: return i18n("Punctuation, Final Quote");
    case QChar::Punctuation_InitialQuote: return i18n("Punctuation, Initial Quote");
    case QChar::Punctuation_Other: return i18n("Punctuation, Other");
    case QChar::Punctuation_Open: return i18n("Punctuation, Open");
    case QChar::Symbol_Currency: return i18n("Symbol, Currency");
    case QChar::Symbol_Modifier: return i18n("Symbol, Modifier");
    case QChar::Symbol_Math: return i18n("Symbol, Math");
    case QChar::Symbol_Other: return i18n("Symbol, Other");
    case QChar::Separator_Line: return i18n("Separator, Line");
    case QChar::Separator_Paragraph: return i18n("Separator, Paragraph");
    case QChar::Separator_Space: return i18n("Separator, Space");
    default: return i18n("Unknown");
    }
}

// Test code, this just searches the names
QList<QChar> KCharSelectData::find(QString s, SearchRange range)
{
    Q_UNUSED(range)
    QTime t;
    t.start();

    QList<QChar> res;

    s = s.simplified();

    QStringList searchStrings = QStringList(QString());
    for (int i = 0; i < s.size(); i++) {
        if (s[i] == ' ') {
            if (!searchStrings.last().isEmpty())
                searchStrings.append(QString());
            continue;
        }
        if (s[i] == '"') {
            if (!searchStrings.last().isEmpty())
                searchStrings.append(QString());
            i++;
            while (s[i] != '"' && i < s.size()) {
                searchStrings.last().append(s[i]);
                i++;
            }
            searchStrings.append(QString());
            continue;
        }
        searchStrings.last().append(s[i]);
    }
    if (searchStrings.last().isEmpty()) {
        searchStrings.removeLast();
    }

    if (searchStrings.count() == 0)
        return res;

    int longestStrIndex = 0;
    if (searchStrings.count() != 1) {
        int maxlen = 0;
        int count = searchStrings.count();
        for (int i = 0; i < count; i++) {
            if (searchStrings[i].count() > maxlen) {
                maxlen = searchStrings[i].count();
                longestStrIndex = i;
            }
        }
    }
    QString longestStr = searchStrings[longestStrIndex];
    searchStrings.removeAt(longestStrIndex);

    int names_size = (sizeof(unicode_names) / sizeof(UnicodeName));
    for (int i = 0; i < names_size; i++) {
        QString name = QString::fromUtf8(unicode_names_strings + unicode_names[i].name_offset);
        if (name.contains(longestStr, Qt::CaseInsensitive)) {
            bool valid = true;
            foreach(QString s, searchStrings) {
                if (!name.contains(s, Qt::CaseInsensitive)) {
                    valid = false;
                    break;
                }
            }
            if (valid) {
                res.append(unicode_names[i].index);
            }
        }
    }

    qDebug() << "search duration (ms)" << t.elapsed() << "results" << res.count();
    return res;
}
