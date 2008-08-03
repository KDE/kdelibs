/*****************************************************************************
 * Copyright (C) 2006 by Peter Penz <peter.penz@gmx.at>                      *
 * Copyright (C) 2006 by Aaron J. Seigo <aseigo@kde.org>                     *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License version 2 as published by the Free Software Foundation.           *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#include "kurlbutton_p.h"

#include "kurlnavigator.h"

#include <kcolorscheme.h>
#include <kicon.h>
#include <klocale.h>
#include <kmenu.h>

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QStyle>
#include <QStyleOptionFocusRect>

KUrlButton::KUrlButton(KUrlNavigator* parent) :
    QPushButton(parent),
    m_displayHint(0),
    m_urlNavigator(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    setMinimumHeight(parent->minimumHeight());

    connect(this, SIGNAL(pressed()), parent, SLOT(requestActivation()));
}

KUrlButton::~KUrlButton()
{
}

void KUrlButton::setDisplayHintEnabled(DisplayHint hint,
                                       bool enable)
{
    if (enable) {
        m_displayHint = m_displayHint | hint;
    } else {
        m_displayHint = m_displayHint & ~hint;
    }
    update();
}

bool KUrlButton::isDisplayHintEnabled(DisplayHint hint) const
{
    return (m_displayHint & hint) > 0;
}

void KUrlButton::enterEvent(QEvent* event)
{
    QPushButton::enterEvent(event);
    setDisplayHintEnabled(EnteredHint, true);
    update();
}

void KUrlButton::leaveEvent(QEvent* event)
{
    QPushButton::leaveEvent(event);
    setDisplayHintEnabled(EnteredHint, false);
    update();
}

void KUrlButton::contextMenuEvent(QContextMenuEvent* event)
{
    Q_UNUSED(event);
    m_urlNavigator->requestActivation();

    KMenu popup(this);

    // provide 'Copy' action, which copies the current URL of
    // the URL navigator into the clipboard
    QAction* copyAction = popup.addAction(KIcon("edit-copy"), i18n("Copy"));

    // provide 'Paste' action, which copies the current clipboard text
    // into the URL navigator
    QAction* pasteAction = popup.addAction(KIcon("edit-paste"), i18n("Paste"));
    QClipboard* clipboard = QApplication::clipboard();
    pasteAction->setEnabled(!clipboard->text().isEmpty());

    popup.addSeparator();

    // provide radiobuttons for toggling between the edit and the navigation mode
    QAction* editAction = popup.addAction(i18n("Edit"));
    editAction->setCheckable(true);

    QAction* navigateAction = popup.addAction(i18n("Navigate"));
    navigateAction->setCheckable(true);

    QActionGroup* modeGroup = new QActionGroup(&popup);
    modeGroup->addAction(editAction);
    modeGroup->addAction(navigateAction);
    if (m_urlNavigator->isUrlEditable()) {
        editAction->setChecked(true);
    } else {
        navigateAction->setChecked(true);
    }
    
    popup.addSeparator();
    
    // allow showing of the full path
    QAction* showFullPathAction = popup.addAction(i18n("Show Full Path"));
    showFullPathAction->setCheckable(true);
    showFullPathAction->setChecked(m_urlNavigator->showFullPath());

    QAction* activatedAction = popup.exec(QCursor::pos());
    if (activatedAction == copyAction) {
        QMimeData* mimeData = new QMimeData();
        mimeData->setText(m_urlNavigator->url().prettyUrl());
        clipboard->setMimeData(mimeData);
    } else if (activatedAction == pasteAction) {
        m_urlNavigator->setUrl(KUrl(clipboard->text()));
    } else if (activatedAction == editAction) {
        m_urlNavigator->setUrlEditable(true);
    } else if (activatedAction == navigateAction) {
        m_urlNavigator->setUrlEditable(false);
    } else if (activatedAction == showFullPathAction) {
        m_urlNavigator->setShowFullPath(showFullPathAction->isChecked());
    }
}

void KUrlButton::drawHoverBackground(QPainter* painter)
{
    const bool isHighlighted = isDisplayHintEnabled(EnteredHint) ||
                               isDisplayHintEnabled(DraggedHint) ||
                               isDisplayHintEnabled(PopupActiveHint);

    QColor backgroundColor = isHighlighted ? palette().color(QPalette::Highlight) : Qt::transparent;
    if (!urlNavigator()->isActive() && isHighlighted) {
        backgroundColor.setAlpha(128);
    }

    if (backgroundColor != Qt::transparent) {
        // TODO: the backgroundColor should be applied to the style
        QStyleOptionViewItemV4 option;
        option.initFrom(this);
        option.state = QStyle::State_Enabled | QStyle::State_MouseOver;
        option.viewItemPosition = QStyleOptionViewItemV4::OnlyOne;
        style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, this);
    }
}

QColor KUrlButton::foregroundColor() const
{
    const bool isHighlighted = isDisplayHintEnabled(EnteredHint) ||
                               isDisplayHintEnabled(DraggedHint) ||
                               isDisplayHintEnabled(PopupActiveHint);

    QColor foregroundColor = palette().color(foregroundRole());
    const bool isActive = m_urlNavigator->isActive();

    int alpha = isActive ? 255 : 128;
    if ((!isDisplayHintEnabled(ActivatedHint) || !isActive) && !isHighlighted) {
        alpha -= alpha / 4;
    }
    foregroundColor.setAlpha(alpha);

    return foregroundColor;
}

#include "kurlbutton_p.moc"
