#ifndef KCOMMANDTEST_H
#define KCOMMANDTEST_H

#include <qobject.h>
class KCommand;

class KCommandTest : public QObject
{
    Q_OBJECT


protected Q_SLOTS:
    void slotCommandExecuted( KCommand* );
    void slotDocumentRestored();

private Q_SLOTS:
    // KMacroCommand tests
    void testMacroCommand();

    // KCommandHistory tests
    void testCommandHistoryAdd();
    void testDocumentRestored();
    void testUndoLimit();

private:
    int m_commandsExecuted;
    int m_documentRestored;
};

#endif /* KCOMMANDTEST_H */

