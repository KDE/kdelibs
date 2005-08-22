#ifndef UI_SMIVBASE_H
#define UI_SMIVBASE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>
#include "SMIVView.h"

class Ui_SMIVBase
{
public:
    QGridLayout *gridLayout;
    QFrame *line;
    QPushButton *pbSelectFiles;
    QPushButton *pbCancel;
    QPushButton *pbSuspend;
    QProgressBar *progressBar;
    QPushButton *pbQuit;
    SMIVView *listView;

    void setupUi(QWidget *SMIVBase)
    {
    SMIVBase->setObjectName(QString::fromUtf8("SMIVBase"));
    SMIVBase->resize(QSize(808, 641).expandedTo(SMIVBase->minimumSizeHint()));
    gridLayout = new QGridLayout(SMIVBase);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(8);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    line = new QFrame(SMIVBase);
    line->setObjectName(QString::fromUtf8("line"));
    line->setFrameShape(QFrame::HLine);

    gridLayout->addWidget(line, 1, 0, 1, 5);

    pbSelectFiles = new QPushButton(SMIVBase);
    pbSelectFiles->setObjectName(QString::fromUtf8("pbSelectFiles"));

    gridLayout->addWidget(pbSelectFiles, 2, 0, 1, 1);

    pbCancel = new QPushButton(SMIVBase);
    pbCancel->setObjectName(QString::fromUtf8("pbCancel"));
    pbCancel->setEnabled(false);

    gridLayout->addWidget(pbCancel, 2, 1, 1, 1);

    pbSuspend = new QPushButton(SMIVBase);
    pbSuspend->setObjectName(QString::fromUtf8("pbSuspend"));
    pbSuspend->setEnabled(false);

    gridLayout->addWidget(pbSuspend, 2, 2, 1, 1);

    progressBar = new QProgressBar(SMIVBase);
    progressBar->setObjectName(QString::fromUtf8("progressBar"));
    progressBar->setEnabled(false);
    progressBar->setValue(0);

    gridLayout->addWidget(progressBar, 2, 3, 1, 1);

    pbQuit = new QPushButton(SMIVBase);
    pbQuit->setObjectName(QString::fromUtf8("pbQuit"));

    gridLayout->addWidget(pbQuit, 2, 4, 1, 1);

    listView = new SMIVView(SMIVBase);
    listView->setObjectName(QString::fromUtf8("listView"));

    gridLayout->addWidget(listView, 0, 0, 1, 5);

    retranslateUi(SMIVBase);

    QMetaObject::connectSlotsByName(SMIVBase);
    } // setupUi

    void retranslateUi(QWidget *SMIVBase)
    {
    SMIVBase->setWindowTitle(QApplication::translate("SMIVBase", "Form"));
    pbSelectFiles->setText(QApplication::translate("SMIVBase", "Select Files..."));
    pbCancel->setText(QApplication::translate("SMIVBase", "Cancel"));
    pbSuspend->setText(QApplication::translate("SMIVBase", "Suspend"));
    pbQuit->setText(QApplication::translate("SMIVBase", "Quit"));
    Q_UNUSED(SMIVBase);
    } // retranslateUi

};

namespace Ui {
    class SMIVBase: public Ui_SMIVBase {};
} // namespace Ui

#endif // UI_SMIVBASE_H
