/* This file is part of the KDE libraries
   Copyright (C) 2000 Torben Weis <weis@kde.org>
   Copyright (C) 2006 David Faure <faure@kde.org>
   Copyright 2013 Sebastian Kügler <sebas@kde.org>

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

#include "kplugintrader.h"
#include "ktraderparsetree_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDirIterator>
#include <QtCore/QJsonObject>

using namespace KTraderParse;


inline QStringList suffixFilters()
{
#if defined(Q_OS_WIN) || defined(Q_OS_CYGWIN)
    return QStringList() << QStringLiteral(".dll");
#else
    return QStringList() << QStringLiteral("*.so")
                         << QStringLiteral("*.dylib")
                         << QStringLiteral("*.bundle")
                         << QStringLiteral("*.sl");
#endif
}

class KPluginTraderSingleton
{
public:
    KPluginTrader instance;
};

Q_GLOBAL_STATIC(KPluginTraderSingleton, s_globalPluginTrader)

KPluginTrader* KPluginTrader::self()
{
    return &s_globalPluginTrader()->instance;
}

KPluginTrader::KPluginTrader()
    : d(0)
{
}

KPluginTrader::~KPluginTrader()
{
}

void KPluginTrader::applyConstraints(KPluginInfo::List& lst, const QString& constraint )
{
    if (lst.isEmpty() || constraint.isEmpty()) {
        return;
    }

    const ParseTreeBase::Ptr constr = parseConstraints(constraint); // for ownership
    const ParseTreeBase* pConstraintTree = constr.data(); // for speed

    if (!constr) { // parse error
        lst.clear();
    } else {
        // Find all plugin infos matching the constraint and remove the rest
        KPluginInfo::List::iterator it = lst.begin();
        while(it != lst.end()) {
            if (matchConstraintPlugin(pConstraintTree, *it, lst) != 1) {
                it = lst.erase(it);
            } else {
                ++it;
            }
        }
    }
}

KPluginInfo::List KPluginTrader::query(const QString& servicetype, const QString& subDirectory, const QString& constraint)
{
    QPluginLoader loader;
    const QStringList libraryPaths = QCoreApplication::libraryPaths();
    KPluginInfo::List lst;

    Q_FOREACH (const QString& dir, libraryPaths) {
        QDirIterator it(dir + QDir::separator() + subDirectory, suffixFilters(), QDir::Files);
        while (it.hasNext()) {
            it.next();
            const QString _f = it.fileInfo().absoluteFilePath();
            loader.setFileName(_f);
            const QVariantList argsWithMetaData = QVariantList() << loader.metaData().toVariantMap();
            KPluginInfo info(argsWithMetaData, _f);
            if (servicetype.isEmpty() || info.serviceTypes().contains(servicetype)) {
                lst << info;
            }
        }
    }
    applyConstraints(lst, constraint);
    return lst;
}
