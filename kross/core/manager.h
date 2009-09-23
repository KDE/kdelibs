/***************************************************************************
 * manager.h
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

#ifndef KROSS_MANAGER_H
#define KROSS_MANAGER_H

#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QtScript/QScriptable>

#include "krossconfig.h"
#include "childreninterface.h"
#include "metatype.h"

namespace Kross {

    // Forward declarations.
    class Interpreter;
    class Action;
    class ActionCollection;
    class InterpreterInfo;

    /**
     * The Manager class is a singleton that provides the main entry
     * point to deal with the Kross Scripting Framework.
     *
     * Use \a Interpreter to just work with some implementated
     * interpreter like python or ruby. While \a Action implements
     * a flexible abstract container to deal with single script files.
     */
    class KROSSCORE_EXPORT Manager
        : public QObject
        , public QScriptable
        , public ChildrenInterface
    {
            Q_OBJECT

        public:

            /**
             * Return the Manager instance. Always use this
             * function to access the Manager singleton.
             */
            static Manager& self();

            /**
             * \return a map with \a InterpreterInfo* instances
             * used to describe interpreters.
             */
            QHash<QString, InterpreterInfo*> interpreterInfos() const;

            /**
             * \return true if there exists an interpreter with the
             * name \p interpretername else false.
             */
            bool hasInterpreterInfo(const QString& interpretername) const;

            /**
             * \return the \a InterpreterInfo* matching to the defined
             * \p interpretername or NULL if there does not exists such
             * a interpreter.
             */
            InterpreterInfo* interpreterInfo(const QString& interpretername) const;

            /**
             * Return the name of the \a Interpreter that feels responsible
             * for the defined \p file .
             *
             * \param file The filename we should try to determinate the
             * interpretername for.
             * \return The name of the \a Interpreter which will be used
             * to execute the file or QString() if we failed to determinate
             * a matching interpreter for the file.
             */
            const QString interpreternameForFile(const QString& file);

            /**
             * Return the \a Interpreter instance defined by
             * the interpretername.
             *
             * \param interpretername The name of the interpreter.
             * e.g. "python" or "kjs".
             * \return The Interpreter instance or NULL if there does not exists
             * an interpreter with such an interpretername.
             */
            Interpreter* interpreter(const QString& interpretername) const;

            /**
             * \return the root \a ActionCollection instance. Each collection
             * could have children of other collections and/or
             * \a Action instances.
             */
            ActionCollection* actionCollection() const;

            /**
             * \return the \a MetaTypeHandler instance for custom types
             * of type \p typeName .
             *
             * \since 4.2
             */
            MetaTypeHandler* metaTypeHandler(const QByteArray& typeName) const;

            /**
             * Register a handler for custom types.
             *
             * See also the \a WrapperInterface class.
             *
             * \param typeName The custom type the handler should handle.
             * \param handler Function that should be called to handle
             * a custom type.
             *
             * \since 4.2
             */
            void registerMetaTypeHandler(const QByteArray& typeName, MetaTypeHandler::FunctionPtr* handler);

            /**
             * Register a handler for custom types.
             *
             * See also the \a WrapperInterface class.
             *
             * \param typeName The custom type the handler should handle.
             * \param handler Function that should be called to handle
             * a custom type.
             *
             * \since 4.2
             */
            void registerMetaTypeHandler(const QByteArray& typeName, MetaTypeHandler::FunctionPtr2* handler);

            /**
             * Register a handler for custom types.
             *
             * See also the \a WrapperInterface class.
             *
             * \param typeName The custom type the handler should handle.
             * \param handler Function that should be called to handle
             * a custom type.
             *
             * \since 4.2
             */
            void registerMetaTypeHandler(const QByteArray& typeName, MetaTypeHandler* handler);

            /**
             * Returns true if strict type handling is enabled.
             *
             * \since 4.2
             */
            bool strictTypesEnabled() const;

            /**
             * Enable more strict type handling. If enabled then scripting-backends don't
             * handle unknown pointer-types where no MetaTypeHandler was registered for.
             * If disabled, such unknown types will be reinterpret_cast to QObject* what
             * allows to also handle unknown QObject's but will also result in a crash
             * if the unknown type isn't a QObject. Per default strict type handling is
             * enabled.
             *
             * \since 4.2
             */
            void setStrictTypesEnabled(bool enabled);

            /**
	     * \return whether \p typeName has a handler assigned or not.
	     */
            bool hasHandlerAssigned(const QByteArray& typeName) const;
        public Q_SLOTS:

            /**
             * \return a list of names of all supported scripting interpreters.
             * The list may contain for example "python", "ruby" and "kjs" depending
             * on what interpreter-plugins are installed.
             */
            QStringList interpreters() const;

            /**
            * \return true if there exists a \a Action QObject instance
            * which is child of this \a Manager instance and is defined as \p name
            * else false is returned.
            */
            bool hasAction(const QString& name);

            /**
            * \return the \a Action QObject instance defined with \p name which is
            * child of this \a Manager instance. If there exists no such \a Action
            * yet, create one.
            */
            QObject* action(const QString& name);

            /**
             * Load and return an external module. Modules are dynamic loadable
             * plugins which could be loaded on demand to provide additional
             * functionality.
             *
             * \param modulename The name of the module we should try to load.
             * \return The QObject instance that repesents the module or NULL
             * if loading failed.
             */
            QObject* module(const QString& modulename);

            /**
             * External modules are dynamically loadable and are normally deleted
             * when the kross library is unloaded.
             * Applications may choose to call deleteModules() instead to control
             * deletion of the modules at another time.
             */
            void deleteModules();

            /**
            * Execute a script file.
            * \param file The script file that should be executed.
            */
            bool executeScriptFile(const QUrl& file = QUrl());

            void addQObject(QObject* obj, const QString &name = QString());
            QObject* qobject(const QString &name) const;
            QStringList qobjectNames() const;

        Q_SIGNALS:

            /**
             * This signal is emitted when the execution of a script is started.
             */
            void started(Kross::Action*);

            /**
             * This signal is emitted when the execution of a script is finished.
             */
            void finished(Kross::Action*);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;

        public:

            /**
             * The constructor. Use \a self() to access the Manager
             * singleton instance and don't call this direct.
             */
            explicit Manager();

            /**
             * Destructor.
             */
            virtual ~Manager();
    };

}

Q_DECLARE_METATYPE(Kross::Manager*)

#endif

