#ifndef TAGWIDGETTEST_H
#define TAGWIDGETTEST_H

#include <QWidget>
#include "tag.h"
#include "tagwidget.h"

class TagWidgetTest : public QWidget
{
    Q_OBJECT

public:
    TagWidgetTest();
    ~TagWidgetTest();

public slots:
    void slotTagClicked(const Nepomuk::Tag&);
    void slotSelectionChanged( const QList<Nepomuk::Tag>& tags );

private slots:
    void enableMinimode( bool enable );
    void alignRight( bool enable );
    void disableClicking( bool enable );
    void setReadOnly( bool enable );

private:
    Nepomuk::TagWidget* m_tagWidget;
};

#endif
