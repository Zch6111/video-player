#ifndef PLAYER_H
#define PLAYER_H

#include <QWidget>
#include <QLabel>
#include <QThread>
#include <QImage>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

class VideoPlayer : public QThread {
    Q_OBJECT
public:
    explicit VideoPlayer(const QString &filePath, QObject *parent = nullptr);
    ~VideoPlayer();
    void run() override;

signals:
    void frameReady(const QImage &image);

private:
    QString filePath;
    AVFormatContext *formatCtx = nullptr;
    AVCodecContext *codecCtx = nullptr;
    SwsContext *swsCtx = nullptr;
    int videoStreamIndex = -1;
};

class PlayerWidget : public QWidget {
    Q_OBJECT
public:
    explicit PlayerWidget(QWidget *parent = nullptr);
    void playVideo(const QString &filePath);

private slots:
    void updateFrame(const QImage &image);

private:
    QLabel *videoLabel;
    VideoPlayer *player = nullptr;
};

#endif // PLAYER_H
