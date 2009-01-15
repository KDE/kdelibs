/***************************************************************************
 * krossconfig.h
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

#ifndef KROSS_MAIN_KROSSCONFIG_H
#define KROSS_MAIN_KROSSCONFIG_H

#include <kross/core/kross_export.h>

#include <QtCore/QString>
#include <QtCore/QMetaType>

namespace Kross {

    // Debugging enabled. Comment the line out to disable all kind of debugging.
    #define KROSS_DEBUG_ENABLED

    #ifdef KROSS_DEBUG_ENABLED

        /**
         * Debugging function.
         */
        KROSSCORE_EXPORT void krossdebug(const QString &s);

        /**
         * Warning function.
         */
        KROSSCORE_EXPORT void krosswarning(const QString &s);

    #else
        // Define these to an empty statement if debugging is disabled.
        #define krossdebug(x)
        #define krosswarning(x)
    #endif

    // Some more debug switches.
    //#define KROSS_OBJECT_METACALL_DEBUG
    //#define KROSS_METATYPE_DEBUG
    //#define KROSS_INTERPRETER_DEBUG
    //#define KROSS_ACTION_DEBUG
    //#define KROSS_ACTIONCOLLECTION_DEBUG

    // The version number of Kross. For example the interpreters use
    // it do be sure there are linked against the correct core version
    // and if the numbers don't match, the interpreter is not loaded.
    #define KROSS_VERSION 12

    // The export macro for interpreter plugins.
    #define KROSS_EXPORT_INTERPRETER( InterpreterImpl ) \
        extern "C" { \
            KDE_EXPORT void* krossinterpreter(int version, Kross::InterpreterInfo* info) { \
                if(version != KROSS_VERSION) { \
                    Kross::krosswarning(QString("Interpreter skipped cause provided version %1 does not match expected version %2.").arg(version).arg(KROSS_VERSION)); \
                    return 0; \
                } \
                return new InterpreterImpl(info); \
            } \
        }

    // The name of the interpreter's library. Those library got loaded
    // dynamically during runtime. Comment out to disable compiling of
    // the interpreter-plugin or to hardcode the location of the lib
    // like I did at the following line.
    //#define KROSS_PYTHON_LIBRARY "/home/kde4/koffice/_build/lib/krosspython.la"
    #define KROSS_PYTHON_LIBRARY "krosspython"
    #define KROSS_RUBY_LIBRARY "krossruby"
    #define KROSS_KJS_LIBRARY "krosskjs"
    #define KROSS_JAVA_LIBRARY "libkrossjava"
    #define KROSS_FALCON_LIBRARY "krossfalcon"
    #define KROSS_QTSCRIPT_LIBRARY "krossqts"
    #define KROSS_LUA_LIBRARY "kloss"

}

#endif

