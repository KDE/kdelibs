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
      m_comboSelect->addAction(QString ("Combo Action %1").arg(i));
      m_buttonSelect->addAction(QString ("Action %1").arg(i));
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

#include "kselectactiontest.moc"

