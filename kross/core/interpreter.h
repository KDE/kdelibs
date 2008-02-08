/***************************************************************************
 * interpreter.h
 * This file is part of the KDE project
 * copyright (C)2004-2006 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KROSS_INTERPRETER_H
#define KROSS_INTERPRETER_H

#include "errorinterface.h"

#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtCore/QMap>
#include <QtCore/QObject>

namespace Kross {

    // Forward declaration.
    class Manager;
    class Action;
    class Script;
    class Interpreter;

    /**
     * The InterpreterInfo class provides abstract information about
     * a \a Interpreter before the interpreter-backend itself is
     * loaded.
     */
    class KROSSCORE_EXPORT InterpreterInfo
    {
        public:

            /**
             * Each interpreter is able to define options we could
             * use to manipulate the interpreter behaviour.
             */
            class Option
            {
                public:

                    /**
                    * Map of options.
                    */
                    typedef QMap< QString, Option* > Map;

                    /**
                     * Constructor.
                     *
                     * \param comment A localized comment that describes
                     * the option.
                     * \param value The QVariant value this option has.
                     */
                    Option(const QString& comment, const QVariant& value)
                        : comment(comment), value(value) {}

                    /// A description of the option.
                    QString comment;

                    /// The value the option has.
                    QVariant value;
            };

            /**
             * Constructor.
             *
             * \param interpretername The name of the interpreter. The name is
             * used internaly as unique identifier for the interpreter and
             * could be for example "python", "ruby" or "javascript".
             * \param funcPtr A pointer to the entry function within the
             * library. The entry function each interpreter-backend does
             * provide looks like this;
             * \code
             * typedef void* (*def_interpreter_func)(int version, Kross::InterpreterInfo*);
             * \endcode
             * The def_interpreter_func function will be used by Kross to load
             * the interpreter's library. The passed version is used to be able
             * to versioning details and we use the KROSS_VERSION defined within
             * the krossconfig.h file here.
             * \param wildcard File wildcard that identifies a by the interpreter
             * supported scripting files. As example Python does define here
             * "*.py" while Java does define "*.java *.class".
             * \param mimetypes The file mimetype that identifies a by the interpreter
             * supported scripting files. As example Python does define "text/x-python"
             * here while Ruby defines "application/x-ruby" and Java "application/java".
             * \param options The optional list of options supported by the interpreter
             * to configure the backend.
             */
            InterpreterInfo(const QString& interpretername, void* funcPtr, const QString& wildcard, const QStringList& mimetypes, const Option::Map& options = Option::Map());

            /**
             * Destructor.
             */
            ~InterpreterInfo();

            /**
             * \return the name of the interpreter. For example "python" or "kjs".
             */
            const QString interpreterName() const;

            /**
             * \return the file-wildcard used to determinate by this interpreter
             * used scriptingfiles. Those filter will be used e.g. with
             * KGlobal::dirs()->findAllResources() as filtermask. For example
             * python just defines it as "*py".
             */
            const QString wildcard() const;

            /**
             * List of mimetypes this interpreter supports.
             * \return QStringList with mimetypes like "application/javascript".
             */
            const QStringList mimeTypes() const;

            /**
             * \return true if an \a Option with that \p key exists else false.
             */
            bool hasOption(const QString& name) const;

            /**
             * \return the option defined with \p name .
             */
            Option* option(const QString& name) const;

            /**
             * \return the reference to the intenal used map with all options.
             */
            Option::Map& options();

            /**
             * \return the value of the option defined with \p name . If there
             * doesn't exists an option with such a name, the \p defaultvalue
             * is returned.
             */
            const QVariant optionValue(const QString& name, const QVariant& defaultvalue = QVariant()) const;

            /**
             * \return the \a Interpreter instance this \a InterpreterInfo
             * is the describer for. If the interpreter that implements the
             * scripting backend isn't loaded yet, this method will trigger
             * the loading of the interpreter's library. Note that this
             * method may return NULL if there is no library for that
             * interpreter installed or if the library is incompatible.
             */
            Interpreter* interpreter();

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

    /**
     * Base class for interpreter implementations.
     *
     * Each scripting backend needs to inherit its own
     * interpreter and implement it.
     *
     * The Interpreter will be managed by the \a Manager
     * class and does provide a factory method to create
     * \a Script implementations.
     */
    class KROSSCORE_EXPORT Interpreter : public QObject, public ErrorInterface
    {
            Q_OBJECT
        public:

            /**
             * Constructor.
             *
             * \param info is the \a InterpreterInfo instance
             *        that describes this interpreter.
             */
            explicit Interpreter(InterpreterInfo* info);

            /**
             * Destructor.
             */
            virtual ~Interpreter();

            /**
             * \return the \a InterpreterInfo that represents
             * this \a Interpreter .
             */
            InterpreterInfo* interpreterInfo() const;

            /**
             * Create and return a new interpreter dependent
             * \a Script instance.
             *
             * \param Action The \a Action
             *        to use for the \a Script instance.
             * \return The from \a Script inherited instance.
             */
            virtual Script* createScript(Action* Action) = 0;

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

}

#endif

