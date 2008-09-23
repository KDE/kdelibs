#include <MainWindow.h>
#include "ui_SampleFormulaWidget.h"
#include <KFormula.h>

MainWindow::MainWindow( QWidget* parent ) : KXmlGuiWindow( parent )
{
    ui = new Ui_SampleFormulaWidget();
    QWidget *widget = new QWidget(this);
    ui->setupUi(widget);
    setCentralWidget(widget);

    ui->text->setText( ui->kformula->mathML());

    connect(ui->text, SIGNAL(textChanged()), this, SLOT(textChanged()));
}

void MainWindow::textChanged() {
    ui->kformula->setMathML(ui->text->toPlainText());
}

#include "MainWindow.moc"

