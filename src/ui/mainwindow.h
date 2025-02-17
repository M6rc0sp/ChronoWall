#pragma once

#include <QMainWindow>
#include <QVBoxLayout>
#include <QTimer>
#include <QListWidget>
#include <QFileInfo>
#include "core/chronoperiod.h"
#include "ui/perioditem.h"
#include "core/chronowallservice.h"

class ChronoWallWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit ChronoWallWindow(QWidget *parent = nullptr);
    ~ChronoWallWindow();

private slots:
    void addNewPeriod();
    void applySettings();
    void adjustPeriods();
    void onPeriodDeleted();

private:
    QVBoxLayout *m_periodsLayout;
    QTimer *m_adjustTimer;
    
    void loadSettings();
    bool validatePeriods();
};