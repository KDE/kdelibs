#ifndef kcontainertest_h
#define kcontainertest_h

#include <qwidget.h>
#include <qdialog.h>
#include <kcontainer.h>

class TestWidget : public KContainerLayout
{
Q_OBJECT
public:
    TestWidget( int orientation = KContainerLayout::Horizontal,
		bool homogeneos = FALSE,
		int spacing = 5);
public slots:
    void changeOrientation();
    void changeHomo();
    void removeSpace();
    void addSpace();
    void addButton();
    void showComplexLayout();
};

class TopLevelTest : public QDialog
{
Q_OBJECT
public:
    TopLevelTest();
};
#endif

