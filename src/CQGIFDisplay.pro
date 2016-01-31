TEMPLATE = app

TARGET = CQGIFDisplay

DEPENDPATH += .

QT += widgets

QMAKE_CXXFLAGS += -std=c++11

CONFIG += debug

# Input
SOURCES += \
CQGIFDisplay.cpp \
CQImageButton.cpp \
CQIntegerEdit.cpp \
CQLabel.cpp \
CQPixmapCache.cpp \
CPixmap.cpp \
CPixmapImage.cpp \
CGIFImage.cpp \
CJPGImage.cpp \
CPNGImage.cpp \
CXPMImage.cpp \

HEADERS += \
CQGIFDisplay.h \
CQImageButton.h \
CQIntegerEdit.h \
CQLabel.h \
CQPixmapCache.h \
CPixmap.h \
CPixmapImage.h \
CGIFImage.h \
CJPGImage.h \
CPNGImage.h \
CXPMImage.h \

DESTDIR     = ../bin
OBJECTS_DIR = ../obj
LIB_DIR     = ../lib

INCLUDEPATH += \
../include \
. \
../../CQUtil/include \
../../CQPixmapEd/include \
../../CImageLib/include \
../../CFont/include \
../../CUndo/include \
../../CFile/include \
../../CMath/include \
../../CStrUtil/include \
../../CUtil/include \

unix:LIBS += \
-L$$LIB_DIR \
-L../../CQUtil/lib \
-L../../CQPixmapEd/lib \
-L../../CQColorSelector/lib \
-L../../CUtil/lib \
-L../../CConfig/lib \
-L../../CUndo/lib \
-L../../CImageLib/lib \
-L../../CRGBName/lib \
-L../../CFont/lib \
-L../../CStrUtil/lib \
-L../../CFile/lib \
-L../../COS/lib \
-L../../CRegExp/lib \
-lCQUtil -lCQPixmapEd -lCQColorSelector -lCConfig -lCUtil -lCImageLib \
-lCRGBName -lCUndo -lCFont -lCFile -lCOS -lCStrUtil -lCRegExp \
-lpng -ljpeg -ltre
