#include <kapp.h>
#include <qpushbt.h>
#include <qlabel.h>

#include "kcontainertest.h"

#define SIZETOFIT 0
#define PACK packEnd

TestWidget::TestWidget(int orientation, bool homogeneos, int spacing)
: KContainerLayout(0L,"",orientation,homogeneos,spacing)
{
    setStartOffset(20);
    setEndOffset(40);
    // Pack a few buttons
    QPushButton *b = new QPushButton(this);
    b->setText("orientation");
    packEnd(b);
    connect( b, SIGNAL(clicked()), this, SLOT(changeOrientation()) );
    b = new QPushButton(this);
    b->setText("homo");
    PACK(b);
    connect( b, SIGNAL(clicked()), this, SLOT(changeHomo()) );
    b = new QPushButton(this);
    b->setText("+space");
    PACK(b);
    connect( b, SIGNAL(clicked()), this, SLOT(addSpace()) );
    b = new QPushButton(this);
    b->setText("-space");
    PACK(b);
    connect( b, SIGNAL(clicked()), this, SLOT(removeSpace()) );
    b = new QPushButton(this);
    b->setText("add button");
    PACK(b);
    connect( b, SIGNAL(clicked()), this, SLOT(addButton()) );
    
    // make it small, since the layout mechanism will grow it apropriately ;)
    resize(10,10);
    show();
}

void TestWidget::changeOrientation()
{
    int i = orientation();
    if (i == KContainerLayout::Horizontal)
	setOrientation(KContainerLayout::Vertical);
    else
	setOrientation(KContainerLayout::Horizontal);
#if SIZETOFIT
    sizeToFit();
#endif
}

void TestWidget::changeHomo()
{
    setHomogeneos(!homogeneos());
#if SIZETOFIT
    sizeToFit();
#endif
}

void TestWidget::addSpace()
{
    setSpacing(spacing()+2);
#if SIZETOFIT
    sizeToFit();
#endif
}

void TestWidget::removeSpace()
{
    setSpacing(((spacing()-2)>0)?spacing()-2:0);
#if SIZETOFIT
    sizeToFit();
#endif
}

void TestWidget::addButton()
{
    QPushButton *b = new QPushButton(this);
    b->setText("dialogbox");
    b->show();
    packStart(b);
    connect( b, SIGNAL(clicked()), this, SLOT(showComplexLayout()) );
#if SIZETOFIT
    sizeToFit();
#endif
}

void TestWidget::showComplexLayout()
{
    TopLevelTest *test = new TopLevelTest();
    warning("test->show()");
    test->exec();
}


TopLevelTest::TopLevelTest()
    : QDialog(0,"dialog",TRUE)
{
    // Create the vertical container for our horizontal sub-containers
    KContainerLayout *container = new KContainerLayout(this,"topbox",KContainerLayout::Vertical,TRUE,0);
    // Pack first row
    KContainerLayout *box = new KContainerLayout(container,"box1",KContainerLayout::Horizontal,FALSE,0);
    // a row with the box description
    box = new KContainerLayout(container,"box2",KContainerLayout::Horizontal,FALSE,0);
    QLabel *label = new QLabel(box);
    label->setText("container with TRUE,0");
    box->packStart(label);
    container->packStart(box);
    box->show();
	    
    box = new KContainerLayout(container,"box2",KContainerLayout::Horizontal,FALSE,0);
    QPushButton *b = new QPushButton(box);
    b->setText("pack");
    box->packStart(b,FALSE,FALSE,0);
    b = new QPushButton(box);
    b->setText("FALSE");
    box->packStart(b,FALSE,FALSE,0);
    b = new QPushButton(box);
    b->setText("FALSE");
    box->packStart(b,FALSE,FALSE,0);
    b = new QPushButton(box);
    b->setText("0");
    box->packStart(b,FALSE,FALSE,0);
    container->packStart(box);
    box->show();

    // Pack second row
    box = new KContainerLayout(container,"box3",KContainerLayout::Horizontal,FALSE,0);
    b = new QPushButton(box);
    b->setText("pack");
    box->packStart(b,TRUE,FALSE,0);
    b = new QPushButton(box);
    b->setText("TRUE");
    box->packStart(b,TRUE,FALSE,0);
    b = new QPushButton(box);
    b->setText("FALSE");
    box->packStart(b,TRUE,FALSE,0);
    b = new QPushButton(box);
    b->setText("0");
    box->packStart(b,TRUE,FALSE,0);
    container->packStart(box);
    box->show();

    // Pack third row
    box = new KContainerLayout(container,"box4",KContainerLayout::Horizontal,FALSE,0);
    b = new QPushButton(box);
    b->setText("pack");
    box->packStart(b,TRUE,TRUE,0);
    b = new QPushButton(box);
    b->setText("TRUE");
    box->packStart(b,TRUE,TRUE,0);
    b = new QPushButton(box);
    b->setText("TRUE");
    box->packStart(b,TRUE,TRUE,0);
    b = new QPushButton(box);
    b->setText("0");
    box->packStart(b,TRUE,TRUE,0);
    container->packStart(box);
    box->show();

    // a row with the box description
    box = new KContainerLayout(container,"box5",
			       KContainerLayout::Horizontal,TRUE,0);
    label = new QLabel(box);
    label->setText("container with TRUE,0");
    box->packStart(label);
    container->packStart(box);
    box->show();
	    
    // Pack second row
    box = new KContainerLayout(container,"box6",
			       KContainerLayout::Horizontal,TRUE,0);
    b = new QPushButton(box);
    b->setText("pack");
    box->packStart(b,TRUE,FALSE,0);
    b = new QPushButton(box);
    b->setText("TRUE");
    box->packStart(b,TRUE,FALSE,0);
    b = new QPushButton(box);
    b->setText("FALSE");
    box->packStart(b,TRUE,FALSE,0);
    b = new QPushButton(box);
    b->setText("0");
    box->packStart(b,TRUE,FALSE,0);
    container->packStart(box);
    box->show();

    // Pack third row
    box = new KContainerLayout(container,"box7",KContainerLayout::Horizontal,TRUE,0);
    b = new QPushButton(box);
    b->setText("pack");
    box->packStart(b,TRUE,TRUE,0);
    b = new QPushButton(box);
    b->setText("TRUE");
    box->packStart(b,TRUE,TRUE,0);
    b = new QPushButton(box);
    b->setText("TRUE");
    box->packStart(b,TRUE,TRUE,0);
    b = new QPushButton(box);
    b->setText("0");
    box->packStart(b,TRUE,TRUE,0);
    container->packStart(box);
    box->show();
    // The last box with the OK button
    box = new KContainerLayout(container,"box8",KContainerLayout::Horizontal,FALSE,0);
    b = new QPushButton(box);
    b->setText("OK");
    box->packStart(b,TRUE);
    connect( b, SIGNAL(clicked()), SLOT(accept()) );
    container->packStart(box);
    box->show();

    container->show();

    // make it small, since the layout mechanism will grow it apropriately ;)
    resize(10,10);
}

int main( int argc, char **argv )
{
    KApplication a( argc, argv );
    TestWidget test(KContainerLayout::Horizontal,TRUE,5);

    a.setMainWidget( &test );
    test.show();
    return a.exec();
}

#include "kcontainertest.moc"

