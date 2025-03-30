QT += core gui widgets
CONFIG += c++11

INCLUDEPATH += "C:/ffmpeg/include"
LIBS += -L"C:/ffmpeg/lib" -lavformat -lavcodec -lavutil -lswscale

# 添加源文件和头文件
SOURCES += \
    main.cpp \
    player.cpp

HEADERS += \
    player.h
