#include <qlayout.h>
#include <qstring.h>
#include <qpushbutton.h>

#include <klineedit.h>
#include <kiconloader.h>

#include "kfiledialog.h"
#include "kfilelineedit.h"

KPathLineEdit::KPathLineEdit
    (const QString & initialPath, QWidget * parent)
    :   QWidget(parent, "PathSelectWidget")
{
    le_path_    = new KLineEdit(initialPath, this);
    pb_select_  = new QPushButton(this);
    
    //pb_select_->setPixmap(BarIcon("fileopen"));
    pb_select_->setFixedWidth(pb_select_->sizeHint().height());

    QObject::connect(pb_select_, SIGNAL(clicked()), this, SLOT(s_browse()));

    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->addWidget(le_path_);
    layout->addWidget(pb_select_);
}

KPathLineEdit::KPathLineEdit(QWidget *parent)
    :   QWidget(parent, "PathSelectWidget")
{
    le_path_    = new KLineEdit(QString::null, this);
    pb_select_  = new QPushButton(this);
    
    //pb_select_->setPixmap(BarIcon("fileopen"));
    pb_select_->setFixedWidth(pb_select_->sizeHint().height());

    QObject::connect(pb_select_, SIGNAL(clicked()), this, SLOT(s_browse()));

    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->addWidget(le_path_);
    layout->addWidget(pb_select_);
}

KPathLineEdit::~KPathLineEdit()
{
}

    QString
KPathLineEdit::path() const
{
    return le_path_->text();
}

    void
KPathLineEdit::setPath(const QString & s)
{
    le_path_->setText(s);
    emit(changed(s));
}
        
    void
KFileLineEdit::s_browse()
{
    KURL url =
        KFileDialog::getOpenURL(le_path_->text(), QString::null, this);
    
    if (!url.isEmpty())
        le_path_->setText(url.filename());
}

    void
KDirLineEdit::s_browse()
{
    QString s = KFileDialog::getExistingDirectory(le_path_->text(), this);
    
    if (!s.isEmpty())
        le_path_->setText(s);
}


#include "kfilelineedit.moc"
