#include <QVBoxLayout>
#include <MainWindow.h>

MainWindow::MainWindow( QWidget* parent ) : KXmlGuiWindow( parent )
{
    m_formula = new FormulaWidget();
    setCentralWidget(m_formula);
    setupGUI();
}
