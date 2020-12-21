TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

msvc:QMAKE_CXXFLAGS += /permissive-

INCLUDEPATH += ../../../eigen \
               core

win32 {
    INCLUDEPATH += ../../../intel/compilers_and_libraries_2019.5.281/windows/mkl/include/
    LIBS += -L$$PWD/../../../intel/compilers_and_libraries_2019.5.281/windows/mkl/lib/intel64_win/ -lmkl_core -lmkl_intel_lp64 -lmkl_sequential
}

unix {
    INCLUDEPATH +=../../../intel/mkl/include/
    LIBS += -L$$PWD/../../../intel/mkl/lib/intel64/ -lmkl_intel_lp64 -lmkl_sequential -lmkl_core
}

SOURCES += \
        main.cpp \
        core/mesh/mesh.cpp \
        core/solver/eigensolver.cpp

HEADERS += \
    core/error/error.h \
    core/fem/fem.h \
    core/mesh/mesh.h \
    core/parser/defs.h \
    core/parser/node.h \
    core/parser/parser.h \
    core/shape/shape.h \
    core/matrix/matrix.h \
    core/solver/eigensolver.h \
    core/solver/solver.h \
    core/value/value.h

