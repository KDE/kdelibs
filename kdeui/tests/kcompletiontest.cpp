#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './kcompletiontest.ui'
**
** Created: Wed Nov 15 20:15:10 2000
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "kcompletiontest.h"

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include <kapplication.h>
#include <klineedit.h>
#include <kcombobox.h>
/*
 *  Constructs a Form1 which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 */
Form1::Form1( QWidget* parent,  const char* name )
    : QWidget( parent, name, WDestructiveClose )
{
    if ( !name )
	setName( "Form1" );
    resize( 559, 465 );
    setCaption(  "Form1" );
    Form1Layout = new QVBoxLayout( this );
    Form1Layout->setSpacing( 6 );
    Form1Layout->setMargin( 11 );

    GroupBox1 = new QGroupBox( this, "GroupBox1" );
    GroupBox1->setTitle( "Completion Test" );
    GroupBox1->setColumnLayout(0, Qt::Vertical );
    GroupBox1->layout()->setSpacing( 0 );
    GroupBox1->layout()->setMargin( 0 );
    GroupBox1Layout = new QVBoxLayout( GroupBox1->layout() );
    GroupBox1Layout->setAlignment( Qt::AlignTop );
    GroupBox1Layout->setSpacing( 6 );
    GroupBox1Layout->setMargin( 11 );

    Layout9 = new QVBoxLayout;
    Layout9->setSpacing( 6 );
    Layout9->setMargin( 0 );

    Layout1 = new QHBoxLayout;
    Layout1->setSpacing( 6 );
    Layout1->setMargin( 0 );

    TextLabel1 = new QLabel( GroupBox1, "TextLabel1" );
    TextLabel1->setText(  "Completion"  );
    Layout1->addWidget( TextLabel1 );

    edit = new KLineEdit( GroupBox1, "edit" );
    Layout1->addWidget( edit );
    Layout9->addLayout( Layout1 );
    edit->completionObject()->setItems( defaultItems() );
    edit->completionObject()->setIgnoreCase( true );
    edit->setFocus();
    QToolTip::add( edit, "right-click to change completion mode" );

    Layout2 = new QHBoxLayout;
    Layout2->setSpacing( 6 );
    Layout2->setMargin( 0 );

    combo = new KHistoryCombo( GroupBox1, "history combo" );
    combo->setCompletionObject( edit->completionObject() );
    // combo->setMaxCount( 5 );
    combo->setHistoryItems( defaultItems(), true );
    connect( combo, SIGNAL( activated( const QString& )),
	     combo, SLOT( addToHistory( const QString& )));
    QToolTip::add( combo, "KHistoryCombo" );
    Layout2->addWidget( combo );

    LineEdit1 = new KLineEdit( GroupBox1, "LineEdit1" );
    Layout2->addWidget( LineEdit1 );

    PushButton1 = new QPushButton( GroupBox1, "PushButton1" );
    PushButton1->setText( "Add" );
    connect( PushButton1, SIGNAL( clicked() ), SLOT( slotAdd() ));
    Layout2->addWidget( PushButton1 );
    Layout9->addLayout( Layout2 );

    Layout3 = new QHBoxLayout;
    Layout3->setSpacing( 6 );
    Layout3->setMargin( 0 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout3->addItem( spacer );

    PushButton1_4 = new QPushButton( GroupBox1, "PushButton1_4" );
    PushButton1_4->setText( "Remove" );
    connect( PushButton1_4, SIGNAL( clicked() ), SLOT( slotRemove() ));
    Layout3->addWidget( PushButton1_4 );
    Layout9->addLayout( Layout3 );

    Layout8 = new QHBoxLayout;
    Layout8->setSpacing( 6 );
    Layout8->setMargin( 0 );

    ListBox1 = new QListBox( GroupBox1, "ListBox1" );
    Layout8->addWidget( ListBox1 );
    connect( ListBox1, SIGNAL( highlighted( const QString& )),
	     SLOT( slotHighlighted( const QString& )));
    QToolTip::add( ListBox1, "Contains the contents of the completion object.\n:x is the weighting, i.e. how often an item has been inserted");

    Layout7 = new QVBoxLayout;
    Layout7->setSpacing( 6 );
    Layout7->setMargin( 0 );

    PushButton1_3 = new QPushButton( GroupBox1, "PushButton1_3" );
    PushButton1_3->setText( "Completion items" );
    connect( PushButton1_3, SIGNAL( clicked() ), SLOT( slotList() ));
    Layout7->addWidget( PushButton1_3 );

    PushButton1_2 = new QPushButton( GroupBox1, "PushButton1_2" );
    PushButton1_2->setText( "Clear" );
    connect( PushButton1_2, SIGNAL( clicked() ),
	     edit->completionObject(), SLOT( clear() ));
    Layout7->addWidget( PushButton1_2 );
    Layout8->addLayout( Layout7 );
    Layout9->addLayout( Layout8 );
    GroupBox1Layout->addLayout( Layout9 );
    Form1Layout->addWidget( GroupBox1 );

    slotList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
Form1::~Form1()
{
    // no need to delete child widgets, Qt does it all for us
}

void Form1::slotAdd()
{
    qDebug("** adding: %s", LineEdit1->text().latin1() );
    edit->completionObject()->addItem( LineEdit1->text() );
    
    QStringList matches = edit->completionObject()->allMatches("S");
    QStringList::ConstIterator it = matches.begin();
    for ( ; it != matches.end(); ++it )
        qDebug("-- %s", (*it).latin1());
}

void Form1::slotRemove()
{
    edit->completionObject()->removeItem( LineEdit1->text() );
}

void Form1::slotList()
{
    ListBox1->clear();
    QStringList items = edit->completionObject()->items();
    ListBox1->insertStringList( items );
}

void Form1::slotHighlighted( const QString& text )
{
    // remove any "weighting"
    int index = text.findRev( ':' );
    if ( index > 0 )
	LineEdit1->setText( text.left( index ) );
    else
	LineEdit1->setText( text );
}


QStringList Form1::defaultItems() const
{
    QStringList items;
    items << "Super" << "Sushi" << "Samson" << "Sucks" << "Sumo" << "Schumi";
    items << "Slashdot" << "sUpEr" << "SUshi" << "sUshi" << "sUShi";
    items << "sushI" << "SushI";
    return items;
}


int main(int argc, char **argv )
{
    KApplication app( argc, argv, "kcompletiontest" );

    Form1 *form = new Form1();
    form->show();

    return app.exec();
}


#include "kcompletiontest.moc"
