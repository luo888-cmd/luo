QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    departmentview.cpp \
    doctorview.cpp \
    idatabase.cpp \
    loginview.cpp \
    main.cpp \
    masterview.cpp \
    patienteditview.cpp \
    patientview.cpp \
    welcomeview.cpp

HEADERS += \
    departmentview.h \
    doctorview.h \
    idatabase.h \
    loginview.h \
    masterview.h \
    patienteditview.h \
    patientview.h \
    welcomeview.h

FORMS += \
    departmentview.ui \
    doctorview.ui \
    loginview.ui \
    masterview.ui \
    patienteditview.ui \
    patientview.ui \
    welcomeview.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    img.qrc

DISTFILES += \
    C:/Users/hebi_/Desktop/images/修改、修改资料.png \
    C:/Users/hebi_/Desktop/images/删除.png \
    C:/Users/hebi_/Desktop/images/医生.png \
    C:/Users/hebi_/Desktop/images/患者.png \
    C:/Users/hebi_/Desktop/images/查找 (1).png \
    C:/Users/hebi_/Desktop/images/欢迎.png \
    C:/Users/hebi_/Desktop/images/添加.png \
    C:/Users/hebi_/Desktop/images/科室.png
