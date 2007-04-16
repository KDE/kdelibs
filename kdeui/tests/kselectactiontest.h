#ifndef KSELECTACTION_TEST_H
#define KSELECTACTION_TEST_H

#include <kxmlguiwindow.h>

class KSelectAction;

class SelectActionTest : public KXmlGuiWindow
{
    Q_OBJECT

public:
    SelectActionTest(QWidget* parent = 0);

public Q_SLOTS:
    void triggered(QAction* action);
    void triggered(int index);
    void triggered(const QString& text);

    void slotActionTriggered(bool state);

    void addAction();
    void removeAction();

private:
    KSelectAction* m_comboSelect;
    KSelectAction* m_buttonSelect;
};

#endif
