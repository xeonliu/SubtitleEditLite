#include "subtitle.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QByteArray>
#include <QStringDecoder>
#include <QStringConverter>

namespace {

QStringDecoder createDecoderForEncoding(SubtitleEncoding encoding) {
    switch (encoding) {
    case SubtitleEncoding::Utf8:
        return QStringDecoder(QStringConverter::Utf8);
    case SubtitleEncoding::Gbk: {
        QStringDecoder decoder("GB18030");
        if (decoder.isValid()) {
            return decoder;
        }
        return QStringDecoder("GBK");
    }
    }
    return QStringDecoder();
}

}

bool SRTParser::parse(const QString& filePath,
                      QVector<SubtitleItem>& subtitles,
                      QString& errorMsg,
                      SubtitleEncoding encoding) {
    subtitles.clear();
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorMsg = "无法打开文件: " + filePath;
        return false;
    }
    
    QByteArray rawData = file.readAll();
    file.close();
    
    QStringDecoder decoder = createDecoderForEncoding(encoding);
    if (!decoder.isValid()) {
        errorMsg = "当前Qt环境不支持所选编码（可能缺少ICU支持）";
        return false;
    }
    
    QString content = decoder.decode(rawData);
    if (decoder.hasError()) {
        errorMsg = "解码字幕内容时出错，请确认文件编码";
        return false;
    }
    
    // 按空行分割字幕块
    QStringList blocks = content.split(QRegularExpression("\\n\\s*\\n"), Qt::SkipEmptyParts);
    
    for (const QString& block : blocks) {
        QStringList lines = block.split('\n', Qt::SkipEmptyParts);
        if (lines.size() < 3) continue;
        
        // 第一行：序号
        bool ok;
        int index = lines[0].trimmed().toInt(&ok);
        if (!ok) continue;
        
        // 第二行：时间戳
        QString timeLine = lines[1].trimmed();
        QRegularExpression timeRegex("(\\d{2}:\\d{2}:\\d{2},\\d{3})\\s*-->\\s*(\\d{2}:\\d{2}:\\d{2},\\d{3})");
        QRegularExpressionMatch match = timeRegex.match(timeLine);
        
        if (!match.hasMatch()) continue;
        
        QTime startTime = parseTime(match.captured(1), ok);
        if (!ok) continue;
        
        QTime endTime = parseTime(match.captured(2), ok);
        if (!ok) continue;
        
        // 剩余行：字幕文本
        QStringList textLines = lines.mid(2);
        QString text = textLines.join("\n");
        
        subtitles.append(SubtitleItem(index, startTime, endTime, text));
    }
    
    if (subtitles.isEmpty()) {
        errorMsg = "未找到有效的字幕条目";
        return false;
    }
    
    return true;
}

bool SRTParser::save(const QString& filePath, const QVector<SubtitleItem>& subtitles, QString& errorMsg) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        errorMsg = "无法保存文件: " + filePath;
        return false;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    
    for (int i = 0; i < subtitles.size(); ++i) {
        const SubtitleItem& item = subtitles[i];
        
        // 序号
        out << (i + 1) << "\n";
        
        // 时间戳
        out << formatTime(item.startTime) << " --> " << formatTime(item.endTime) << "\n";
        
        // 文本
        out << item.text << "\n";
        
        // 空行分隔
        if (i < subtitles.size() - 1) {
            out << "\n";
        }
    }
    
    file.close();
    return true;
}

QTime SRTParser::parseTime(const QString& timeStr, bool& ok) {
    ok = false;
    
    // 格式: HH:MM:SS,mmm
    QRegularExpression regex("(\\d{2}):(\\d{2}):(\\d{2}),(\\d{3})");
    QRegularExpressionMatch match = regex.match(timeStr);
    
    if (!match.hasMatch()) return QTime();
    
    int hours = match.captured(1).toInt();
    int minutes = match.captured(2).toInt();
    int seconds = match.captured(3).toInt();
    int milliseconds = match.captured(4).toInt();
    
    QTime time(hours, minutes, seconds, milliseconds);
    if (!time.isValid()) return QTime();
    
    ok = true;
    return time;
}

QString SRTParser::formatTime(const QTime& time) {
    // 格式: HH:MM:SS,mmm
    return QString("%1:%2:%3,%4")
        .arg(time.hour(), 2, 10, QChar('0'))
        .arg(time.minute(), 2, 10, QChar('0'))
        .arg(time.second(), 2, 10, QChar('0'))
        .arg(time.msec(), 3, 10, QChar('0'));
}

void SRTParser::shiftTime(QVector<SubtitleItem>& subtitles, int milliseconds) {
    for (SubtitleItem& item : subtitles) {
        item.startTime = item.startTime.addMSecs(milliseconds);
        item.endTime = item.endTime.addMSecs(milliseconds);
    }
}

void SRTParser::pointSync(QVector<SubtitleItem>& subtitles, 
                          int point1Index, const QTime& newPoint1Time,
                          int point2Index, const QTime& newPoint2Time) {
    if (point1Index < 0 || point1Index >= subtitles.size() ||
        point2Index < 0 || point2Index >= subtitles.size() ||
        point1Index == point2Index) {
        return;
    }
    
    // 获取原始时间点（使用开始时间）
    QTime oldPoint1Time = subtitles[point1Index].startTime;
    QTime oldPoint2Time = subtitles[point2Index].startTime;
    
    // 计算原始时间差和新时间差（毫秒）
    int oldDiff = oldPoint1Time.msecsTo(oldPoint2Time);
    int newDiff = newPoint1Time.msecsTo(newPoint2Time);
    
    if (oldDiff == 0) return; // 避免除零
    
    // 计算缩放比例
    double scale = static_cast<double>(newDiff) / oldDiff;
    
    // 调整所有字幕
    for (SubtitleItem& item : subtitles) {
        // 计算相对于point1的原始偏移
        int startOffset = oldPoint1Time.msecsTo(item.startTime);
        int endOffset = oldPoint1Time.msecsTo(item.endTime);
        
        // 应用缩放和新的基准点
        int newStartOffset = static_cast<int>(startOffset * scale);
        int newEndOffset = static_cast<int>(endOffset * scale);
        
        item.startTime = newPoint1Time.addMSecs(newStartOffset);
        item.endTime = newPoint1Time.addMSecs(newEndOffset);
    }
}
