#include <MainWindow.h>
#include "ui_SampleFormulaWidget.h"
#include <KFormula.h>
#include <kconfig.h>

MainWindow::MainWindow( QWidget* parent ) : KXmlGuiWindow( parent )
{
    ui = new Ui_SampleFormulaWidget();
    QWidget *widget = new QWidget(this);
    ui->setupUi(widget);
    setCentralWidget(widget);

    KConfig config;
    KConfigGroup generalGroup( &config, "General" );
    QString mathml = generalGroup.readEntry("mathml", "<mrow>>mfrac><mi>x</mi></mfrac></mrow>");

    ui->text->setText( mathml );
    ui->kformula->setMathML(mathml);

    connect(ui->text, SIGNAL(textChanged()), this, SLOT(textChanged()));
}

MainWindow::~MainWindow() {
    KConfig config;
    KConfigGroup generalGroup( &config, "General" );
    generalGroup.writeEntry("mathml", ui->text->toPlainText() );
}

void MainWindow::textChanged() {
    ui->kformula->setMathML(ui->text->toPlainText());
}

#include "MainWindow.moc"

