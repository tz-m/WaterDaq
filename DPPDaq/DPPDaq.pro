TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    main.cpp

LIBS += -lCAENDigitizer
QMAKE_CXXFLAGS +=

#ROOT
INCLUDEPATH += $$system(${ROOTSYS}/bin/root-config --incdir)
LIBS += $$system(${ROOTSYS}/bin/root-config --glibs)

HEADERS += date.h
