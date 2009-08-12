/***************************************************************************
 * action.h
 * This file is part of the KDE project
 * copyright (C)2004-2007 by Sebastian Sauer (mail@dipe.org)
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

#ifndef KROSS_ACTION_H
#define KROSS_ACTION_H

#include <QtCore/QVariant>
#include <QtCore/QObject>
#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtGui/QAction>
#include <QtXml/QDomAttr>
#include <QtScript/QScriptable>

#include "errorinterface.h"
#include "childreninterface.h"

namespace Kross {

    class Script;

    /**
     * The Action class is an abstract container to deal with scripts
     * like a single standalone script file. Each action holds a reference
     * to the matching @a Kross::Interpreter created @a Kross::Script
     * instance.
     *
     * The \a Manager takes care of handling the \a Action instances
     * application by providing access to \a ActionCollection containers
     * for those \a Action instances.
     *
     * Once you've such a Action instance you're able to perform actions
     * with it like executing scripting code.
     *
     * Following sample shows "Hello World." executed with the python
     * interpreter:
     * \code
     * # Create a new Kross::Action instance.
     * Kross::Action* action = new Kross::Action(0,"MyFirstScript");
     * # Set the interpreter we like to use. This could be e.g. "python", "ruby" or "kjs".
     * action->setInterpreter("python");
     * # Set the scripting code.
     * action->setCode("print \"Hello World.\"");
     * # Execute the scripting code.
     * action->trigger();
     * \endcode
     *
     * Following sample demonstrates how to execute an external python script
     * file. The script file itself is named "mytest.py" and contains:
     * \code
     * # this function got called from within C++
     * def myfunction(args):
     *     print "Arguments are: %s" % args
     * # Import the published QObject's
     * import MyFirstQObject, MySecondQObject
     * # Call a slot MyFirstQObject provides.
     * MyFirstQObject.someSlot("Some string")
     * # Set a property MySecondQObject provides.
     * MySecondQObject.someProperty = "Other string"
     * \endcode
     * Then you are able to load the script file, publish QObject instances
     * and let the script do whatever it likes to do:
     * \code
     * # Publish a QObject instance for all Kross::Action instances.
     * Kross::Manager::self().addObject(myqobject1, "MyFirstQObject")
     * # Create a new Kross::Action instance.
     * Kross::Action* action = new Kross::Action(0,"MySecondScript");
     * # Publish a QObject instance only for the Kross::Action instance.
     * action->addObject(myqobject2, "MySecondQObject");
     * # Set the script file we like to execute.
     * action->setFile("/home/myuser/mytest.py");
     * # Execute the script file.
     * action->trigger();
     * # Call the "myfunction" defined in the "mytest.py" python script.
     * QVariant result = action->callFunction("myfunction", QVariantList()<<"Arg");
     * \endcode
     */
    class KROSSCORE_EXPORT Action
        : public QAction
        , public QScriptable
        , public ChildrenInterface
        , public ErrorInterface
    {
            Q_OBJECT

        public:

           /**
             * Constructor.
             *
             * \param parent The parent QObject this \a Action is a child of.
             * \param name The unique name this Action has. It's used
             * e.g. at the \a Manager to identify the Action. The
             * name is accessible via \a QObject::objectName .
             * \deprecated since 4.3: pass search path to fromDomElement() and toDomElement()
             */
            Action(QObject* parent, const QString& name, const QDir& packagepath = QDir()); //BIC may be removed in favour of the next c'tor

            /**
             * Constructor.
             *
             * \param parent The parent QObject this \a Action is a child of.
             * \param url The URL should point to a valid scripting file.
             * This \a Action will be filled with the content of the
             * file (e.g. the file is read and \a code should return
             * its content and it's also tried to determine the
             * \a interpreter ). Remember to use QUrl c'tor explicitly.
             * The name will be set to url.path()
             */
            Action(QObject* parent, const QUrl& url);

            /**
             * Destructor.
             */
            virtual ~Action();

            /**
             * Method to read settings from the QDomElement \p element that
             * contains details about e.g. the displayed text, the file to
             * execute or the used interpreter.
             * \todo BIC merge
             */
            void fromDomElement(const QDomElement& element);

            /**
             * Method to read settings from the QDomElement \p element that
             * contains details about e.g. the displayed text, the file to
             * execute or the used interpreter.
             * 
             * \param searchPath List of directories where to search the script if its path is relative
             * First item is given the highest priority.
             */
            void fromDomElement(const QDomElement& element, const QStringList& searchPath/* = QStringList()*/);

            /**
             * \return a QDomElement that contains the settings like e.g. the
             * displayed text, the file to execute or the used interpreter
             * of this \a Action instance.
             */
            QDomElement toDomElement() const;

            /**
             * \return a QDomElement that contains the settings like e.g. the
             * displayed text, the file to execute or the used interpreter
             * of this \a Action instance.
             * \param searchPath if given, find the closest directory containing the scriptfile
             *  and write relative filepath
             */
            QDomElement toDomElement(const QStringList& searchPath/* = QStringList()*/) const;

            /**
             * Initialize the \a Script instance.
             *
             * Normally there is no need to call this function directly because
             * if will be called internally if needed (e.g. on \a execute ).
             *
             * \return true if the initialization was successful else
             * false is returned.
             */
            bool initialize();

            /**
             * Finalize the \a Script instance and frees any cached or still
             * running executions. Normally there is no need to call this
             * function directly because the \a Action will take care
             * of calling it if needed.
             */
            void finalize();

            /**
             * \return true if the action is finalized, which means the
             * action is currently not running.
             */
            bool isFinalized() const;

            /**
             * \return the \a Kross::Script implementation used by the scripting
             * backend. This returns NULL until the action got triggered or if
             * there was an error before that.
             *
             * Normaly it shouldn't be necessary to deal with the scripting backend
             * depending instance of a \a Kross::Script implementation since this
             * \a Action class already decorates all the things needed. It
             * may however be useful to provide additional interpreter dependent
             * functionality.
             */
            Script* script() const;

        public Q_SLOTS:

            /**
             * \return the objectName for this Action.
             */
            QString name() const;

            /**
             * \return the version number this Action has.
             * Per default 0 is returned.
             */
            int version() const;

            /**
             * \return the optional description for this Action.
             */
            QString description() const;

            /**
             * Set the optional description for this Action.
             */
            void setDescription(const QString& description);

            /**
             * Return the name of the icon.
             */
            QString iconName() const;

            /**
             * Set the name of the icon to \p iconname .
             */
            void setIconName(const QString& iconname);

            /**
             * Return true if this Action is enabled else false is returned.
             */
            bool isEnabled() const;

            /**
             * Set the enable state of this Action to \p enabled .
             */
            void setEnabled(bool enabled);

            /**
             * \return the script file that should be executed.
             */
            QString file() const;

            /**
             * Set the script file that should be executed.
             */
            bool setFile(const QString& scriptfile);

            /**
             * \return the scriptcode this Action holds.
             */
            QByteArray code() const;

            /**
             * Set the scriptcode \p code this Action should execute.
             */
            void setCode(const QByteArray& code);

            /**
             * \return the name of the interpreter. Could be for
             * example "python" or "ruby".
             */
            QString interpreter() const;

            /**
             * Set the name of the interpreter (javascript, python or ruby).
             */
            void setInterpreter(const QString& interpretername);

            /**
             * \return the current path the script is running in or
             * an empty string if there is no current path defined.
             */
            QString currentPath() const;

            /**
             * Add a QObject instance to the action. This instance will
             * be published to scripts.
             */
            void addQObject(QObject* obj, const QString &name = QString());

            /**
             * \return the QObject with the object name \p name .
             */
            QObject* qobject(const QString &name) const;

            /**
             * \return a list of QObject object names.
             */
            QStringList qobjectNames() const;

            /**
             * \return a map of options this \a Action defines.
             * The options are returned call-by-ref, so you are able to
             * manipulate them.
             */
            QVariantMap options() const;

            /**
             * \return the value of the option defined with \p name .
             * If there doesn't exist an option with such a name,
             * the \p defaultvalue is returned.
             */
            QVariant option(const QString& name, const QVariant& defaultvalue = QVariant());

            /**
             * Set the \a Interpreter::Option value.
             */
            bool setOption(const QString& name, const QVariant& value);

            /**
             * \return the list of functionnames.
             */
            QStringList functionNames();

            /**
             * Call a function in the script.
             *
             * \param name The name of the function which should be called.
             * \param args The optional list of arguments.
             */
            QVariant callFunction(const QString& name, const QVariantList& args = QVariantList());

            /**
             * Evaluate some scripting code.
             *
             * Example how this can be used:
             * \code
             * Kross::Action* a = new Kross::Action(0, "MyScript");
             * a->setInterpreter("python");
             * a->setCode("def myFunc(x): return x");
             * a->execute();
             * int three = a->evaluate("1+2").toInt(); // returns 3
             * int nine = a->evaluate("myFunc(9)").toInt(); // returns 9
             * \endcode
             *
             * \param code The scripting code to evaluate.
             * \return The return value of the evaluation.
             */
            QVariant evaluate(const QByteArray& code);

        Q_SIGNALS:

            /**
             * This signal is emitted if the content of the Action
             * was changed. The \a ActionCollection instances this Action
             * is a child of are connected with this signal to fire up
             * their own updated signal if an Action of them was updated.
             */
            void updated();

            /// This signal is emitted when the data of the Action is changed
            void dataChanged(Action*);

            /**
             * This signal is emitted before the script got executed.
             */
            void started(Kross::Action*);

            /**
             * This signal is emitted after the script got executed.
             */
            void finished(Kross::Action*);

            /**
             * This signal is emitted once a script finalized.
             */
            void finalized(Kross::Action*);

        private Q_SLOTS:

            /**
             * This private slot is connected with the \a QAction::triggered
             * signal. To execute the script just emit that signal and this
             * slot tries to execute the script.
             */
            void slotTriggered();

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

}

Q_DECLARE_METATYPE(Kross::Action*)

#endif

