#include "perioditem.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QTimeEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QFileInfo>
#include <QDebug>
#include <QRegularExpression>

PeriodItem::~PeriodItem() {
    // Limpar widgets
    delete startTime;
    delete endTime;
    delete thumbnailLabel;
    delete imagePathLabel;
    delete selectImageButton;
    delete deleteButton;
    
    // Limpar dados
    imagePath.clear();
    displayText.clear();
}

PeriodItem::PeriodItem(QWidget *parent) : QWidget(parent) {
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(10);
    
    startTime = new QTimeEdit(this);
    endTime = new QTimeEdit(this);
    thumbnailLabel = new QLabel(this);
    thumbnailLabel->setFixedSize(80, 45);  // 16:9 ratio
    thumbnailLabel->setScaledContents(true);
    imagePathLabel = new QLabel(this);
    imagePathLabel->setWordWrap(true);
    imagePathLabel->setMinimumWidth(80);
    imagePathLabel->setMaximumWidth(150);
    imagePathLabel->setFixedHeight(40);
    imagePathLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    imagePathLabel->setStyleSheet(
        "QLabel {"
        "   padding: 5px;"
        "   background-color: palette(base);"
        "   border: 1px solid palette(mid);"
        "   border-radius: 3px;"
        "   font-size: 9pt;"
        "}"
    );
    selectImageButton = new QPushButton(tr("Select Image"), this);
    deleteButton = new QPushButton(tr("Delete"), this);
    
    layout->addWidget(new QLabel(tr("From:")));
    layout->addWidget(startTime);
    layout->addWidget(new QLabel(tr("To:")));
    layout->addWidget(endTime);
    layout->addWidget(thumbnailLabel);
    layout->addWidget(imagePathLabel);
    layout->addWidget(selectImageButton);
    layout->addWidget(deleteButton);
    
    connect(selectImageButton, &QPushButton::clicked, this, &PeriodItem::selectImage);
    connect(deleteButton, &QPushButton::clicked, this, &PeriodItem::deleteLater);
    setupConnections();
}

void PeriodItem::setupConnections() {
    // Usar QueuedConnection para evitar recursão
    connect(startTime, &QTimeEdit::timeChanged, this, &PeriodItem::onTimeChanged,
            Qt::QueuedConnection);
    connect(endTime, &QTimeEdit::timeChanged, this, &PeriodItem::onTimeChanged,
            Qt::QueuedConnection);
    
    // Remover conexões duplicadas
    disconnect(selectImageButton, &QPushButton::clicked, this, &PeriodItem::selectImage);
    disconnect(deleteButton, &QPushButton::clicked, this, &PeriodItem::deleteLater);
    
    connect(selectImageButton, &QPushButton::clicked, this, &PeriodItem::selectImage);
    connect(deleteButton, &QPushButton::clicked, this, &PeriodItem::deleteLater);
}

void PeriodItem::onTimeChanged() {
    // Evitar emitir sinal se estiver bloqueado
    if (!signalsBlocked()) {
        emit timeChanged();
    }
}

void PeriodItem::setStartTime(const QTime &time) {
    startTime->setTime(time);
    emit timeChanged();
}

void PeriodItem::setEndTime(const QTime &time) {
    endTime->setTime(time);
    emit timeChanged();
}

void PeriodItem::deleteLater() {
    emit deleted();
    QWidget::deleteLater();
}

void PeriodItem::updateThumbnail() {
    if (!imagePath.isEmpty()) {
        QPixmap thumbnail = ThumbnailCache::instance().getThumbnail(imagePath, QSize(80, 45));
        thumbnailLabel->setPixmap(thumbnail);
    }
}

void PeriodItem::updateDisplayText() {
    if (imagePath.isEmpty()) {
        imagePathLabel->setText(tr("No image selected"));
        return;
    }

    QString name = QFileInfo(imagePath).fileName();
    qDebug() << QObject::tr("Updating display - Original name:") << name;

    if (name.length() > 18) {
        name = name.left(18) + "...";
        qDebug() << QObject::tr("Updating display - Truncated name:") << name;
    }

    displayText = name;
    imagePathLabel->setText(displayText);
    imagePathLabel->setToolTip(imagePath);
}

void PeriodItem::setPeriod(const ChronoPeriod& period) {
    startTime->setTime(period.startTime);
    endTime->setTime(period.endTime);
    imagePath = period.wallpaper;
    updateDisplayText();  // Usar novo método
    updateThumbnail();
}

ChronoPeriod PeriodItem::getPeriod() const {
    ChronoPeriod period;
    period.startTime = startTime->time();
    period.endTime = endTime->time();
    period.wallpaper = imagePath;
    return period;
}

void PeriodItem::selectImage() {
    QString file = QFileDialog::getOpenFileName(this,
        tr("Select Wallpaper"),
        QDir::homePath(),
        tr("Images (*.png *.jpg *.jpeg)"));
        
    if (!file.isEmpty()) {
        imagePath = file;
        updateDisplayText();  // Usar novo método
        updateThumbnail();
    }
}
