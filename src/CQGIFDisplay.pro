TEMPLATE = app

TARGET = CQGIFDisplay

DEPENDPATH += .

QT += widgets

QMAKE_CXXFLAGS += \
-std=c++17 \
-DCQUTIL_IMAGE \

CONFIG += debug

MOC_DIR = .moc

# Input
SOURCES += \
CQGIFDisplay.cpp \
CQLabel.cpp \
CPixmap.cpp \
CPixmapImage.cpp \
CGIFImage.cpp \
CJPGImage.cpp \
CPNGImage.cpp \
CXPMImage.cpp \

HEADERS += \
CQGIFDisplay.h \
CQLabel.h \
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
../../CFileUtil/include \
../../CFile/include \
../../CMath/include \
../../CStrUtil/include \
../../CUtil/include \
../../COS/include \
../../CRGBName/include \

unix:LIBS += \
-L$$LIB_DIR \
-L../../CQUtil/lib \
-L../../CQPixmapEd/lib \
-L../../CQToolTip/lib \
-L../../CQColorSelector/lib \
-L../../CUtil/lib \
-L../../CConfig/lib \
-L../../CUndo/lib \
-L../../CImageLib/lib \
-L../../CRGBName/lib \
-L../../CFont/lib \
-L../../CMath/lib \
-L../../CStrUtil/lib \
-L../../CFileUtil/lib \
-L../../CFile/lib \
-L../../COS/lib \
-L../../CRegExp/lib \
-lCQPixmapEd -lCQUtil -lCQColorSelector -lCConfig -lCUtil -lCImageLib \
-lCRGBName -lCUndo -lCFont -lCFileUtil -lCFile -lCMath -lCOS -lCStrUtil -lCRegExp \
-lpng -ljpeg -ltre
