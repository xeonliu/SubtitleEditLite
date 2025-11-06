#ifndef POINTSYNCDIALOG_H
#define POINTSYNCDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVector>
#include "subtitle.h"

struct SyncPoint {
    int sourceIndex;      // 源字幕索引（左侧）
    int referenceIndex;   // 参考字幕索引（右侧）
    QTime sourceTime;     // 源字幕时间
    QTime referenceTime;  // 参考字幕时间
    
    SyncPoint() : sourceIndex(-1), referenceIndex(-1) {}
    SyncPoint(int srcIdx, int refIdx, const QTime& srcTime, const QTime& refTime)
        : sourceIndex(srcIdx), referenceIndex(refIdx), 
          sourceTime(srcTime), referenceTime(refTime) {}
};

class PointSyncDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PointSyncDialog(const QVector<SubtitleItem>& sourceSubtitles, 
                            QWidget *parent = nullptr);
    ~PointSyncDialog();
    
    // 获取同步后的字幕
    QVector<SubtitleItem> getSyncedSubtitles() const;
    
private slots:
    void onLoadReference();
    void onSourceSelectionChanged();
    void onReferenceSelectionChanged();
    void onAddSyncPoint();
    void onRemoveSyncPoint();
    void onApply();
    void onReset();
    void onFinish();
    void onSyncPointDoubleClicked(QListWidgetItem* item);
    
private:
    void setupUI();
    void updateSourceTable();
    void updateSourceTable(const QVector<SubtitleItem>& subtitles);
    void updateReferenceTable();
    void updateSyncPointsList();
    void highlightReferenceByTime(const QTime& sourceTime);
    void applySyncTransformation();
    
    // UI组件
    QTableWidget* m_sourceTable;
    QTableWidget* m_referenceTable;
    QListWidget* m_syncPointsList;
    QPushButton* m_loadRefButton;
    QPushButton* m_addPointButton;
    QPushButton* m_removePointButton;
    QPushButton* m_applyButton;
    QPushButton* m_resetButton;
    QPushButton* m_finishButton;
    QPushButton* m_closeButton;
    
    // 数据
    QVector<SubtitleItem> m_originalSubtitles;  // 原始字幕（不变）
    QVector<SubtitleItem> m_sourceSubtitles;     // 当前显示的字幕（可能已同步）
    QVector<SubtitleItem> m_referenceSubtitles;
    QVector<SubtitleItem> m_syncedSubtitles;
    QVector<SyncPoint> m_syncPoints;
    
    bool m_applied;
};

#endif // POINTSYNCDIALOG_H
