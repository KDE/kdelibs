#ifndef KCOMMANDTEST_H
#define KCOMMANDTEST_H

#include <QtCore/QObject>
class K3Command;

class KCommandTest : public QObject
{
    Q_OBJECT


protected Q_SLOTS:
    void slotCommandExecuted( K3Command* );
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

