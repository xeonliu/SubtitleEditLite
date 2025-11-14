#ifndef SUBTITLE_H
#define SUBTITLE_H

#include <QString>
#include <QTime>
#include <QVector>

enum class SubtitleEncoding {
    Utf8,
    Gbk
};

struct SubtitleItem {
    int index;
    QTime startTime;
    QTime endTime;
    QString text;
    
    SubtitleItem() : index(0) {}
    SubtitleItem(int idx, const QTime& start, const QTime& end, const QString& txt)
        : index(idx), startTime(start), endTime(end), text(txt) {}
    
    // 计算时长（毫秒）
    int duration() const {
        return startTime.msecsTo(endTime);
    }
};

class SRTParser {
public:
    // 解析SRT文件
    static bool parse(const QString& filePath,
                      QVector<SubtitleItem>& subtitles,
                      QString& errorMsg,
                      SubtitleEncoding encoding = SubtitleEncoding::Utf8);
    
    // 保存为SRT文件
    static bool save(const QString& filePath, const QVector<SubtitleItem>& subtitles, QString& errorMsg);
    
    // 时间字符串转QTime
    static QTime parseTime(const QString& timeStr, bool& ok);
    
    // QTime转时间字符串
    static QString formatTime(const QTime& time);
    
    // 偏移所有字幕时间
    static void shiftTime(QVector<SubtitleItem>& subtitles, int milliseconds);
    
    // Point Sync: 使用两个同步点调整时间
    static void pointSync(QVector<SubtitleItem>& subtitles, 
                         int point1Index, const QTime& newPoint1Time,
                         int point2Index, const QTime& newPoint2Time);
};

#endif // SUBTITLE_H
