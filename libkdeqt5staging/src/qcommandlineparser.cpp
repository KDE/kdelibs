/****************************************************************************
**
** Copyright (C) 2012 author Laszlo Papp <lpapp@kde.org>
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qcommandlineparser.h"

#include <qcoreapplication.h>
#include <qhash.h>
#include <stdio.h>
#include <stdlib.h>

QT_BEGIN_NAMESPACE

typedef QHash<QString, quint16> NameHash_t;

// The maximum number of options allowed.
static const NameHash_t::mapped_type maxOptionCount = ~0 - 1;

// Special value for "not found" when doing hash lookups.
static const NameHash_t::mapped_type optionNotFound = ~0;

class QCommandLineParserPrivate
{
public:
    inline QCommandLineParserPrivate()
        : parseAfterDoubleDash(true), needsParsing(true)
    { }

    bool parse();
    bool parse(const QStringList &arguments);
    bool isSet(const QString &name) const;
    QStringList aliases(const QString &name) const;
    void showHelp();

    //! The command line options used for parsing
    QList<QCommandLineOption> commandLineOptionList;

    //! Hash mapping option names to their offsets in commandLineOptionList and optionArgumentList.
    NameHash_t nameHash;

    //! Option arguments found (only for WithValue options)
    QHash<quint16, QStringList> optionArgumentListHash;

    //! Names of options found on the command line.
    QStringList optionNames;

    //! Arguments which did not belong to any option.
    QStringList remainingArgumentList;

    //! Names of options which were unknown.
    QStringList unknownOptionNames;

    //! Application version
    QString version;

    //! Application description
    QString description;

    /*!
        Boolean variable whether or not to stop the command line argument
        parsing after the double dash occurence without any options names involved
        ('--').

        Set to \c true by default.
     */
    bool parseAfterDoubleDash;

    //! True if parse() needs to be called
    bool needsParsing;
};

QStringList QCommandLineParserPrivate::aliases(const QString &optionName) const
{
    const NameHash_t::mapped_type optionOffset = nameHash.value(optionName, optionNotFound);
    if (optionOffset == optionNotFound)
        return QStringList();
    return commandLineOptionList.at(optionOffset).names();
}

/*!
    \since 5.2
    \class QCommandLineParser

    \brief The QCommandLineParser class provides a means for handling the
    options on the command line interface.

    This parser finds the options and their values on the command line. The
    parser handles short names, long names, more than one name for the same
    option, and option values.

    The class can parse the builtin and custom options passed to the class. It
    can subsequently return the option value(s) found, or a list of values
    without assigned option names (i.e. remaining values).

    Options on the command line are recognized as starting with a single or
    double dash character(s). The option "-" (single dash) is a special case,
    and not treated as an option. By default, the parser will stop parsing once
    the option "--" (double dash) is encountered, although this behaviour can be
    changed.

    Short options are single letters. The option "v" would be specified by
    passing "-v" on the command line. Short options cannot be bundled due to the
    existing limitations, namely: certain tools have already been using the
    -longname pattern. Therefore, the bundled short options could essentially
    clash with a long option name.

    Long options are more than one letter long. The long option "foobar" would
    be passed as "--foobar" or "-foobar". Long options can not obviously be
    bundled together either.

    Short options, taking an argument, cannot use the remaining characters in
    the same argument. For example, if "v" takes an argument, passing "-vfoobar"
    cannot treat "foobar" as v's argument since "vfoobar" could clash with the
    equally named long option. One way to put the values is to have assignment
    operator to mark the end of the short name, as shown here: "-v=value".
    If there is no assignment operator, the next argument is used.

    Long options are similar as they also require an assignment operator to
    mark the end of the long name, such as shown here: "--foobar=value". If
    there is no assignment operator, the next argument is used - even if it
    starts with a dash.

    Using an option value is encouraged by the class instead of counting the
    option occurences on the command line when the number of option occurences
    would otherwise define the exact operation. For example, the class does not
    handle the option "-vvvv" passed, as expected. It considers this as a long
    option name "vvvv". The preferred usage is "-v=4" or "-v 4" in those cases.
    It is somewhat a simpler form, and does not potentially clash with the
    equally named long option names.

    The parser does not support optional arguments - if an option is set to
    require an argument, one must be present. If such an option is placed last
    and has no argument, the option will be treated as if it had not been
    specified.

    The parser does not automatically support negating or disabling long options
    by using the format "--disable-foobar" or "--no-foobar". Although, a caller
    could make an option with "no-foobar" as one of its names, and handle the
    case explicitly themselves.

    The value of the options can be a string, string list or just the fact
    whether or not the option is set. There are convenience methods established
    in this class for getting the value(s). It is not necessary for the caller
    to convert to other types manually.

    The parsing happens behind the scenes, thus it is not necessary for the
    caller to call that explicitely. Once the values are requested, the parsing
    takes place internally on demand.

    \sa QCommandLineOption, QCoreApplication
*/

/*!
    \internal

    Constructs a command line parser object.

    The command line parser should never be constructed explicitely.
    QCoreApplication handles the construction when the application launches.
*/
QCommandLineParser::QCommandLineParser()
    : d(new QCommandLineParserPrivate)
{
}

/*!
    \internal

    Destroys the command line parser object.

    The command line parser should never be destructed explicitely.
    QCoreApplication handles the destruction when the application terminates.
*/
QCommandLineParser::~QCommandLineParser()
{
    delete d;
}

/*!
    Add the option \a option to look for while parsing.

    Returns true if the option adding was successful; otherwise returns false.

    The option adding fails, if there is no name attached to the option, or
    there is a name attached that clashes with an option name added before.
    There is also currently a maximum limit of 65535 options. Subsequent
    additions fail.

    Adding the option may also fail if memory cannot be allocated.
 */
bool QCommandLineParser::addOption(const QCommandLineOption &option)
{
    d->needsParsing = true;
    QStringList optionNames = option.names();

    if (d->commandLineOptionList.size() < maxOptionCount && !optionNames.isEmpty()) {

        foreach (const QString &name, optionNames) {
            if (d->nameHash.contains(name)) {
                return false;
            }
        }

        d->commandLineOptionList.append(option);

        const quint16 offset = (quint16)(d->commandLineOptionList.size() - 1);

        foreach (const QString &name, optionNames) {
            d->nameHash.insert(name, offset);
        }

        return true;
    }

    return false;
}

/*!
    Sets the application \a version and adds the -v / --version option.
    This option is handled automatically by QCommandLineParser.
*/
void QCommandLineParser::addVersionOption(const QString &version)
{
    d->version = version;
    addOption(QCommandLineOption(QStringList() << QLatin1String("v") << QLatin1String("version"), QObject::tr("Displays version information.")));
}

/*!
    Returns the application version set in addVersionOption().
*/
QString QCommandLineParser::applicationVersion() const
{
    return d->version;
}

/*!
    Adds the help option (-h, --help and /? on Windows)
    This option is handled automatically by QCommandLineParser.

    The application \a description will be displayed when this option is used.
    Example:
        addHelpOption(QCoreApplication::translate("main", "The best application in the world"));
*/
void QCommandLineParser::addHelpOption(const QString &description)
{
    d->description = description;
#ifdef Q_OS_WIN32
    addOption(QCommandLineOption(QStringList() << QLatin1String("h") << QLatin1String("help") << QLatin1String("?"), QObject::tr("Displays this help.")));
#else
    addOption(QCommandLineOption(QStringList() << QLatin1String("h") << QLatin1String("help"), QObject::tr("Displays this help.")));
#endif
}

/*!
    Returns the application description set in addHelpOption().
*/
QString QCommandLineParser::applicationDescription() const
{
    return d->description;
}

/*!
    \internal

    Parse the command line arguments.

    Returns true if the command line parsing was successful; otherwise returns
    false.

    The command line is obtained from the current \c QCoreApplication
    instance - it will fail if this is not available. The first argument
    in the list is the program name and is skipped.

    \sa parse(const QStringList&), QCoreApplication::arguments(), QCoreApplication::commandLineParser()
 */
bool QCommandLineParserPrivate::parse()
{
    if (!needsParsing) {
        return true;
    }
    needsParsing = false;
    QCoreApplication *pApp = QCoreApplication::instance();
    if (pApp) {
        QStringList args = pApp->arguments();
        if (!args.isEmpty()) {
            args.removeFirst();
            return parse(args);
        }
    }
    return false;

}

/*!
    \internal

    Parse the list of arguments \a arguments.

    Returns true if the command line parsing was successful; otherwise returns
    false.

    Any results from a previous parse operation are removed. If
    \c m_bStopParsingAtDoubleDash is \c true the parser will not look for
    further options once it encounters the option "--"; this does not
    include when "--" follows an option that requires an argument.

    Options that were successfully recognized, and their arguments, are
    removed from the input list. If \c m_bRemoveUnknownLongNames is
    \c true, unrecognized options are removed and placed into a list of
    unknown option names. Anything left over is placed into a list of
    leftover arguments.

    A long option that does not take an argument will still be recognized
    if encountered in the form "--foo=value". In this case, the argument
    value will be ignored.
 */
bool QCommandLineParserPrivate::parse(const QStringList &arguments)
{
    const QStringList emptyList;
    const QString     emptyString(QLatin1String(""));
    const QString     doubleDashString(QLatin1String("--"));
    const QLatin1Char dashChar('-');
    const QLatin1Char slashChar('/');
    const QLatin1Char assignChar('=');

    remainingArgumentList.clear();
    optionNames.clear();
    unknownOptionNames.clear();

    for (QStringList::const_iterator argumentIterator = arguments.begin(); argumentIterator != arguments.end() ; ++argumentIterator) {
        QString argument = *argumentIterator;

        if (argument.startsWith(doubleDashString)) {
            if (argument.length() > 2) {
                QString optionName = argument.mid(2).section(assignChar, 0, 0);

                if (nameHash.contains(optionName)) {
                    optionNames.append(optionName);
                    const NameHash_t::mapped_type optionOffset = *nameHash.find(optionName);
                    const QCommandLineOption::OptionType type = commandLineOptionList.at(optionOffset).optionType();

                    if (type == QCommandLineOption::WithValue) {
                        if (!argument.contains(assignChar)) {
                            ++argumentIterator;

                            if (argumentIterator != arguments.end()) {
                                optionArgumentListHash[optionOffset].append(*argumentIterator);
                            }
                        } else {
                            optionArgumentListHash[optionOffset].append(argument.section(assignChar, 1));
                        }
                    }
                } else {
                    unknownOptionNames.append(optionName);
                }
            }
            else {
                if (parseAfterDoubleDash == true) {
                    remainingArgumentList.append(argument);
                } else {
                    break;
                }
            }
        }

        else if (
#ifdef Q_OS_WIN
                argument.startsWith(slashChar) ||
#endif
                argument.startsWith(dashChar)) {
            QString optionName = argument.mid(1);
            if (!optionName.isEmpty()) {
                if (nameHash.contains(optionName)) {
                    optionNames.append(optionName);
                    const NameHash_t::mapped_type optionOffset = *nameHash.find(optionName);
                    const QCommandLineOption::OptionType type = commandLineOptionList.at(optionOffset).optionType();
                    if (type == QCommandLineOption::WithValue) {
                        if (!argument.contains(assignChar)) {
                            ++argumentIterator;

                            if (argumentIterator != arguments.end()) {
                                optionArgumentListHash[optionOffset].append(*argumentIterator);
                            }
                        } else {
                            optionArgumentListHash[optionOffset].append(argument.section(assignChar, 1));
                        }
                    }
                } else {
                    unknownOptionNames.append(optionName);
                }
            } else {
                remainingArgumentList.append(argument);
            }
        } else {
            remainingArgumentList.append(argument);
        }
    }

    if (!version.isEmpty() && isSet(QStringLiteral("version"))) {
        printf("%s %s\n", qPrintable(QCoreApplication::applicationName()), qPrintable(version));
        ::exit(0);
    }

    if (!description.isEmpty() && isSet(QStringLiteral("help"))) {
        showHelp();
    }

    return true;
}

/*!
    Return true if the option \a name was set, false otherwise.

    This is the recommended way to check for options with no values.

    The name provided can be any long or short name of any option that was
    added with \c addOption(). All the options names are treated as being
    equivalent. If the name is not recognized or that option was not present,
    false is returned.
 */

bool QCommandLineParser::isSet(const QString &name) const
{
    d->parse();
    return d->isSet(name);
}

bool QCommandLineParserPrivate::isSet(const QString &name) const
{
    if (optionNames.contains(name))
        return true;
    foreach (const QString &optionName, optionNames) {
        if (aliases(optionName).contains(name))
            return true;
    }
    return false;
}

/*!
    Return the option value found for the given option name \a optionName, or
    null string if not found.

    The name provided can be any long or short name of any option that was
    added with \c addOption(). All the option names are treated as being
    equivalent. If the name is not recognized or that option was not present, a
    null string is returned.

    For options found by the parser, an empty string is returned if the
    option does not take an argument, otherwise the last argument found for
    that option is returned.

    \sa arguments()
 */

QString QCommandLineParser::argument(const QString &optionName) const
{
    d->parse();
    const QStringList argumentList = arguments(optionName);

    if (!argumentList.isEmpty()) {
        return argumentList.last();
    }

    return QString();
}

/*!
    Return a list of option values found for the given option name \a
    optionName, or null string if not found.

    The name provided can be any long or short name of any option that was
    added with \c addOption(). All the options names are treated as being
    equivalent. If the name is not recognized or that option was not present, a
    null string is returned.

    For options found by the parser, the list will contain an entry for
    each time the option was encountered by the parser. These entries
    will always be an empty string for options that do not take an argument.
    Options that do take an argument will have the list populated with the
    argument values in the order they were found.

    \sa argument()
 */

QStringList QCommandLineParser::arguments(const QString &optionName) const
{
    d->parse();
    const NameHash_t::mapped_type optionOffset = d->nameHash.value(optionName, optionNotFound);
    if (optionOffset != optionNotFound) {
        QStringList args = d->optionArgumentListHash.value(optionOffset);
        if (args.isEmpty())
            args = d->commandLineOptionList.at(optionOffset).defaultValues();
        return args;
    }

    return QStringList();
}

/*!
    Return a list of remaining arguments.

    These are all of the arguments that were not recognized as part of an
    option.
 */

QStringList QCommandLineParser::remainingArguments() const
{
    d->parse();
    return d->remainingArgumentList;
}

/*!
    Return a list of option names that were found.

    This returns a list of all the recognized option names found by the
    parser, in the order in which they were found. For any long options
    that were in the form "--foo=value", the value part will have been
    dropped.

    The names in this list do not include the preceding dash characters.
    Names may appear more than once in this list if they were encountered
    more than once by the parser.

    Any entry in the list can be used with \c getArgument() or with
    \c getArgumentList() to get any relevant arguments.
 */

QStringList QCommandLineParser::optionNames() const
{
    d->parse();
    return d->optionNames;
}

/*!
    Return a list of unknown option names.

    This list will include both long an short name options that were not
    recognized. For any long options that were in the form "--foo=value",
    the value part will have been dropped and only the long name is added.

    The names in this list do not include the preceding dash characters.
    Names may appear more than once in this list if they were encountered
    more than once by the parser.

    \sa optionNames()
 */

QStringList QCommandLineParser::unknownOptionNames() const
{
    d->parse();
    return d->unknownOptionNames;
}

/*!
    Displays the help information, and exits the application.
    This is automatically triggered by the --help option, but can also
    be used to display the help when the user is not invoking the
    application correctly.
*/
void QCommandLineParser::showHelp()
{
    d->showHelp();
}

void QCommandLineParserPrivate::showHelp()
{
    const QString exeName = QCoreApplication::instance()->arguments().first();
    QString usage = QCoreApplication::translate("QCommandLineParser", "Usage: %1").arg(exeName);
    if (!commandLineOptionList.isEmpty()) {
        usage += QLatin1Char(' ');
        usage += QCoreApplication::translate("QCommandLineParser", "[options]");
        // TODO what about remaining arguments? e.g. "file"
    }
    fprintf(stdout, "%s\n\n%s\n\n", qPrintable(usage), qPrintable(description));
    if (!commandLineOptionList.isEmpty()) {
        fprintf(stdout, "%s\n", qPrintable(QCoreApplication::translate("QCommandLineParser", "Options:")));
    }
    QString optionFormatString = QString::fromLatin1("  %1 %2");
    foreach (const QCommandLineOption &option, commandLineOptionList) {
        QStringList optionNames;
        foreach (const QString &optionName, option.names()) {
            if (optionName.length() == 1)
                optionNames.append(QLatin1Char('-') + optionName);
            else
                optionNames.append(QStringLiteral("--") + optionName);
        }
        const QString optionNamesString = optionNames.join(QStringLiteral(", "));
        const QString optionString = optionFormatString.arg(optionNamesString, -25).arg(option.description());
        fprintf(stdout, "%s\n", qPrintable(optionString));
    }
    ::exit(0);
}

QT_END_NAMESPACE
