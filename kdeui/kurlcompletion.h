#ifndef kurlcompletion_h
#define kurlcompletion_h

#include <qstring.h>
#include <qstrlist.h>
#include <qobject.h>

/**
 * URL completion helper. To use this, connect
 * it to a @ref KLined widget, which emits signals for completion
 * (Tab/Ctrl-S) and rotation (Ctrl-D).
 *
 * Limitations: Currently only file completion is supported.
 *
 * @see KLined
 * @author Henner Zeller <zeller@think.de>, based on code by Torben Weis.
 * @version $Id$
 */
class KURLCompletion : public QObject
{
    Q_OBJECT

public:
    KURLCompletion( const QString& dir = QString::null);
    ~KURLCompletion();

public slots:
    void make_completion ();
    void make_rotation ();
    void edited (const QString&);
    
signals:
    void setText (const QString&);
	
private:
    bool is_fileurl (QString &url, bool &ambigous_beginning) const;
    void GetList (QString dir, QString & match);
    bool CompleteDir (QString &dir);
    QString directory;

    bool new_guess, completed_dir, self_update;
    QString the_text;
    QStrList possibilityList;
    QString guess;
    QString qual_dir;
};

#endif
