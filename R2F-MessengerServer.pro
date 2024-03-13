QT += core quick network sql

CONFIG += c++17

CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS += -fsanitize=address
    QMAKE_LFLAGS += -fsanitize=address
    QMAKE_CXXFLAGS += -c -Wthread-safety
    QMAKE_LFLAGS += -c -Wthread-safety
}

SOURCES += \
        src/BackEnd.cpp \
        src/ConnectedClientManager.cpp \
        src/Database.cpp \
        src/MessageProcessor.cpp \
        src/ServerManager.cpp \
        src/SessionKey.cpp \
        src/main.cpp \
        src/server.cpp

RESOURCES += qml.qrc

QML_IMPORT_PATH =

QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    include/BackEnd.h \
    include/ConnectedClientManager.h \
    include/Database.h \
    include/MessageProcessor.h \
    include/ServerManager.h \
    include/SessionKey.h \
    include/messagecode.h \
    include/server.h \
    include/stdafx.h

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../usr/local/lib/release/ -lgmp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../usr/local/lib/debug/ -lgmp
else:unix:LIBS+= -L/usr/local/lib -lgmp

INCLUDEPATH += $$PWD/../../../../../usr/local/lib
DEPENDPATH += $$PWD/../../../../../usr/local/lib

unix:LIBS+= -L/usr/local/lib -lgmp

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../../../../usr/local/lib/release/libgmp.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../../../../usr/local/lib/debug/libgmp.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../../../../usr/local/lib/release/gmp.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../../../../usr/local/lib/debug/gmp.lib
