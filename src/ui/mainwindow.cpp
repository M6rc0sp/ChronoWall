#include "mainwindow.h"
#include <QVBoxLayout>
#include <QToolBar>
#include <QPushButton>
#include <QScrollArea>
#include <QMessageBox>
#include "core/chronowallservice.h"

ChronoWallWindow::~ChronoWallWindow() {
    if (m_adjustTimer) {
        m_adjustTimer->stop();
        delete m_adjustTimer;
    }

    while (m_periodsLayout->count() > 0) {
        auto item = m_periodsLayout->itemAt(0)->widget();
        m_periodsLayout->removeWidget(item);
        delete item;
    }
}

ChronoWallWindow::ChronoWallWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("ChronoWall");
    setMinimumSize(800, 400);

    // Widget Central
    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    auto *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    // Barra de botões fixa (substitui a toolbar)
    auto *buttonBar = new QWidget(this);
    auto *buttonLayout = new QHBoxLayout(buttonBar);
    buttonLayout->setContentsMargins(5, 5, 5, 5);
    buttonLayout->setSpacing(5);

    auto *addButton = new QPushButton(tr("Add New Period"), this);
    buttonLayout->addWidget(addButton);
    connect(addButton, SIGNAL(clicked()), this, SLOT(addNewPeriod()));

    buttonLayout->addStretch(); // Espaço flexível à direita
    mainLayout->addWidget(buttonBar);

    // Lista de períodos
    m_periodsLayout = new QVBoxLayout;
    auto *periodsContainer = new QWidget;
    periodsContainer->setLayout(m_periodsLayout);
    
    auto *scrollArea = new QScrollArea;
    scrollArea->setWidget(periodsContainer);
    scrollArea->setWidgetResizable(true);
    mainLayout->addWidget(scrollArea);

    // Botão Aplicar
    auto *applyButton = new QPushButton(tr("Apply Settings"), this);
    applyButton->setStyleSheet("QPushButton { padding: 10px; font-weight: bold; }");
    connect(applyButton, &QPushButton::clicked, this, &ChronoWallWindow::applySettings);
    mainLayout->addWidget(applyButton);

    // Timer para debounce com intervalo menor
    m_adjustTimer = new QTimer(this);
    m_adjustTimer->setSingleShot(true);
    m_adjustTimer->setInterval(10); // Reduzido para 10ms
    connect(m_adjustTimer, &QTimer::timeout, this, &ChronoWallWindow::adjustPeriods);

    loadSettings();
}

void ChronoWallWindow::addNewPeriod() {
    try {
        int totalPeriods = m_periodsLayout->count() + 1;
        int secsPerPeriod = (24 * 3600) / totalPeriods;

        // Criar novo período
        auto newItem = new PeriodItem(this);
        connect(newItem, &PeriodItem::timeChanged, this, [this]() { 
            m_adjustTimer->start(); 
        });
        connect(newItem, &PeriodItem::deleted, this, &ChronoWallWindow::onPeriodDeleted);
        m_periodsLayout->addWidget(newItem);

        // Redistribuir TODOS os períodos igualmente
        QTime currentTime(0, 0);
        qDebug() << QObject::tr("Redistributing periods from") << currentTime.toString();

        for (int i = 0; i < m_periodsLayout->count(); ++i) {
            if (auto *item = qobject_cast<PeriodItem*>(m_periodsLayout->itemAt(i)->widget())) {
                item->blockSignals(true);
                
                QTime endTime;
                if (i == m_periodsLayout->count() - 1) {
                    endTime = QTime(0, 0);
                } else {
                    endTime = currentTime.addSecs(secsPerPeriod);
                }

                item->setStartTime(currentTime);
                item->setEndTime(endTime);
                currentTime = endTime;
                
                item->blockSignals(false);
            }
        }
        
    } catch (const std::exception &e) {
        qCritical() << QObject::tr("Error in addNewPeriod():") << e.what();
    }
}

void ChronoWallWindow::onPeriodDeleted() {
    m_adjustTimer->start();
}

void ChronoWallWindow::adjustPeriods() {
    QList<PeriodItem*> items;
    for (int i = 0; i < m_periodsLayout->count(); ++i) {
        if (auto *item = qobject_cast<PeriodItem*>(m_periodsLayout->itemAt(i)->widget())) {
            items.append(item);
        }
    }

    if (items.isEmpty()) return;

    // Propagar mudanças
    for (int i = 0; i < items.size() - 1; ++i) {
        QTime currentEnd = items[i]->getEndTime();
        QTime nextStart = items[i + 1]->getStartTime();
        
        if (currentEnd != nextStart) {
            items[i + 1]->setStartTime(currentEnd);
        }
    }

    if (items.size() > 1) {
        QTime lastEnd = items.last()->getEndTime();
        QTime firstStart = items.first()->getStartTime();
        
        if (lastEnd != firstStart) {
            items.last()->setEndTime(firstStart);
        }
    }
}

void ChronoWallWindow::applySettings() {
    if (!validatePeriods()) return;
    
    QVector<ChronoPeriod> periods;
    for(int i = 0; i < m_periodsLayout->count(); i++) {
        auto *item = qobject_cast<PeriodItem*>(m_periodsLayout->itemAt(i)->widget());
        if(item) periods.append(item->getPeriod());
    }
    
    ChronoWallService::instance().updatePeriods(periods);
    
    QMessageBox::information(this, tr("Success"), 
        tr("Settings saved and applied successfully!"));
}

void ChronoWallWindow::loadSettings() {
    const auto &periods = ChronoWallService::instance().periods();
    for(const auto &period : periods) {
        auto *item = new PeriodItem(this);
        item->setPeriod(period);
        m_periodsLayout->addWidget(item);
        connect(item, &PeriodItem::timeChanged, this, [this]() { 
            m_adjustTimer->start(); 
        });
        connect(item, &PeriodItem::deleted, this, &ChronoWallWindow::onPeriodDeleted);
    }
}

bool ChronoWallWindow::validatePeriods() {
    if (m_periodsLayout->count() == 0) {
        QMessageBox::warning(this, tr("Warning"), tr("Add at least one period."));
        return false;
    }

    QVector<QPair<QTime, QTime>> periods;
    for(int i = 0; i < m_periodsLayout->count(); i++) {
        auto *item = qobject_cast<PeriodItem*>(m_periodsLayout->itemAt(i)->widget());
        if (!item) continue;

        QTime start = item->getStartTime();
        QTime end = item->getEndTime();
        
        // Verificar se tem imagem definida
        if (item->getPeriod().wallpaper.isEmpty()) {
            QMessageBox::warning(this, tr("Warning"),
                tr("Period %1 has no image defined.").arg(i + 1));
            return false;
        }

        // Verificar continuidade com período anterior
        if (i > 0) {
            auto *prevItem = qobject_cast<PeriodItem*>(
                m_periodsLayout->itemAt(i-1)->widget());
            if (prevItem && prevItem->getEndTime() != start) {
                QMessageBox::warning(this, tr("Warning"),
                    tr("Period %1 must start when period %2 ends.")
                    .arg(i + 1).arg(i));
                return false;
            }
        }

        periods.append({start, end});
    }

    // Verificar se o último período fecha com o primeiro
    auto *firstItem = qobject_cast<PeriodItem*>(m_periodsLayout->itemAt(0)->widget());
    auto *lastItem = qobject_cast<PeriodItem*>(
        m_periodsLayout->itemAt(m_periodsLayout->count()-1)->widget());
    
    if (firstItem && lastItem) {
        if (lastItem->getEndTime() != firstItem->getStartTime()) {
            QMessageBox::warning(this, tr("Warning"),
                tr("The last period must end when the first begins to cover 24h."));
            return false;
        }
    }

    return true;
}