#-------------------------------------------------
#
# Project created by QtCreator 2018-10-25T16:00:52
#
#-------------------------------------------------

QT       += core gui charts xml network

greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

TARGET = mct
TEMPLATE = app
#qt >= 5.10
VERSION = 00.00.01.06

CONFIG += C++11 #use lambda

#release:DEFINES += QT_NO_WARNING_OUTPUT \
#                   QT_NO_DEBUG_OUTPUT

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += ../include/mystd.h           \
    ../include/mydebug.h                \
    ../include/mydef.h                  \
    ../source/device/bus.h              \
    ../source/device/device.h           \
    ../source/device/MegaGateway.h      \
    ../source/device/megarobot.h        \
    ../source/device/mrqdevice.h        \
    ../source/device/storage.h          \
    ../source/device/errorcode.h        \
    ../source/device/export.h           \
    ../source/device/stdafx.h           \
    ../source/device/system.h           \
    ../source/sys/sysapi.h              \
    ../source/sys/megaxml.h             \
    ../source/sys/sacsvstream.h         \
    ../source/sys/xthread.h             \
    ../source/mrp/mdatarow.h            \
    ../source/mrp/mdataset.h            \
    ../source/model/megatablemodel.h    \
    ../source/model/h2actionmodel.h     \
    ../source/model/h2actionitem.h      \
    ../source/model/comboxdelegate.h    \
    ../source/model/checkdelegate.h     \
    ../source/model/doublespinboxdelegate.h \
    ../source/model/diagnosisitem.h     \
    ../source/model/diagnosismodel.h    \
    ../source/model/errmgritem.h        \
    ../source/model/errmgrmodel.h       \
    ../source/model/debugitem.h         \
    ../source/model/debugmodel.h        \
    ../source/wnd/mainwindow.h          \
    ../source/wnd/h2ops.h               \
    ../source/wnd/h2action.h            \
    ../source/wnd/aboutdlg.h            \
    ../source/wnd/h2status.h            \
    ../source/wnd/h2product.h           \
    ../source/wnd/roboconfig.h          \
    ../source/wnd/xrobo.h               \
    ../source/wnd/h2robo.h              \
    ../source/wnd/h2configuration.h     \
    ../source/wnd/h2errmgr.h            \
    ../source/wnd/h2jogmode.h           \
    ../source/wnd/h2homing.h            \
    ../source/wnd/helppanel.h           \
    ../source/wnd/xconfig.h             \
    ../source/wnd/h2measurement.h       \
    ../source/widget/megalcdnumber.h    \
    ../source/widget/megamessagebox.h   \
    ../source/widget/megasplinechart.h  \
    ../source/widget/megainterface.h

SOURCES += ../source/main/main.cpp      \
    ../source/device/bus.c              \
    ../source/device/device.c           \
    ../source/device/megarobot.c        \
    ../source/device/mrqdevice.c        \
    ../source/device/storage.c          \
    ../source/device/errorcode.c        \
    ../source/device/system.c           \
    ../source/sys/sysapi.cpp            \
    ../source/sys/megaxml.cpp           \
    ../source/sys/sacsvstream.cpp       \
    ../source/sys/xthread.cpp           \
    ../source/mrp/mdatarow.cpp          \
    ../source/mrp/mdataset.cpp          \
    ../source/model/diagnosisitem.cpp   \
    ../source/model/diagnosismodel.cpp  \
    ../source/model/errmgritem.cpp      \
    ../source/model/errmgrmodel.cpp     \
    ../source/model/debugitem.cpp       \
    ../source/model/debugmodel.cpp      \
    ../source/model/megatablemodel.cpp  \
    ../source/model/h2actionmodel.cpp   \
    ../source/model/h2actionitem.cpp    \
    ../source/model/comboxdelegate.cpp  \
    ../source/model/checkdelegate.cpp   \
    ../source/model/doublespinboxdelegate.cpp \
    ../source/wnd/mainwindow.cpp        \
    ../source/wnd/h2ops.cpp             \
    ../source/wnd/h2action.cpp          \
    ../source/wnd/aboutdlg.cpp          \
    ../source/wnd/h2status.cpp          \
    ../source/wnd/h2configuration.cpp   \
    ../source/wnd/h2errmgr.cpp          \
    ../source/wnd/h2jogmode.cpp         \
    ../source/wnd/h2homing.cpp          \
    ../source/wnd/helppanel.cpp         \
    ../source/wnd/xconfig.cpp           \
    ../source/wnd/h2measurement.cpp     \
    ../source/wnd/h2product.cpp         \
    ../source/wnd/roboconfig.cpp        \
    ../source/wnd/xrobo.cpp             \
    ../source/wnd/h2robo.cpp            \
    ../source/widget/megalcdnumber.cpp  \
    ../source/widget/megamessagebox.cpp \
    ../source/widget/megasplinechart.cpp\
    ../source/widget/megainterface.cpp



!win32: HEADERS += ../source/device/vxi11/vxi11.h
!win32: HEADERS += ../source/device/vxi11/vxi11_user.h
!win32: SOURCES += ../source/device/vxi11/vxi11_clnt.c
!win32: SOURCES += ../source/device/vxi11/vxi11_user.c
!win32: SOURCES += ../source/device/vxi11/vxi11_xdr.c


FORMS += ../source/wnd/mainwindow.ui    \
    ../source/wnd/h2ops.ui              \
    ../source/wnd/h2action.ui           \
    ../source/wnd/aboutdlg.ui           \
    ../source/wnd/h2status.ui           \
    ../source/wnd/h2configuration.ui    \
    ../source/wnd/h2errmgr.ui           \
    ../source/wnd/h2jogmode.ui          \
    ../source/wnd/h2homing.ui           \
    ../source/wnd/helppanel.ui          \
    ../source/wnd/h2measurement.ui      \
    ../source/wnd/h2product.ui          \
    ../source/wnd/roboconfig.ui         \
    ../source/widget/megainterface.ui

INCLUDEPATH += ../include
INCLUDEPATH += ../source/sys
INCLUDEPATH += ../source/mrp
INCLUDEPATH += ../source/device
INCLUDEPATH += ../source/model
INCLUDEPATH += ../source/wnd
INCLUDEPATH += ../source/widget


win32:INCLUDEPATH += "C:/Program Files (x86)/IVI Foundation/VISA/WinNT/Include"

win32:LIBS += -L"C:/Program Files (x86)/IVI Foundation/VISA/WinNT/lib/msc"
win32:LIBS += -lvisa32

win32:LIBS += -L"../3rdlib"
win32:LIBS += -llibws2_32 -llibiphlpapi

RESOURCES += ../res/res.qrc

RC_ICONS = ../res/image/megarobo.ico

TRANSLATIONS += ../res/ts/qt_CN.ts
TRANSLATIONS += ../res/ts/qt_EN.ts

#target.path=/home/megarobo/MCT/
#INSTALLS += target
