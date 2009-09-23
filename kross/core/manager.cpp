/***************************************************************************
 * manager.cpp
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

#include "manager.h"
#include "interpreter.h"
#include "action.h"
#include "actioncollection.h"

#include <QtCore/QObject>
#include <QtCore/QArgument>
#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/QFileInfo>
#include <QtCore/QPointer>
#include <QtCore/QLibrary>
#include <QtCore/QCoreApplication>

#include <kstandarddirs.h>
#include <klocale.h>

extern "C"
{
    typedef QObject* (*def_module_func)();
}

using namespace Kross;

namespace Kross {

    /// @internal
    class Manager::Private
    {
        public:
            /// List of \a InterpreterInfo instances.
            QHash< QString, InterpreterInfo* > interpreterinfos;

            /// List of the interpreter names.
            QStringList interpreters;

            /// Loaded modules.
            QHash< QString, QPointer<QObject> > modules;

            /// The collection of \a Action instances.
            ActionCollection* collection;

            /// List with custom handlers for metatypes.
            QHash<QByteArray, MetaTypeHandler*> wrappers;

            /// Strict type handling enabled or disabled.
            bool strictTypesEnabled;
    };

}

Q_GLOBAL_STATIC(Manager, _self)

Manager& Manager::self()
{
    return *_self();
}

void* loadLibrary(const char* libname, const char* functionname)
{
    QLibrary lib(libname);
    lib.setLoadHints( QLibrary::ExportExternalSymbolsHint );
    if( ! lib.load() ) {
        const QString err = QString("Error: %1").arg(lib.errorString());

        //TODO move that functionality out of Kross since we like to be Qt-only
        foreach(const QString &dir, KStandardDirs().resourceDirs("module")) {
            lib.setFileName( QFileInfo(dir, libname).filePath() );
            lib.setLoadHints( QLibrary::ExportExternalSymbolsHint );
            if( lib.load() )
                break;
        }

        /*
        if( ! lib.isLoaded() ) {
            foreach(const QString& path, QCoreApplication::instance()->libraryPaths()) {
                lib.setFileName( QFileInfo(path, libname).filePath() );
                lib.setLoadHints( QLibrary::ExportExternalSymbolsHint );
                if( lib.load() )
                    break;
            }
        }
        */

        if( ! lib.isLoaded() ) {
            #ifdef KROSS_INTERPRETER_DEBUG
                if( strcmp(functionname, "krossinterpreter") == 0 )
                    krossdebug( QString("Kross Interpreter '%1' not available: %2").arg(libname).arg(err) );
                else if( strcmp(functionname, "krossmodule") == 0 )
                    krossdebug( QString("Kross Module '%1' not available: %2").arg(libname).arg(err) );
                else
                    krosswarning( QString("Failed to load unknown type of '%1' library: %2").arg(libname).arg(err) );
            #endif
            return 0;
        }
    }
    void* funcPtr = lib.resolve(functionname);
    Q_ASSERT(funcPtr);
    return funcPtr;
}

Manager::Manager()
    : QObject()
    , QScriptable()
    , ChildrenInterface()
    , d( new Private() )
{
    d->strictTypesEnabled = true;
    setObjectName("Kross");
    d->collection = new ActionCollection("main");

#ifdef KROSS_PYTHON_LIBRARY
    if( void* funcPtr = loadLibrary(KROSS_PYTHON_LIBRARY, "krossinterpreter") ) {
        d->interpreterinfos.insert("python",
            new InterpreterInfo("python",
                funcPtr, // library
                "*.py", // file filter-wildcard
                QStringList() << "text/x-python" // mimetypes
            )
        );
    }
#endif

#ifdef KROSS_RUBY_LIBRARY
    if( void* funcPtr = loadLibrary(KROSS_RUBY_LIBRARY, "krossinterpreter") ) {
        InterpreterInfo::Option::Map options;
        options.insert("safelevel", new InterpreterInfo::Option(
            i18n("Level of safety of the Ruby interpreter"),
            QVariant(0) )); // 0 -> unsafe, 4 -> very safe
        d->interpreterinfos.insert("ruby",
            new InterpreterInfo("ruby",
                funcPtr, // library
                "*.rb", // file filter-wildcard
                QStringList() << /* "text/x-ruby" << */ "application/x-ruby", // mimetypes
                options // options
            )
        );
    }
#endif

#ifdef KROSS_JAVA_LIBRARY
    if( void* funcPtr = loadLibrary(KROSS_JAVA_LIBRARY, "krossinterpreter") ) {
        d->interpreterinfos.insert("java",
            new InterpreterInfo("java",
                funcPtr, // library
                "*.java *.class *.jar", // file filter-wildcard
                QStringList() << "application/java" // mimetypes
            )
        );
    }
#endif

#ifdef KROSS_KJS_LIBRARY
    if( void* funcPtr = loadLibrary(KROSS_KJS_LIBRARY, "krossinterpreter") ) {
        d->interpreterinfos.insert("javascript",
            new InterpreterInfo("javascript",
                funcPtr, // library
                "*.js", // file filter-wildcard
                QStringList() << "application/javascript" // mimetypes
            )
        );
    }
#endif

#ifdef KROSS_FALCON_LIBRARY
    if( void* funcPtr = loadLibrary(KROSS_FALCON_LIBRARY, "krossinterpreter") ) {
        d->interpreterinfos.insert("falcon",
            new InterpreterInfo("falcon",
                funcPtr, // library
                "*.fal", // file filter-wildcard
                QStringList() << "application/x-falcon" // mimetypes
            )
        );
    }
#endif

#ifdef KROSS_QTSCRIPT_LIBRARY
    if( void* funcPtr = loadLibrary(KROSS_QTSCRIPT_LIBRARY, "krossinterpreter") ) {
        d->interpreterinfos.insert("qtscript",
            new InterpreterInfo("qtscript",
                funcPtr, // library
                "*.es", // file filter-wildcard
                QStringList() << "application/ecmascript" // mimetypes
            )
        );
    }
#endif

#ifdef KROSS_LUA_LIBRARY
    if( void* funcPtr = loadLibrary(KROSS_LUA_LIBRARY, "krossinterpreter") ) {
        d->interpreterinfos.insert("lua",
            new InterpreterInfo("lua",
                funcPtr, // library
                "*.lua *.luac", // file filter-wildcard
                QStringList() << "application/x-lua" // mimetypes
            )
        );
    }
#endif

    // fill the list of supported interpreternames.
    QHash<QString, InterpreterInfo*>::Iterator it( d->interpreterinfos.begin() );
    for(; it != d->interpreterinfos.end(); ++it)
        if( it.value() )
            d->interpreters << it.key();
    d->interpreters.sort();

    // publish ourself.
    ChildrenInterface::addObject(this, "Kross");
}

Manager::~Manager()
{
    qDeleteAll(d->wrappers);
    qDeleteAll(d->interpreterinfos);
    qDeleteAll(d->modules);
    delete d->collection;
    delete d;
}

QHash< QString, InterpreterInfo* > Manager::interpreterInfos() const
{
    return d->interpreterinfos;
}

bool Manager::hasInterpreterInfo(const QString& interpretername) const
{
    return d->interpreterinfos.contains(interpretername) && d->interpreterinfos[interpretername];
}

InterpreterInfo* Manager::interpreterInfo(const QString& interpretername) const
{
    return hasInterpreterInfo(interpretername) ? d->interpreterinfos[interpretername] : 0;
}

const QString Manager::interpreternameForFile(const QString& file)
{
    QRegExp rx;
    rx.setPatternSyntax(QRegExp::Wildcard);
    for(QHash<QString, InterpreterInfo*>::Iterator it = d->interpreterinfos.begin(); it != d->interpreterinfos.end(); ++it) {
        if( ! it.value() )
            continue;
        foreach(const QString &wildcard, it.value()->wildcard().split(' ', QString::SkipEmptyParts)) {
            rx.setPattern( wildcard );
            if( rx.exactMatch(file) )
                return it.value()->interpreterName();
        }
    }
    return QString();
}

Interpreter* Manager::interpreter(const QString& interpretername) const
{
    if( ! hasInterpreterInfo(interpretername) ) {
        krosswarning( QString("No such interpreter '%1'").arg(interpretername) );
        return 0;
    }
    return d->interpreterinfos[interpretername]->interpreter();
}

QStringList Manager::interpreters() const
{
    return d->interpreters;
}

ActionCollection* Manager::actionCollection() const
{
    return d->collection;
}

bool Manager::hasAction(const QString& name)
{
    return findChild< Action* >(name) != 0L;
}

QObject* Manager::action(const QString& name)
{
    Action* action = findChild< Action* >(name);
    if(! action) {
        action = new Action(this, name);
#if 0
        d->actioncollection->insert(action); //FIXME should we really remember the action?
#endif
    }
    return action;
}

QObject* Manager::module(const QString& modulename)
{
    if( d->modules.contains(modulename) ) {
        QObject* obj = d->modules[modulename];
        if( obj )
            return obj;
    }

    if( modulename.isEmpty() || modulename.contains( QRegExp("[^a-zA-Z0-9]") ) ) {
        krosswarning( QString("Invalid module name '%1'").arg(modulename) );
        return 0;
    }

    QByteArray libraryname = QString("krossmodule%1").arg(modulename).toLower().toLatin1();

#if 0
    KLibLoader* loader = KLibLoader::self();
    KLibrary* lib = loader->library( libraryname, QLibrary::ExportExternalSymbolsHint );
    if( ! lib ) { //FIXME this fallback-code should be in KLibLoader imho.
        lib = loader->library( QString("lib%1").arg(libraryname), QLibrary::ExportExternalSymbolsHint );
        if( ! lib ) {
            krosswarning( QString("Failed to load module '%1': %2").arg(modulename).arg(loader->lastErrorMessage()) );
            return 0;
        }
    }

    def_module_func func;
    func = (def_module_func) lib->resolveFunction("krossmodule");
    if( ! func ) {
        krosswarning( QString("Failed to determinate init function in module '%1'").arg(modulename) );
        return 0;
    }

    QObject* module = (QObject*) (func)(); // call the function
    lib->unload(); // unload the library

    if( ! module ) {
        krosswarning( QString("Failed to load module object '%1'").arg(modulename) );
        return 0;
    }
#else
    if( void* funcPtr = loadLibrary(libraryname, "krossmodule") ) {
        def_module_func func = (def_module_func) funcPtr;
        Q_ASSERT( func );
        QObject* module = (QObject*) (func)(); // call the function
        Q_ASSERT( module );
        //krossdebug( QString("Manager::module Module successfully loaded: modulename=%1 module.objectName=%2 module.className=%3").arg(modulename).arg(module->objectName()).arg(module->metaObject()->className()) );
        d->modules.insert(modulename, module);
        return module;
    }
    else {
        krosswarning( QString("Failed to load module '%1'").arg(modulename) );
    }
#endif
    return 0;
}

void Manager::deleteModules()
{
    qDeleteAll(d->modules);
    d->modules.clear();
}

bool Manager::executeScriptFile(const QUrl& file)
{
    krossdebug( QString("Manager::executeScriptFile() file='%1'").arg(file.toString()) );
    Action* action = new Action(0 /*no parent*/, file);
    action->trigger();
    bool ok = ! action->hadError();
    delete action; //action->delayedDestruct();
    return ok;
}

void Manager::addQObject(QObject* obj, const QString &name)
{
    this->addObject(obj, name);
}

QObject* Manager::qobject(const QString &name) const
{
    return this->object(name);
}

QStringList Manager::qobjectNames() const
{
    return this->objects().keys();
}

MetaTypeHandler* Manager::metaTypeHandler(const QByteArray& typeName) const
{
    return d->wrappers.contains(typeName) ? d->wrappers[typeName] : 0;
}

void Manager::registerMetaTypeHandler(const QByteArray& typeName, MetaTypeHandler::FunctionPtr* handler)
{
    d->wrappers.insert(typeName, new MetaTypeHandler(handler));
}

void Manager::registerMetaTypeHandler(const QByteArray& typeName, MetaTypeHandler::FunctionPtr2* handler)
{
    d->wrappers.insert(typeName, new MetaTypeHandler(handler));
}

void Manager::registerMetaTypeHandler(const QByteArray& typeName, MetaTypeHandler* handler)
{
    d->wrappers.insert(typeName, handler);
}

bool Manager::strictTypesEnabled() const
{
    return d->strictTypesEnabled;
}

void Manager::setStrictTypesEnabled(bool enabled)
{
    d->strictTypesEnabled = enabled;
}

bool Manager::hasHandlerAssigned(const QByteArray& typeName) const
{
    return d->wrappers.contains(typeName);
}

#include "manager.moc"
