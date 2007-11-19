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
//#include "variant.h"

#include <QtCore/QObject>
#include <QtCore/QArgument>
#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/QFileInfo>

#include <klibloader.h>

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
    };

}


Manager& Manager::self()
{
    K_GLOBAL_STATIC(Manager, _self)
    return *_self;
}

Manager::Manager()
    : QObject()
    , ChildrenInterface()
    , d( new Private() )
{
    setObjectName("Kross");
    d->collection = new ActionCollection("main");

#ifdef KROSS_PYTHON_LIBRARY
    QString pythonlib = QFile::encodeName( KLibLoader::self()->findLibrary(QLatin1String(KROSS_PYTHON_LIBRARY)) );
    if(! pythonlib.isEmpty()) { // If the Kross Python plugin exists we offer is as supported scripting language.
        InterpreterInfo::Option::Map pythonoptions;
        d->interpreterinfos.insert("python",
            new InterpreterInfo("python",
                pythonlib, // library
                "*.py", // file filter-wildcard
                QStringList() << "text/x-python", // mimetypes
                pythonoptions // options
            )
        );
    } else {
        #ifdef KROSS_INTERPRETER_DEBUG
            krossdebug("Python interpreter for kross is unavailable");
        #endif
    }
#endif

#ifdef KROSS_RUBY_LIBRARY
    QString rubylib = QFile::encodeName( KLibLoader::self()->findLibrary(QLatin1String(KROSS_RUBY_LIBRARY)) );
    if(! rubylib.isEmpty()) { // If the Kross Ruby plugin exists we offer is as supported scripting language.
        InterpreterInfo::Option::Map rubyoptions;
        rubyoptions.insert("safelevel",
            new InterpreterInfo::Option(
                i18n("Level of safety of the Ruby interpreter"),
                QVariant(0) // 0 -> unsafe, 4 -> very safe
            )
        );
        d->interpreterinfos.insert("ruby",
            new InterpreterInfo("ruby",
                rubylib, // library
                "*.rb", // file filter-wildcard
                QStringList() << /* "text/x-ruby" << */ "application/x-ruby", // mimetypes
                rubyoptions // options
            )
        );
    } else {
        #ifdef KROSS_INTERPRETER_DEBUG
            krossdebug("Ruby interpreter for kross is unavailable");
        #endif
    }
#endif

#ifdef KROSS_JAVA_LIBRARY
    QString javalib = QFile::encodeName( KLibLoader::self()->findLibrary(QLatin1String(KROSS_JAVA_LIBRARY)) );
    if(! javalib.isEmpty()) {
        InterpreterInfo::Option::Map javaoptions;
        d->interpreterinfos.insert("java",
            new InterpreterInfo("java",
                javalib, // library
                "*.java *.class *.jar", // file filter-wildcard
                QStringList() << "application/java", // mimetypes
                javaoptions // options
            )
        );
    } else {
        #ifdef KROSS_INTERPRETER_DEBUG
            krossdebug("KDE Java interpreter for kross is unavailable");
        #endif
    }
#endif

#ifdef KROSS_KJS_LIBRARY
    QString kjslib = QFile::encodeName( KLibLoader::self()->findLibrary(QLatin1String(KROSS_KJS_LIBRARY)) );
    if(! kjslib.isEmpty()) { // If the Kjs plugin exists we offer is as supported scripting language.
        InterpreterInfo::Option::Map kjsoptions;
        kjsoptions.insert("restricted",
            new InterpreterInfo::Option(
                i18n("Restricted mode for untrusted scripts"),
                QVariant(true) // per default enabled
            )
        );
        d->interpreterinfos.insert("javascript",
            new InterpreterInfo("javascript",
                kjslib, // library
                "*.js", // file filter-wildcard
                QStringList() << "application/javascript", // mimetypes
                kjsoptions // options
            )
        );
    } else {
        #ifdef KROSS_INTERPRETER_DEBUG
            krossdebug("KDE JavaScript interpreter for kross is unavailable");
        #endif
    }
#endif

#ifdef KROSS_FALCON_LIBRARY
    QString falconlib = QFile::encodeName( KLibLoader::self()->findLibrary(QLatin1String(KROSS_FALCON_LIBRARY)) );
    if(! falconlib.isEmpty()) {
        InterpreterInfo::Option::Map falconptions;
        d->interpreterinfos.insert("falcon",
            new InterpreterInfo("falcon",
                falconlib, // library
                "*.fal", // file filter-wildcard
                QStringList() << "application/x-falcon", // mimetypes
                falconptions // options
                )
            );
    } else {
        #ifdef KROSS_INTERPRETER_DEBUG
            krossdebug("Falcon interpreter for kross is unavailable");
        #endif
    }
#endif

    // fill the list of supported interpreternames.
    QHash<QString, InterpreterInfo*>::Iterator it( d->interpreterinfos.begin() );
    for(; it != d->interpreterinfos.end(); ++it)
        if( it.value() )
            d->interpreters << it.key();
    //d->interpreters.sort();

    // publish ourself.
    ChildrenInterface::addObject(this, "Kross");
}

Manager::~Manager()
{
    qDeleteAll(d->interpreterinfos.values());
    qDeleteAll(d->modules.values());
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
        foreach(QString wildcard, it.value()->wildcard().split(" ", QString::SkipEmptyParts)) {
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

    QString libraryname = QString("krossmodule%1").arg(modulename).toLower();
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

    //krossdebug( QString("Manager::module Module successfully loaded: modulename=%1 module.objectName=%2 module.className=%3").arg(modulename).arg(module->objectName()).arg(module->metaObject()->className()) );
    d->modules.insert(modulename, module);
    return module;
}

bool Manager::executeScriptFile(const QString& file)
{
    krossdebug( QString("Manager::executeScriptFile() file='%1'").arg(file) );
    Action* action = new Action(0 /*no parent*/, QUrl(file));
    action->trigger();
    bool ok = ! action->hadError();
    delete action; //action->delayedDestruct();
    return ok;
}

#include "manager.moc"
