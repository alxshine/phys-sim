cmake_minimum_required(VERSION 3.8)
project(MassSpring)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        Exercise.cpp
        MassSpring.cpp
        Point.cpp
        Point.h
        Scene.cpp
        Scene.h
        Spring.cpp
        Spring.h
        Vec2.h)

add_executable(MassSpring ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(MassSpring ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})