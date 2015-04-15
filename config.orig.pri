
windows {

INCLUDEPATH += \
    d:/work/openssl/include \
    D:/work/boost/boost_1_55_0

LIBS += \
    -Ld:/work/openssl/lib \
    -LD:/work/boost/boost_1_55_0/stage/lib

win32-g++ {

LIBS += \
    -lboost_system-mgw48-mt-1_55 \
    -lboost_thread-mgw48-mt-1_55
}

}

