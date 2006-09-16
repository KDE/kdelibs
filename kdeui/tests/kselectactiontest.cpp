#include <QToolBar>

#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kmenubar.h>
#include <kdebug.h>

#include "kselectactiontest.h"

#include <kselectaction.h>

int main( int argc, char **argv )
{
    KCmdLineArgs::init( argc, argv, "kselectactiontest", "KSelectActionTest", "kselectaction test app", "1.0" );
    KApplication app;

    SelectActionTest* test = new SelectActionTest;
    test->show();

    return app.exec();
}

SelectActionTest::SelectActionTest(QWidget *parent)
    : KMainWindow(parent)
    , m_comboSelect(new KSelectAction("Combo Selection", actionCollection(), "combo"))
    , m_buttonSelect(new KSelectAction("Button Selection", actionCollection(), "button"))
{
    for (int i = 0; i < 7; ++i) {
      QAction* action = m_comboSelect->addAction(QString ("Combo Action %1").arg(i));
      connect(action, SIGNAL(triggered(bool)), SLOT(slotActionTriggered(bool)));
      action = m_buttonSelect->addAction(QString ("Action %1").arg(i));
      connect(action, SIGNAL(triggered(bool)), SLOT(slotActionTriggered(bool)));
    }

    m_comboSelect->setToolBarMode(KSelectAction::ComboBoxMode);
    connect(m_comboSelect, SIGNAL(triggered(QAction*)), SLOT(triggered(QAction*)));
    connect(m_comboSelect, SIGNAL(triggered(int)), SLOT(triggered(int)));
    connect(m_comboSelect, SIGNAL(triggered(const QString&)), SLOT(triggered(const QString&)));

    m_buttonSelect->setToolBarMode(KSelectAction::MenuMode);
    connect(m_buttonSelect, SIGNAL(triggered(QAction*)), SLOT(triggered(QAction*)));
    connect(m_buttonSelect, SIGNAL(triggered(int)), SLOT(triggered(int)));
    connect(m_buttonSelect, SIGNAL(triggered(const QString&)), SLOT(triggered(const QString&)));

    menuBar()->addAction(m_comboSelect);
    menuBar()->addAction(m_buttonSelect);
    menuBar()->addAction("Add an action", this, SLOT(addAction()));
    menuBar()->addAction("Remove an action", this, SLOT(removeAction()));

    QToolBar* toolBar = addToolBar("Test");
    toolBar->addAction(m_comboSelect);
    toolBar->addAction(m_buttonSelect);
}

void SelectActionTest::triggered(QAction* action)
{
  kDebug() << k_funcinfo << action << endl;
}

void SelectActionTest::triggered(int index)
{
  kDebug() << k_funcinfo << index << endl;
}

void SelectActionTest::triggered(const QString& text)
{
  kDebug() << k_funcinfo << '"' << text << '"' << endl;
}

void SelectActionTest::addAction()
{
    QAction* action = m_comboSelect->addAction(QString ("Combo Action %1").arg(m_comboSelect->actions().count()));
    connect(action, SIGNAL(triggered(bool)), SLOT(slotActionTriggered(bool)));
    action = m_buttonSelect->addAction(QString ("Action %1").arg(m_buttonSelect->actions().count()));
    connect(action, SIGNAL(triggered(bool)), SLOT(slotActionTriggered(bool)));
}

void SelectActionTest::removeAction()
{
    if (!m_comboSelect->actions().isEmpty())
        m_comboSelect->removeAction(m_comboSelect->actions().last());

    if (!m_buttonSelect->actions().isEmpty())
        m_buttonSelect->removeAction(m_buttonSelect->actions().last());
}

void SelectActionTest::slotActionTriggered(bool state)
{
    kDebug() << k_funcinfo << sender() << " state " << state << endl;
}

#include "kselectactiontest.moc"

