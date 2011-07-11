/* This file is part of the KDE libraries
    Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>

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

#include "kkeysequencewidget.h"
#include "kkeysequencewidget_p.h"

#include "kkeyserver.h"

#include <QKeyEvent>
#include <QTimer>
#include <QtCore/QHash>
#include <QHBoxLayout>
#include <QToolButton>
#include <QApplication>

#include <kglobalaccel.h>
#include <kicon.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kshortcut.h>
#include <kaction.h>
#include <kactioncollection.h>

#include "kdebug.h"

class KKeySequenceWidgetPrivate
{
public:
    KKeySequenceWidgetPrivate(KKeySequenceWidget *q);

    void init();

    static QKeySequence appendToSequence(const QKeySequence& seq, int keyQt);
    static bool isOkWhenModifierless(int keyQt);

    void updateShortcutDisplay();
    void startRecording();

    /**
     * Conflicts the key sequence @a seq with a current standard
     * shortcut?
     */
    bool conflictWithStandardShortcuts(const QKeySequence &seq);

    /**
     * Conflicts the key sequence @a seq with a current local
     * shortcut?
     */
    bool conflictWithLocalShortcuts(const QKeySequence &seq);

    /**
     * Conflicts the key sequence @a seq with a current global
     * shortcut?
     */
    bool conflictWithGlobalShortcuts(const QKeySequence &seq);

    /**
     * Get permission to steal the shortcut @seq from the standard shortcut @a std.
     */
    bool stealStandardShortcut(KStandardShortcut::StandardShortcut std, const QKeySequence &seq);

    bool checkAgainstStandardShortcuts() const
    {
        return checkAgainstShortcutTypes & KKeySequenceWidget::StandardShortcuts;
    }

    bool checkAgainstGlobalShortcuts() const
    {
        return checkAgainstShortcutTypes & KKeySequenceWidget::GlobalShortcuts;
    }

    bool checkAgainstLocalShortcuts() const
    {
        return checkAgainstShortcutTypes & KKeySequenceWidget::LocalShortcuts;
    }

    void controlModifierlessTimout()
    {
        if (nKey != 0 && !modifierKeys) {
            // No modifier key pressed currently. Start the timout
            modifierlessTimeout.start(600);
        } else {
            // A modifier is pressed. Stop the timeout
            modifierlessTimeout.stop();
        }

    }


    void cancelRecording()
    {
        keySequence = oldKeySequence;
        doneRecording();
    }


    bool promptStealShortcutSystemwide(
            QWidget *parent,
            const QHash<QKeySequence, QList<KGlobalShortcutInfo> > &shortcuts,
            const QKeySequence &sequence)
    {
        if (shortcuts.isEmpty()) {
            // Usage error. Just say no
            return false;
        }

        QString clashingKeys = "";
        Q_FOREACH (const QKeySequence &seq, shortcuts.keys()) {
            Q_FOREACH (const KGlobalShortcutInfo &info, shortcuts[seq]) {
                clashingKeys += i18n("Shortcut '%1' in Application %2 for action %3\n",
                        seq.toString(),
                        info.componentFriendlyName(),
                        info.friendlyName());
            }
        }

        const int hashSize = shortcuts.size();

        QString message = i18ncp("%1 is the number of conflicts (hidden), %2 is the key sequence of the shortcut that is problematic",
                                 "The shortcut '%2' conflicts with the following key combination:\n",
                                 "The shortcut '%2' conflicts with the following key combinations:\n",
                                 hashSize, sequence.toString());
        message+=clashingKeys;

        QString title = i18ncp("%1 is the number of shortcuts with which there is a conflict",
                               "Conflict with Registered Global Shortcut", "Conflict with Registered Global Shortcuts", hashSize);

        return KMessageBox::warningContinueCancel(parent, message, title, KGuiItem(i18n("Reassign")))
               == KMessageBox::Continue;
    }


//private slot
    void doneRecording(bool validate = true);

//members
    KKeySequenceWidget *const q;
    QHBoxLayout *layout;
    KKeySequenceButton *keyButton;
    QToolButton *clearButton;

    QKeySequence keySequence;
    QKeySequence oldKeySequence;
    QTimer modifierlessTimeout;
    bool allowModifierless;
    uint nKey;
    uint modifierKeys;
    bool isRecording;
    bool multiKeyShortcutsAllowed;
    QString componentName;

    //! Check the key sequence against KStandardShortcut::find()
    KKeySequenceWidget::ShortcutTypes checkAgainstShortcutTypes;

    /**
     * The list of action to check against for conflict shortcut
     */
    QList<QAction*> checkList; // deprecated

    /**
     * The list of action collections to check against for conflict shortcut
     */
    QList<KActionCollection*> checkActionCollections;

    /**
     * The action to steal the shortcut from.
     */
    QList<KAction*> stealActions;

    bool stealShortcuts(const QList<KAction *> &actions, const QKeySequence &seq);
    void wontStealShortcut(QAction *item, const QKeySequence &seq);

};

KKeySequenceWidgetPrivate::KKeySequenceWidgetPrivate(KKeySequenceWidget *q)
    : q(q)
     ,layout(NULL)
     ,keyButton(NULL)
     ,clearButton(NULL)
     ,allowModifierless(false)
     ,nKey(0)
     ,modifierKeys(0)
     ,isRecording(false)
     ,multiKeyShortcutsAllowed(true)
     ,componentName()
     ,checkAgainstShortcutTypes(KKeySequenceWidget::LocalShortcuts & KKeySequenceWidget::GlobalShortcuts)
     ,stealActions()
{}


bool KKeySequenceWidgetPrivate::stealShortcuts(
        const QList<KAction *> &actions,
        const QKeySequence &seq)
{

    const int listSize = actions.size();

    QString title = i18ncp("%1 is the number of conflicts", "Shortcut Conflict", "Shortcut Conflicts", listSize);

    QString conflictingShortcuts;
    Q_FOREACH(const KAction *action, actions) {
        conflictingShortcuts += i18n("Shortcut '%1' for action '%2'\n",
                action->shortcut().toString(QKeySequence::NativeText),
                KGlobal::locale()->removeAcceleratorMarker(action->text()));
    }
    QString message = i18ncp("%1 is the number of ambigious shortcut clashes (hidden)",
            "The \"%2\" shortcut is ambiguous with the following shortcut.\n"
            "Do you want to assign an empty shortcut to this action?\n"
            "%3",
            "The \"%2\" shortcut is ambiguous with the following shortcuts.\n"
            "Do you want to assign an empty shortcut to these actions?\n"
            "%3",
            listSize,
            seq.toString(QKeySequence::NativeText),
            conflictingShortcuts);

    if (KMessageBox::warningContinueCancel(q, message, title, KGuiItem(i18n("Reassign"))) != KMessageBox::Continue)
        return false;

    return true;
}

void KKeySequenceWidgetPrivate::wontStealShortcut(QAction *item, const QKeySequence &seq)
{
    QString title( i18n( "Shortcut conflict" ) );
    QString msg( i18n( "<qt>The '%1' key combination is already used by the <b>%2</b> action.<br>"
            "Please select a different one.</qt>", seq.toString(QKeySequence::NativeText) ,
            KGlobal::locale()->removeAcceleratorMarker(item->text()) ) );
    KMessageBox::sorry( q, msg );
}


KKeySequenceWidget::KKeySequenceWidget(QWidget *parent)
 : QWidget(parent),
   d(new KKeySequenceWidgetPrivate(this))
{
    d->init();
    setFocusProxy( d->keyButton );
    connect(d->keyButton, SIGNAL(clicked()), this, SLOT(captureKeySequence()));
    connect(d->clearButton, SIGNAL(clicked()), this, SLOT(clearKeySequence()));
    connect(&d->modifierlessTimeout, SIGNAL(timeout()), this, SLOT(doneRecording()));
    //TODO: how to adopt style changes at runtime?
    /*QFont modFont = d->clearButton->font();
    modFont.setStyleHint(QFont::TypeWriter);
    d->clearButton->setFont(modFont);*/
    d->updateShortcutDisplay();
}


void KKeySequenceWidgetPrivate::init()
{
    layout = new QHBoxLayout(q);
    layout->setMargin(0);

    keyButton = new KKeySequenceButton(this, q);
    keyButton->setFocusPolicy(Qt::StrongFocus);
    keyButton->setIcon(KIcon("configure"));
    keyButton->setToolTip(i18n("Click on the button, then enter the shortcut like you would in the program.\nExample for Ctrl+a: hold the Ctrl key and press a."));
    layout->addWidget(keyButton);

    clearButton = new QToolButton(q);
    layout->addWidget(clearButton);

    if (qApp->isLeftToRight())
        clearButton->setIcon(KIcon("edit-clear-locationbar-rtl"));
    else
        clearButton->setIcon(KIcon("edit-clear-locationbar-ltr"));
}


KKeySequenceWidget::~KKeySequenceWidget ()
{
    delete d;
}


KKeySequenceWidget::ShortcutTypes KKeySequenceWidget::checkForConflictsAgainst() const
{
    return d->checkAgainstShortcutTypes;
}


void KKeySequenceWidget::setComponentName(const QString &componentName)
{
    d->componentName = componentName;
}

bool KKeySequenceWidget::multiKeyShortcutsAllowed() const
{
    return d->multiKeyShortcutsAllowed;
}


void KKeySequenceWidget::setMultiKeyShortcutsAllowed(bool allowed)
{
    d->multiKeyShortcutsAllowed = allowed;
}


void KKeySequenceWidget::setCheckForConflictsAgainst(ShortcutTypes types)
{
    d->checkAgainstShortcutTypes = types;
}

void KKeySequenceWidget::setModifierlessAllowed(bool allow)
{
    d->allowModifierless = allow;
}


bool KKeySequenceWidget::isKeySequenceAvailable(const QKeySequence &keySequence) const
{
    if (keySequence.isEmpty())
        return true;
    return ! ( d->conflictWithLocalShortcuts(keySequence)
               || d->conflictWithGlobalShortcuts(keySequence)
               || d->conflictWithStandardShortcuts(keySequence));
}


bool KKeySequenceWidget::isModifierlessAllowed()
{
    return d->allowModifierless;
}


void KKeySequenceWidget::setClearButtonShown(bool show)
{
    d->clearButton->setVisible(show);
}

#ifndef KDE_NO_DEPRECATED
void KKeySequenceWidget::setCheckActionList(const QList<QAction*> &checkList) // deprecated
{
    d->checkList = checkList;
    Q_ASSERT(d->checkActionCollections.isEmpty()); // don't call this method if you call setCheckActionCollections!
}
#endif

void KKeySequenceWidget::setCheckActionCollections(const QList<KActionCollection *>& actionCollections)
{
    d->checkActionCollections = actionCollections;
}

//slot
void KKeySequenceWidget::captureKeySequence()
{
    d->startRecording();
}


QKeySequence KKeySequenceWidget::keySequence() const
{
    return d->keySequence;
}


//slot
void KKeySequenceWidget::setKeySequence(const QKeySequence &seq, Validation validate)
{
    // oldKeySequence holds the key sequence before recording started, if setKeySequence()
    // is called while not recording then set oldKeySequence to the existing sequence so
    // that the keySequenceChanged() signal is emitted if the new and previous key
    // sequences are different
    if (!d->isRecording)
        d->oldKeySequence = d->keySequence;

    d->keySequence = seq;
    d->doneRecording(validate == Validate);
}


//slot
void KKeySequenceWidget::clearKeySequence()
{
    setKeySequence(QKeySequence());
}

//slot
void KKeySequenceWidget::applyStealShortcut()
{
    QSet<KActionCollection *> changedCollections;

    Q_FOREACH (KAction *stealAction, d->stealActions) {

        // Stealing a shortcut means setting it to an empty one.
        stealAction->setShortcut(KShortcut(), KAction::ActiveShortcut);

        // The following code will find the action we are about to
        // steal from and save it's actioncollection.
        KActionCollection* parentCollection = 0;
        foreach(KActionCollection* collection, d->checkActionCollections) {
            if (collection->actions().contains(stealAction)) {
                parentCollection = collection;
                break;
            }
        }

        // Remember the changed collection
        if (parentCollection) {
            changedCollections.insert(parentCollection);
        }
    }

    Q_FOREACH (KActionCollection *col, changedCollections) {
            col->writeSettings();
    }

    d->stealActions.clear();
}

void KKeySequenceButton::setText(const QString &text)
{
    QPushButton::setText(text);
    //setFixedSize( sizeHint().width()+12, sizeHint().height()+8 );
}


void KKeySequenceWidgetPrivate::startRecording()
{
    nKey = 0;
    modifierKeys = 0;
    oldKeySequence = keySequence;
    keySequence = QKeySequence();
    isRecording = true;
    keyButton->grabKeyboard();

    if (!QWidget::keyboardGrabber()) {
        kWarning() << "Failed to grab the keyboard! Most likely qt's nograb option is active";
    }

    keyButton->setDown(true);
    updateShortcutDisplay();
}


void KKeySequenceWidgetPrivate::doneRecording(bool validate)
{
    modifierlessTimeout.stop();
    isRecording = false;
    keyButton->releaseKeyboard();
    keyButton->setDown(false);
    stealActions.clear();

    if (keySequence==oldKeySequence) {
        // The sequence hasn't changed
        updateShortcutDisplay();
        return;
    }

    if (validate && !q->isKeySequenceAvailable(keySequence)) {
        // The sequence had conflicts and the user said no to stealing it
        keySequence = oldKeySequence;
    } else {
        emit q->keySequenceChanged(keySequence);
    }

    updateShortcutDisplay();
}


bool KKeySequenceWidgetPrivate::conflictWithGlobalShortcuts(const QKeySequence &keySequence)
{
#ifdef Q_WS_WIN
    //on windows F12 is reserved by the debugger at all times, so we can't use it for a global shortcut
    if (KKeySequenceWidget::GlobalShortcuts && keySequence.toString().contains("F12")) {
        QString title = i18n("Reserved Shortcut");
        QString message = i18n("The F12 key is reserved on Windows, so cannot be used for a global shortcut.\n"
                               "Please choose another one.");

        KMessageBox::sorry(q, message, title);
        return false;
    }
#endif

    if (!(checkAgainstShortcutTypes & KKeySequenceWidget::GlobalShortcuts)) {
        return false;
    }

    // Global shortcuts are on key+modifier shortcuts. They can clash with
    // each of the keys of a multi key shortcut.
    QHash<QKeySequence, QList<KGlobalShortcutInfo> > others;
    for (uint i=0; i<keySequence.count(); ++i) {
        QKeySequence tmp(keySequence[i]);

        if (!KGlobalAccel::isGlobalShortcutAvailable(tmp, componentName)) {
                others.insert(tmp, KGlobalAccel::getGlobalShortcutsByKey(tmp));
        }
    }

    if (!others.isEmpty()
            && !promptStealShortcutSystemwide(q, others, keySequence)) {
        return true;
    }

    // The user approved stealing the shortcut. We have to steal
    // it immediately because KAction::setGlobalShortcut() refuses
    // to set a global shortcut that is already used. There is no
    // error it just silently fails. So be nice because this is
    // most likely the first action that is done in the slot
    // listening to keySequenceChanged().
    for (uint i=0; i<keySequence.count(); ++i) {
        KGlobalAccel::stealShortcutSystemwide(keySequence[i]);
    }
    return false;
}


bool KKeySequenceWidgetPrivate::conflictWithLocalShortcuts(const QKeySequence &keySequence)
{
    if (!(checkAgainstShortcutTypes & KKeySequenceWidget::LocalShortcuts)) {
        return false;
    }

    // We have actions both in the deprecated checkList and the
    // checkActionCollections list. Add all the actions to a single list to
    // be able to process them in a single loop below.
    // Note that this can't be done in setCheckActionCollections(), because we
    // keep pointers to the action collections, and between the call to
    // setCheckActionCollections() and this function some actions might already be
    // removed from the collection again.
    QList<QAction*> allActions;
    allActions += checkList;
    foreach(KActionCollection* collection, checkActionCollections) {
        allActions += collection->actions();
    }

    // Because of multikey shortcuts we can have clashes with many shortcuts.
    //
    // Example 1:
    //
    // Application currently uses 'CTRL-X,a', 'CTRL-X,f' and 'CTRL-X,CTRL-F'
    // and the user wants to use 'CTRL-X'. 'CTRL-X' will only trigger as
    // 'activatedAmbiguously()' for obvious reasons.
    //
    // Example 2:
    //
    // Application currently uses 'CTRL-X'. User wants to use 'CTRL-X,CTRL-F'.
    // This will shadow 'CTRL-X' for the same reason as above.
    //
    // Example 3:
    //
    // Some weird combination of Example 1 and 2 with three shortcuts using
    // 1/2/3 key shortcuts. I think you can imagine.
    QList<KAction*> conflictingActions;

    //find conflicting shortcuts with existing actions
    foreach(QAction * qaction , allActions ) {
        KAction *kaction=qobject_cast<KAction*>(qaction);
        if(kaction) {
            if(kaction->shortcut().conflictsWith(keySequence)) {
                // A conflict with a KAction. If that action is configurable
                // ask the user what to do. If not reject this keySequence.
                if(kaction->isShortcutConfigurable ()) {
                    conflictingActions.append(kaction);
                } else {
                    wontStealShortcut(kaction, keySequence);
                    return true;
                }
            }
        } else {
            if(qaction->shortcut() == keySequence) {
                // A conflict with a QAction. Don't know why :-( but we won't
                // steal from that kind of actions.
                wontStealShortcut(qaction, keySequence);
                return true;
            }
        }
    }

    if (conflictingActions.isEmpty()) {
        // No conflicting shortcuts found.
        return false;
    }

    if(stealShortcuts(conflictingActions, keySequence)) {
        stealActions = conflictingActions;
        // Announce that the user
        // agreed
        Q_FOREACH (KAction *stealAction, stealActions) {
            emit q->stealShortcut(
                keySequence,
                stealAction);
        }
        return false;
    } else {
        return true;
    }
}


bool KKeySequenceWidgetPrivate::conflictWithStandardShortcuts(const QKeySequence &keySequence)
{
    if (!(checkAgainstShortcutTypes & KKeySequenceWidget::StandardShortcuts)) {
        return false;
    }

    KStandardShortcut::StandardShortcut ssc = KStandardShortcut::find(keySequence);
    if (ssc != KStandardShortcut::AccelNone && !stealStandardShortcut(ssc, keySequence)) {
        return true;
    }
    return false;
}


bool KKeySequenceWidgetPrivate::stealStandardShortcut(KStandardShortcut::StandardShortcut std, const QKeySequence &seq)
{
    QString title = i18n("Conflict with Standard Application Shortcut");
    QString message = i18n("The '%1' key combination is also used for the standard action "
                           "\"%2\" that some applications use.\n"
                           "Do you really want to use it as a global shortcut as well?",
                           seq.toString(QKeySequence::NativeText), KStandardShortcut::label(std));

    if (KMessageBox::warningContinueCancel(q, message, title, KGuiItem(i18n("Reassign"))) != KMessageBox::Continue) {
        return false;
    }
    return true;
}


void KKeySequenceWidgetPrivate::updateShortcutDisplay()
{
    //empty string if no non-modifier was pressed
    QString s = keySequence.toString(QKeySequence::NativeText);
    s.replace('&', QLatin1String("&&"));

    if (isRecording) {
        if (modifierKeys) {
            if (!s.isEmpty()) s.append(",");
            if (modifierKeys & Qt::META)  s += KKeyServer::modToStringUser(Qt::META) + '+';
#if defined(Q_WS_MAC)
            if (modifierKeys & Qt::ALT)   s += KKeyServer::modToStringUser(Qt::ALT) + '+';
            if (modifierKeys & Qt::CTRL)  s += KKeyServer::modToStringUser(Qt::CTRL) + '+';
#elif defined(Q_WS_X11)
            if (modifierKeys & Qt::CTRL)  s += KKeyServer::modToStringUser(Qt::CTRL) + '+';
            if (modifierKeys & Qt::ALT)   s += KKeyServer::modToStringUser(Qt::ALT) + '+';
#endif
            if (modifierKeys & Qt::SHIFT) s += KKeyServer::modToStringUser(Qt::SHIFT) + '+';

        } else if (nKey == 0) {
            s = i18nc("What the user inputs now will be taken as the new shortcut", "Input");
        }
        //make it clear that input is still going on
        s.append(" ...");
    }

    if (s.isEmpty()) {
        s = i18nc("No shortcut defined", "None");
    }

    s.prepend(' ');
    s.append(' ');
    keyButton->setText(s);
}


KKeySequenceButton::~KKeySequenceButton()
{
}


//prevent Qt from special casing Tab and Backtab
bool KKeySequenceButton::event (QEvent* e)
{
    if (d->isRecording && e->type() == QEvent::KeyPress) {
        keyPressEvent(static_cast<QKeyEvent *>(e));
        return true;
    }

    // The shortcut 'alt+c' ( or any other dialog local action shortcut )
    // ended the recording and triggered the action associated with the
    // action. In case of 'alt+c' ending the dialog.  It seems that those
    // ShortcutOverride events get sent even if grabKeyboard() is active.
    if (d->isRecording && e->type() == QEvent::ShortcutOverride) {
        e->accept();
        return true;
    }

    return QPushButton::event(e);
}


void KKeySequenceButton::keyPressEvent(QKeyEvent *e)
{
    int keyQt = e->key();
    if (keyQt == -1) {
        // Qt sometimes returns garbage keycodes, I observed -1, if it doesn't know a key.
        // We cannot do anything useful with those (several keys have -1, indistinguishable)
        // and QKeySequence.toString() will also yield a garbage string.
        KMessageBox::sorry(this,
                i18n("The key you just pressed is not supported by Qt."),
                i18n("Unsupported Key"));
        return d->cancelRecording();
    }

    uint newModifiers = e->modifiers() & (Qt::SHIFT | Qt::CTRL | Qt::ALT | Qt::META);

    //don't have the return or space key appear as first key of the sequence when they
    //were pressed to start editing - catch and them and imitate their effect
    if (!d->isRecording && ((keyQt == Qt::Key_Return || keyQt == Qt::Key_Space))) {
        d->startRecording();
        d->modifierKeys = newModifiers;
        d->updateShortcutDisplay();
        return;
    }

    // We get events even if recording isn't active.
    if (!d->isRecording)
        return QPushButton::keyPressEvent(e);

    e->accept();
    d->modifierKeys = newModifiers;


    switch(keyQt) {
    case Qt::Key_AltGr: //or else we get unicode salad
        return;
    case Qt::Key_Shift:
    case Qt::Key_Control:
    case Qt::Key_Alt:
    case Qt::Key_Meta:
    case Qt::Key_Menu: //unused (yes, but why?)
        d->controlModifierlessTimout();
        d->updateShortcutDisplay();
        break;
    default:

        if (d->nKey == 0 && !(d->modifierKeys & ~Qt::SHIFT)) {
            // It's the first key and no modifier pressed. Check if this is
            // allowed
            if (!(KKeySequenceWidgetPrivate::isOkWhenModifierless(keyQt)
                            || d->allowModifierless)) {
                // No it's not
                return;
            }
        }

        // We now have a valid key press.
        if (keyQt) {
            if ((keyQt == Qt::Key_Backtab) && (d->modifierKeys & Qt::SHIFT)) {
                keyQt = Qt::Key_Tab | d->modifierKeys;
            }
            else if (KKeyServer::isShiftAsModifierAllowed(keyQt)) {
                keyQt |= d->modifierKeys;
            }
            else
                keyQt |= (d->modifierKeys & ~Qt::SHIFT);

            if (d->nKey == 0) {
                d->keySequence = QKeySequence(keyQt);
            } else {
                d->keySequence =
                  KKeySequenceWidgetPrivate::appendToSequence(d->keySequence, keyQt);
            }

            d->nKey++;
            if ((!d->multiKeyShortcutsAllowed) || (d->nKey >= 4)) {
                d->doneRecording();
                return;
            }
            d->controlModifierlessTimout();
            d->updateShortcutDisplay();
        }
    }
}


void KKeySequenceButton::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == -1) {
        // ignore garbage, see keyPressEvent()
        return;
    }

    if (!d->isRecording)
        return QPushButton::keyReleaseEvent(e);

    e->accept();

    uint newModifiers = e->modifiers() & (Qt::SHIFT | Qt::CTRL | Qt::ALT | Qt::META);

    //if a modifier that belongs to the shortcut was released...
    if ((newModifiers & d->modifierKeys) < d->modifierKeys) {
        d->modifierKeys = newModifiers;
        d->controlModifierlessTimout();
        d->updateShortcutDisplay();
    }
}


//static
QKeySequence KKeySequenceWidgetPrivate::appendToSequence(const QKeySequence& seq, int keyQt)
{
    switch (seq.count()) {
    case 0:
        return QKeySequence(keyQt);
    case 1:
        return QKeySequence(seq[0], keyQt);
    case 2:
        return QKeySequence(seq[0], seq[1], keyQt);
    case 3:
        return QKeySequence(seq[0], seq[1], seq[2], keyQt);
    default:
        return seq;
    }
}


//static
bool KKeySequenceWidgetPrivate::isOkWhenModifierless(int keyQt)
{
    //this whole function is a hack, but especially the first line of code
    if (QKeySequence(keyQt).toString().length() == 1)
        return false;

    switch (keyQt) {
    case Qt::Key_Return:
    case Qt::Key_Space:
    case Qt::Key_Tab:
    case Qt::Key_Backtab: //does this ever happen?
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
        return false;
    default:
        return true;
    }
}

#include "kkeysequencewidget.moc"
#include "kkeysequencewidget_p.moc"
