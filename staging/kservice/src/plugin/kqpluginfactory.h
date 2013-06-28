/*
 * Copyright 2013  Sebastian Kügler <sebas@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef KQPLUGINFACTORY_H
#define KQPLUGINFACTORY_H

#include <QObject>
#include <QVariantList>

class KQPluginFactory
{
    public:
        virtual ~KQPluginFactory() {}

        virtual QObject* create(QObject* parent, const QVariantList& args) = 0;

        template<class impl>
        impl *createInstance(QObject *parent, const QVariantList &args = QVariantList())
        {
            return qobject_cast<impl*>(create(parent, args));
        }
};

#define KQPluginFactory_iid "org.kde.KQPluginFactory"

Q_DECLARE_INTERFACE(KQPluginFactory, KQPluginFactory_iid)

#define K_PLUGIN_HEADER(name, baseclass, jsonfile) \
class name : public QObject, public KQPluginFactory \
{ \
    Q_OBJECT \
    Q_PLUGIN_METADATA(IID "KQPluginFactory_iid" FILE jsonfile) \
    Q_INTERFACES(KQPluginFactory) \
\
    public: \
        QObject* create(QObject* parent = 0, const QVariantList& args = QVariantList()) \
        { \
            return new baseclass(parent, args); \
        } \
\
}; \
\


#endif // KQPLUGINFACTORYINTERFACE_H
