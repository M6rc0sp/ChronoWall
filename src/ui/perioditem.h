#pragma once

#include <QWidget>
#include <QLabel>
#include <QTimeEdit>
#include <QPushButton>
#include "core/chronoperiod.h"
#include "utils/thumbnailcache.h"

class PeriodItem : public QWidget {
    Q_OBJECT
public:
    explicit PeriodItem(QWidget *parent = nullptr);
    
    void updateThumbnail();
    void setPeriod(const ChronoPeriod &period);
    ChronoPeriod getPeriod() const;
    
    QTime getStartTime() const { return startTime->time(); }
    QTime getEndTime() const { return endTime->time(); }
    void setStartTime(const QTime &time);
    void setEndTime(const QTime &time);
    void deleteLater();

    virtual ~PeriodItem();

signals:
    void timeChanged();
    void deleted();

private slots:
    void selectImage();
    void onTimeChanged();

private:
    void setupConnections();

private:
    QTimeEdit *startTime;
    QTimeEdit *endTime;
    QLabel *thumbnailLabel;
    QLabel *imagePathLabel;
    QPushButton *selectImageButton;
    QPushButton *deleteButton;
    QString imagePath;
    QString displayText;

    void updateDisplayText();
};
