#include <QApplication>
#include <QWidget>
#include <QtCore/QEvent>
#include <QCheckBox>

#include "kprogressdialog.h"

class MyWidget : public QWidget {
public:
    MyWidget() : QWidget()
    {
        setFixedSize(200, 80);
        Cancelled = new QCheckBox("Cancelled", this);
        Progress = new KProgressDialog(this);
        startTimer(50);
        Progress->setLabelText("label text");
        Progress->setAllowCancel(false);
        Progress->showCancelButton(true);
        Progress->setButtonText("button text");
        Progress->setAutoClose(false);
        steps = 300;
        Progress->progressBar()->setRange(0, steps);

    }

private:
    KProgressDialog *Progress;
    QCheckBox *Cancelled;

    int steps;

    void timerEvent(QTimerEvent *);
};

void MyWidget::timerEvent(QTimerEvent *)
{
    Progress->progressBar()->setValue(Progress->progressBar()->value()+1);
    Cancelled->setCheckState(Progress->wasCancelled() ? Qt::Checked : Qt::Unchecked);
}

int main(int argc, char *argv[])
{
    QApplication::setApplicationName("test");
    QApplication app(argc, argv);
    MyWidget w;

    w.show();

    int ret = app.exec();
    return ret;
}
