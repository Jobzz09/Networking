TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        src/client.cpp \
        src/main.cpp

LIBS += /usr/local/lib/libboost_system.a  \
        /usr/local/lib/libboost_thread.a \
        /usr/local/lib/libboost_log.a \
        /usr/local/lib/libboost_filesystem.a \
        /usr/local/lib/libboost_log_setup.a \

LIBS += -pthread

HEADERS += \
    include/client.h
