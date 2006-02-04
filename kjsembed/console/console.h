#ifndef CONSOLE_H
#define CONSOLE_H
#include <QMainWindow>
#include <QStandardItemModel>

#include "kjsembed/kjsembed.h"

class KJSObjectModel;

class Console : public QMainWindow
{
    Q_OBJECT
    public:
    Console( QWidget *parent = 0);
    ~Console();

    public slots:
        void on_mExecute_clicked();
        void on_actionOpenScript_activated();
        void on_actionCloseScript_activated();
        void on_actionQuit_activated();
        void on_actionRun_activated();
        void on_actionRunTo_activated();
        void on_actionStep_activated();
        void on_actionStop_activated();

    private:
        void updateModel(const QModelIndex &parent, KJS::Object &obj );

        KJSEmbed::Engine mKernel;
        KJSObjectModel *m_model;
        QModelIndex m_root;
};
#endif
