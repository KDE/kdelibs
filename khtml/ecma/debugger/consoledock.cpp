#include <QVBoxLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>

#include "consoledock.h"
#include "consoledock.moc"

ConsoleDock::ConsoleDock(QWidget *parent)
    : QDockWidget("Console", parent)
{
    QFrame *mainFrame = new QFrame;

    consoleEdit = new QTextEdit;
    consoleEdit->setReadOnly(true);
    consoleInput = new QLineEdit;
    consoleInputButton = new QPushButton("Enter");

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->setSpacing(0);
    bottomLayout->setMargin(0);
    bottomLayout->addWidget(consoleInput);
    bottomLayout->addWidget(consoleInputButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(mainFrame);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->addWidget(consoleEdit);
    mainLayout->addLayout(bottomLayout);

    setWidget(mainFrame);
}

ConsoleDock::~ConsoleDock()
{
}
