#ifndef _KPANNER_TEST_H
#define _KPANNER_TEST_H

class TopLevel : public QWidget
{
    Q_OBJECT
public:

    TopLevel( QWidget *parent=0, const char *name=0 );
protected:
    void resizeEvent(QResizeEvent *);
    KPanner *panner, *panner1;
    QPushButton *ok, *cancel;
    QListBox *box;
    QMultiLineEdit *e;
public slots:
    void pannerHasChanged();
    void okPressed();
};

#endif
