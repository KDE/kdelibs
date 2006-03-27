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

#include <qstring.h>
#include <kdelibs_export.h>

class KLocale;
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
 *   mystring = i18n( say_something() );
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
 * \warning You need to call i18nc( context, stringVar ) later on, not just
 * i18n( stringVar ).
 */
#define I18N_NOOP2(comment,x) x
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
 *   QString msg = i18np("1 image in album %1", "%n images in album %1",
 *                       numImages, albumName);
 * \endcode
 * Note the %n placeholder, which indicates the number the plural form shall
 * be decided upon. It will be substituted by the first argument after the
 * message strings (numImages in the example above).
 *
 * Finally, message might need both context and plural, which is provided by
 * i18ncp call:
 * \code
 *   QString msg = i18ncp("Personal file", "1 file", "%n files", numFiles);
 * \endcode
 *
 * Be carefull not to use literal string as first argument after message text
 * in basic i18n() call. In debug mode, it will even trigger the static assert,
 * giving error at compile time. This is in order to prevent misnamed calls:
 * it may happen that you add context or plural to previously basic message,
 * but forget to change the name of the call.
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
 * parameters. In particular, you shoul @e never use some custom way to
 * format arguments, as subs methods will also properly localize them:
 * \code
 *   QString s = i18n("Rounds: %1", myNumberFormat(n, 8)); // bad, number not localized
 *   QString s = ki18n("Rounds: %1").subs(n, 8).toString(); // good, number localized
 * \endcode
 *
 * There are also context, plural and context-plural variants:
 * \code
 *   QString s = ki18nc("No function", "None").toString();
 *   QString s = ki18np("1 file found", "%n files found").subs(n).toString();
 *   QString s = ki18ncp("Personal file", "1 file", "%n files").subs(n).toString();
 * \endcode
 *
 * Plural placeholder %n in this case will be substituted by the first
 * argument supplied by subs. If you give non-number as the first argument,
 * you will get a runtime error mark in the message (in debug mode).
 *
 * If you need translation using locale (ie. KLocale object) other than the
 * default, you can use overloaded toString method which takes pointer to a
 * locale:
 * \code
 *   KLocale *myLocale;
 *   ...
 *   ki18n("Welcome!").toString(myLocale);
 * \endcode
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
 * \li Always provide exactly as many arguments as there are unique
 *     placeholders. Otherwise you will get error marks in messages at runtime
 *     (when compiled in debug mode). The only exception is plural placeholder,
 *     which need not be present in singular form.
 *
 * \li Plural placeholder %n will be substituted by the first supplied
 *     argument. If you give non-number as the first argument, you will get a
 *     runtime error mark in the message (in debug mode).
 *
 * @see <a href="http://developer.kde.org/documentation/library/">[[Here goes the link to Programmer's i18n howto, sec. How to prepare the code]]</a>
 * @see KLocale
 * @author Chusslove Illich \<caslav.ilic@gmx.net\>
 */
class KDECORE_EXPORT KLocalizedString
{

    friend KLocalizedString ki18n (const char* msg);
    friend KLocalizedString ki18nc (const char *ctxt, const char *msg);
    friend KLocalizedString ki18np (const char *singular, const char *plural);
    friend KLocalizedString ki18ncp (const char *ctxt,
                                     const char *singular, const char *plural);

public:
    /**
     * Constructs an empty message, which is not valid for finalization.
     * Usefull when you later need to assign KLocalizedString obtained by one
     * of ki18n* calls.
     *
     * @see isEmpty()
     */
    explicit KLocalizedString ();

    /**
     * Copy constructor.
     */
    KLocalizedString (const KLocalizedString& rhs);

    /**
     * Assignment operator.
     */
    KLocalizedString& operator= (const KLocalizedString& rhs);

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
     * Empty messages are not valid for finalization; if you use toString()
     * on them, you will get error mark instead of empty QString (in debug mode).
     *
     * @return @c true if the message is empty, else @c false
     */
    bool isEmpty() const;

    /**
     * Substitutes an integer argument into the message.
     *
     * @param a the argument
     * @param fieldWidth width of the formated field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @return resultant KLocalizedString
     */
    KLocalizedString subs (int a, int fieldWidth = 0) const;

    /**
     * Substitutes an unsigned integer argument into the message.
     *
     * @param a the argument
     * @param fieldWidth width of the formated field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @return resultant KLocalizedString
     */
    KLocalizedString subs (uint a, int fieldWidth = 0) const;

    /**
     * Substitutes a long argument into the message.
     *
     * @param a the argument
     * @param fieldWidth width of the formated field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @return resultant KLocalizedString
     */
    KLocalizedString subs (long a, int fieldWidth = 0) const;

    /**
     * Substitutes an unsigned long argument into the message.
     *
     * @param a the argument
     * @param fieldWidth width of the formated field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @return resultant KLocalizedString
     */
    KLocalizedString subs (ulong a, int fieldWidth = 0) const;

    /**
     * Substitutes a double argument into the message.
     *
     * @param a the argument
     * @param fieldWidth width of the formated field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @param fmt type of floating point formating, like in QString::arg
     * @param prec number of digits after the decimal separator
     *
     * @see QString::arg()
     * @return resultant KLocalizedString
     */
    KLocalizedString subs (double a, int fieldWidth = 0,
                           char fmt = 'g', int prec = -1) const;

    /**
     * Substitutes a QChar argument into the message.
     *
     * @param a the argument
     * @param fieldWidth width of the formated field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @return resultant KLocalizedString
     */
    KLocalizedString subs (QChar a, int fieldWidth = 0) const;

    /**
     * Substitutes a QString argument into the message.
     *
     * @param a the argument
     * @param fieldWidth width of the formated field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @return resultant KLocalizedString
     */
    KLocalizedString subs (const QString& a, int fieldWidth = 0) const;

private:
    explicit KLocalizedString (const char *ctxt,
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
extern KLocalizedString ki18n (const char* msg);

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
extern KLocalizedString ki18nc (const char *ctxt, const char *msg);

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
extern KLocalizedString ki18np (const char *singular, const char *plural);

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
extern KLocalizedString ki18ncp (const char *ctxt, const char *singular, const char *plural);

#ifndef NDEBUG
#define I18N_ERR_MSG String_literal_as_second_argument_to_i18n___Perhaps_you_need_i18nc_or_i18np
template <typename T, int s> class I18nTypeCheck {public: static void I18N_ERR_MSG () {}};
template <int s> class I18nTypeCheck<char[s], s> {};
#define STATIC_ASSERT_NOT_LITERAL_STRING(T) I18nTypeCheck<T, sizeof(T)>::I18N_ERR_MSG ();
#else
#define STATIC_ASSERT_NOT_LITERAL_STRING(T)
#endif

// >>> Basic calls
inline QString i18n (const char *text)
{
    return ki18n(text).toString();
}
template <typename A1>
inline QString i18n (const char *text, const A1 &a1)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18n(text).subs(a1).toString();
}
template <typename A1, typename A2>
inline QString i18n (const char *text, const A1 &a1, const A2 &a2)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18n(text).subs(a1).subs(a2).toString();
}
template <typename A1, typename A2, typename A3>
inline QString i18n (const char *text, const A1 &a1, const A2 &a2, const A3 &a3)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18n(text).subs(a1).subs(a2).subs(a3).toString();
}
template <typename A1, typename A2, typename A3, typename A4>
inline QString i18n (const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18n(text).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
template <typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString i18n (const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18n(text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
// <<< End of basic calls

// >>> Context calls
inline QString i18nc (const char *ctxt, const char *text)
{
    return ki18nc(ctxt, text).toString();
}
template <typename A1>
inline QString i18nc (const char *ctxt, const char *text, const A1 &a1)
{
    return ki18nc(ctxt, text).subs(a1).toString();
}
template <typename A1, typename A2>
inline QString i18nc (const char *ctxt, const char *text, const A1 &a1, const A2 &a2)
{
    return ki18nc(ctxt, text).subs(a1).subs(a2).toString();
}
template <typename A1, typename A2, typename A3>
inline QString i18nc (const char *ctxt, const char *text, const A1 &a1, const A2 &a2, const A3 &a3)
{
    return ki18nc(ctxt, text).subs(a1).subs(a2).subs(a3).toString();
}
template <typename A1, typename A2, typename A3, typename A4>
inline QString i18nc (const char *ctxt, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    return ki18nc(ctxt, text).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
template <typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString i18nc (const char *ctxt, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    return ki18nc(ctxt, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
// <<< End of context calls

// >>> Plural calls
inline QString i18np (const char *sing, const char *plur, int n)
{
    return ki18np(sing, plur).subs(n).toString();
}
template <typename A1>
inline QString i18np (const char *sing, const char *plur, int n, const A1 &a1)
{
    return ki18np(sing, plur).subs(n).subs(a1).toString();
}
template <typename A1, typename A2>
inline QString i18np (const char *sing, const char *plur, int n, const A1 &a1, const A2 &a2)
{
    return ki18np(sing, plur).subs(n).subs(a1).subs(a2).toString();
}
template <typename A1, typename A2, typename A3>
inline QString i18np (const char *sing, const char *plur, int n, const A1 &a1, const A2 &a2, const A3 &a3)
{
    return ki18np(sing, plur).subs(n).subs(a1).subs(a2).subs(a3).toString();
}
template <typename A1, typename A2, typename A3, typename A4>
inline QString i18np (const char *sing, const char *plur, int n, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    return ki18np(sing, plur).subs(n).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
template <typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString i18np (const char *sing, const char *plur, int n, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    return ki18np(sing, plur).subs(n).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
// <<< End of plural calls

// >>> Context plural calls
inline QString i18ncp (const char *ctxt, const char *sing, const char *plur, int n)
{
    return ki18ncp(ctxt, sing, plur).subs(n).toString();
}
template <typename A1>
inline QString i18ncp (const char *ctxt, const char *sing, const char *plur, int n, const A1 &a1)
{
    return ki18ncp(ctxt, sing, plur).subs(n).subs(a1).toString();
}
template <typename A1, typename A2>
inline QString i18ncp (const char *ctxt, const char *sing, const char *plur, int n, const A1 &a1, const A2 &a2)
{
    return ki18ncp(ctxt, sing, plur).subs(n).subs(a1).subs(a2).toString();
}
template <typename A1, typename A2, typename A3>
inline QString i18ncp (const char *ctxt, const char *sing, const char *plur, int n, const A1 &a1, const A2 &a2, const A3 &a3)
{
    return ki18ncp(ctxt, sing, plur).subs(n).subs(a1).subs(a2).subs(a3).toString();
}
template <typename A1, typename A2, typename A3, typename A4>
inline QString i18ncp (const char *ctxt, const char *sing, const char *plur, int n, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    return ki18ncp(ctxt, sing, plur).subs(n).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
template <typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString i18ncp (const char *ctxt, const char *sing, const char *plur, int n, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    return ki18ncp(ctxt, sing, plur).subs(n).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
// <<< End of context plural calls

/**
 * Qt3's uic generates i18n( "msg", "comment" ) calls which conflict
 * with our i18n method. We use uic -tr tr2i18n to redirect
 * to the right i18n() function
**/
inline QString tr2i18n(const char* message, const char* =0) {
    if (!message || !message[0])
        return QString();
    return i18n(message);
}

#endif
