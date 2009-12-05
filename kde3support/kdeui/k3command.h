/* This file is part of the KDE project
   Copyright (C) 2000 Werner Trobin <trobin@kde.org>
   Copyright (C) 2000,2006 David Faure <faure@kde.org>

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

#ifndef kcommand_h
#define kcommand_h

#include <kde3support_export.h>

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QObject>

class KAction;
class KActionCollection;
class QAction;

/**
 * The abstract base class for all Commands. Commands are used to
 * store information needed for Undo/Redo functionality...
 *
 * Note: you might want to use the QUndo framework instead.
 * It didn't exist when K3Command was written.
 */
class KDE3SUPPORT_EXPORT K3Command
{
protected:
    /**
     * Creates a command.
     */
    K3Command();

public:
    virtual ~K3Command();

    /**
     * The main method: executes this command.
     * Implement here what this command is about, and remember to
     * record any information that will be helpful for #unexecute.
     */
    virtual void execute() = 0;
    /**
     * Unexecutes (undo) this command.
     * Implement here the steps to take for undoing the command.
     * If your application uses actions for everything (it should),
     * and if you implement unexecute correctly, the application is in the same
     * state after unexecute as it was before execute. This means, the next
     * call to execute will do the same thing as it did the first time.
     */
    virtual void unexecute() = 0;

    /**
     * @return the name of this command, translated, since it will appear
     * in the menus.
     */
    virtual QString name() const = 0;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class Private;
    Private* const d;
    Q_DISABLE_COPY( K3Command )
};

/**
 * A command which stores its name.
 * It is more memory-efficient to use K3Command and to implement the name() method,
 * but in some cases it's more simple or more flexible to store the name at creation time.
 *
 * Note: you might want to use the QUndo framework instead.
 * It didn't exist when K3Command was written.
 */
class KDE3SUPPORT_EXPORT K3NamedCommand : public K3Command
{
protected:
    /**
     * Creates a command.
     * @param name the name of this command, translated, since it will appear
     * in the menus.
     */
    K3NamedCommand( const QString &name );

public:
    /**
     * @return the name of this command
     */
    virtual QString name() const;
    /**
     * Updates the name of this command.
     * Rarely necessary.
     */
    void setName( const QString &name );

    virtual ~K3NamedCommand();

protected:
    virtual void virtual_hook( int id, void* data );

private:
    class Private;
    Private* const d;
    Q_DISABLE_COPY( K3NamedCommand )
};

/**
 * A Macro Command is a command that holds several sub-commands.
 * It will appear as one to the user and in the command history,
 * but it can use the implementation of multiple commands internally.
 */
class KDE3SUPPORT_EXPORT K3MacroCommand : public K3NamedCommand
{
public:
    /**
     * Creates a macro command. You will then need to call addCommand
     * for each subcommand to be added to this macro command.
     * @param name the name of this command, translated, since it will appear
     * in the menus.
     */
    K3MacroCommand( const QString & name );
    virtual ~K3MacroCommand();

    /**
     * Appends a command to this macro command.
     * The ownership is transferred to the macro command.
     */
    void addCommand(K3Command *command);

    /**
     * Executes this command, i.e. execute all the sub-commands
     * in the order in which they were added.
     */
    virtual void execute();
    /**
     * Undoes the execution of this command, i.e. #unexecute all the sub-commands
     * in the _reverse_ order to the one in which they were added.
     */
    virtual void unexecute();

protected:
    const QList<K3Command *> commands() const;

    virtual void virtual_hook( int id, void* data );

private:
    class Private;
    Private* const d;
    Q_DISABLE_COPY( K3MacroCommand )
};


/**
 * The command history stores a (user) configurable amount of
 * Commands. It keeps track of its size and deletes commands
 * if it gets too large. The user can set a maximum undo and
 * a maximum redo limit (e.g. max. 50 undo / 30 redo commands).
 * The K3CommandHistory keeps track of the "borders" and deletes
 * commands, if appropriate. It also activates/deactivates the
 * undo/redo actions in the menu and changes the text according
 * to the name of the command.
 *
 * Note: you might want to use the QUndo framework instead.
 * It didn't exist when K3Command was written.
 */
class KDE3SUPPORT_EXPORT K3CommandHistory : public QObject {
    Q_OBJECT
public:
    /**
     * Creates a command history, to store commands.
     * This constructor doesn't create actions, so you need to call
     * #undo and #redo yourself.
     */
    K3CommandHistory();

    /**
     * Creates a command history, to store commands.
     * This also creates an undo and a redo action, in the @p actionCollection,
     * using the standard names ("edit_undo" and "edit_redo").
     * @param withMenus if true, the actions will display a menu when plugged
     * into a toolbar.
     * @param actionCollection the parent collection
     */
    K3CommandHistory(KActionCollection *actionCollection, bool withMenus = true);

    /**
     * Destructs the command history object.
     */
    virtual ~K3CommandHistory();

    /**
     * Erases all the undo/redo history.
     * Use this when reloading the data, for instance, since this invalidates
     * all the commands.
     */
    void clear();

    /**
     * Adds a command to the history. Call this for each @p command you create.
     * Unless you set @p execute to false, this will also execute the command.
     * This means, most of the application's code will look like
     *    MyCommand * cmd = new MyCommand( parameters );
     *    m_historyCommand.addCommand( cmd );
     *
     * Note that the command history takes ownership of the command, it will delete
     * it when the undo limit is reached, or when deleting the command history itself.
     */
    void addCommand(K3Command *command, bool execute=true);

    /**
     * @return the maximum number of items in the undo history
     */
    int undoLimit() const;
    /**
     * Sets the maximum number of items in the undo history.
     */
    void setUndoLimit(int limit);
    /**
     * @return the maximum number of items in the redo history
     */
    int redoLimit() const;
    /**
     * Sets the maximum number of items in the redo history.
     */
    void setRedoLimit(int limit);

    /**
     * Enable or disable the undo and redo actions.
     * This isn't usually necessary, but this method can be useful if
     * you disable all actions (to go to a "readonly" state), and then
     * want to come back to a readwrite mode.
     */
    void updateActions();

    /**
     * @return the present command, i.e. the one that undo() would unexecute.
     * This can be used to e.g. show selection.
     */
    K3Command * presentCommand() const;

    /**
     * @return true if undo is available,
     * i.e. there is at least one command that can be undone right now
     */
    bool isUndoAvailable() const;

    /**
     * @return true if redo is available
     * i.e. there is at least one command that can be redone right now
     */
    bool isRedoAvailable() const;

    /**
     * @return the list of next @p maxCommands actions that will be undone by undo()
     * The returned list is empty if !isUndoAvailable().
     * Otherwise the list starts with the next command to undo,
     * i.e. the order of the commands in the list is the reverse of the
     * chronological order of the commands.
     * @param maxCommands maximum number of commands requested. 0 means no maximum,
     * all stored undo commands (within undoLimit()) are returned.
     */
    QList<K3Command *> undoCommands( int maxCommands = 0 ) const;

    /**
     * @return the list of next @p maxCommands actions that will be redone by redo()
     * The returned list is empty if !isRedoAvailable().
     * Otherwise the list starts with the next command to redo.
     * @param maxCommands maximum number of commands requested. 0 means no maximum,
     * all stored redo commands (within redoLimit()) are returned.
     */
    QList<K3Command *> redoCommands( int maxCommands = 0 ) const;

public Q_SLOTS:
    /**
     * Undoes the last action.
     * Call this if you don't use the builtin KActions.
     */
    virtual void undo();
    /**
     * Redoes the last undone action.
     * Call this if you don't use the builtin KActions.
     */
    virtual void redo();
    /**
     * Remembers when you saved the document.
     * Call this right after saving the document. As soon as
     * the history reaches the current index again (via some
     * undo/redo operations) it will emit documentRestored
     * If you implemented undo/redo properly the document is
     * the same you saved before.
     */
    virtual void documentSaved();

Q_SIGNALS:
    /**
     * Emitted every time a command is executed
     * (whether by addCommand, undo or redo).
     * You can use this to update the GUI, for instance.
     * @param command was executed
     */
    void commandExecuted(K3Command *command);

    /**
     * Emitted every time we reach the index where you
     * saved the document for the last time. See documentSaved
     */
    void documentRestored();

    /**
     * Emitted whenever the command history has changed,
     * i.e. after addCommand, undo or redo.
     * This is used by the actions to update themselves.
     */
    void commandHistoryChanged();

private:
    void clipCommands();  // ensures that the limits are kept

private:
    class K3CommandHistoryPrivate;
    K3CommandHistoryPrivate * const d;
    Q_DISABLE_COPY( K3CommandHistory )
};

#include <ktoolbarpopupaction.h>

/**
 * This type of action is used to show undo or redo actions in the menu or in the
 * toolbars.
 * This action will keep itself up to date and change the text based on the undo
 * history, plus it will disable itself when there is nothing to undo/redo.
 * You will typically need two instances of this action per view (e.g. the mainwindow).
 * @code
    new KUndoRedoAction( KUndoRedoAction::Undo, view->actionCollection(), m_history );
    new KUndoRedoAction( KUndoRedoAction::Redo, view->actionCollection(), m_history );
   @endcode
 * Note that there is no need to connect or even keep a reference to the object as
 * all work is done automatically.
 *
 * Note: you might want to use the QUndo framework instead.
 * It didn't exist when K3Command was written.
 */
class K3UndoRedoAction : public KToolBarPopupAction
{
    Q_OBJECT
public:
    enum Type { Undo, Redo };
    K3UndoRedoAction( Type type, KActionCollection* actionCollection, K3CommandHistory* commandHistory );

private Q_SLOTS:
    void slotAboutToShow();
    void slotActionTriggered( QAction *action );
    void slotCommandHistoryChanged();

private:
    class Private;
    Private* const d;
    Q_DISABLE_COPY( K3UndoRedoAction )
};

#endif
