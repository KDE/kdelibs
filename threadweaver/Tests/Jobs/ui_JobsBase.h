#ifndef UI_JOBSBASE_H
#define UI_JOBSBASE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QLCDNumber>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QWidget>

class Ui_JobsBase
{
public:
    QGridLayout *gridLayout;
    QSpacerItem *spacerItem;
    QPushButton *pbStart;
    QPushButton *pbStop;
    QPushButton *pbQuit;
    QLCDNumber *lcdNumJobsRem;
    QSpacerItem *spacerItem1;
    QCheckBox *cbLog;
    QFrame *horizontalLine;
    QSpacerItem *spacerItem2;
    QLabel *label;
    QProgressBar *pbProgress;
    QLabel *label_2;

    void setupUi(QWidget *JobsBase)
    {
    JobsBase->setObjectName(QString::fromUtf8("JobsBase"));
    JobsBase->resize(QSize(400, 300).expandedTo(JobsBase->minimumSizeHint()));
    gridLayout = new QGridLayout(JobsBase);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(9);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    spacerItem = new QSpacerItem(75, 31, QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout->addItem(spacerItem, 4, 1, 1, 1);

    pbStart = new QPushButton(JobsBase);
    pbStart->setObjectName(QString::fromUtf8("pbStart"));
    pbStart->setEnabled(false);

    gridLayout->addWidget(pbStart, 6, 0, 1, 1);

    pbStop = new QPushButton(JobsBase);
    pbStop->setObjectName(QString::fromUtf8("pbStop"));
    pbStop->setEnabled(false);

    gridLayout->addWidget(pbStop, 6, 1, 1, 1);

    pbQuit = new QPushButton(JobsBase);
    pbQuit->setObjectName(QString::fromUtf8("pbQuit"));
    pbQuit->setEnabled(false);

    gridLayout->addWidget(pbQuit, 6, 4, 1, 1);

    lcdNumJobsRem = new QLCDNumber(JobsBase);
    lcdNumJobsRem->setObjectName(QString::fromUtf8("lcdNumJobsRem"));
    lcdNumJobsRem->setMinimumSize(QSize(32, 48));
    QFont font;
    font.setFamily(QString::fromUtf8("Sans Serif"));
    font.setPointSize(14);
    font.setBold(false);
    font.setItalic(false);
    font.setUnderline(false);
    font.setWeight(50);
    font.setStrikeOut(false);
    lcdNumJobsRem->setFont(font);
    lcdNumJobsRem->setFrameShape(QFrame::NoFrame);
    lcdNumJobsRem->setFrameShadow(QFrame::Plain);
    lcdNumJobsRem->setSegmentStyle(QLCDNumber::Flat);

    gridLayout->addWidget(lcdNumJobsRem, 1, 4, 1, 1);

    spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    gridLayout->addItem(spacerItem1, 1, 3, 1, 1);

    cbLog = new QCheckBox(JobsBase);
    cbLog->setObjectName(QString::fromUtf8("cbLog"));

    gridLayout->addWidget(cbLog, 3, 0, 1, 5);

    horizontalLine = new QFrame(JobsBase);
    horizontalLine->setObjectName(QString::fromUtf8("horizontalLine"));
    horizontalLine->setFrameShape(QFrame::HLine);
    horizontalLine->setFrameShadow(QFrame::Sunken);

    gridLayout->addWidget(horizontalLine, 5, 0, 1, 5);

    spacerItem2 = new QSpacerItem(136, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    gridLayout->addItem(spacerItem2, 6, 2, 1, 2);

    label = new QLabel(JobsBase);
    label->setObjectName(QString::fromUtf8("label"));
    label->setCursor(QCursor(static_cast<Qt::CursorShape>(4)));
    label->setFrameShape(QFrame::Panel);
    label->setFrameShadow(QFrame::Plain);
    label->setLineWidth(2);
    label->setWordWrap(true);
    label->setMargin(6);

    gridLayout->addWidget(label, 0, 0, 1, 5);

    pbProgress = new QProgressBar(JobsBase);
    pbProgress->setObjectName(QString::fromUtf8("pbProgress"));
    pbProgress->setValue(0);
    pbProgress->setTextVisible(false);

    gridLayout->addWidget(pbProgress, 2, 0, 1, 5);

    label_2 = new QLabel(JobsBase);
    label_2->setObjectName(QString::fromUtf8("label_2"));

    gridLayout->addWidget(label_2, 1, 0, 1, 3);

    QWidget::setTabOrder(pbStart, pbStop);
    QWidget::setTabOrder(pbStop, pbQuit);
    retranslateUi(JobsBase);

    QMetaObject::connectSlotsByName(JobsBase);
    } // setupUi

    void retranslateUi(QWidget *JobsBase)
    {
    JobsBase->setWindowTitle(QApplication::translate("JobsBase", "ThreadWeaver Jobs Examples"));
    JobsBase->setToolTip(QApplication::translate("JobsBase", "The program executes 100 jobs in 4 threads. Each job waits for a random amount of milliseconds between 1 and 1000."));
    pbStart->setText(QApplication::translate("JobsBase", "Start"));
    pbStop->setText(QApplication::translate("JobsBase", "Stop"));
    pbQuit->setText(QApplication::translate("JobsBase", "Quit"));
    cbLog->setToolTip(QApplication::translate("JobsBase", "Check to see logging information about thread activity. Watch the console output to see the information."));
    cbLog->setText(QApplication::translate("JobsBase", "Log Thread Activity"));
    label->setText(QApplication::translate("JobsBase", "GUI based example for the Weaver Thread Manager"));
    label_2->setText(QApplication::translate("JobsBase", "Remaining number of jobs:"));
    Q_UNUSED(JobsBase);
    } // retranslateUi

};

namespace Ui {
    class JobsBase: public Ui_JobsBase {};
} // namespace Ui

#endif // UI_JOBSBASE_H
