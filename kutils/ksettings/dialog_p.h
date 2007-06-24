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

#ifndef KSETTINGS_DIALOG_P_H
#define KSETTINGS_DIALOG_P_H

#include "dialog.h"
#include "../kcmoduleinfo.h"

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtGui/QLabel>
#include <QtGui/QWidget>

#include <kpagewidgetmodel.h>
#include <kservice.h>
#include <kplugininfo.h>
#include <kvbox.h>

namespace KSettings
{

struct GroupInfo
{
    QString id;
    QString name;
    QString comment;
    QString icon;
    int weight;
    QString parentid;
    QWidget *page;
};

// The TreeList can get really complicated. That's why a tree data structure
// is necessary to make it suck less
class PageNode
{
    private:
        typedef QList<PageNode*> List;
        enum Type { KCM, Group, Root };
        union Value
        {
            KCModuleInfo *kcm;
            GroupInfo *group;
        };
        Type m_type;
        Value m_value;

        Dialog *m_dialog;
        List m_children;
        PageNode *m_parent;
        bool m_visible;
        bool m_dirty;
        KPageWidgetItem *m_pageWidgetItem;

    protected:
        PageNode(KCModuleInfo *info, PageNode *parent)
            : m_type(KCM), m_parent(parent), m_visible(true), m_dirty(true)
        {
            m_value.kcm = info;
            m_dialog = parent->m_dialog;
        }

        PageNode(GroupInfo &group, PageNode *parent)
            : m_type(Group), m_parent(parent), m_visible(true), m_dirty(true)
        {
            m_value.group = new GroupInfo(group);
            m_value.group->page = 0;
            m_dialog = parent->m_dialog;
        }

        void bubbleSort(List::Iterator begin, List::Iterator end)
        {
            --end;
            bool finished;
            List::Iterator lastswapped = begin;
            List::Iterator i;
            List::Iterator j;
            while(begin != end)
            {
                finished = true;
                i = j = end;
                do {
                    --j;
                    if (**i < **j)
                    {
                        finished = false;
                        qSwap(*i, *j);
                        lastswapped = j;
                    }
                    --i;
                } while(j != begin);
                if (finished)
                    return;
                ++lastswapped;
                begin = lastswapped;
            }
        }

    public:
        PageNode(Dialog *dialog)
            : m_type(Root), m_dialog(dialog), m_parent(0), m_visible(true), m_dirty(true)
        {
        }

        ~PageNode()
        {
            if (KCM == m_type)
                delete m_value.kcm;
            else if (Group == m_type)
                delete m_value.group;
            List::Iterator end = m_children.end();
            for (List::Iterator it = m_children.begin(); it != end; ++it)
                delete (*it);
        }

        int weight() const
        {
            int w = (KCM == m_type) ? m_value.kcm->weight()
                : m_value.group->weight;
            kDebug(700) << k_funcinfo << name() << " " << w << endl;
            return w;
        }

        bool operator<(const PageNode &rhs) const
        {
            return weight() < rhs.weight();
        }

        bool isVisible();
        void makeDirty()
        {
            m_dirty = true;
            List::Iterator end = m_children.end();
            for (List::Iterator it = m_children.begin(); it != end; ++it)
                (*it)->makeDirty();
        }

        QString name() const
        {
            if (Root == m_type)
                return QString::fromAscii("root node");
            return (KCM == m_type) ? m_value.kcm->moduleName()
                : m_value.group->name;
        }

        void setPageWidgetItem(KPageWidgetItem *item)
        {
            m_pageWidgetItem = item;
        }

        KPageWidgetItem* parentPageWidgetItem() const
        {
            PageNode *node = m_parent;
            if (node->m_type == Root)
                return 0;
            else
                return node->m_pageWidgetItem;
        }

        void addToDialog(KCMultiDialog *dlg, const QStringList& arguments)
        {
            kDebug(700) << k_funcinfo << "for " << name() << endl;
            if (! isVisible())
                return;

            if (KCM == m_type)
            {
                m_pageWidgetItem = dlg->addModule(*m_value.kcm,
                        parentPageWidgetItem(),
                        arguments);
                return;
            }
            if (Group == m_type && 0 == m_value.group->page)
            {
                KVBox *page = new KVBox();
                QLabel *comment = new QLabel(m_value.group->comment, page);
                comment->setTextFormat(Qt::RichText);
                m_value.group->page = page;

                m_pageWidgetItem = dlg->addPage(page, m_value.group->name);
                m_pageWidgetItem->setIcon(KIcon(m_value.group->icon));
            }
            List::Iterator end = m_children.end();
            for (List::Iterator it = m_children.begin(); it != end; ++it)
                (*it)->addToDialog(dlg, arguments);
        }

        void removeFromDialog(KCMultiDialog *dlg)
        {
            kDebug(700) << k_funcinfo << "for " << name() << endl;
            if (KCM == m_type)
                return;
            if (Root == m_type)
                dlg->clear();
            List::Iterator end = m_children.end();
            for (List::Iterator it = m_children.begin(); it != end; ++it)
                (*it)->removeFromDialog(dlg);
            if (Group == m_type)
            {
                delete m_value.group->page;
                m_value.group->page = 0;
            }
        }

        void sort()
        {
            kDebug(700) << k_funcinfo << name() << endl;
            if (m_children.isEmpty())
                return;
            List::Iterator begin = m_children.begin();
            List::Iterator end = m_children.end();
            bubbleSort(begin, end);
            for (List::Iterator it = begin ; it != end; ++it)
                (*it)->sort();
        }

        bool insert(GroupInfo &group)
        {
            if (group.parentid.isNull())
            {
                if (Root == m_type)
                {
                    m_children.append(new PageNode(group, this));
                    return true;
                }
                else
                    kFatal(700) << "wrong PageNode insertion"
                        << kBacktrace() << endl;
            }
            if (Group == m_type && group.parentid == m_value.group->id)
            {
                m_children.append(new PageNode(group, this));
                return true;
            }
            List::Iterator end = m_children.end();
            for (List::Iterator it = m_children.begin(); it != end; ++it)
                if ((*it)->insert(group))
                    return true;
            // no parent with the right parentid
            if (Root == m_type)
            {
                m_children.append(new PageNode(group, this));
                return true;
            }
            return false;
        }

        bool insert(KCModuleInfo *info, const QString &parentid)
        {
            if (parentid.isNull())
            {
                if (Root == m_type)
                {
                    m_children.append(new PageNode(info, this));
                    return true;
                }
                else
                    kFatal(700) << "wrong PageNode insertion"
                        << kBacktrace() << endl;
            }
            if (Group == m_type && parentid == m_value.group->id)
            {
                m_children.append(new PageNode(info, this));
                return true;
            }
            List::Iterator end = m_children.end();
            for (List::Iterator it = m_children.begin(); it != end; ++it)
                if ((*it)->insert(info, parentid))
                    return true;
            // no parent with the right parentid
            if (Root == m_type)
            {
                m_children.append(new PageNode(info, this));
                return true;
            }
            return false;
        }

        bool needTree()
        {
            List::ConstIterator end = m_children.end();
            for (List::ConstIterator it = m_children.begin(); it != end; ++it)
                if ((*it)->m_children.count() > 0)
                    return true;
            return false;
        }

        bool singleChild()
        {
            return (m_children.count() == 1);
        }
};

class DialogPrivate
{
    friend class PageNode;
    Q_DECLARE_PUBLIC(Dialog)
    protected:
        DialogPrivate(Dialog *d, QWidget *p);

        PageNode pagetree;

        QStringList registeredComponents;
        QStringList componentBlacklist;
        QList<KService::Ptr> services;
        QMap<QString, KPluginInfo*> plugininfomap;
        QStringList arguments;

        KCMultiDialog *dlg;
        QWidget *parentwidget;
        Dialog* q_ptr;

        bool staticlistview;

        void _k_configureTree();
        void _k_updateTreeList();
        void _k_syncConfiguration();
        void _k_reparseConfiguration(const QByteArray &a);

    private:
        /**
         * @internal
         * Check whether the plugin associated with this KCM is enabled.
         */
        bool isPluginForKCMEnabled(KCModuleInfo *) const;

        QList<KService::Ptr> instanceServices();
        QList<KService::Ptr> parentComponentsServices(const QStringList &);

        /**
         * @internal
         * Read the .setdlg file and add it to the groupmap
         */
        void parseGroupFile(const QString &);

        /**
         * @internal
         * If this module is put into a TreeList hierarchy this will return a
         * list of the names of the parent modules.
         */
        //QStringList parentModuleNames(KCModuleInfo *);

        /**
         * @internal
         * This method is called only once. The KCMultiDialog is not created
         * until it's really needed. So if some method needs to access d->dlg it
         * checks for 0 and if it's not created this method will do it.
         */
        void createDialogFromServices(KCMultiDialog *parent);

        /**
         * @internal
         * This method is called in the constructor right after creating the
         * list of service pointers. It removes duplicates from that list, so
         * each entry in the list has a unique name()
         */
        void removeDuplicateServices();
};

bool PageNode::isVisible()
{
    if (m_dirty) {
        if (KCM == m_type) {
            m_visible = m_dialog->d_ptr->isPluginForKCMEnabled(m_value.kcm);
        } else {
            m_visible = false;
            List::Iterator end = m_children.end();
            for (List::Iterator it = m_children.begin(); it != end; ++it) {
                if ((*it)->isVisible()) {
                    m_visible = true;
                    break;
                }
            }
        }
        m_dirty = false;
    }
    kDebug(700) << k_funcinfo << "returns " << m_visible << endl;
    return m_visible;
}

} // namespace KSettings
#endif // KSETTINGS_DIALOG_P_H
