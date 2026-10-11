QT += core gui widgets
CONFIG += c++17

INCLUDEPATH += src modules/personnel modules/livraisons

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    modules/personnel/personnel.cpp \
    modules/personnel/barchart.cpp \
    modules/livraisons/livraisons.cpp

HEADERS += \
    src/mainwindow.h \
    src/logindialog.h \
    src/style.h \
    modules/personnel/personnel.h \
    modules/personnel/barchart.h \
    modules/livraisons/livraisons.h

FORMS += \
    modules/personnel/personnel.ui \
    modules/livraisons/livraisons.ui
