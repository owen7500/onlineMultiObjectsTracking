#-------------------------------------------------
#
# Project created by QtCreator 2016-06-23T21:37:36
#
#-------------------------------------------------

QT       += core gui
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = multiTargetTracking
TEMPLATE = app


SOURCES += main.cpp\
    objectTracker.cpp \
    vibe.cpp \
    dp/AdaptiveMedianBGS.cpp \
    dp/DPAdaptiveMedianBGS.cpp \
    dp/Error.cpp \
    dp/Image.cpp \
    dp/MeanBGS.cpp \
    targetTrackingUI.cpp \
    videotools.cpp \
    lb/BGModel.cpp \
    lb/BGModelSom.cpp \
    lb/LBAdaptiveSOM.cpp \
    dbg/RDBG.cpp \
    trackingfeaturefactory.cpp \
    FrameDifferenceBGS.cpp

HEADERS  += \
    feature_base.hpp \
    FrameDifferenceBGS.h \
    hashFeature.hpp \
    IBGS.h \
    objectTracker.h \
    targetObject.hpp \
    vibe.h \
    dp/AdaptiveMedianBGS.h \
    dp/Bgs.h \
    dp/BgsParams.h \
    dp/DPAdaptiveMedianBGS.h \
    dp/Error.h \
    dp/Image.h \
    dp/MeanBGS.h \
    videotools.hpp \
    targetTrackingUI.h \
    lb/BGModel.h \
    lb/BGModelSom.h \
    lb/LBAdaptiveSOM.h \
    dbg/RDBG.h \
    trackingFeature/ceddfeature.hpp \
    trackingFeature/colorFeature.hpp \
    trackingFeature/feature_base.hpp \
    trackingFeature/hashFeature.hpp \
    trackingFeature/CEDD/CEDD_descriptor.h \
    trackingFeature/CEDD/CEDDQuant.h \
    trackingFeature/CEDD/cosSimilarity.h \
    trackingFeature/CEDD/Fuzzy10Bins.h \
    trackingFeature/CEDD/Fuzzy24Bins.h \
    trackingfeaturefactory.h \
    FricBgs.hpp

FORMS    += targettracking.ui


#INCLUDEPATH += D:\opencv3\opencv\qtBin\include\
#            += D:\opencv3\opencv\qtBin\include\opencv\
#            += D:\opencv3\opencv\qtBin\include\opencv2\

##opencv set
#LIBS += D:\opencv3\opencv\qtBin\bin\libopencv_core310.dll
#LIBS += D:\opencv3\opencv\qtBin\bin\libopencv_features2d310.dll
#LIBS += D:\opencv3\opencv\qtBin\bin\libopencv_highgui310.dll
#LIBS += D:\opencv3\opencv\qtBin\bin\libopencv_imgproc310.dll
#LIBS += D:\opencv3\opencv\qtBin\bin\libopencv_video310.dll
#LIBS += D:\opencv3\opencv\qtBin\bin\libopencv_objdetect310.dll
#LIBS += D:\opencv3\opencv\qtBin\bin\libopencv_ximgproc310.dll
#LIBS += D:\opencv3\opencv\qtBin\bin\libopencv_xobjdetect310.dll
#LIBS += D:\opencv3\opencv\qtBin\bin\libopencv_imgcodecs310.dll
#LIBS += D:\opencv3\opencv\qtBin\bin\libopencv_videoio310.dll
#LIBS += D:\opencv3\opencv\qtBin\bin\opencv_ffmpeg310.dll

INCLUDEPATH += D:\opencv\bin\install\include\
            += D:\opencv\bin\install\include\opencv\
            += D:\opencv\bin\install\include\opencv2\

LIBS += D:\opencv\bin\install\x64\mingw\bin\libopencv_calib3d2410.dll
LIBS += D:\opencv\bin\install\x64\mingw\bin\libopencv_contrib2410.dll
LIBS += D:\opencv\bin\install\x64\mingw\bin\libopencv_core2410.dll
LIBS += D:\opencv\bin\install\x64\mingw\bin\libopencv_features2d2410.dll
LIBS += D:\opencv\bin\install\x64\mingw\bin\libopencv_gpu2410.dll
LIBS += D:\opencv\bin\install\x64\mingw\bin\libopencv_highgui2410.dll
LIBS += D:\opencv\bin\install\x64\mingw\bin\libopencv_imgproc2410.dll
LIBS += D:\opencv\bin\install\x64\mingw\bin\libopencv_photo2410.dll
LIBS += D:\opencv\bin\install\x64\mingw\bin\libopencv_superres2410.dll
LIBS += D:\opencv\bin\install\x64\mingw\bin\libopencv_video2410.dll
LIBS += D:\opencv\bin\install\x64\mingw\bin\libopencv_videostab2410.dll
