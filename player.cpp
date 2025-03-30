#include "player.h"
#include <QVBoxLayout>
#include <QDebug>

VideoPlayer::VideoPlayer(const QString &filePath, QObject *parent)
    : QThread(parent), filePath(filePath) {
    avformat_network_init();
}

VideoPlayer::~VideoPlayer() {
    if (codecCtx) avcodec_free_context(&codecCtx);
    if (formatCtx) avformat_close_input(&formatCtx);
    if (swsCtx) sws_freeContext(swsCtx);
}

void VideoPlayer::run() {
    // 打开视频文件
    if (avformat_open_input(&formatCtx, filePath.toUtf8().data(), nullptr, nullptr) != 0) {
        qDebug() << "无法打开视频文件:" << filePath;
        return;
    }
    if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
        qDebug() << "无法获取视频流信息";
        return;
    }

    // 查找视频流
    for (unsigned i = 0; i < formatCtx->nb_streams; i++) {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }
    if (videoStreamIndex == -1) {
        qDebug() << "未找到视频流";
        return;
    }

    // 查找解码器并打开
    const AVCodec *codec = avcodec_find_decoder(formatCtx->streams[videoStreamIndex]->codecpar->codec_id);

    if (!codec) {
        qDebug() << "未找到合适的解码器";
        return;
    }
    codecCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecCtx, formatCtx->streams[videoStreamIndex]->codecpar);
    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        qDebug() << "解码器无法打开";
        return;
    }

    // 初始化缩放上下文，将原始帧转换为 RGB 格式
    swsCtx = sws_getContext(codecCtx->width, codecCtx->height, codecCtx->pix_fmt,
                            codecCtx->width, codecCtx->height, AV_PIX_FMT_RGB24,
                            SWS_BILINEAR, nullptr, nullptr, nullptr);

    AVPacket packet;
    AVFrame *frame = av_frame_alloc();
    AVFrame *rgbFrame = av_frame_alloc();
    int bufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, codecCtx->width, codecCtx->height, 1);
    uint8_t *buffer = (uint8_t *)av_malloc(bufferSize);
    av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer, AV_PIX_FMT_RGB24,
                         codecCtx->width, codecCtx->height, 1);

    // 读取视频帧并解码
    while (av_read_frame(formatCtx, &packet) >= 0) {
        if (packet.stream_index == videoStreamIndex) {
            if (avcodec_send_packet(codecCtx, &packet) == 0) {
                while (avcodec_receive_frame(codecCtx, frame) == 0) {
                    // 转换为 RGB
                    sws_scale(swsCtx, frame->data, frame->linesize, 0, codecCtx->height,
                              rgbFrame->data, rgbFrame->linesize);

                    QImage image(rgbFrame->data[0], codecCtx->width, codecCtx->height,
                                 rgbFrame->linesize[0], QImage::Format_RGB888);
                    emit frameReady(image.copy());  // 拷贝数据，防止数据被覆盖

                    msleep(40);  // 简单控制帧率，约 25 FPS
                }
            }
        }
        av_packet_unref(&packet);
    }

    av_free(buffer);
    av_frame_free(&frame);
    av_frame_free(&rgbFrame);
}

PlayerWidget::PlayerWidget(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    videoLabel = new QLabel(this);
    videoLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(videoLabel);
    setLayout(layout);
}

void PlayerWidget::playVideo(const QString &filePath) {
    if (player) {
        player->quit();
        player->wait();
        delete player;
    }
    player = new VideoPlayer(filePath, this);
    connect(player, &VideoPlayer::frameReady, this, &PlayerWidget::updateFrame);
    player->start();
}

void PlayerWidget::updateFrame(const QImage &image) {
    videoLabel->setPixmap(QPixmap::fromImage(image));
}
