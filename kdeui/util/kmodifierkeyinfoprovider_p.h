/*
    Copyright 2009  Michael Leupold <lemma@confuego.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KMODIFIERKEYINFOPROVIDER_P_H
#define KMODIFIERKEYINFOPROVIDER_P_H

#include <QHash>
#include <QWidget>
#include <QSet>

#if defined Q_WS_X11
    #include <X11/Xlib.h>
    #include <X11/XKBlib.h>
    #include <fixx11h.h>
    #include <QtCore/QAbstractEventDispatcher>
    bool kmodifierKeyInfoEventFilter(void *message);
#endif

/**
 * Background class that implements the behaviour of KModifierKeyInfo for
 * the different supported platforms.
 * @internal
 */
class KModifierKeyInfoProvider : public QWidget
{
    Q_OBJECT

public:
    enum ModifierState {
        Nothing = 0x0,
        Pressed = 0x1,
        Latched = 0x2,
        Locked = 0x4
    };
    Q_DECLARE_FLAGS(ModifierStates, ModifierState)

    KModifierKeyInfoProvider();
    ~KModifierKeyInfoProvider();

    /**
     * Detect if a key is pressed.
     * @param key Modifier key to query
     * @return true if the key is pressed, false if it isn't.
     */
    bool isKeyPressed(Qt::Key key) const;
    
    /**
     * Detect if a key is latched.
     * @param key Modifier key to query
     * @return true if the key is latched, false if it isn't.
     */
    bool isKeyLatched(Qt::Key key) const;

    /**
     * Set the latched state of a key.
     * @param key Modifier to set the latched state for
     * @param latched true to latch the key, false to unlatch it
     * @return true if the key is known, false else
     */
    bool setKeyLatched(Qt::Key key, bool latched);
    
    /**
     * Detect if a key is locked.
     * @param key Modifier key to query
     * @return true if the key is locked, false if it isn't.
     */
    bool isKeyLocked(Qt::Key key) const;

    /**
     * Set the locked state of a key.
     * @param key Modifier to set the locked state for
     * @param latched true to lock the key, false to unlock it
     * @return true if the key is known, false else
     */
    bool setKeyLocked(Qt::Key key, bool locked);

    /**
     * Check if a mouse button is pressed.
     * @param button Mouse button to check
     * @return true if pressed, false else
     */
    bool isButtonPressed(Qt::MouseButton button) const;
    
    /**
     * Check if a key is known/can be queried
     * @param key Modifier key to check
     * @return true if the key is known, false if it isn't.
     */
    bool knowsKey(Qt::Key key) const;

    /**
     * Get a list of known keys
     * @return List of known keys.
     */
    const QList<Qt::Key> knownKeys() const;

Q_SIGNALS:
    void keyLatched(Qt::Key key, bool state);
    void keyLocked(Qt::Key key, bool state);
    void keyPressed(Qt::Key key, bool state);
    void buttonPressed(Qt::MouseButton button, bool state);
    void keyAdded(Qt::Key key);
    void keyRemoved(Qt::Key key);
    
protected:
#if defined Q_WS_X11
    // Interfacing with QAbstractEventDispatcher
    friend bool kmodifierKeyInfoEventFilter(void *message);
    static QAbstractEventDispatcher::EventFilter s_nextFilter;
    static QSet<KModifierKeyInfoProvider*> s_providerList;
    
    // event handler for incoming X11 events
    virtual bool x11Event(XEvent *event);
    void xkbUpdateModifierMapping();
    void xkbModifierStateChanged(unsigned char mods, unsigned char latched_mods,
                                 unsigned char locked_mods);
    void xkbButtonStateChanged(unsigned short ptr_buttons);
#endif

private:
    // the state of each known modifier
    QHash<Qt::Key, ModifierStates> m_modifierStates;
    // the state of each known mouse button
    QHash<Qt::MouseButton, bool> m_buttonStates;

#if defined Q_WS_X11
    int m_xkbEv;
    bool m_xkbAvailable;

    // maps a Qt::Key to a modifier mask
    QHash<Qt::Key, unsigned int> m_xkbModifiers;
    // maps a Qt::MouseButton to a button mask
    QHash<Qt::MouseButton, unsigned short> m_xkbButtons;

    // has the eventfilter been installed yet?
    static bool s_eventFilterInstalled;
    // is the eventfilter currently enabled?
    static bool s_eventFilterEnabled;
#endif
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KModifierKeyInfoProvider::ModifierStates)

#endif
