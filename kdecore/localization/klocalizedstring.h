/*  This file is part of the KDE libraries
    Copyright (C) 2006 Chusslove Illich <caslav.ilic@gmx.net>

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
#ifndef KLOCALIZEDSTRING_H
#define KLOCALIZEDSTRING_H

#include <kdecore_export.h>


#include <QtCore/QChar>
#include <QtCore/QLatin1Char>
#include <QtCore/QStringList>

class KLocale;
struct KCatalogName;
class KLocalizedStringPrivate;

/**
 * \file klocalizedstring.h
 */

#ifndef I18N_NOOP
/**
 * I18N_NOOP marks a string to be translated without translating it.
 * Do not use this unless you know you need it.
 * http://developer.kde.org/documentation/other/developer-faq.html#q2.11.2
 *
 * Example usage where say_something() returns either "hello" or "goodbye":
 * \code
 *   (void) I18N_NOOP("hello");
 *   (void) I18N_NOOP("goodbye");
 *   ...
 *   mystring = i18n(say_something());
 * \endcode
 */
#define I18N_NOOP(x) x
#endif

#ifndef I18N_NOOP2
/**
 * If the string is too ambiguous to be translated well to a non-english
 * language, use this instead of I18N_NOOP to separate lookup string and
 * english.
 *
 * Example usage where say_something() returns either "hello" or "goodbye":
 * \code
 *   (void) I18N_NOOP2("greeting", "hello");
 *   (void) I18N_NOOP2("greeting", "goodbye");
 *   ...
 *   mystring = i18nc("greeting", say_something());
 * \endcode
 * \warning You need to call i18nc(context, stringVar) later on, not just
 * i18n(stringVar).
 *
 * @see I18N_NOOP2_NOSTRIP
 */
#define I18N_NOOP2(comment,x) x
#endif

#ifndef I18N_NOOP2_NOSTRIP
/**
 * @since 4.3
 *
 * Similar to I18N_NOOP2, except that neither of the two arguments is omitted.
 * This is typically used when contexts need to differ between static entries,
 * and only some of the entries need context:
 * \code
 *   struct MyTextLabels
 *   {
 *     int labelId;
 *     const char *context;
 *     const char *text;
 *   };
 *   const MyTextLabels labels[] = {
 *     { 10, I18N_NOOP2_NOSTRIP("new game", "New") },
 *     { 20, 0, I18N_NOOP("End Turn") },
 *     { 30, I18N_NOOP2_NOSTRIP("default move", "Default") },
 *     { 40, 0, I18N_NOOP("Quit") },
 *     ...
 *   };
 *   ...
 *   QString itemLabel = i18nc(labels[item].context, labels[item].text);
 * \endcode
 * Note that i18nc() will not have any problems with context being null,
 * it will simply behave as ordinary i18n().
 */
#define I18N_NOOP2_NOSTRIP(ctxt, text) ctxt, text
#endif

/**
 * @short Class for producing and handling localized messages
 *
 * KLocalizedString handles translation and specific needs of argument
 * substitution and formatting in localized message strings.
 *
 * Topics:
 *  - @ref gen_usage
 *  - @ref spec_usage
 *  - @ref subs_notes
 *  - @ref other_ref
 *
 * \section gen_usage General Usage
 *
 * This class should mostly not be used directly, but through wrapper i18n
 * calls which return QString, for localization of user visible messages in
 * applications.
 *
 * For the most frequent message type, the one without any arguments, you would
 * use simply:
 * \code
 *   QString msg = i18n("Just plain info");
 * \endcode
 *
 * If there are arguments to be substitued into the message, you just add them
 * after the message string:
 * \code
 *   QString msg = i18n("%1 has scored %2", playerName, score);
 * \endcode
 * There can be up to some final number of arguments added like this (i18n
 * is realized by overloaded templates). If you overrun this number, use
 * ki18n* series of calls (described below). You can use several types as
 * arguments, see subs methods.
 *
 * Sometimes a short message can be ambiguous in English, then you would use
 * the context version, i18nc. There the first string is context, and the
 * second is the message which really gets displayed:
 * \code
 *   QString msg = i18nc("Player name - score", "%1 - %2", playerName, score);
 * \endcode
 *
 * While English diferentiates plural forms only between 1 and else, in other
 * languages it might not be that simple, or it might be simpler. To handle
 * this properly, use plural call, i18np:
 * \code
 *   QString msg = i18np("One image in album %2", "%1 images in album %2",
 *                       numImages, albumName);
 * \endcode
 * Note that the plural form shall be decided by first integer-valued argument,
 * (numImages in the example above). In rare cases when there are two integer
 * arguments, you should take care to order them properly.
 *
 * Finally, message might need both context and plural, which is provided by
 * i18ncp call:
 * \code
 *   QString msg = i18ncp("Personal file", "One file", "%1 files", numFiles);
 * \endcode
 *
 * Be carefull not to use literal string as first argument after message text
 * in basic i18n() call. In debug mode, it will even trigger the static assert,
 * giving error at compile time. This is in order to prevent misnamed calls:
 * it may happen that you add context or plural to previously basic message,
 * but forget to change the name of the call.
 *
 * All message strings are expected to pass for well-formed XML, whether or
 * not the output device supports some form of markup.
 * Thus, predefined XML entities are always available: &amp;lt;, &amp;gt;,
 * &amp;amp;, &amp;apos;, and &amp;quot;. E.g. if you need a non-tag
 * less-than sign, use &amp;lt; entity instead.
 * The exception to the well-formed XML requirement is the ampersand (&amp;),
 * which is used a lot for marking accelerators, so you should not write it
 * as &amp;amp; (except in the very unlikely case when the construct with
 * the naked ampersand can be interpreted as an entity in itself).
 *
 * \section spec_usage Specialized Usage
 *
 * There are some situations where i18n* calls are not sufficient or
 * convenient. For one, if you need to substitute many arguments. Or, if you
 * find that you need to defer the substitution. For this you can use the
 * ki18n call which returns a KLocalizedString, substitute arguments
 * using its subs methods, and finalize the translation by calling
 * its toString method. For example:
 * \code
 *   KLocalizedString ks;
 *   case (reportSource) {
 *     SRC_ENG: ks = ki18n("Engineering reports: %1"); break;
 *     SRC_HEL: ks = ki18n("Helm reports: %1"); break;
 *     SRC_SON: ks = ki18n("Sonar reports: %1"); break;
 *     default: ks = ki18n("General report: %1");
 *   }
 *   QString msg = ks.subs(reportText).toString();
 * \endcode
 *
 * Another case is when you want extra formatting of arguments, like field
 * width or number of decimals. subs methods can take these formatting
 * parameters. In particular, you should @e never use some custom way to
 * format arguments, as subs methods will also properly localize them:
 * \code
 *   QString s = i18n("Rounds: %1", myNumberFormat(n, 8)); // bad, number not localized
 *   QString s = ki18n("Rounds: %1").subs(n, 8).toString(); // good, number localized
 * \endcode
 *
 * There are also context, plural and context-plural variants:
 * \code
 *   QString s = ki18nc("No function", "None").toString();
 *   QString s = ki18np("File found", "%1 files found").subs(n).toString();
 *   QString s = ki18ncp("Personal file", "One file", "%1 files").subs(n).toString();
 * \endcode
 *
 * If you need translation using locale (ie. KLocale object) other than the
 * default, you can use overloaded toString method which takes pointer to a
 * locale:
 * \code
 *   KLocale *myLocale;
 *   ...
 *   ki18n("Welcome").toString(myLocale);
 * \endcode
 *
 * Translators have a capability to script translations at runtime, which is
 * for the most part transparent to the programmer. However, sometimes the
 * programmer may help by providing some @e dynamic context to the message,
 * using the inContext method of KLocalizedString.  Unlike the ordinary
 * context, this one changes at runtime; translators have the means to fetch
 * it and use it to script the translation properly. An example:
 * \code
 *   KLocalizedString ks = ki18nc("%1 is user name; may have "
 *                                "dynamic context gender=[male,female]",
 *                                "%1 went offline");
 *   if (knownUsers.contains(user) && !knownUsers[user].gender.isEmpty()) {
 *     ks = ks.inContext("gender", knownUsers[user].gender);
 *   }
 *   QString msg = ks.subs(user).toString();
 * \endcode
 *  Several dynamic contexts, with different keys, can be added like this.
 *
 * \section subs_notes Placeholder Substitution
 *
 * Hopefully, for the most part placeholders are being substituted the way
 * you would intuitively expect them to be. Nevertheless:
 *
 * \li Placeholders are substituted in one pass, so no need to worry about
 *     argument itself containing a placeholder.
 *
 * \li All same-numbered placeholders are substituted with same argument.
 *
 * \li Placeholders directly index arguments: they should be numbered from 1
 *     upwards, without gaps in the sequence so that each argument is indexed.
 *     Otherwise you will get error marks in messages at runtime (when compiled
 *     in debug mode), and any invalid placeholder will be left unsubstituted.
 *     The exception is plural-deciding argument in plural call, where it is
 *     allowed to drop its placeholder in either singular or plural form.
 *
 * \li If none of the arguments supplied to a plural call is integer-valued,
 *     you will get an error mark in message at runtime (in debug mode).
 *
 * \li Plain number arguments will be normally formatted as if they denote
 *     amounts, according to language rules (thousands separation, etc.)
 *     But sometimes a number is a numerical identifier (e.g. port number),
 *     and to be treated as such, wrap the placeholder with the numid tag:
 *     \code
 *       QString msg = i18n("Using port <numid>%1</numid>", port);
 *     \endcode
 *
 * \section other_ref Further References
 *
 * <a href="http://techbase.kde.org/">KDE Techbase</a> contains a
 * <a href="http://techbase.kde.org/Development/Tutorials/Localization">
 * series of tutorials</a> on preparing the code for localization (and on
 * internationalization process in general), where the intended patterns of
 * usage of i18n API are covered in great detail.
 *
 * All i18n'd messages, whether sent to widgets expecting plain text or
 * allowing Qt rich text (HTML), support the new KDE semantic markup for
 * user interface text, KUIT in short. Semantic markup both increases the
 * consistency of visual presentation for the end user, and provides extra
 * information to translators, so that translations can be of higher quality.
 * KUIT is documented in an
 * <a href="http://techbase.kde.org/Development/Tutorials/Localization/i18n_Semantics">
 * Techbase article</a> as well.
 *
 * @see KLocale
 * @author Chusslove Illich \<caslav.ilic@gmx.net\>
 */
class KDECORE_EXPORT KLocalizedString
{

    friend KLocalizedString KDECORE_EXPORT ki18n (const char* msg);
    friend KLocalizedString KDECORE_EXPORT ki18nc (const char *ctxt, const char *msg);
    friend KLocalizedString KDECORE_EXPORT ki18np (const char *singular, const char *plural);
    friend KLocalizedString KDECORE_EXPORT ki18ncp (const char *ctxt,
                                     const char *singular, const char *plural);

public:
    /**
     * Constructs an empty message, which is not valid for finalization.
     * Useful when you later need to assign KLocalizedString obtained by one
     * of ki18n* calls.
     *
     * @see isEmpty()
     */
    explicit KLocalizedString ();

    /**
     * Copy constructor.
     */
    KLocalizedString (const KLocalizedString &rhs);

    /**
     * Assignment operator.
     */
    KLocalizedString& operator= (const KLocalizedString &rhs);

    /**
     * Destructor.
     */
    ~KLocalizedString ();

    /**
     * Finalizes the translation, creates QString with placeholders
     * substituted. Translations is obtained from default locale.
     *
     * If there was any mismatch between placeholders and arguments
     * returned string will contain error marks (in debug mode).
     *
     * @return finalized translation
     */
    QString toString () const;

    /**
     * Finalizes the translation, creates QString with placeholders
     * substituted. Translations is obtained from given locale. If locale
     * is NULL, original message is used instead of translated.
     *
     * If there was any mismatch between placeholders and arguments
     * returned string will contain error marks (in debug mode).
     *
     * @param locale locale from which translations are to be taken
     * @return finalized translation
     */
    QString toString (const KLocale *locale) const;

    /**
     * Checks whether the message is empty. This will happen if you just
     * constructed the object via default constructor.
     *
     * Empty messages are not valid for finalization; if you use toString() on
     * them, you will get error mark instead of empty QString (in debug mode).
     *
     * @return @c true if the message is empty, else @c false
     */
    bool isEmpty() const;

    /**
     * Substitutes an int argument into the message.
     *
     * @param a the argument
     * @param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @param base the radix used to represent the number as a string.
     *             Valid values range from 2 to 36
     * @param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * @return resultant KLocalizedString
     */
    KLocalizedString subs (int a, int fieldWidth = 0, int base = 10,
                           const QChar &fillChar = QLatin1Char(' ')) const;

    /**
     * Substitutes an unsigned int argument into the message.
     *
     * @param a the argument
     * @param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @param base the radix used to represent the number as a string.
     *             Valid values range from 2 to 36
     * @param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * @return resultant KLocalizedString
     */
    KLocalizedString subs (uint a, int fieldWidth = 0, int base = 10,
                           const QChar &fillChar = QLatin1Char(' ')) const;

    /**
     * Substitutes a long argument into the message.
     *
     * @param a the argument
     * @param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @param base the radix used to represent the number as a string.
     *             Valid values range from 2 to 36
     * @param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * @return resultant KLocalizedString
     */
    KLocalizedString subs (long a, int fieldWidth = 0, int base = 10,
                           const QChar &fillChar = QLatin1Char(' ')) const;

    /**
     * Substitutes an unsigned long argument into the message.
     *
     * @param a the argument
     * @param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @param base the radix used to represent the number as a string.
     *             Valid values range from 2 to 36
     * @param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * @return resultant KLocalizedString
     */
    KLocalizedString subs (ulong a, int fieldWidth = 0, int base = 10,
                           const QChar &fillChar = QLatin1Char(' ')) const;

    /**
     * Substitutes a long long argument into the message.
     *
     * @param a the argument
     * @param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @param base the radix used to represent the number as a string.
     *             Valid values range from 2 to 36
     * @param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * @return resultant KLocalizedString
     */
    KLocalizedString subs (qlonglong a, int fieldWidth = 0, int base = 10,
                           const QChar &fillChar = QLatin1Char(' ')) const;

    /**
     * Substitutes an unsigned long long argument into the message.
     *
     * @param a the argument
     * @param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @param base the radix used to represent the number as a string.
     *             Valid values range from 2 to 36
     * @param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * @return resultant KLocalizedString
     */
    KLocalizedString subs (qulonglong a, int fieldWidth = 0, int base = 10,
                           const QChar &fillChar = QLatin1Char(' ')) const;

    /**
     * Substitutes a double argument into the message.
     *
     * @param a the argument
     * @param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @param format type of floating point formating, like in QString::arg
     * @param precision number of digits after the decimal separator
     * @param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * @return resultant KLocalizedString
     */
    KLocalizedString subs (double a, int fieldWidth = 0,
                           char format = 'g', int precision = -1,
                           const QChar &fillChar = QLatin1Char(' ')) const;

    /**
     * Substitutes a QChar argument into the message.
     *
     * @param a the argument
     * @param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * @return resultant KLocalizedString
     */
    KLocalizedString subs (QChar a, int fieldWidth = 0,
                           const QChar &fillChar = QLatin1Char(' ')) const;

    /**
     * Substitutes a QString argument into the message.
     *
     * @param a the argument
     * @param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * @return resultant KLocalizedString
     */
    KLocalizedString subs (const QString &a, int fieldWidth = 0,
                           const QChar &fillChar = QLatin1Char(' ')) const;

    /**
     * Adds dynamic context to the message.
     *
     * @param key context key
     * @param text context value
     * @return resultant KLocalizedString
     */
    KLocalizedString inContext (const QString &key,
                                const QString &text) const;

    /**
     * @internal Called from KLocale on addition or removal of catalogs.
     */
    static void notifyCatalogsUpdated (const QStringList &languages,
                                       const QList<KCatalogName> &catalogs);

private:
    KLocalizedString (const char *ctxt,
                      const char *msg, const char *plural);

    KLocalizedStringPrivate * const d;
};

/**
* Creates localized string from a given message.
* Normaly you should use i18n() templates instead, as you need real
* KLocalizedString object only in special cases.
* All text arguments must be UTF-8 encoded and must not be empty or NULL.
*
* @param msg message text
* @return created KLocalizedString
*/
extern KLocalizedString KDECORE_EXPORT ki18n (const char* msg);

/**
* Creates localized string from a given message, with added context.
* Context is only for disambiguation purposes (both for lookup and
* for translators), it is not part of the message.
* Normaly you should use i18nc() templates instead, as you need real
* KLocalizedString object only in special cases.
* All text arguments must be UTF-8 encoded and must not be empty or NULL.
*
* @param ctxt context text
* @param msg message text
* @return created KLocalizedString
*/
extern KLocalizedString KDECORE_EXPORT ki18nc (const char *ctxt, const char *msg);

/**
* Creates localized string from a given plural and singular form.
* Normaly you should use i18np() templates instead, as you need real
* KLocalizedString object only in special cases.
* All text arguments must be UTF-8 encoded and must not be empty or NULL.
*
* @param singular message text in singular
* @param plural message text in plural
* @return created KLocalizedString
*/
extern KLocalizedString KDECORE_EXPORT ki18np (const char *singular, const char *plural);

/**
* Creates localized string from a given plural and singular form,
* with added context.
* Context is only for disambiguation purposes (both for lookup and
* for translators), it is not part of the message.
* Normaly you should use i18ncp() templates instead, as you need real
* KLocalizedString object only in special cases.
* All text arguments must be UTF-8 encoded and must not be empty or NULL.
*
* @param ctxt context text
* @param singular message text in singular
* @param plural message text in plural
* @return created KLocalizedString
*/
extern KLocalizedString KDECORE_EXPORT ki18ncp (const char *ctxt, const char *singular, const char *plural);

/**
 * Qt's uic generated translation calls go through numerous indirections
 * unnecessary in our case. So we use uic -tr tr2i18n to redirect them
 * to our i18n API.
**/
inline QString tr2i18n (const char *message, const char *comment = 0) {
    if (comment && comment[0] && message && message[0]) {
        return ki18nc(comment, message).toString();
    }
    else if (message && message[0]) {
        return ki18n(message).toString();
    }
    else {
        return QString();
    }
}

#ifndef NDEBUG
#define I18N_ERR_MSG String_literal_as_second_argument_to_i18n___Perhaps_you_need_i18nc_or_i18np
template <typename T, int s> class I18nTypeCheck {public: static void I18N_ERR_MSG () {}};
template <int s> class I18nTypeCheck<char[s], s> {};
#define STATIC_ASSERT_NOT_LITERAL_STRING(T) I18nTypeCheck<T, sizeof(T)>::I18N_ERR_MSG ();
#else
#define STATIC_ASSERT_NOT_LITERAL_STRING(T)
#endif

// >>>>> Basic calls
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string.
 * @param text string to be localized
 * @return localized string
 */
inline QString i18n (const char *text)
{
  return ki18n(text).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 1 argument.
 * @param text string to be localized
 * @param a1 first argument
 * @return localized string
 */
template <typename A1>
inline QString i18n (const char *text, const A1 &a1)
{
  STATIC_ASSERT_NOT_LITERAL_STRING(A1)
  return ki18n(text).subs(a1).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 2 arguments.
 * @param text string to be localized
 * @param a1 first argument
 * @param a2 second argument
 * @return localized string
 */
template <typename A1, typename A2>
inline QString i18n (const char *text, const A1 &a1, const A2 &a2)
{
  STATIC_ASSERT_NOT_LITERAL_STRING(A1)
  return ki18n(text).subs(a1).subs(a2).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 3 arguments.
 * @param text string to be localized
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3>
inline QString i18n (const char *text, const A1 &a1, const A2 &a2, const A3 &a3)
{
  STATIC_ASSERT_NOT_LITERAL_STRING(A1)
  return ki18n(text).subs(a1).subs(a2).subs(a3).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 4 arguments.
 * @param text string to be localized
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4>
inline QString i18n (const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
  STATIC_ASSERT_NOT_LITERAL_STRING(A1)
  return ki18n(text).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 5 arguments.
 * @param text string to be localized
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @param a5 fifth argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString i18n (const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
  STATIC_ASSERT_NOT_LITERAL_STRING(A1)
  return ki18n(text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 6 arguments.
 * @param text string to be localized
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @param a5 fifth argument
 * @param a6 sixth argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline QString i18n (const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
  STATIC_ASSERT_NOT_LITERAL_STRING(A1)
  return ki18n(text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 7 arguments.
 * @param text string to be localized
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @param a5 fifth argument
 * @param a6 sixth argument
 * @param a7 seventh argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline QString i18n (const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7)
{
  STATIC_ASSERT_NOT_LITERAL_STRING(A1)
  return ki18n(text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 8 arguments.
 * @param text string to be localized
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @param a5 fifth argument
 * @param a6 sixth argument
 * @param a7 seventh argument
 * @param a8 eighth argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline QString i18n (const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8)
{
  STATIC_ASSERT_NOT_LITERAL_STRING(A1)
  return ki18n(text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 9 arguments.
 * @param text string to be localized
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @param a5 fifth argument
 * @param a6 sixth argument
 * @param a7 seventh argument
 * @param a8 eighth argument
 * @param a9 ninth argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline QString i18n (const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8, const A9 &a9)
{
  STATIC_ASSERT_NOT_LITERAL_STRING(A1)
  return ki18n(text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).subs(a9).toString();
}
// <<<<<<< End of basic calls

// >>>>> Context calls
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string and a context.
 * @param ctxt context of the string
 * @param text string to be localized
 * @return localized string
 */
inline QString i18nc (const char *ctxt, const char *text)
{
  return ki18nc(ctxt, text).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 1 argument and a context.
 * @param ctxt context of the string
 * @param text string to be localized
 * @param a1 first argument
 * @return localized string
 */
template <typename A1>
inline QString i18nc (const char *ctxt, const char *text, const A1 &a1)
{
  return ki18nc(ctxt, text).subs(a1).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 2 arguments and a context.
 * @param ctxt context of the string
 * @param text string to be localized
 * @param a1 first argument
 * @param a2 second argument
 * @return localized string
 */
template <typename A1, typename A2>
inline QString i18nc (const char *ctxt, const char *text, const A1 &a1, const A2 &a2)
{
  return ki18nc(ctxt, text).subs(a1).subs(a2).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 3 arguments and a context.
 * @param ctxt context of the string
 * @param text string to be localized
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3>
inline QString i18nc (const char *ctxt, const char *text, const A1 &a1, const A2 &a2, const A3 &a3)
{
  return ki18nc(ctxt, text).subs(a1).subs(a2).subs(a3).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 4 arguments and a context.
 * @param ctxt context of the string
 * @param text string to be localized
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4>
inline QString i18nc (const char *ctxt, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
  return ki18nc(ctxt, text).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 5 arguments and a context.
 * @param ctxt context of the string
 * @param text string to be localized
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @param a5 fifth argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString i18nc (const char *ctxt, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
  return ki18nc(ctxt, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 6 arguments and a context.
 * @param ctxt context of the string
 * @param text string to be localized
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @param a5 fifth argument
 * @param a6 sixth argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline QString i18nc (const char *ctxt, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
  return ki18nc(ctxt, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 7 arguments and a context.
 * @param ctxt context of the string
 * @param text string to be localized
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @param a5 fifth argument
 * @param a6 sixth argument
 * @param a7 seventh argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline QString i18nc (const char *ctxt, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7)
{
  return ki18nc(ctxt, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 8 arguments and a context.
 * @param ctxt context of the string
 * @param text string to be localized
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @param a5 fifth argument
 * @param a6 sixth argument
 * @param a7 seventh argument
 * @param a8 eighth argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline QString i18nc (const char *ctxt, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8)
{
  return ki18nc(ctxt, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 9 arguments and a context.
 * @param ctxt context of the string
 * @param text string to be localized
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @param a5 fifth argument
 * @param a6 sixth argument
 * @param a7 seventh argument
 * @param a8 eighth argument
 * @param a9 ninth argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline QString i18nc (const char *ctxt, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8, const A9 &a9)
{
  return ki18nc(ctxt, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).subs(a9).toString();
}
// <<<<< End of context calls

// >>>>> Plural calls
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 1 argument using correct plural form.
 * @param sing string to be localized in singular
 * @param plur string to be localized in plural
 * @param a1 first argument
 * @return localized string
 */
template <typename A1>
inline QString i18np (const char *sing, const char *plur, const A1 &a1)
{
  return ki18np(sing, plur).subs(a1).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 2 arguments using correct plural form.
 * @param sing string to be localized in singular
 * @param plur string to be localized in plural
 * @param a1 first argument
 * @param a2 second argument
 * @return localized string
 */
template <typename A1, typename A2>
inline QString i18np (const char *sing, const char *plur, const A1 &a1, const A2 &a2)
{
  return ki18np(sing, plur).subs(a1).subs(a2).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 3 arguments using correct plural form.
 * @param sing string to be localized in singular
 * @param plur string to be localized in plural
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3>
inline QString i18np (const char *sing, const char *plur, const A1 &a1, const A2 &a2, const A3 &a3)
{
  return ki18np(sing, plur).subs(a1).subs(a2).subs(a3).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 4 arguments using correct plural form.
 * @param sing string to be localized in singular
 * @param plur string to be localized in plural
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4>
inline QString i18np (const char *sing, const char *plur, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
  return ki18np(sing, plur).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 5 arguments using correct plural form.
 * @param sing string to be localized in singular
 * @param plur string to be localized in plural
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @param a5 fifth argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString i18np (const char *sing, const char *plur, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
  return ki18np(sing, plur).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 6 arguments using correct plural form.
 * @param sing string to be localized in singular
 * @param plur string to be localized in plural
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @param a5 fifth argument
 * @param a6 sixth argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline QString i18np (const char *sing, const char *plur, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
  return ki18np(sing, plur).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 7 arguments using correct plural form.
 * @param sing string to be localized in singular
 * @param plur string to be localized in plural
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @param a5 fifth argument
 * @param a6 sixth argument
 * @param a7 seventh argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline QString i18np (const char *sing, const char *plur, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7)
{
  return ki18np(sing, plur).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 8 arguments using correct plural form.
 * @param sing string to be localized in singular
 * @param plur string to be localized in plural
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @param a5 fifth argument
 * @param a6 sixth argument
 * @param a7 seventh argument
 * @param a8 eighth argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline QString i18np (const char *sing, const char *plur, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8)
{
  return ki18np(sing, plur).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 9 arguments using correct plural form.
 * @param sing string to be localized in singular
 * @param plur string to be localized in plural
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @param a5 fifth argument
 * @param a6 sixth argument
 * @param a7 seventh argument
 * @param a8 eighth argument
 * @param a9 ninth argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline QString i18np (const char *sing, const char *plur, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8, const A9 &a9)
{
  return ki18np(sing, plur).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).subs(a9).toString();
}
// <<<<< End of plural calls

// >>>>> Context-plural calls
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 1 argument and a context using correct plural form.
 * @param ctxt context of the string
 * @param sing string to be localized in singular
 * @param plur string to be localized in plural
 * @param a1 first argument
 * @return localized string
 */
template <typename A1>
inline QString i18ncp (const char *ctxt, const char *sing, const char *plur, const A1 &a1)
{
  return ki18ncp(ctxt, sing, plur).subs(a1).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 2 arguments and a context using correct plural form.
 * @param ctxt context of the string
 * @param sing string to be localized in singular
 * @param plur string to be localized in plural
 * @param a1 first argument
 * @param a2 second argument
 * @return localized string
 */
template <typename A1, typename A2>
inline QString i18ncp (const char *ctxt, const char *sing, const char *plur, const A1 &a1, const A2 &a2)
{
  return ki18ncp(ctxt, sing, plur).subs(a1).subs(a2).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 3 arguments and a context using correct plural form.
 * @param ctxt context of the string
 * @param sing string to be localized in singular
 * @param plur string to be localized in plural
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3>
inline QString i18ncp (const char *ctxt, const char *sing, const char *plur, const A1 &a1, const A2 &a2, const A3 &a3)
{
  return ki18ncp(ctxt, sing, plur).subs(a1).subs(a2).subs(a3).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 4 arguments and a context using correct plural form.
 * @param ctxt context of the string
 * @param sing string to be localized in singular
 * @param plur string to be localized in plural
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4>
inline QString i18ncp (const char *ctxt, const char *sing, const char *plur, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
  return ki18ncp(ctxt, sing, plur).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 5 arguments and a context using correct plural form.
 * @param ctxt context of the string
 * @param sing string to be localized in singular
 * @param plur string to be localized in plural
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @param a5 fifth argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString i18ncp (const char *ctxt, const char *sing, const char *plur, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
  return ki18ncp(ctxt, sing, plur).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 6 arguments and a context using correct plural form.
 * @param ctxt context of the string
 * @param sing string to be localized in singular
 * @param plur string to be localized in plural
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @param a5 fifth argument
 * @param a6 sixth argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline QString i18ncp (const char *ctxt, const char *sing, const char *plur, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
  return ki18ncp(ctxt, sing, plur).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 7 arguments and a context using correct plural form.
 * @param ctxt context of the string
 * @param sing string to be localized in singular
 * @param plur string to be localized in plural
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @param a5 fifth argument
 * @param a6 sixth argument
 * @param a7 seventh argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline QString i18ncp (const char *ctxt, const char *sing, const char *plur, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7)
{
  return ki18ncp(ctxt, sing, plur).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 8 arguments and a context using correct plural form.
 * @param ctxt context of the string
 * @param sing string to be localized in singular
 * @param plur string to be localized in plural
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @param a5 fifth argument
 * @param a6 sixth argument
 * @param a7 seventh argument
 * @param a8 eighth argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline QString i18ncp (const char *ctxt, const char *sing, const char *plur, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8)
{
  return ki18ncp(ctxt, sing, plur).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).toString();
}
// Autogenerated; contact KLocalizedString maintainer for batch changes.
/**
 * Returns a localized version of a string with 9 arguments and a context using correct plural form.
 * @param ctxt context of the string
 * @param sing string to be localized in singular
 * @param plur string to be localized in plural
 * @param a1 first argument
 * @param a2 second argument
 * @param a3 third argument
 * @param a4 fourth argument
 * @param a5 fifth argument
 * @param a6 sixth argument
 * @param a7 seventh argument
 * @param a8 eighth argument
 * @param a9 ninth argument
 * @return localized string
 */
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline QString i18ncp (const char *ctxt, const char *sing, const char *plur, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8, const A9 &a9)
{
  return ki18ncp(ctxt, sing, plur).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).subs(a9).toString();
}
// <<<<< End of context-plural calls

#endif
