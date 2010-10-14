/* This file is part of the KDE libraries
 *
 * Copyright (c) 2010 Aurélien Gâteau <agateau@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#ifndef KDUALACTION_H
#define KDUALACTION_H

#include <kdeui_export.h>
#include <kaction.h>

class KDualActionPrivate;

/**
 * @short An action which can alternate between two texts/icons when triggered.
 *
 * KDualAction should be used when you want to create an action which will
 * return to its original state when triggered again but which should not be
 * rendered as a checkable widget because it is more appropriate to change the
 * text/icon of the action instead.
 *
 * The most common example of use are actions to show/hide a UI element: it is
 * more intuitive to create an action whose text is "Show Foo" when Foo is
 * hidden before changing to "Hide Foo" when Foo is made visible than creating
 * a checkable "Show Foo" action.
 *
 * You should use KDualAction to implement this kind of actions instead of
 * KToggleAction because KToggleAction is rendered as a checkable widget: if
 * you use KToggleAction to implement the previous example, "Show Foo" will be
 * unchecked and "Hide Foo" will be checked.
 *
 * Porting from KToggleAction to KDualAction:
 *
 * 1. If you used the KToggleAction constructor which accepts the action text,
 * adjust the constructor: KDualAction accepts both inactive and active text.
 *
 * 2. Replace connections to the checked(bool) signal with a connection to the
 * activeChanged(bool) (or activeChangedByUser(bool))
 *
 * 3. Replace calls to setChecked()/isChecked() with setActive()/isActive()
 *
 * 4. Replace calls to setCheckedState(guiItem) with
 * setGuiItemForState(KDualAction::ActiveState, guiItem)
 *
 * @since 4.6
 */
class KDEUI_EXPORT KDualAction : public KAction
{
    Q_OBJECT
public:
    enum State {
        InactiveState,
        ActiveState
    };
    /**
     * Constructs a KDualAction with the specified parent. Texts must be set
     * with setTextForState() or setGuiItemForState().
     */
    explicit KDualAction(QObject *parent);

    /**
     * Constructs a KDualAction with the specified parent and texts.
     */
    KDualAction(const QString &inactiveText, const QString &activeText, QObject *parent);

    ~KDualAction();

    /**
     * Sets the icon, text and tooltip for the state with a KGuiItem.
     */
    void setGuiItemForState(State state, const KGuiItem &item);

    /**
     * Gets the KGuiItem for the state.
     */
    KGuiItem guiItemForState(State state) const;

    /**
     * Sets the text for the state.
     */
    void setTextForState(State state, const QString &text);

    /**
     * Gets the text for the state.
     */
    QString textForState(State state) const;

    /**
     * Convenience method to set the icon for both active and inactive states.
     */
    void setIconForStates(const QIcon &icon);

    /**
     * Sets the icon for the state.
     */
    void setIconForState(State state, const QIcon &icon);

    /**
     * Gets the icon for the state.
     */
    QIcon iconForState(State) const;

    /**
     * Sets the tooltip for the state.
     */
    void setToolTipForState(State state, const QString &tip);

    /**
     * Gets the tooltip for the state.
     */
    QString toolTipForState(State state) const;

    /**
     * Returns the action state.
     */
    bool isActive() const;

    /**
     * Defines whether the current action should automatically be changed when
     * the user triggers this action. The default value is true.
     */
    void setAutoToggle(bool);

    /**
     * Returns whether the current action will automatically be changed when
     * the user triggers this action.
     */
    bool autoToggle() const;

public Q_SLOTS:
    /**
     * Sets the action state.
     * activeChanged() will be emitted but not activeChangedByUser().
     */
    void setActive(bool state);

Q_SIGNALS:
    /**
     * Emitted when the state changes. This signal is emitted when the user
     * trigger the action and when setActive() is called.
     */
    void activeChanged(bool);

    /**
     * Only emitted when the state changes because the user triggered the
     * action.
     */
    void activeChangedByUser(bool);

private:
    Q_PRIVATE_SLOT(d, void slotTriggered())
    KDualActionPrivate *const d;
    friend class KDualActionPrivate;
};

#endif /* KDUALACTION_H */
