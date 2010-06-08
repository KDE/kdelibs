#include "tagwidgettest.h"
#include "tagwidget.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <kdebug.h>


TagWidgetTest::TagWidgetTest()
    : QWidget()
{
    m_tagWidget = new Nepomuk::TagWidget(this);
    m_tagWidget->setMaxTagsShown(8);
    QVBoxLayout* lay = new QVBoxLayout(this);
    lay->addWidget(m_tagWidget);
    connect(m_tagWidget, SIGNAL(tagClicked(Nepomuk::Tag)),
            this, SLOT(slotTagClicked(Nepomuk::Tag)));
    connect(m_tagWidget, SIGNAL(selectionChanged(QList<Nepomuk::Tag>)),
            this, SLOT(slotSelectionChanged(QList<Nepomuk::Tag>)));

    QCheckBox* box = new QCheckBox( "Minimode", this );
    connect(box, SIGNAL(toggled(bool)), this, SLOT(enableMinimode(bool)));
    lay->addWidget(box);

    box = new QCheckBox( "Align Right", this );
    connect(box, SIGNAL(toggled(bool)), this, SLOT(alignRight(bool)));
    lay->addWidget(box);

    box = new QCheckBox( "Disable clicking", this );
    connect(box, SIGNAL(toggled(bool)), this, SLOT(disableClicking(bool)));
    lay->addWidget(box);

    box = new QCheckBox( "Read only", this );
    connect(box, SIGNAL(toggled(bool)), this, SLOT(setReadOnly(bool)));
    lay->addWidget(box);
}

TagWidgetTest::~TagWidgetTest()
{
}


void TagWidgetTest::slotTagClicked(const Nepomuk::Tag& tag)
{
    kDebug() << "Tag clicked:" << tag.resourceUri() << tag.genericLabel();
}


void TagWidgetTest::slotSelectionChanged( const QList<Nepomuk::Tag>& tags )
{
    QStringList ts;
    foreach(const Nepomuk::Tag& tag, tags)
        ts << tag.genericLabel();
    kDebug() << "Selection changed:" << ts;
}


void TagWidgetTest::enableMinimode( bool enable )
{
    Nepomuk::TagWidget::ModeFlags flags = m_tagWidget->modeFlags();
    if( enable ) {
        flags |= Nepomuk::TagWidget::MiniMode;
        flags &= ~Nepomuk::TagWidget::StandardMode;
    }
    else {
        flags |= Nepomuk::TagWidget::StandardMode;
        flags &= ~Nepomuk::TagWidget::MiniMode;
    }
    m_tagWidget->setModeFlags( flags );
}


void TagWidgetTest::alignRight( bool enable )
{
    if( enable )
        m_tagWidget->setAlignment( Qt::AlignRight );
    else
        m_tagWidget->setAlignment( Qt::AlignLeft );
}


void TagWidgetTest::disableClicking( bool enable )
{
    Nepomuk::TagWidget::ModeFlags flags = m_tagWidget->modeFlags();
    m_tagWidget->setModeFlags( enable ? flags | Nepomuk::TagWidget::DisableTagClicking : flags & ~Nepomuk::TagWidget::DisableTagClicking );
}


void TagWidgetTest::setReadOnly( bool enable )
{
    Nepomuk::TagWidget::ModeFlags flags = m_tagWidget->modeFlags();
    m_tagWidget->setModeFlags( enable ? flags | Nepomuk::TagWidget::ReadOnly : flags & ~Nepomuk::TagWidget::ReadOnly );
}

#include "tagwidgettest.moc"
