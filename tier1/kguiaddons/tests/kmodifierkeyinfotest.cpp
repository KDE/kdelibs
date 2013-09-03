/*
    Copyright 2009  Michael Leupold <lemma@confuego.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QWidget>
#include <QPushButton>
#include <QDebug>
#include <QCheckBox>

#include "kmodifierkeyinfo.h"

template <typename A, typename B, typename C>
class Triple
{
public:
    Triple() {}
    Triple(const A _first, const B _second, const C _third)
        : first(_first), second(_second), third(_third) {};
    A first;
    B second;
    C third;
};

class TestWidget : public QWidget
{
    Q_OBJECT

public:
    TestWidget();

protected Q_SLOTS:
    void keyPressed(Qt::Key key, bool state);
    void keyLatched(Qt::Key key, bool state);
    void keyLocked(Qt::Key key, bool state);
    void mouseButtonPressed(Qt::MouseButton button, bool state);
    void latch();
    void lock();
    void keyAdded(Qt::Key) {
        qDebug() << "Key added";
    }
    void keyRemoved(Qt::Key) {
        qDebug() << "Key removed";
    }

private:
    KModifierKeyInfo m_lock;
    QMap<Qt::Key, Triple<QCheckBox*,QCheckBox*,QCheckBox*> > m_leds;
    QMap<Qt::MouseButton, QCheckBox*> m_mouseLeds;
};

TestWidget::TestWidget() : QWidget(0), m_lock(this)
{
    QMap<Qt::Key, QString> mods;
    mods.insert(Qt::Key_Shift, "Shift");
    mods.insert(Qt::Key_Control, "Ctrl");
    mods.insert(Qt::Key_Alt, "Alt");
    mods.insert(Qt::Key_Meta, "Meta");
    mods.insert(Qt::Key_Super_L, "Super");
    mods.insert(Qt::Key_Hyper_L, "Hyper");
    mods.insert(Qt::Key_AltGr, "AltGr");
    mods.insert(Qt::Key_NumLock, "NumLock");
    mods.insert(Qt::Key_CapsLock, "CapsLock");
    mods.insert(Qt::Key_ScrollLock, "ScrollLock");

    QMap<Qt::MouseButton, QString> buttons;
    buttons.insert(Qt::LeftButton, "Left Button");
    buttons.insert(Qt::RightButton, "Right Button");
    buttons.insert(Qt::MidButton, "Middle Button");
    buttons.insert(Qt::XButton1, "First X Button");
    buttons.insert(Qt::XButton2, "Second X Button");

    QVBoxLayout *layout = new QVBoxLayout(this);

    QMap<Qt::Key, QString>::const_iterator it;
    QMap<Qt::Key, QString>::const_iterator end = mods.constEnd();
    for (it = mods.constBegin(); it != end; ++it) {
        if (m_lock.knowsKey(it.key())) {
            QHBoxLayout *hlayout = new QHBoxLayout;
            QCheckBox *pressed = new QCheckBox(this);
            QCheckBox *latched = new QCheckBox(this);
            QCheckBox *locked = new QCheckBox(this);
            QPushButton *latch = new QPushButton("latch", this);
            latch->setProperty("modifier", it.key());
            connect(latch, SIGNAL(clicked()), SLOT(latch()));
            QPushButton *lock = new QPushButton("lock", this);
            lock->setProperty("modifier", it.key());
            connect(lock, SIGNAL(clicked()), SLOT(lock()));
            pressed->setChecked(m_lock.isKeyPressed(it.key()));
            latched->setChecked(m_lock.isKeyLatched(it.key()));
            locked->setChecked(m_lock.isKeyLocked(it.key()));
            m_leds.insert(it.key(), Triple<QCheckBox*,QCheckBox*,QCheckBox*>(pressed, latched, locked));
            hlayout->addWidget(pressed);
            hlayout->addWidget(latched);
            hlayout->addWidget(locked);
            hlayout->addWidget(new QLabel(it.value()));
            hlayout->addWidget(latch);
            hlayout->addWidget(lock);
            layout->addLayout(hlayout);
        }
    }

    QMap<Qt::MouseButton, QString>::const_iterator it2;
    QMap<Qt::MouseButton, QString>::const_iterator end2 = buttons.constEnd();
    for (it2 = buttons.constBegin(); it2 != end2; ++it2) {
        QHBoxLayout *hlayout = new QHBoxLayout;
        QCheckBox *pressed = new QCheckBox(this);
        pressed->setChecked(m_lock.isButtonPressed(it2.key()));
        m_mouseLeds.insert(it2.key(), pressed);
        hlayout->addWidget(pressed);
        hlayout->addWidget(new QLabel(it2.value()));
        layout->addLayout(hlayout);
    }

    setLayout(layout);

    connect(&m_lock, SIGNAL(keyPressed(Qt::Key,bool)), SLOT(keyPressed(Qt::Key,bool)));
    connect(&m_lock, SIGNAL(keyLatched(Qt::Key,bool)), SLOT(keyLatched(Qt::Key,bool)));
    connect(&m_lock, SIGNAL(keyLocked(Qt::Key,bool)), SLOT(keyLocked(Qt::Key,bool)));
    connect(&m_lock, SIGNAL(buttonPressed(Qt::MouseButton,bool)),
                     SLOT(mouseButtonPressed(Qt::MouseButton,bool)));
    connect(&m_lock, SIGNAL(keyAdded(Qt::Key)), SLOT(keyAdded(Qt::Key)));
    connect(&m_lock, SIGNAL(keyRemoved(Qt::Key)), SLOT(keyRemoved(Qt::Key)));
}

void TestWidget::keyPressed(Qt::Key key, bool pressed)
{
    if (m_leds.contains(key)) {
        m_leds[key].first->setChecked(pressed);
    }
}

void TestWidget::keyLatched(Qt::Key key, bool latched)
{
    if (m_leds.contains(key)) {
        m_leds[key].second->setChecked(latched);
    }
}

void TestWidget::keyLocked(Qt::Key key, bool locked)
{
    if (m_leds.contains(key)) {
        m_leds[key].third->setChecked(locked);
    }
}

void TestWidget::mouseButtonPressed(Qt::MouseButton button, bool pressed)
{
    if (m_mouseLeds.contains(button)) {
        m_mouseLeds[button]->setChecked(pressed);
    }
}

void TestWidget::latch()
{
    Qt::Key key = (Qt::Key)sender()->property("modifier").toInt();
    m_lock.setKeyLatched(key, !m_lock.isKeyLatched(key));
}

void TestWidget::lock()
{
    Qt::Key key = (Qt::Key)sender()->property("modifier").toInt();
    m_lock.setKeyLocked(key, !m_lock.isKeyLocked(key));
}

int main(int argc, char *argv[])
{
    QApplication::setApplicationName("simple");

    QApplication app(argc, argv);
    TestWidget mainWidget;
    mainWidget.show();

    return app.exec();
}

#include "kmodifierkeyinfotest.moc"
