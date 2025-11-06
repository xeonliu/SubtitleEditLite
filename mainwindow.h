#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QStandardItemModel>
#include "subtitle.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 文件操作
    void onOpenFile();
    void onSaveFile();
    void onSaveAsFile();
    
    // 编辑操作
    void onTimeShift();
    void onPointSync();
    
    // 表格编辑
    void onTableDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void onSelectionChanged();
    
private:
    Ui::MainWindow *ui;
    
    QVector<SubtitleItem> m_subtitles;
    QStandardItemModel* m_tableModel;
    QString m_currentFilePath;
    bool m_modified;
    
    // 辅助函数
    void loadSubtitles(const QString& filePath);
    void saveSubtitles(const QString& filePath);
    void updateTableView();
    void setModified(bool modified);
    void updateWindowTitle();
};
#endif // MAINWINDOW_H
