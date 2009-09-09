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

#include <kaction.h>
#include <kdebug.h>
#include <kstringhandler.h>

#include <QTreeWidgetItem>

KShortcutsEditorItem::KShortcutsEditorItem(QTreeWidgetItem *parent, KAction *action)
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
    m_actionNameInTable = i18nc("@item:intable Action name in shortcuts configuration", "%1", KGlobal::locale()->removeAcceleratorMarker(m_action->text()));
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
            return m_action->shapeGesture().shapeName();
        case RockerGesture:
            return m_action->rockerGesture().rockerName();
        default:
            break;
        }
        break;
    case Qt::DecorationRole:
        if (column == Name)
            return m_action->icon();
        else
            return KIcon();
        break;
    case Qt::WhatsThisRole:
        return m_action->whatsThis();
    case Qt::ToolTipRole:
        // There is no such thing as a KAction::description(). So we have
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
            if (!m_action->isShortcutConfigurable()) {
                return false;
            }
            return true;
        case GlobalPrimary:
        case GlobalAlternate:
            if (!m_action->isGlobalShortcutEnabled()) {
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
            ret.setValue(m_action->shapeGesture());
            return ret; }
        case RockerGesture: {
            QVariant ret;
            ret.setValue(m_action->rockerGesture());
            return ret; }
        default:
            // Column not valid for this role
            Q_ASSERT(false);
            return QVariant();
        }

    case DefaultShortcutRole:
        switch(column) {
        case LocalPrimary:
            return m_action->shortcut(KAction::DefaultShortcut).primary();
        case LocalAlternate:
            return m_action->shortcut(KAction::DefaultShortcut).alternate();
        case GlobalPrimary:
            return m_action->globalShortcut(KAction::DefaultShortcut).primary();
        case GlobalAlternate:
            return m_action->globalShortcut(KAction::DefaultShortcut).alternate();
        case ShapeGesture: {
            QVariant ret;
            ret.setValue(m_action->shapeGesture(KAction::DefaultShortcut));
            return ret; }
        case RockerGesture: {
            QVariant ret;
            ret.setValue(m_action->rockerGesture(KAction::DefaultShortcut));
            return ret; }
        default:
            // Column not valid for this role
            Q_ASSERT(false);
            return QVariant();
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
    switch (column) {
    case LocalPrimary:
        return m_action->shortcut().primary();
    case LocalAlternate:
        return m_action->shortcut().alternate();
    case GlobalPrimary:
        return m_action->globalShortcut().primary();
    case GlobalAlternate:
        return m_action->globalShortcut().alternate();
    default:
        return QKeySequence();
    }
}


void KShortcutsEditorItem::setKeySequence(uint column, const QKeySequence &seq)
{
    KShortcut ks;
    if (column == GlobalPrimary || column == GlobalAlternate) {
        ks = m_action->globalShortcut();
        if (!m_oldGlobalShortcut)
            m_oldGlobalShortcut = new KShortcut(ks);
     } else {
        ks = m_action->shortcut();
        if (!m_oldLocalShortcut)
            m_oldLocalShortcut = new KShortcut(ks);
    }

    if (column == LocalAlternate || column == GlobalAlternate)
        ks.setAlternate(seq);
    else
        ks.setPrimary(seq);

    //avoid also setting the default shortcut - what we are setting here is custom by definition
    if (column == GlobalPrimary || column == GlobalAlternate) {
        m_action->setGlobalShortcut(ks, KAction::ActiveShortcut, KAction::NoAutoloading);
    } else {
        m_action->setShortcut(ks, KAction::ActiveShortcut);
    }

    updateModified();
}


void KShortcutsEditorItem::setShapeGesture(const KShapeGesture &gst)
{
    if (!m_oldShapeGesture) {
        m_oldShapeGesture = new KShapeGesture(gst);
    }
    m_action->setShapeGesture(gst);
    updateModified();
}


void KShortcutsEditorItem::setRockerGesture(const KRockerGesture &gst)
{
    if (!m_oldRockerGesture) {
        m_oldRockerGesture = new KRockerGesture(gst);
    }
    m_action->setRockerGesture(gst);
    updateModified();
}


//our definition of modified is "modified since the chooser was shown".
void KShortcutsEditorItem::updateModified()
{
    if (m_oldLocalShortcut && *m_oldLocalShortcut == m_action->shortcut()) {
        delete m_oldLocalShortcut;
        m_oldLocalShortcut = 0;
    }
    if (m_oldGlobalShortcut && *m_oldGlobalShortcut == m_action->globalShortcut()) {
        delete m_oldGlobalShortcut;
        m_oldGlobalShortcut = 0;
    }
    if (m_oldShapeGesture && *m_oldShapeGesture == m_action->shapeGesture()) {
        delete m_oldShapeGesture;
        m_oldShapeGesture = 0;
    }
    if (m_oldRockerGesture && *m_oldRockerGesture == m_action->rockerGesture()) {
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
            return m_oldLocalShortcut->primary() != m_action->shortcut().primary();
        else
            return m_oldLocalShortcut->alternate() != m_action->shortcut().alternate();
    case GlobalPrimary:
    case GlobalAlternate:
        if (!m_oldGlobalShortcut)
            return false;
        if (column == GlobalPrimary)
            return m_oldGlobalShortcut->primary() != m_action->globalShortcut().primary();
        else
            return m_oldGlobalShortcut->alternate() != m_action->globalShortcut().alternate();
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
        m_action->setShortcut(*m_oldLocalShortcut, KAction::ActiveShortcut);
    }

    if (m_oldGlobalShortcut) {
        m_action->setGlobalShortcut(*m_oldGlobalShortcut, KAction::ActiveShortcut,
                                    KAction::NoAutoloading);
    }

    if (m_oldShapeGesture) {
        m_action->setShapeGesture(*m_oldShapeGesture);
    }

    if (m_oldRockerGesture) {
        m_action->setRockerGesture(*m_oldRockerGesture);
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
