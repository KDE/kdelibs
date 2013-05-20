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

#include "qcommandlineoption.h"

#include "qset.h"

QT_BEGIN_NAMESPACE

class QCommandLineOptionPrivate : public QSharedData
{
public:
    inline QCommandLineOptionPrivate()
        : isRequired(false)
    { }

    //! The list of names used for this option.
    QStringList nameSet;

    //! The documentation name for the value, if one is expected
    //! Example: "-o <file>" means valueName == "file"
    QString valueName;

    //! Whether the option is required or optional.
    bool isRequired;

    //! The description used for this option.
    QString description;

    //! The list of default values used for this option.
    QStringList defaultValues;
};

/*!
    \since 5.2
    \class QCommandLineOption
    \brief The QCommandLineOption class provides a means for command line option

    This class is used to describe an option on the command line. It allows
    different ways of defining the same option with multiple aliases possible.
    It is also used to describe how the option is used - it may be a flag used
    once or more, or take an argument or multiple.

    \sa QCommandLineParser
*/

/*!
    Constructs a command line option object
*/
QCommandLineOption::QCommandLineOption()
    : d(new QCommandLineOptionPrivate)
{
}

/*!
    Constructs a command line option object with the given arguments.
*/
QCommandLineOption::QCommandLineOption(const QStringList &names, const QString &description,
                                       const QString &valueName, bool required,
                                       const QStringList &defaultValues)
    : d(new QCommandLineOptionPrivate)
{
    setRequired(required);
    setNames(names);
    setValueName(valueName);
    setDescription(description);
    setDefaultValues(defaultValues);
}

QCommandLineOption::QCommandLineOption(const QCommandLineOption &other):
    d(other.d)
{
}

/*!
    Destroys the command line option object.
*/
QCommandLineOption::~QCommandLineOption()
{
}

QCommandLineOption&
QCommandLineOption::operator=(const QCommandLineOption &other)
{
    d = other.d;
    return *this;
}

bool QCommandLineOption::operator==(const QCommandLineOption &other) const
{
    return (d->nameSet == other.names() && d->valueName == other.valueName()
            && d->isRequired == other.required()
            && d->description == other.description() && d->defaultValues == other.defaultValues());
}

/*!
    Return the names set for this option.

    \sa setNames()
 */
QStringList QCommandLineOption::names() const
{
    return d->nameSet;
}

/*!
    Set the list of names used for this option to \a names

    The name can be either short or long. Any name in the list that is one
    character in length is a short name. Option names must not be empty,
    must not start with a dash character, and cannot be repeated.

    \sa names()
 */
void QCommandLineOption::setNames(const QStringList &names)
{
    foreach (const QString &name, names) {
        if (name.isEmpty())
            qWarning("Option names cannot be empty");
        else if (name.startsWith(QLatin1Char('-')))
            qWarning("Option names cannot start with a '-'");
        else if (name.startsWith(QLatin1Char('?')))
            qWarning("Option names cannot start with a '?'");
        else
            d->nameSet.append(name);
    }
}

/*!
    Set the name of the expected value, for the documentation.

    Options without a value assigned have a boolean-like behavior:
    either the user specifies --foobar or they don't.

    Options with a value assigned, need to set a name for the expected value,
    for the documentation of the option in the help output. An option with names "o" and "output",
    and a value name of "file" will appear as  "-o, --output <file>".

    The application should call QCommandLineParser::argument() if it expects the
    option to be present only once, and QCommandLineParser::arguments() if it expects
    that option to be present multiple times.

    \sa valueName()
 */
void QCommandLineOption::setValueName(const QString &valueName)
{
    d->valueName = valueName;
}

/*!
    Return the name of the expected value.

    If empty, the option doesn't take a value.

    \sa setValueName()
 */
QString QCommandLineOption::valueName() const
{
    return d->valueName;
}

/*!
    Set whether or not this option is required to \a required

    If this is set to true as required, the option needs to be set explicitely
    and does not have a default value. Otherwise if it is set to false, this option
    becomes optional and the user is not obligated to define it when running the
    application.

    \sa required()
 */
void QCommandLineOption::setRequired(bool required)
{
    d->isRequired = required;
}

/*!
    Return if the option is required. The default value is false.

    \sa setRequired()
 */
bool QCommandLineOption::required() const
{
    return d->isRequired;
}

/*!
    Set the description used for this option to \a description

    The description is used for instance while prompting some help output to the
    user of the application.

    \sa description()
 */
void QCommandLineOption::setDescription(const QString &description)
{
    d->description = description;
}

/*!
    Return the description set for this option.

    \sa setDescription()
 */
QString QCommandLineOption::description() const
{
    return d->description;
}

/*!
   Set the list of default values used for this option to \a defaultValues

   The default values are used, if the user of the application does not specify
   them explicitely via the command line or other user interface.

   \sa defaultValues()
 */
void QCommandLineOption::setDefaultValues(const QStringList &defaultValues)
{
    d->defaultValues = defaultValues;
}

/*!
    Return the default values set for this option.

    \sa setDefaultValues()
 */
QStringList QCommandLineOption::defaultValues() const
{
    return d->defaultValues;
}

QT_END_NAMESPACE
