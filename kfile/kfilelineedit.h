#ifndef KFILELINEEDIT_H
#define KFILELINEEDIT_H

// Qt includes
#include <qstring.h>
#include <qwidget.h>

class QPushButton;
class KLineEdit;

/**
 * @short Browsable file/dir selector
 * Quick hack to lay out a QLineEdit and a QPushButton to provide a megawidget
 * that lets the user select a dir or a file path.
 * @author Rikkus
 */
class KPathLineEdit : public QWidget
{
Q_OBJECT

public:
    KPathLineEdit(const QString & initialPath, QWidget * parent);
    KPathLineEdit(QWidget *parent);
    virtual ~KPathLineEdit();
    virtual QString path() const;
    virtual void setPath(const QString &);
        
protected slots:
    virtual void s_browse() = 0;

signals:
    virtual void changed(const QString &);

protected:
    KLineEdit *le_path_;
    QPushButton *pb_select_;
};

/**
 * @short Browsable file selector
 * All required functions are in @ref KPathLineEdit
 * @author Rikkus
 */
class KFileLineEdit : public KPathLineEdit
{
Q_OBJECT

public:
    KFileLineEdit(const QString & initialPath, QWidget * parent)
        : KPathLineEdit(initialPath, parent)
    {}

protected slots:
    virtual void s_browse();
};

/**
 * @short Browsable directory selector
 * All required functions are in @ref KPathLineEdit
 * @author Rikkus
 */
class KDirLineEdit : public KPathLineEdit
{
Q_OBJECT
public:
    KDirLineEdit(const QString & initialPath, QWidget * parent)
         : KPathLineEdit(initialPath, parent)
    {}

protected slots:
     virtual void s_browse();
};

#endif

