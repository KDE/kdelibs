#ifndef KCURSOR_PRIVATE_H
#define KCURSOR_PRIVATE_H

#include <qcursor.h>
#include <qobject.h>

class QPoint;
class QTimer;
class QWidget;

/**
 * I don't want the eventFilter to be in KCursor, so we have another class
 * for that stuff
 * @internal
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
*/
class KCursorPrivate : public QObject
{
    friend class KCursor; // to shut up the compiler
    Q_OBJECT

public:
    static KCursorPrivate *self();
    void start();
    void stop();
    void hideCursor( QWidget * );
    void unhideCursor( QWidget * );
    virtual bool eventFilter( QObject *o, QEvent *e );

    int hideCursorDelay;

private slots:
    void slotHideCursor();
    void slotWidgetDestroyed();

private:
    KCursorPrivate();
    ~KCursorPrivate();

    bool insideWidget( const QPoint&, QWidget * );

    int count;
    bool isCursorHidden;
    bool isOwnCursor;
    bool enabled;
    QCursor oldCursor;
    QTimer *autoHideTimer;
    QWidget *hideWidget;
    static KCursorPrivate *s_self;
};



#endif // KCURSOR_PRIVATE_H
