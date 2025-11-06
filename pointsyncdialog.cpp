#include "pointsyncdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QGroupBox>
#include <QSplitter>
#include <QColor>
#include <QBrush>
#include <cmath>

PointSyncDialog::PointSyncDialog(const QVector<SubtitleItem>& sourceSubtitles, QWidget *parent)
    : QDialog(parent)
    , m_originalSubtitles(sourceSubtitles)
    , m_sourceSubtitles(sourceSubtitles)
    , m_applied(false)
{
    setWindowTitle("点同步 - 通过参考字幕同步");
    resize(1200, 700);
    
    setupUI();
    updateSourceTable();
}

PointSyncDialog::~PointSyncDialog()
{
}

void PointSyncDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 顶部说明
    QLabel* infoLabel = new QLabel(
        "选择左侧（当前）和右侧（参考）字幕中应该同步的点，添加到中间列表。"
        "可以添加多个同步点以实现分段线性调整。", this);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #666; padding: 5px; background: #f0f0f0; border-radius: 3px;");
    mainLayout->addWidget(infoLabel);
    
    // 主要内容区域 - 使用Splitter
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    
    // 左侧 - 源字幕表格
    QWidget* leftWidget = new QWidget(this);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* sourceLabel = new QLabel("<b>当前字幕（源）</b>", leftWidget);
    leftLayout->addWidget(sourceLabel);
    
    m_sourceTable = new QTableWidget(leftWidget);
    m_sourceTable->setColumnCount(3);
    m_sourceTable->setHorizontalHeaderLabels({"序号", "时间", "文本"});
    m_sourceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_sourceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_sourceTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_sourceTable->horizontalHeader()->setStretchLastSection(true);
    m_sourceTable->setColumnWidth(0, 50);
    m_sourceTable->setColumnWidth(1, 100);
    leftLayout->addWidget(m_sourceTable);
    
    splitter->addWidget(leftWidget);
    
    // 中间 - 同步点列表和按钮
    QWidget* centerWidget = new QWidget(this);
    QVBoxLayout* centerLayout = new QVBoxLayout(centerWidget);
    centerLayout->setContentsMargins(5, 0, 5, 0);
    
    QLabel* syncLabel = new QLabel("<b>同步点</b>", centerWidget);
    centerLayout->addWidget(syncLabel);
    
    m_syncPointsList = new QListWidget(centerWidget);
    m_syncPointsList->setMaximumWidth(250);
    m_syncPointsList->setMinimumWidth(200);
    centerLayout->addWidget(m_syncPointsList);
    
    m_addPointButton = new QPushButton("添加同步点 →", centerWidget);
    m_addPointButton->setEnabled(false);
    centerLayout->addWidget(m_addPointButton);
    
    m_removePointButton = new QPushButton("移除选中", centerWidget);
    m_removePointButton->setEnabled(false);
    centerLayout->addWidget(m_removePointButton);
    
    centerLayout->addStretch();
    
    splitter->addWidget(centerWidget);
    
    // 右侧 - 参考字幕表格
    QWidget* rightWidget = new QWidget(this);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    
    QHBoxLayout* rightHeaderLayout = new QHBoxLayout();
    QLabel* refLabel = new QLabel("<b>参考字幕</b>", rightWidget);
    rightHeaderLayout->addWidget(refLabel);
    rightHeaderLayout->addStretch();
    m_loadRefButton = new QPushButton("加载参考字幕...", rightWidget);
    rightHeaderLayout->addWidget(m_loadRefButton);
    rightLayout->addLayout(rightHeaderLayout);
    
    m_referenceTable = new QTableWidget(rightWidget);
    m_referenceTable->setColumnCount(3);
    m_referenceTable->setHorizontalHeaderLabels({"序号", "时间", "文本"});
    m_referenceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_referenceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_referenceTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_referenceTable->horizontalHeader()->setStretchLastSection(true);
    m_referenceTable->setColumnWidth(0, 50);
    m_referenceTable->setColumnWidth(1, 100);
    rightLayout->addWidget(m_referenceTable);
    
    splitter->addWidget(rightWidget);
    
    // 设置splitter比例
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    splitter->setStretchFactor(2, 3);
    
    mainLayout->addWidget(splitter);
    
    // 底部按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_resetButton = new QPushButton("重置 (Reset)", this);
    m_resetButton->setEnabled(false);
    buttonLayout->addWidget(m_resetButton);
    
    buttonLayout->addStretch();
    
    m_applyButton = new QPushButton("应用预览 (Apply)", this);
    m_applyButton->setEnabled(false);
    m_applyButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 8px 20px; font-weight: bold; }");
    buttonLayout->addWidget(m_applyButton);
    
    m_finishButton = new QPushButton("完成 (Finish)", this);
    m_finishButton->setEnabled(false);
    m_finishButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 8px 20px; font-weight: bold; }");
    buttonLayout->addWidget(m_finishButton);
    
    m_closeButton = new QPushButton("取消", this);
    buttonLayout->addWidget(m_closeButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号槽
    connect(m_loadRefButton, &QPushButton::clicked, this, &PointSyncDialog::onLoadReference);
    connect(m_addPointButton, &QPushButton::clicked, this, &PointSyncDialog::onAddSyncPoint);
    connect(m_removePointButton, &QPushButton::clicked, this, &PointSyncDialog::onRemoveSyncPoint);
    connect(m_applyButton, &QPushButton::clicked, this, &PointSyncDialog::onApply);
    connect(m_resetButton, &QPushButton::clicked, this, &PointSyncDialog::onReset);
    connect(m_finishButton, &QPushButton::clicked, this, &PointSyncDialog::onFinish);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::reject);
    
    connect(m_sourceTable, &QTableWidget::itemSelectionChanged, 
            this, &PointSyncDialog::onSourceSelectionChanged);
    connect(m_referenceTable, &QTableWidget::itemSelectionChanged, 
            this, &PointSyncDialog::onReferenceSelectionChanged);
    connect(m_syncPointsList, &QListWidget::itemDoubleClicked,
            this, &PointSyncDialog::onSyncPointDoubleClicked);
}

void PointSyncDialog::updateSourceTable()
{
    updateSourceTable(m_sourceSubtitles);
}

void PointSyncDialog::updateSourceTable(const QVector<SubtitleItem>& subtitles)
{
    m_sourceTable->setRowCount(0);
    
    for (int i = 0; i < subtitles.size(); ++i) {
        const SubtitleItem& item = subtitles[i];
        
        int row = m_sourceTable->rowCount();
        m_sourceTable->insertRow(row);
        
        m_sourceTable->setItem(row, 0, new QTableWidgetItem(QString::number(i + 1)));
        m_sourceTable->setItem(row, 1, new QTableWidgetItem(SRTParser::formatTime(item.startTime)));
        m_sourceTable->setItem(row, 2, new QTableWidgetItem(item.text.left(50) + (item.text.length() > 50 ? "..." : "")));
    }
}

void PointSyncDialog::updateReferenceTable()
{
    m_referenceTable->setRowCount(0);
    
    for (int i = 0; i < m_referenceSubtitles.size(); ++i) {
        const SubtitleItem& item = m_referenceSubtitles[i];
        
        int row = m_referenceTable->rowCount();
        m_referenceTable->insertRow(row);
        
        m_referenceTable->setItem(row, 0, new QTableWidgetItem(QString::number(i + 1)));
        m_referenceTable->setItem(row, 1, new QTableWidgetItem(SRTParser::formatTime(item.startTime)));
        m_referenceTable->setItem(row, 2, new QTableWidgetItem(item.text.left(50) + (item.text.length() > 50 ? "..." : "")));
    }
}

void PointSyncDialog::updateSyncPointsList()
{
    m_syncPointsList->clear();
    
    for (int i = 0; i < m_syncPoints.size(); ++i) {
        const SyncPoint& sp = m_syncPoints[i];
        QString text = QString("点%1: [%2] %3 ↔ [%4] %5")
            .arg(i + 1)
            .arg(sp.sourceIndex + 1)
            .arg(SRTParser::formatTime(sp.sourceTime))
            .arg(sp.referenceIndex + 1)
            .arg(SRTParser::formatTime(sp.referenceTime));
        m_syncPointsList->addItem(text);
    }
    
    m_removePointButton->setEnabled(!m_syncPoints.isEmpty());
    m_applyButton->setEnabled(m_syncPoints.size() >= 2);
}

void PointSyncDialog::onLoadReference()
{
    QString filePath = QFileDialog::getOpenFileName(this, "加载参考字幕", "", "SRT文件 (*.srt);;所有文件 (*)");
    if (filePath.isEmpty()) return;
    
    QString errorMsg;
    if (!SRTParser::parse(filePath, m_referenceSubtitles, errorMsg)) {
        QMessageBox::critical(this, "错误", "无法加载参考字幕：\n" + errorMsg);
        return;
    }
    
    updateReferenceTable();
    QMessageBox::information(this, "成功", QString("已加载 %1 条参考字幕").arg(m_referenceSubtitles.size()));
}

void PointSyncDialog::onSourceSelectionChanged()
{
    QList<QTableWidgetItem*> selected = m_sourceTable->selectedItems();
    if (selected.isEmpty()) {
        m_addPointButton->setEnabled(false);
        return;
    }
    
    int row = selected[0]->row();
    if (row >= 0 && row < m_sourceSubtitles.size()) {
        QTime sourceTime = m_sourceSubtitles[row].startTime;
        highlightReferenceByTime(sourceTime);
        
        // 检查是否可以添加同步点
        QList<QTableWidgetItem*> refSelected = m_referenceTable->selectedItems();
        m_addPointButton->setEnabled(!refSelected.isEmpty());
    }
}

void PointSyncDialog::onReferenceSelectionChanged()
{
    QList<QTableWidgetItem*> srcSelected = m_sourceTable->selectedItems();
    QList<QTableWidgetItem*> refSelected = m_referenceTable->selectedItems();
    
    m_addPointButton->setEnabled(!srcSelected.isEmpty() && !refSelected.isEmpty());
}

void PointSyncDialog::highlightReferenceByTime(const QTime& sourceTime)
{
    if (m_referenceSubtitles.isEmpty()) return;
    
    // 先清除所有高亮
    for (int i = 0; i < m_referenceTable->rowCount(); ++i) {
        for (int col = 0; col < m_referenceTable->columnCount(); ++col) {
            QTableWidgetItem* item = m_referenceTable->item(i, col);
            if (item) {
                item->setBackground(QBrush(Qt::white));
            }
        }
    }
    
    // 计算每个参考字幕与源字幕的时间差（绝对值）
    QVector<QPair<int, int>> timeDiffs; // <行号, 时间差毫秒>
    
    for (int i = 0; i < m_referenceSubtitles.size(); ++i) {
        int diff = std::abs(sourceTime.msecsTo(m_referenceSubtitles[i].startTime));
        timeDiffs.append(qMakePair(i, diff));
    }
    
    // 找到最小时间差
    int minDiff = timeDiffs[0].second;
    for (const auto& pair : timeDiffs) {
        minDiff = qMin(minDiff, pair.second);
    }
    
    // 如果最小时间差超过30秒，说明根本不可能匹配，不高亮任何行
    if (minDiff > 30000) {
        return;
    }
    
    // 计算时间差的标准差（用于确定高亮范围）
    // 只考虑时间差较小的那些候选项
    QVector<int> candidateDiffs;
    for (const auto& pair : timeDiffs) {
        if (pair.second <= minDiff + 10000) { // 只取最小值+10秒内的
            candidateDiffs.append(pair.second);
        }
    }
    
    // 计算平均值和标准差
    double sum = 0;
    for (int diff : candidateDiffs) {
        sum += diff;
    }
    double mean = candidateDiffs.isEmpty() ? 0 : sum / candidateDiffs.size();
    
    double variance = 0;
    for (int diff : candidateDiffs) {
        variance += (diff - mean) * (diff - mean);
    }
    double stdDev = candidateDiffs.size() <= 1 ? 1000 : std::sqrt(variance / candidateDiffs.size());
    
    // 使用高斯分布的思想：计算每个候选项的"概率"
    // 只高亮在 minDiff + 2*stdDev 范围内的，且概率总和归一化
    double totalProbability = 0;
    QVector<QPair<int, double>> probabilities; // <行号, 概率>
    
    double threshold = minDiff + std::max(2.0 * stdDev, 3000.0); // 至少3秒阈值
    
    for (const auto& pair : timeDiffs) {
        int diff = pair.second;
        if (diff <= threshold) {
            // 使用指数衰减函数计算"概率"（越近概率越高）
            // probability = exp(-diff^2 / (2 * scale^2))
            double scale = std::max(stdDev, 1000.0); // 至少1秒的scale
            double prob = std::exp(-std::pow(diff / scale, 2) / 2.0);
            probabilities.append(qMakePair(pair.first, prob));
            totalProbability += prob;
        }
    }
    
    // 归一化概率，只有当累积概率达到0.95时才停止（保留最可能的候选项）
    // 按概率从大到小排序
    std::sort(probabilities.begin(), probabilities.end(), 
              [](const QPair<int, double>& a, const QPair<int, double>& b) {
                  return a.second > b.second;
              });
    
    double cumulativeProbability = 0;
    QVector<QPair<int, double>> selectedCandidates;
    
    for (const auto& prob : probabilities) {
        if (cumulativeProbability >= 0.95 && selectedCandidates.size() >= 3) {
            break; // 已经包含了95%的可能性，且至少有3个候选
        }
        selectedCandidates.append(prob);
        cumulativeProbability += prob.second / totalProbability;
    }
    
    // 设置颜色 - 只给筛选出的候选项上色
    for (const auto& candidate : selectedCandidates) {
        int rowIndex = candidate.first;
        double normalizedProb = candidate.second / (selectedCandidates.isEmpty() ? 1.0 : selectedCandidates[0].second);
        
        // 根据归一化概率设置颜色深度
        int alpha = static_cast<int>(80 + normalizedProb * 170); // 80-250
        QColor color(100, 150, 255, alpha);
        
        for (int col = 0; col < m_referenceTable->columnCount(); ++col) {
            QTableWidgetItem* item = m_referenceTable->item(rowIndex, col);
            if (item) {
                item->setBackground(QBrush(color));
            }
        }
    }
}

void PointSyncDialog::onAddSyncPoint()
{
    QList<QTableWidgetItem*> srcSelected = m_sourceTable->selectedItems();
    QList<QTableWidgetItem*> refSelected = m_referenceTable->selectedItems();
    
    if (srcSelected.isEmpty() || refSelected.isEmpty()) return;
    
    int srcRow = srcSelected[0]->row();
    int refRow = refSelected[0]->row();
    
    if (srcRow < 0 || srcRow >= m_sourceSubtitles.size() ||
        refRow < 0 || refRow >= m_referenceSubtitles.size()) {
        return;
    }
    
    // 检查是否已存在相同的源索引
    for (const SyncPoint& sp : m_syncPoints) {
        if (sp.sourceIndex == srcRow) {
            QMessageBox::warning(this, "警告", "该源字幕已经有同步点了，请先移除或选择其他字幕。");
            return;
        }
    }
    
    // 使用原始字幕的时间作为同步点，这样重复应用时不会有问题
    SyncPoint sp(srcRow, refRow, 
                 m_originalSubtitles[srcRow].startTime,
                 m_referenceSubtitles[refRow].startTime);
    
    // 按源时间排序插入
    int insertPos = 0;
    for (int i = 0; i < m_syncPoints.size(); ++i) {
        if (m_syncPoints[i].sourceTime < sp.sourceTime) {
            insertPos = i + 1;
        }
    }
    m_syncPoints.insert(insertPos, sp);
    
    updateSyncPointsList();
}

void PointSyncDialog::onRemoveSyncPoint()
{
    QListWidgetItem* selected = m_syncPointsList->currentItem();
    if (!selected) return;
    
    int index = m_syncPointsList->row(selected);
    if (index >= 0 && index < m_syncPoints.size()) {
        m_syncPoints.removeAt(index);
        updateSyncPointsList();
    }
}

void PointSyncDialog::onSyncPointDoubleClicked(QListWidgetItem* item)
{
    int index = m_syncPointsList->row(item);
    if (index >= 0 && index < m_syncPoints.size()) {
        const SyncPoint& sp = m_syncPoints[index];
        m_sourceTable->selectRow(sp.sourceIndex);
        m_referenceTable->selectRow(sp.referenceIndex);
    }
}

void PointSyncDialog::onApply()
{
    if (m_syncPoints.size() < 2) {
        QMessageBox::warning(this, "警告", "至少需要2个同步点才能进行同步");
        return;
    }
    
    applySyncTransformation();
    
    // 更新左侧表格显示同步后的效果
    m_sourceSubtitles = m_syncedSubtitles;
    updateSourceTable(m_syncedSubtitles);
    
    // 启用重置和完成按钮
    m_resetButton->setEnabled(true);
    m_finishButton->setEnabled(true);
    
    // 清除表格选择，避免高亮混淆
    m_sourceTable->clearSelection();
    m_referenceTable->clearSelection();
    
    // 状态栏提示（如果有的话）
    QMessageBox::information(this, "预览成功", 
        QString("已应用 %1 个同步点的分段线性变换\n"
                "左侧表格已更新显示同步后的时间\n"
                "您可以继续调整同步点，或点击'完成'按钮确认")
        .arg(m_syncPoints.size()));
}

void PointSyncDialog::onReset()
{
    // 重置到原始状态
    m_sourceSubtitles = m_originalSubtitles;
    m_syncedSubtitles.clear();
    updateSourceTable(m_originalSubtitles);
    
    m_resetButton->setEnabled(false);
    m_finishButton->setEnabled(false);
    
    QMessageBox::information(this, "已重置", "已恢复到原始字幕状态");
}

void PointSyncDialog::onFinish()
{
    if (!m_finishButton->isEnabled()) {
        QMessageBox::warning(this, "提示", "请先应用同步后再完成");
        return;
    }
    
    m_applied = true;
    accept();
}

void PointSyncDialog::applySyncTransformation()
{
    // 总是从原始字幕开始计算，这样可以重复应用而不累积误差
    m_syncedSubtitles = m_originalSubtitles;
    
    // 分段线性变换
    for (int i = 0; i < m_syncedSubtitles.size(); ++i) {
        SubtitleItem& item = m_syncedSubtitles[i];
        QTime oldStartTime = item.startTime;
        QTime oldEndTime = item.endTime;
        
        // 找到当前字幕所在的段
        int segmentIndex = -1;
        for (int j = 0; j < m_syncPoints.size() - 1; ++j) {
            if (i >= m_syncPoints[j].sourceIndex && i <= m_syncPoints[j + 1].sourceIndex) {
                segmentIndex = j;
                break;
            }
        }
        
        if (segmentIndex == -1) {
            // 在第一个点之前
            if (i < m_syncPoints[0].sourceIndex) {
                segmentIndex = -1; // 使用第一个点外推
            }
            // 在最后一个点之后
            else if (i > m_syncPoints.last().sourceIndex) {
                segmentIndex = m_syncPoints.size() - 1; // 使用最后一个点外推
            }
        }
        
        // 应用线性变换
        if (segmentIndex == -1) {
            // 第一个点之前：使用前两个点的斜率外推
            const SyncPoint& p1 = m_syncPoints[0];
            const SyncPoint& p2 = m_syncPoints[1];
            
            int oldDiff = p1.sourceTime.msecsTo(p2.sourceTime);
            int newDiff = p1.referenceTime.msecsTo(p2.referenceTime);
            
            if (oldDiff != 0) {
                double scale = static_cast<double>(newDiff) / oldDiff;
                int offsetFromP1 = p1.sourceTime.msecsTo(oldStartTime);
                int newOffsetFromP1 = static_cast<int>(offsetFromP1 * scale);
                
                item.startTime = p1.referenceTime.addMSecs(newOffsetFromP1);
                item.endTime = item.startTime.addMSecs(item.duration());
            }
        }
        else if (segmentIndex == m_syncPoints.size() - 1) {
            // 最后一个点之后：使用最后两个点的斜率外推
            const SyncPoint& p1 = m_syncPoints[m_syncPoints.size() - 2];
            const SyncPoint& p2 = m_syncPoints[m_syncPoints.size() - 1];
            
            int oldDiff = p1.sourceTime.msecsTo(p2.sourceTime);
            int newDiff = p1.referenceTime.msecsTo(p2.referenceTime);
            
            if (oldDiff != 0) {
                double scale = static_cast<double>(newDiff) / oldDiff;
                int offsetFromP2 = p2.sourceTime.msecsTo(oldStartTime);
                int newOffsetFromP2 = static_cast<int>(offsetFromP2 * scale);
                
                item.startTime = p2.referenceTime.addMSecs(newOffsetFromP2);
                item.endTime = item.startTime.addMSecs(item.duration());
            }
        }
        else {
            // 在两个点之间：线性插值
            const SyncPoint& p1 = m_syncPoints[segmentIndex];
            const SyncPoint& p2 = m_syncPoints[segmentIndex + 1];
            
            int oldDiff = p1.sourceTime.msecsTo(p2.sourceTime);
            int newDiff = p1.referenceTime.msecsTo(p2.referenceTime);
            
            if (oldDiff != 0) {
                double scale = static_cast<double>(newDiff) / oldDiff;
                int offsetFromP1 = p1.sourceTime.msecsTo(oldStartTime);
                int newOffsetFromP1 = static_cast<int>(offsetFromP1 * scale);
                
                item.startTime = p1.referenceTime.addMSecs(newOffsetFromP1);
                item.endTime = item.startTime.addMSecs(item.duration());
            }
        }
    }
}

QVector<SubtitleItem> PointSyncDialog::getSyncedSubtitles() const
{
    return m_applied ? m_syncedSubtitles : m_sourceSubtitles;
}
