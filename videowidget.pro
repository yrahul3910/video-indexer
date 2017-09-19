TEMPLATE = app
TARGET = videowidget

QT += multimedia multimediawidgets

HEADERS = \
    videoplayer.h

SOURCES = \
    main.cpp \
    videoplayer.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/multimediawidgets/videowidget
INSTALLS += target

QT+=widgets

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += pocketsphinx

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += sphinxbase

unix: PKGCONFIG += libavcodec

unix: PKGCONFIG += libavformat

unix: PKGCONFIG += libswresample

unix: PKGCONFIG += libavdevice

