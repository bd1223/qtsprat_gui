#-------------------------------------------------
#
# Project created by QtCreator 2016-02-27T00:05:13
#
#-------------------------------------------------

QT       += core gui webkit webkitwidgets printsupport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtsprat
TEMPLATE = app

SOURCES += main.cpp
SOURCES += network_dialog.cpp
SOURCES += add_pseudo_dialog.cpp
SOURCES += segment_dialog.cpp
SOURCES += find_add_dialog.cpp
SOURCES += find_dialog.cpp
SOURCES += sil_dialog.cpp
SOURCES += flyout_dialog.cpp
SOURCES += map_dialog.cpp
SOURCES += config_dialog.cpp
SOURCES += qtsprat.cpp
SOURCES += met_range_dialog.cpp
SOURCES += cmd_chain_dialog.cpp
SOURCES += playback_dialog.cpp
SOURCES += saf_maint_dialog.cpp

HEADERS += qtsprat.h
HEADERS += network_dialog.h
HEADERS += add_pseudo_dialog.h
HEADERS += segment_dialog.h
HEADERS += find_add_dialog.h
HEADERS += find_dialog.h
HEADERS += sil_dialog.h
HEADERS += flyout_dialog.h
HEADERS += map_dialog.h
HEADERS += config_dialog.h
HEADERS += met_range_dialog.h
HEADERS += cmd_chain_dialog.h
HEADERS += playback_dialog.h
HEADERS += saf_maint_dialog.h

FORMS   += qtsprat.ui
FORMS   += network_dialog.ui
FORMS   += add_pseudo_dialog.ui
FORMS   += segment_dialog.ui
FORMS   += find_add_dialog.ui
FORMS   += find_dialog.ui
FORMS   += sil_dialog.ui
FORMS   += flyout_dialog.ui
FORMS   += map_dialog.ui
FORMS   += config_dialog.ui
FORMS   += met_range_dialog.ui
FORMS   += cmd_chain_dialog.ui
FORMS   += playback_dialog.ui
FORMS   += saf_maint_dialog.ui

RESOURCES += qtsprat.qrc

DISTFILES +=
