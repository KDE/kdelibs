/* 
 * KURLCompletion
 * Completion of URL's (currently just files), to
 * be connected to KLined() which emits signals for completion (Tab/Ctrl-S)
 * and rotation (Ctrl-D)
 *
 * 10/1997 Henner Zeller <zeller@think.de>
 * This code is based on Torben Weis' KFileEntry
 */

#ifndef kurlcompletion_h
#define kurlcompletion_h

#include <qstring.h>
#include <qstrlist.h>
#include <qobject.h>

class KURLCompletion : public QObject
{
    Q_OBJECT

public slots:
    void make_completion ();
    void make_rotation ();
    void edited (const QString&);
    
signals:
    void setText (const QString&);
	
public:
    KURLCompletion( const QString& dir = QString::null);
    ~KURLCompletion();

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
