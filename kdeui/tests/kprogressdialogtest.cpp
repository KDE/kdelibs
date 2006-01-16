#include <QWidget>
#include <QTimerEvent>
#include <QCheckBox>

#include <kapplication.h>
#include <kcmdlineargs.h>

#include "kprogressdialog.h"

class MyWidget : public QWidget {
public:
    MyWidget() : QWidget()
    {
        setFixedSize(200, 80);
        Cancelled = new QCheckBox("Cancelled", this);
        Progress = new KProgressDialog(this);
        startTimer(50);
        Progress->setLabel("label text");
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
    Progress->progressBar()->advance(1);
    Cancelled->setCheckState(Progress->wasCancelled() ? Qt::Checked : Qt::Unchecked);
}

int main(int argc, char *argv[])
{
    KCmdLineArgs::init( argc, argv, "test", "Test" ,"test app" ,"1.0" );
    KApplication app;
    MyWidget w;

    app.setMainWidget(&w);

    w.show();

    int ret = app.exec();
    return ret;
}
