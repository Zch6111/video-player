#include <QApplication>
#include "player.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    PlayerWidget w;
    w.resize(2, 1);
    w.show();

    w.playVideo("C:\\FFmpegPlayer\\videoplayer\\0215.mp4");
    return app.exec();
}
