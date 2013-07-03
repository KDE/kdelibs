/****************************************************************************
**
** Copyright (C) 2013 Laszlo Papp <lpapp@kde.org>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
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

typedef QHash<QString, int> NameHash_t;

// Special value for "not found" when doing hash lookups.
static const NameHash_t::mapped_type optionNotFound = ~0;

class QCommandLineParserPrivate
{
public:
    inline QCommandLineParserPrivate()
        : builtinVersionOption(false),
          needsParsing(true)
    { }

    void parse(const QStringList &args);
    void checkParsed(const char *method);
    QStringList aliases(const QString &name) const;
    void showHelp();

    //! The command line options used for parsing
    QList<QCommandLineOption> commandLineOptionList;

    //! Hash mapping option names to their offsets in commandLineOptionList and optionArgumentList.
    NameHash_t nameHash;

    //! Option values found (only for options with a value)
    QHash<int, QStringList> optionValuesHash;

    //! Names of options found on the command line.
    QStringList optionNames;

    //! Arguments which did not belong to any option.
    QStringList remainingArgumentList;

    //! Names of options which were unknown.
    QStringList unknownOptionNames;

    //! Whether addVersionOption was called
    bool builtinVersionOption;

    //! Application description
    QString description;

    //! Documentation for remaining arguments
    QString remainingArgsHelpText;

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
    \inmodule QtCore
    \ingroup tools

    \brief The QCommandLineParser class provides a means for handling the
    command line options.

    QCoreApplication provides the command-line arguments as a simple list of strings.
    QCommandLineParser provides the ability to define a set of options, parse the
    command-line arguments, and store which options have actually been used, as
    well as option values.

    Any argument that isn't an option (i.e. doesn't start with a \c{-}, nor
    \c{/} on Windows) is stored as a "remaining argument".

    The parser handles short names, long names, more than one name for the same
    option, and option values.

    Options on the command line are recognized as starting with a single or
    double \c{-} character(s). On Windows, they can also start with a \c{/} character.
    The option \c{-} (single dash alone) is a special case, often meaning standard
    input, and not treated as an option. The parser will treat everything after the
    option \c{--} (double dash) as remaining arguments.

    Short options are single letters. The option \c{v} would be specified by
    passing \c{-v} on the command line. Short options cannot be bundled due to the
    existing limitations, namely: certain tools have already been using the
    \c{-longname} pattern. Therefore, the bundled short options could essentially
    clash with a long option name.

    Long options are more than one letter long. The long option \c{verbose} would
    be passed as \c{--verbose} or \c{-verbose}. Long options can not obviously be
    bundled together either.

    Passing values to options can be done using the assignment operator: \c{-v=value}
    \c{--verbose=value}, or a space: \c{-v value} \c{--verbose value}, i.e. the next
    argument is used as value (even if it starts with a \c{-}).

    The value cannot be provided by using the remaining characters after a short option,
    in the same argument. For example, if \c{v} takes an argument, passing \c{-vverbose}
    cannot treat \c{verbose} as v's argument since \c{vverbose} could clash with the
    equally named long option.

    Using an option value is encouraged by the class instead of counting the
    option occurrences on the command line when the number of option occurrences
    would otherwise define the exact operation. For example, the class does not
    handle the option \c{-vvvv} passed, as expected. It considers this as a long
    option name \c{vvvv}. The preferred usage is \c{-v=4} or \c{-v 4} in those cases.
    It is somewhat a simpler form, and does not potentially clash with the
    equally named long option names.

    The parser does not support optional values - if an option is set to
    require a value, one must be present. If such an option is placed last
    and has no value, the option will be treated as if it had not been
    specified.

    The parser does not automatically support negating or disabling long options
    by using the format \c{--disable-option} or \c{--no-option}. However, it is
    possible to handle this case explicitly by making an option with \c{no-option}
    as one of its names, and handle the case explicitly themselves.

    The value of the options can be a string, string list or just the fact
    whether or not the option is set. There are convenience methods established
    in this class for getting the value(s). It is not necessary for the caller
    to convert to other types manually.

    \sa QCommandLineOption, QCoreApplication
*/

/*!
    Constructs a command line parser object.
*/
QCommandLineParser::QCommandLineParser()
    : d(new QCommandLineParserPrivate)
{
}

/*!
    Destroys the command line parser object.
*/
QCommandLineParser::~QCommandLineParser()
{
    delete d;
}

/*!
    Adds the option \a option to look for while parsing.

    Returns true if adding the option was successful; otherwise returns false.

    Adding the option fails if there is no name attached to the option, or
    the option has a name that clashes with an option name added before.
 */
bool QCommandLineParser::addOption(const QCommandLineOption &option)
{
    QStringList optionNames = option.names();

    if (!optionNames.isEmpty()) {
        foreach (const QString &name, optionNames) {
            if (d->nameHash.contains(name))
                return false;
        }

        d->commandLineOptionList.append(option);

        const int offset = d->commandLineOptionList.size() - 1;

        foreach (const QString &name, optionNames)
            d->nameHash.insert(name, offset);

        return true;
    }

    return false;
}

/*!
    Adds the -v / --version option.
    This option is handled automatically by QCommandLineParser.
    You can set the actual version string using QCoreApplication::setApplicationVersion()
*/
void QCommandLineParser::addVersionOption()
{
    d->builtinVersionOption = true;
    addOption(QCommandLineOption(QStringList() << QLatin1String("v") << QLatin1String("version"), tr("Displays version information.")));
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
    addOption(QCommandLineOption(QStringList()
#ifdef Q_OS_WIN
                << QLatin1String("?")
#endif
                << QLatin1String("h")
                << QLatin1String("help"), tr("Displays this help.")));
}

/*!
    Returns the application description set in addHelpOption().
*/
QString QCommandLineParser::applicationDescription() const
{
    return d->description;
}

/*!
    Sets the description for the remaining arguments, as it
    should appear when using --help, to \a helpText.
    Example: "file", or "[files]" (if optional), or "source destination" (for "cp").

    \sa addHelpOption()
*/
void QCommandLineParser::setRemainingArgumentsHelpText(const QString &helpText)
{
    d->remainingArgsHelpText = helpText;
}


/*!
    Parses the command line \a arguments.

    Most programs don't need to call this, a simple call to process(app) is enough.

    parse() is more low-level, and only does the parsing. The application will have to
    take care of the error handling on unknown options, using unknownOptionNames().
    This can be useful for instance to show a graphical error message in graphical programs.

    Calling parse() instead of process() can also be useful in order to ignore unknown
    options temporarily, because more option definitions will be provided later on
    (depending on one of the arguments), before calling process().

    Don't forget that \a arguments starts with the name of the executable (ignored, though).
*/
void QCommandLineParser::parse(const QStringList &arguments)
{
    d->parse(arguments);
}

/*!
    Process the command line arguments.

    This means both parsing them, and handling the builtin options,
    --version if addVersionOption was called, --help if addHelpOption was called,
    as well as aborting on unknown option names, with an error message.

    The command line is obtained from the QCoreApplication instance \a app.

    \sa QCoreApplication::arguments()
 */
void QCommandLineParser::process(const QCoreApplication &app)
{
    d->parse(app.arguments());

    if (d->builtinVersionOption && isSet(QStringLiteral("version"))) {
        printf("%s %s\n", qPrintable(QCoreApplication::applicationName()), qPrintable(QCoreApplication::applicationVersion()));
        ::exit(0);
    }

    if (!d->description.isEmpty() && isSet(QStringLiteral("help")))
        showHelp();

    if (d->unknownOptionNames.count() == 1) {
        fprintf(stderr, "Unknown option '%s'.\n", qPrintable(d->unknownOptionNames.first()));
        ::exit(1);
    } else if (d->unknownOptionNames.count() > 1) {
        fprintf(stderr, "Unknown options: %s.\n", qPrintable(d->unknownOptionNames.join(QLatin1String(", "))));
        ::exit(1);
    }
}

void QCommandLineParserPrivate::checkParsed(const char *method)
{
    if (needsParsing) {
        qWarning("QCommandLineParser: call process or parse before %s", method);
    }
}

/*!
    \internal

    Parse the list of arguments \a arguments.

    Returns true if the command line parsing was successful; otherwise returns
    false.

    Any results from a previous parse operation are removed.
    The parser will not look for further options once it encounters the option
    \c{--}; this does not include when \c{--} follows an option that requires a value.

    Options that were successfully recognized, and their values, are
    removed from the input list. If \c m_bRemoveUnknownLongNames is
    \c true, unrecognized options are removed and placed into a list of
    unknown option names. Anything left over is placed into a list of
    leftover arguments.

    A long option that does not take a value will still be recognized
    if encountered in the form \c{--option=value}. In this case, the argument
    value will be ignored.
 */
void QCommandLineParserPrivate::parse(const QStringList &args)
{
    needsParsing = false;

    const QString     doubleDashString(QStringLiteral("--"));
    const QLatin1Char dashChar('-');
    const QLatin1Char slashChar('/');
    const QLatin1Char assignChar('=');

    bool doubleDashFound = false;
    remainingArgumentList.clear();
    optionNames.clear();
    unknownOptionNames.clear();
    optionValuesHash.clear();

    QStringList arguments = args;
    arguments.removeFirst();

    for (QStringList::const_iterator argumentIterator = arguments.constBegin(); argumentIterator != arguments.constEnd() ; ++argumentIterator) {
        QString argument = *argumentIterator;

        if (doubleDashFound) {
            remainingArgumentList.append(argument);
        } else if (argument.startsWith(doubleDashString)) {
            if (argument.length() > 2) {
                QString optionName = argument.mid(2).section(assignChar, 0, 0);

                const NameHash_t::const_iterator nameHashIt = nameHash.constFind(optionName);
                if (nameHashIt != nameHash.constEnd()) {
                    optionNames.append(optionName);
                    const NameHash_t::mapped_type optionOffset = *nameHashIt;
                    const bool withValue = !commandLineOptionList.at(optionOffset).valueName().isEmpty();
                    if (withValue) {
                        if (!argument.contains(assignChar)) {
                            ++argumentIterator;

                            if (argumentIterator != arguments.constEnd())
                                optionValuesHash[optionOffset].append(*argumentIterator);
                        } else {
                            optionValuesHash[optionOffset].append(argument.section(assignChar, 1));
                        }
                    }
                } else {
                    unknownOptionNames.append(optionName);
                }
            } else {
                doubleDashFound = true;
            }
        }

        else if (
#ifdef Q_OS_WIN
                argument.startsWith(slashChar) ||
#endif
                argument.startsWith(dashChar)) {
            QString optionName = argument.mid(1);
            if (!optionName.isEmpty()) {
                const NameHash_t::const_iterator nameHashIt = nameHash.constFind(optionName);
                if (nameHashIt != nameHash.constEnd()) {
                    optionNames.append(optionName);
                    const NameHash_t::mapped_type optionOffset = *nameHashIt;
                    const bool withValue = !commandLineOptionList.at(optionOffset).valueName().isEmpty();
                    if (withValue) {
                        if (!argument.contains(assignChar)) {
                            ++argumentIterator;

                            if (argumentIterator != arguments.constEnd( ))
                                optionValuesHash[optionOffset].append(*argumentIterator);
                        } else {
                            optionValuesHash[optionOffset].append(argument.section(assignChar, 1));
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
}

/*!
    Checks whether the option \a name was passed to the application.

    Returns true if the option \a name was set, false otherwise.

    This is the recommended way to check for options with no values.

    The name provided can be any long or short name of any option that was
    added with \c addOption(). All the options names are treated as being
    equivalent. If the name is not recognized or that option was not present,
    false is returned.

    Example:
    \snippet code/src_corelib_tools_qcommandlineparser.cpp 0
 */

bool QCommandLineParser::isSet(const QString &name) const
{
    d->checkParsed("isSet");
    if (d->optionNames.contains(name))
        return true;
    foreach (const QString &optionName, d->optionNames) {
        if (d->aliases(optionName).contains(name))
            return true;
    }
    return false;
}

/*!
    Returns the option value found for the given option name \a optionName, or
    an empty string if not found.

    The name provided can be any long or short name of any option that was
    added with \c addOption(). All the option names are treated as being
    equivalent. If the name is not recognized or that option was not present, an
    empty string is returned.

    For options found by the parser, an empty string is returned if the
    option does not take a value, otherwise the last value found for
    that option is returned.

    \sa values()
 */

QString QCommandLineParser::value(const QString &optionName) const
{
    d->checkParsed("value");
    const QStringList valueList = values(optionName);

    if (!valueList.isEmpty())
        return valueList.last();

    return QString();
}

/*!
    Returns a list of option values found for the given option name \a
    optionName, or an empty list if not found.

    The name provided can be any long or short name of any option that was
    added with \c addOption(). All the options names are treated as being
    equivalent. If the name is not recognized or that option was not present, an
    empty list is returned.

    For options found by the parser, the list will contain an entry for
    each time the option was encountered by the parser. These entries
    will always be an empty string for options that do not take a value.
    Options that do take a value will have the list populated with the
    values in the order they were found.

    \sa value()
 */

QStringList QCommandLineParser::values(const QString &optionName) const
{
    d->checkParsed("values");
    const NameHash_t::mapped_type optionOffset = d->nameHash.value(optionName, optionNotFound);
    if (optionOffset != optionNotFound) {
        QStringList values = d->optionValuesHash.value(optionOffset);
        if (values.isEmpty())
            values = d->commandLineOptionList.at(optionOffset).defaultValues();
        return values;
    }

    return QStringList();
}

/*!
    Returns a list of remaining arguments.

    These are all of the arguments that were not recognized as part of an
    option.
 */

QStringList QCommandLineParser::remainingArguments() const
{
    d->checkParsed("remainingArguments");
    return d->remainingArgumentList;
}

/*!
    Returns a list of option names that were found.

    This returns a list of all the recognized option names found by the
    parser, in the order in which they were found. For any long options
    that were in the form {--option=value}, the value part will have been
    dropped.

    The names in this list do not include the preceding dash characters.
    Names may appear more than once in this list if they were encountered
    more than once by the parser.

    Any entry in the list can be used with \c value() or with
    \c values() to get any relevant option values.
 */

QStringList QCommandLineParser::optionNames() const
{
    d->checkParsed("optionNames");
    return d->optionNames;
}

/*!
    Returns a list of unknown option names.

    This list will include both long an short name options that were not
    recognized. For any long options that were in the form {--option=value},
    the value part will have been dropped and only the long name is added.

    The names in this list do not include the preceding dash characters.
    Names may appear more than once in this list if they were encountered
    more than once by the parser.

    \sa optionNames()
 */

QStringList QCommandLineParser::unknownOptionNames() const
{
    d->checkParsed("unknownOptionNames");
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
    QString usage = QCommandLineParser::tr("Usage: %1").arg(exeName);
    if (!commandLineOptionList.isEmpty()) {
        usage += QLatin1Char(' ');
        usage += QCommandLineParser::tr("[options]");
    }
    if (!remainingArgsHelpText.isEmpty()) {
        usage += QLatin1Char(' ');
        usage += remainingArgsHelpText;
    }
    fprintf(stdout, "%s\n\n%s\n\n", qPrintable(usage), qPrintable(description));
    if (!commandLineOptionList.isEmpty()) {
        fprintf(stdout, "%s\n", qPrintable(QCommandLineParser::tr("Options:")));
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
        QString optionNamesString = optionNames.join(QStringLiteral(", "));
        if (!option.valueName().isEmpty())
            optionNamesString += QStringLiteral(" <") + option.valueName() + QLatin1Char('>');
        const QString optionString = optionFormatString.arg(optionNamesString, -25).arg(option.description());
        fprintf(stdout, "%s\n", qPrintable(optionString));
    }
    ::exit(0);
}

QT_END_NAMESPACE
