#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "pointsyncdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QTimeEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_tableModel(new QStandardItemModel(this))
    , m_modified(false)
{
    ui->setupUi(this);
    
    // 设置表格模型
    m_tableModel->setHorizontalHeaderLabels({"序号", "开始时间", "结束时间", "字幕文本"});
    ui->tableView->setModel(m_tableModel);
    
    // 设置列宽
    ui->tableView->setColumnWidth(0, 60);
    ui->tableView->setColumnWidth(1, 120);
    ui->tableView->setColumnWidth(2, 120);
    ui->tableView->setColumnWidth(3, 400);
    
    // 连接信号槽
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::onSaveFile);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::onSaveAsFile);
    connect(ui->actionTimeShift, &QAction::triggered, this, &MainWindow::onTimeShift);
    connect(ui->actionPointSync, &QAction::triggered, this, &MainWindow::onPointSync);
    
    connect(m_tableModel, &QStandardItemModel::dataChanged, this, &MainWindow::onTableDataChanged);
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged, 
            this, &MainWindow::onSelectionChanged);
    
    updateWindowTitle();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onOpenFile() {
    QString filePath = QFileDialog::getOpenFileName(this, "打开SRT文件", "", "SRT文件 (*.srt);;所有文件 (*)");
    if (filePath.isEmpty()) return;
    
    loadSubtitles(filePath);
}

void MainWindow::onSaveFile() {
    if (m_currentFilePath.isEmpty()) {
        onSaveAsFile();
        return;
    }
    
    saveSubtitles(m_currentFilePath);
}

void MainWindow::onSaveAsFile() {
    QString filePath = QFileDialog::getSaveFileName(this, "保存SRT文件", "", "SRT文件 (*.srt);;所有文件 (*)");
    if (filePath.isEmpty()) return;
    
    saveSubtitles(filePath);
}

void MainWindow::onTimeShift() {
    if (m_subtitles.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先打开一个SRT文件");
        return;
    }
    
    // 创建时间偏移对话框
    QDialog dialog(this);
    dialog.setWindowTitle("时间平移");
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    QLabel* label = new QLabel("输入时间偏移量（毫秒）：", &dialog);
    layout->addWidget(label);
    
    QSpinBox* spinBox = new QSpinBox(&dialog);
    spinBox->setRange(-3600000, 3600000); // -1小时到+1小时
    spinBox->setSingleStep(100);
    spinBox->setValue(0);
    spinBox->setSuffix(" ms");
    layout->addWidget(spinBox);
    
    QLabel* infoLabel = new QLabel("正数为向后延迟，负数为向前提前", &dialog);
    infoLabel->setStyleSheet("color: gray; font-size: 10pt;");
    layout->addWidget(infoLabel);
    
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttonBox);
    
    if (dialog.exec() == QDialog::Accepted) {
        int milliseconds = spinBox->value();
        SRTParser::shiftTime(m_subtitles, milliseconds);
        updateTableView();
        setModified(true);
        ui->statusbar->showMessage(QString("已应用 %1 毫秒的时间偏移").arg(milliseconds), 3000);
    }
}

void MainWindow::onPointSync() {
    if (m_subtitles.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先打开一个SRT文件");
        return;
    }
    
    // 创建并显示点同步对话框
    PointSyncDialog dialog(m_subtitles, this);
    
    if (dialog.exec() == QDialog::Accepted) {
        m_subtitles = dialog.getSyncedSubtitles();
        updateTableView();
        setModified(true);
        ui->statusbar->showMessage("已应用点同步", 3000);
    }
}

void MainWindow::onTableDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight) {
    Q_UNUSED(bottomRight);
    
    if (topLeft.row() < 0 || topLeft.row() >= m_subtitles.size()) return;
    
    int row = topLeft.row();
    int col = topLeft.column();
    
    SubtitleItem& item = m_subtitles[row];
    QString text = m_tableModel->item(row, col)->text();
    
    bool ok;
    switch (col) {
    case 0: // 序号
        item.index = text.toInt(&ok);
        break;
    case 1: // 开始时间
        item.startTime = SRTParser::parseTime(text, ok);
        if (!ok) {
            QMessageBox::warning(this, "错误", "时间格式不正确，应为 HH:MM:SS,mmm");
            updateTableView();
            return;
        }
        break;
    case 2: // 结束时间
        item.endTime = SRTParser::parseTime(text, ok);
        if (!ok) {
            QMessageBox::warning(this, "错误", "时间格式不正确，应为 HH:MM:SS,mmm");
            updateTableView();
            return;
        }
        break;
    case 3: // 文本
        item.text = text;
        break;
    }
    
    setModified(true);
}

void MainWindow::onSelectionChanged() {
    // 可以在此处添加选中行的处理逻辑
}

void MainWindow::loadSubtitles(const QString& filePath) {
    QString errorMsg;
    if (!SRTParser::parse(filePath, m_subtitles, errorMsg)) {
        QMessageBox::critical(this, "错误", "无法加载文件：\n" + errorMsg);
        return;
    }
    
    m_currentFilePath = filePath;
    updateTableView();
    setModified(false);
    ui->statusbar->showMessage(QString("已加载 %1 条字幕").arg(m_subtitles.size()), 3000);
}

void MainWindow::saveSubtitles(const QString& filePath) {
    QString errorMsg;
    if (!SRTParser::save(filePath, m_subtitles, errorMsg)) {
        QMessageBox::critical(this, "错误", "无法保存文件：\n" + errorMsg);
        return;
    }
    
    m_currentFilePath = filePath;
    setModified(false);
    ui->statusbar->showMessage("文件已保存", 3000);
}

void MainWindow::updateTableView() {
    // 断开信号以避免触发dataChanged
    disconnect(m_tableModel, &QStandardItemModel::dataChanged, this, &MainWindow::onTableDataChanged);
    
    m_tableModel->removeRows(0, m_tableModel->rowCount());
    
    for (const SubtitleItem& item : m_subtitles) {
        QList<QStandardItem*> rowItems;
        
        QStandardItem* indexItem = new QStandardItem(QString::number(item.index));
        QStandardItem* startItem = new QStandardItem(SRTParser::formatTime(item.startTime));
        QStandardItem* endItem = new QStandardItem(SRTParser::formatTime(item.endTime));
        QStandardItem* textItem = new QStandardItem(item.text);
        
        rowItems << indexItem << startItem << endItem << textItem;
        m_tableModel->appendRow(rowItems);
    }
    
    // 重新连接信号
    connect(m_tableModel, &QStandardItemModel::dataChanged, this, &MainWindow::onTableDataChanged);
}

void MainWindow::setModified(bool modified) {
    m_modified = modified;
    updateWindowTitle();
}

void MainWindow::updateWindowTitle() {
    QString title = "SRT字幕编辑器";
    if (!m_currentFilePath.isEmpty()) {
        QFileInfo fileInfo(m_currentFilePath);
        title += " - " + fileInfo.fileName();
    }
    if (m_modified) {
        title += " *";
    }
    setWindowTitle(title);
}
