/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef KUTILS_KCMODULEFACTORY_H
#define KUTILS_KCMODULEFACTORY_H

#include "klibloader.h"

class KCModuleFactoryPrivate;
class KCModule;
class KCModuleInfo;
class QWidget;
class QStringList;

/**
 * Specialized KLibFactory for KCModule construction.
 *
 * This class is used to create KCModule instances and is especially interesting to create more than
 * one type of KCModule from the same DSO. E.g. a subclass could be used to load all python modules
 * using a special key in the desktop file which is accessible through the \p kcmInfo parameter
 * passed to the create method.
 *
 * Example:
 * \code
 *
 * class PythonKCMLoader : public KCModuleFactory
 * {
 *     Q_OBJECT
 *     public:
 *         PythonKCMLoader(QObject *parent = 0);
 *         KCModule *create(const KCModuleInfo &kcmInfo, QWidget *parent, const QStringList &args);
 * };
 *
 * PythonKCMLoader::PythonKCMLoader(QObject *parent)
 *     : KCModuleFactory(parent)
 * {
 * }
 *
 * KCModule *PythonKCMLoader::create(const KCModuleInfo &kcmInfo, QWidget *parent, const QStringList &args)
 * {
 *     const KService::Ptr service = kcmInfo.service();
 *     const QVariant pythonKCMVariant = service->property(QLatin1String("X-KDE-PhythonKCM"));
 *     if (!pythonKCMVariant.isValid()) {
 *         return 0;
 *     }
 *     const QString pythonKCM = pythonKCMVariant.toString();
 *     // load python module from the information in pythonKCM
 *     // and return it:
 *     return module;
 * }
 *
 * Q_EXPORT_PLUGIN2(pythonkcmloader, PythonKCMLoader)
 * \endcode
 *
 * The first argument to Q_EXPORT_PLUGIN2 is the name of the plugin, i.e. the first argument you
 * pass to kde4_add_plugin in CMakeLists.txt.
 *
 * \author Matthias Kretz <kretz@kde.org>
 */
class KCModuleFactory : public KLibFactory
{
    Q_OBJECT
    public:
        explicit KCModuleFactory(QObject *parent = 0);
        ~KCModuleFactory();

        /**
         * Returns a new instance of the KCModule subclass identified by \p kcmInfo.
         *
         * \param kcmInfo The KCModuleInfo for the .desktop file of the KCM that should be loaded.
         * \param parent The parent QWidget that should be passed to the KCModule constructor.
         * \param args The argument list that should be passed to the KCModule constructor.
         */
        virtual KCModule *create(const KCModuleInfo &kcmInfo, QWidget *parent, const QStringList &args) = 0;

    private:
        KCModuleFactoryPrivate *const d;
};

#endif // KUTILS_KCMODULEFACTORY_H
