#include "mimetypeviewer.h"
#include "ui_mimetypeviewer.h"

#include <QFileDialog>
#include <QMimeDatabase>
#include <QDebug>

MimeTypeViewer::MimeTypeViewer(QWidget *theParent) :
    QWidget(theParent),
    ui(new Ui::MimeTypeViewer),
    dataBase(new QMimeDatabase)
{
    ui->setupUi(this);
    ui->addTypesButton->hide(); // hack
    connect(ui->openFileButton, SIGNAL(clicked()), SLOT(onOpenFileButtonClicked()));
}

MimeTypeViewer::~MimeTypeViewer()
{
    delete ui;
}

void MimeTypeViewer::onOpenFileButtonClicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select file"));
    if (file.isEmpty())
        return;

    ui->fileLineEdit->setText(file);
    QMimeType mimeType = dataBase->findByFile(QFileInfo(file));
    if (mimeType.isValid())
        setMimeType(mimeType);
}

void MimeTypeViewer::updateNameList()
{
    ui->nameList->clear();
    foreach (const QMimeType &mime, dataBase->allMimeTypes()) {
        ui->nameList->addItem(mime.name());
    }
    ui->nameList->sortItems();
}

void MimeTypeViewer::setMimeType(const QMimeType &mimeType)
{
    ui->mimeTypeLabel->setText(mimeType.name());
    ui->commentLabel->setText(mimeType.comment());
    ui->genericIconNameLabel->setText(mimeType.genericIconName());
    ui->iconNameLabel->setText(mimeType.iconName());

    ui->globPatternsLabel->setText(mimeType.globPatterns().join(", "));
    ui->parentMimeTypesLabel->setText(mimeType.parentMimeTypes().join(", "));
    ui->allParentMimeTypesLabel->setText(mimeType.allParentMimeTypes().join(", "));

    ui->suffixesLabel->setText(mimeType.suffixes().join(", "));
    ui->preferredSuffixLabel->setText(mimeType.preferredSuffix());
    ui->filterStringLabel->setText(mimeType.filterString());
}
