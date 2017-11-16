cmake_minimum_required(VERSION 3.8)
project(FEM)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

include_directories(utils ${OPENGL_LIBRARIES} ${OPENGL_LIBRARIES})

set(SOURCE_FILES
        utils/HSV2RGB.h
        utils/Mat3x3.h
        utils/PCGT.h
        utils/SparseSymMat.h
        utils/Vec2.h
        utils/Vec3.h
        FEM.cpp
        FEModel.cpp
        FEModel.h
        LinTriElement.cpp
        LinTriElement.h)

add_executable(FEM ${SOURCE_FILES})

target_link_libraries(FEM ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})