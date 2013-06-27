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

class KQPluginFactory
{
    public:
        //virtual KQPluginFactory();
        //virtual ~KQPluginFactory();
        virtual ~KQPluginFactory() {}
        template<typename T>T *create(QObject *parent = 0, const QVariantList &args = QVariantList());

        virtual QObject* createPlugin(const QString &name) = 0;
};

template<typename T>
inline T *KQPluginFactory::create(QObject *parent, const QVariantList &args)
{
    QObject *o = create(T::staticMetaObject.className(), parent && parent->isWidgetType() ? reinterpret_cast<QWidget *>(parent): 0, parent, args, QString());

    T *t = qobject_cast<T *>(o);
    if (!t) {
        delete o;
    }
    return t;
}

#define KQPluginFactory_iid "org.kde.KQPluginFactory"

Q_DECLARE_INTERFACE(KQPluginFactory, KQPluginFactory_iid)

#endif // KQPLUGINFACTORYINTERFACE_H
