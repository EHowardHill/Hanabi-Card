QT += core gui widgets

TARGET = myapp
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp

# Adjust the path to yaml-cpp includes if needed:
# INCLUDEPATH += /usr/include

# Link against yaml-cpp
LIBS += -lyaml-cpp
