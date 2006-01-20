#include "kcommandtest.h"
#include "kcommandtest.moc"

#include "qtest_kde.h"

#include <kactioncollection.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>

QTEST_KDEMAIN(KCommandTest, GUI)

class KTestCommand : public KNamedCommand
{
public:
    KTestCommand( const QString& name ) : KNamedCommand( name ) {}
    ~KTestCommand() {
        deletedCommands.append( name() );
    }
    virtual void execute() {
        executedCommands.append( name() );
    }
    virtual void unexecute() {
        unexecutedCommands.append( name() );
    }
    static QStringList executedCommands;
    static QStringList unexecutedCommands;
    static QStringList deletedCommands;
    static void clearLists() {
        executedCommands.clear();
        unexecutedCommands.clear();
        deletedCommands.clear();
    }
};

QStringList KTestCommand::executedCommands;
QStringList KTestCommand::unexecutedCommands;
QStringList KTestCommand::deletedCommands;

void KCommandTest::testMacroCommand()
{
    {
        KMacroCommand macroCommand( "foo" );
        macroCommand.execute(); // no-op
        macroCommand.unexecute(); // no-op
        KTestCommand* c1 = new KTestCommand( "1" );
        macroCommand.addCommand( c1 );
        KTestCommand* c2 = new KTestCommand( "2" );
        macroCommand.addCommand( c2 );
        macroCommand.execute();
        QCOMPARE( KTestCommand::executedCommands.join( "," ), QString( "1,2" ) );
        macroCommand.unexecute();
        QCOMPARE( KTestCommand::unexecutedCommands.join( "," ), QString( "2,1" ) ); // reverse order
    } // end of scope for KMacroCommand
    QCOMPARE( KTestCommand::deletedCommands.join( "," ), QString( "1,2" ) ); // ok the order doesn't matter ;)

    KTestCommand::clearLists();
}

void KCommandTest::testCommandHistoryAdd()
{
    KTestCommand::clearLists();
    m_commandsExecuted = 0;
    m_documentRestored = 0;
    KActionCollection actionCollection( ( QWidget*)0 );
    {
        KCommandHistory ch( &actionCollection );
        connect( &ch, SIGNAL( commandExecuted( KCommand* ) ), this, SLOT( slotCommandExecuted( KCommand* ) ) );
        connect( &ch, SIGNAL( documentRestored() ), this, SLOT( slotDocumentRestored() ) );

        KAction* undo = actionCollection.action( "edit_undo" );
        QVERIFY( undo );
        QVERIFY( !undo->isEnabled() );
        QCOMPARE( undo->text(), i18n( "&Undo" ) );
        KAction* redo = actionCollection.action( "edit_redo" );
        QVERIFY( redo );
        QVERIFY( !redo->isEnabled() );
        QCOMPARE( redo->text(), i18n( "&Redo" ) );
        ch.updateActions();
        QVERIFY( !undo->isEnabled() );
        QVERIFY( !redo->isEnabled() );

        KTestCommand* c1 = new KTestCommand( "1" );
        ch.addCommand( c1 ); // executes the action
        QCOMPARE( KTestCommand::executedCommands.join( "," ), QString( "1" ) );
        QVERIFY( undo->isEnabled() );
        QCOMPARE( undo->text(), i18n( "&Undo: %1" ).arg(1) );
        QVERIFY( !redo->isEnabled() );
        ch.updateActions();
        QVERIFY( undo->isEnabled() );
        QVERIFY( !redo->isEnabled() );
        QCOMPARE( m_documentRestored, 0 );
        ch.undo();
        QCOMPARE( m_documentRestored, 1 );
        QCOMPARE( KTestCommand::unexecutedCommands.join( "," ), QString( "1" ) );
        QVERIFY( !undo->isEnabled() );
        QCOMPARE( undo->text(), i18n( "&Undo" ) );
        QVERIFY( redo->isEnabled() );
        ch.redo();
        QCOMPARE( KTestCommand::executedCommands.join( "," ), QString( "1,1" ) );
        QVERIFY( undo->isEnabled() );
        QVERIFY( !redo->isEnabled() );
        KTestCommand::clearLists();

        KTestCommand* c2 = new KTestCommand( "2" );
        ch.addCommand( c2 );
        QVERIFY( undo->isEnabled() );
        QCOMPARE( undo->text(), i18n( "&Undo: %1" ).arg(2) );
        QVERIFY( !redo->isEnabled() );
        ch.undo();
        QVERIFY( undo->isEnabled() );
        QVERIFY( redo->isEnabled() );
        QCOMPARE( redo->text(), i18n( "&Redo: %1" ).arg(2) );
        ch.updateActions();
        QVERIFY( undo->isEnabled() );
        QVERIFY( redo->isEnabled() );
        QCOMPARE( KTestCommand::unexecutedCommands.join( "," ), QString( "2" ) );
        ch.undo();
        QCOMPARE( m_documentRestored, 2 );
        QVERIFY( !undo->isEnabled() );
        QVERIFY( redo->isEnabled() );
        ch.updateActions();
        QVERIFY( !undo->isEnabled() );
        QVERIFY( redo->isEnabled() );
        QCOMPARE( redo->text(), i18n( "&Redo: %1" ).arg(1) );
        QCOMPARE( KTestCommand::unexecutedCommands.join( "," ), QString( "2,1" ) );
        ch.redo();
        QCOMPARE( KTestCommand::executedCommands.join( "," ), QString( "2,1" ) );
        KTestCommand::clearLists();

        // c3, check that it truncates history, i.e. the history is now 1,3
        KTestCommand* c3 = new KTestCommand( "3" );
        ch.addCommand( c3 );
        // c2 got deleted
        QCOMPARE( KTestCommand::deletedCommands.join( "," ), QString( "2" ) );
        ch.undo();
        QVERIFY( undo->isEnabled() );
        QVERIFY( redo->isEnabled() );
        ch.undo();
        QCOMPARE( m_documentRestored, 3 );
        QVERIFY( !undo->isEnabled() );
        QVERIFY( redo->isEnabled() );

        ch.redo();
        ch.redo();
        QVERIFY( !redo->isEnabled() );
        QCOMPARE( redo->text(), i18n( "&Redo" ) );
    }
    QCOMPARE( m_commandsExecuted, 12 ); // every add, undo and redo
    QCOMPARE( m_documentRestored, 3 );
    QCOMPARE( KTestCommand::deletedCommands.join( "," ), QString( "2,1,3" ) ); // ok the order doesn't matter ;)
}

void KCommandTest::testDocumentRestored()
{
    KTestCommand::clearLists();
    m_commandsExecuted = 0;
    m_documentRestored = 0;
    {
        KCommandHistory ch;
        connect( &ch, SIGNAL( commandExecuted( KCommand* ) ), this, SLOT( slotCommandExecuted( KCommand* ) ) );
        connect( &ch, SIGNAL( documentRestored() ), this, SLOT( slotDocumentRestored() ) );

        ch.documentSaved(); // saved with empty history

        KTestCommand* c1 = new KTestCommand( "1" );
        ch.addCommand( c1 ); // executes the action
        ch.undo();
        QCOMPARE( m_documentRestored, 1 );

        ch.redo();
        ch.documentSaved(); // now saved with one item in the history
        KTestCommand* c2 = new KTestCommand( "2" );
        ch.addCommand( c2 );
        QCOMPARE( m_documentRestored, 1 );
        ch.undo();
        QCOMPARE( m_documentRestored, 2 );

        ch.undo();
        ch.redo();
        QCOMPARE( m_documentRestored, 3 );
    }
    QCOMPARE( m_commandsExecuted, 7 ); // every add, undo and redo
}

void KCommandTest::testUndoLimit()
{
    KTestCommand::clearLists();
    m_commandsExecuted = 0;
    m_documentRestored = 0;
    KActionCollection actionCollection( ( QWidget*)0 );
    {
        KCommandHistory ch( &actionCollection );
        ch.setUndoLimit( 2 );
        ch.setRedoLimit( 2 );

        KTestCommand* c1 = new KTestCommand( "1" );
        ch.addCommand( c1 ); // executes the action
        KTestCommand* c2 = new KTestCommand( "2" );
        ch.addCommand( c2 );
        KTestCommand* c3 = new KTestCommand( "3" );
        ch.addCommand( c3 );
        // c1 should have been removed now; let's check that we can only undo twice.
        ch.undo();
        ch.undo();
        QCOMPARE( m_documentRestored, 0 );

        KAction* undo = actionCollection.action( "edit_undo" );
        QVERIFY( undo );
        QVERIFY( !undo->isEnabled() );
        KAction* redo = actionCollection.action( "edit_redo" );
        QVERIFY( redo );
        QVERIFY( redo->isEnabled() );
        QCOMPARE( redo->text(), i18n( "&Redo: %1" ).arg( 2 ) );
        ch.redo();
        QVERIFY( undo->isEnabled() );
        QVERIFY( redo->isEnabled() );
        QCOMPARE( redo->text(), i18n( "&Redo: %1" ).arg( 3 ) );
        ch.redo();

        ch.setUndoLimit( 1 );
        QVERIFY( undo->isEnabled() );
        ch.undo();
        QVERIFY( !undo->isEnabled() );
        QVERIFY( redo->isEnabled() );
        QCOMPARE( redo->text(), i18n( "&Redo: %1" ).arg( 3 ) );

        redo->setEnabled( false ); // imagine the app goes into readonly mode
        ch.updateActions(); // then back to readwrite, so it calls this
        QVERIFY( !undo->isEnabled() );
        QVERIFY( redo->isEnabled() );

        // test clear
        ch.clear();
        QVERIFY( !undo->isEnabled() );
        QVERIFY( !redo->isEnabled() );
        QCOMPARE( undo->text(), i18n( "&Undo" ) );
        QCOMPARE( redo->text(), i18n( "&Redo" ) );
        QCOMPARE( KTestCommand::deletedCommands.join( "," ), QString( "1,2,3" ) );
    }
    QCOMPARE( m_documentRestored, 0 );
}

// Helper slots

void KCommandTest::slotCommandExecuted( KCommand* )
{
    ++m_commandsExecuted;
}

void KCommandTest::slotDocumentRestored()
{
    ++m_documentRestored;
}
