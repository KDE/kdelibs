/* This file is part of the KDE libraries Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997 Nicolas Hadacek <hadacek@kde.org>
    Copyright (C) 1998 Matthias Ettrich <ettrich@kde.org>
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
    Copyright (C) 2007 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kshortcutsdialog_p.h"

#include <QAction>
#include <kgesturemap.h>
#include <kglobalaccel.h>
#include <kdebug.h>
#include <kstringhandler.h>

#include <QTreeWidgetItem>

KShortcutsEditorItem::KShortcutsEditorItem(QTreeWidgetItem *parent, QAction *action)
    : QTreeWidgetItem(parent, ActionItem)
    , m_action(action)
    , m_isNameBold(false)
    , m_oldLocalShortcut(0)
    , m_oldGlobalShortcut(0)
    , m_oldShapeGesture(0)
    , m_oldRockerGesture(0)
{
    // Filtering message requested by translators (scripting).
    m_id = m_action->objectName();
    m_actionNameInTable = i18nc("@item:intable Action name in shortcuts configuration", "%1", KLocalizedString::removeAcceleratorMarker(m_action->text()));
    if (m_actionNameInTable.isEmpty()) {
        kWarning() << "Action without text!" << m_action->objectName();
        m_actionNameInTable = m_id;
    }
}


KShortcutsEditorItem::~KShortcutsEditorItem()
{
    delete m_oldLocalShortcut;
    delete m_oldGlobalShortcut;
    delete m_oldShapeGesture;
    delete m_oldRockerGesture;
}


bool KShortcutsEditorItem::isModified() const
{
    return m_oldLocalShortcut || m_oldGlobalShortcut || m_oldShapeGesture || m_oldRockerGesture;
}


QVariant KShortcutsEditorItem::data(int column, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        switch(column) {
        case Name:
            return m_actionNameInTable;
        case Id:
            return m_id;
        case LocalPrimary:
        case LocalAlternate:
        case GlobalPrimary:
        case GlobalAlternate:
            return keySequence(column);
        case ShapeGesture:
            return KGestureMap::self()->shapeGesture(m_action).shapeName();
        case RockerGesture:
            return KGestureMap::self()->rockerGesture(m_action).rockerName();
        default:
            break;
        }
        break;
    case Qt::DecorationRole:
        if (column == Name)
            return m_action->icon();
        else
            return QIcon();
        break;
    case Qt::WhatsThisRole:
        return m_action->whatsThis();
    case Qt::ToolTipRole:
        // There is no such thing as a QAction::description(). So we have
        // nothing to display here.
        return QVariant();
    case Qt::FontRole:
        if (column == Name && m_isNameBold) {
            QFont modifiedFont = treeWidget()->font();
            modifiedFont.setBold(true);
            return modifiedFont;
        }
        break;
    case KExtendableItemDelegate::ShowExtensionIndicatorRole:
        switch (column) {
        case Name:
            return false;
        case LocalPrimary:
        case LocalAlternate:
            return !m_action->property("isShortcutConfigurable").isValid()
                || m_action->property("isShortcutConfigurable").toBool();
        case GlobalPrimary:
        case GlobalAlternate:
            if (KGlobalAccel::self()->hasShortcut(m_action)) {
                return false;
            }
            return true;
        default:
            return false;
        }
    //the following are custom roles, defined in this source file only
    case ShortcutRole:
        switch(column) {
        case LocalPrimary:
        case LocalAlternate:
        case GlobalPrimary:
        case GlobalAlternate:
            return keySequence(column);
        case ShapeGesture: { //scoping for "ret"
            QVariant ret;
            ret.setValue(KGestureMap::self()->shapeGesture(m_action));
            return ret; }
        case RockerGesture: {
            QVariant ret;
            ret.setValue(KGestureMap::self()->rockerGesture(m_action));
            return ret; }
        default:
            // Column not valid for this role
            Q_ASSERT(false);
            return QVariant();
        }


    case DefaultShortcutRole: {
        QList<QKeySequence> defaultShortcuts = m_action->property("defaultShortcuts").value<QList<QKeySequence> >();
        KShortcut defaultGlobalShortcuts = KGlobalAccel::self()->defaultShortcut(m_action);

        switch(column) {
        case LocalPrimary:
            return defaultShortcuts.isEmpty() ? QKeySequence() : defaultShortcuts.at(0);
        case LocalAlternate:
            return defaultShortcuts.size() <= 1 ? QKeySequence() : defaultShortcuts.at(1);
        case GlobalPrimary:
            return defaultGlobalShortcuts.primary();
        case GlobalAlternate:
            return defaultGlobalShortcuts.alternate();
        case ShapeGesture: {
            QVariant ret;
            ret.setValue(KGestureMap::self()->defaultShapeGesture(m_action));
            return ret; }
        case RockerGesture: {
            QVariant ret;
            ret.setValue(KGestureMap::self()->defaultRockerGesture(m_action));
            return ret; }
        default:
            // Column not valid for this role
            Q_ASSERT(false);
            return QVariant();
        }
    }
    case ObjectRole:
        return qVariantFromValue((QObject*)m_action);

    default:
        break;
    }

    return QVariant();
}


bool KShortcutsEditorItem::operator<(const QTreeWidgetItem &other) const
{
    const int column = treeWidget() ? treeWidget()->sortColumn() : 0;
    return KStringHandler::naturalCompare(text(column), other.text(column)) < 0;
}


QKeySequence KShortcutsEditorItem::keySequence(uint column) const
{
    QList<QKeySequence> shortcuts = m_action->shortcuts();
    KShortcut globalShortcut = KGlobalAccel::self()->shortcut(m_action);

    switch (column) {
    case LocalPrimary:
        return shortcuts.isEmpty() ? QKeySequence() : shortcuts.at(0);
    case LocalAlternate:
        return shortcuts.size() <= 1 ? QKeySequence() : shortcuts.at(1);
    case GlobalPrimary:
        return globalShortcut.primary();
    case GlobalAlternate:
        return globalShortcut.alternate();
    default:
        return QKeySequence();
    }
}


void KShortcutsEditorItem::setKeySequence(uint column, const QKeySequence &seq)
{
    KShortcut ks;
    if (column == GlobalPrimary || column == GlobalAlternate) {
        ks = KGlobalAccel::self()->shortcut(m_action);
        if (!m_oldGlobalShortcut)
            m_oldGlobalShortcut = new KShortcut(ks);
     } else {
        ks = KShortcut(m_action->shortcuts());
        if (!m_oldLocalShortcut)
            m_oldLocalShortcut = new KShortcut(ks);
    }

    if (column == LocalAlternate || column == GlobalAlternate)
        ks.setAlternate(seq);
    else
        ks.setPrimary(seq);

    //avoid also setting the default shortcut - what we are setting here is custom by definition
    if (column == GlobalPrimary || column == GlobalAlternate) {
        KGlobalAccel::self()->setShortcut(m_action, ks, KGlobalAccel::NoAutoloading);
    } else {
        m_action->setShortcuts(ks);
    }

    updateModified();
}


void KShortcutsEditorItem::setShapeGesture(const KShapeGesture &gst)
{
    if (!m_oldShapeGesture) {
        m_oldShapeGesture = new KShapeGesture(gst);
    }
    KGestureMap::self()->setShapeGesture(m_action, gst);
    KGestureMap::self()->setDefaultShapeGesture(m_action, gst);
    updateModified();
}


void KShortcutsEditorItem::setRockerGesture(const KRockerGesture &gst)
{
    if (!m_oldRockerGesture) {
        m_oldRockerGesture = new KRockerGesture(gst);
    }
    KGestureMap::self()->setRockerGesture(m_action, gst);
    KGestureMap::self()->setDefaultRockerGesture(m_action, gst);
    updateModified();
}


//our definition of modified is "modified since the chooser was shown".
void KShortcutsEditorItem::updateModified()
{
    if (m_oldLocalShortcut && *m_oldLocalShortcut == KShortcut(m_action->shortcuts())) {
        delete m_oldLocalShortcut;
        m_oldLocalShortcut = 0;
    }
    if (m_oldGlobalShortcut && *m_oldGlobalShortcut == KGlobalAccel::self()->shortcut(m_action)) {
        delete m_oldGlobalShortcut;
        m_oldGlobalShortcut = 0;
    }
    if (m_oldShapeGesture && *m_oldShapeGesture == KGestureMap::self()->shapeGesture(m_action)) {
        delete m_oldShapeGesture;
        m_oldShapeGesture = 0;
    }
    if (m_oldRockerGesture && *m_oldRockerGesture == KGestureMap::self()->rockerGesture(m_action)) {
        delete m_oldRockerGesture;
        m_oldRockerGesture = 0;
    }
}


bool KShortcutsEditorItem::isModified(uint column) const
{
    switch (column) {
    case Name:
        return false;
    case LocalPrimary:
    case LocalAlternate:
        if (!m_oldLocalShortcut)
            return false;
        if (column == LocalPrimary)
            return m_oldLocalShortcut->primary() != KShortcut(m_action->shortcuts()).primary();
        else
            return m_oldLocalShortcut->alternate() != KShortcut(m_action->shortcut()).alternate();
    case GlobalPrimary:
    case GlobalAlternate:
        if (!m_oldGlobalShortcut)
            return false;
        if (column == GlobalPrimary)
            return m_oldGlobalShortcut->primary() != KGlobalAccel::self()->shortcut(m_action).primary();
        else
            return m_oldGlobalShortcut->alternate() != KGlobalAccel::self()->shortcut(m_action).alternate();
    case ShapeGesture:
        return static_cast<bool>(m_oldShapeGesture);
    case RockerGesture:
        return static_cast<bool>(m_oldRockerGesture);
    default:
        return false;
    }
}



void KShortcutsEditorItem::undo()
{
#ifndef NDEBUG
    if (m_oldLocalShortcut || m_oldGlobalShortcut || m_oldShapeGesture || m_oldRockerGesture ) {
        kDebug(125) << "Undoing changes for " << data(Name, Qt::DisplayRole).toString();
    }
#endif
    if (m_oldLocalShortcut) {
        // We only ever reset the active Shortcut
        m_action->setShortcuts(*m_oldLocalShortcut);
    }

    if (m_oldGlobalShortcut) {
        KGlobalAccel::self()->setShortcut(m_action, *m_oldGlobalShortcut, KGlobalAccel::NoAutoloading);
    }

    if (m_oldShapeGesture) {
        KGestureMap::self()->setShapeGesture(m_action, *m_oldShapeGesture);
        KGestureMap::self()->setDefaultShapeGesture(m_action, *m_oldShapeGesture);
    }

    if (m_oldRockerGesture) {
        KGestureMap::self()->setRockerGesture(m_action, *m_oldRockerGesture);
        KGestureMap::self()->setDefaultRockerGesture(m_action, *m_oldRockerGesture);
    }

    updateModified();
}


void KShortcutsEditorItem::commit()
{
#ifndef NDEBUG
    if (m_oldLocalShortcut || m_oldGlobalShortcut || m_oldShapeGesture || m_oldRockerGesture ) {
        kDebug(125) << "Committing changes for " << data(Name, Qt::DisplayRole).toString();
    }
#endif

    delete m_oldLocalShortcut;
    m_oldLocalShortcut = 0;
    delete m_oldGlobalShortcut;
    m_oldGlobalShortcut = 0;
    delete m_oldShapeGesture;
    m_oldShapeGesture = 0;
    delete m_oldRockerGesture;
    m_oldRockerGesture = 0;
}
