# Name of project and executable
project(OpenCV_Stream)

# set cmake version
cmake_minimum_required(VERSION 2.8)

# activate latest c++ compiler version
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 ")

# set flags to configure the warning settings
# Note: warnings are compiler specific
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")


# set build type to Debug/Release
set(CMAKE_BUILD_TYPE "Debug")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

find_package(Threads REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(Qt5Core)
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

# Populate a CMake variable with the sources
set(APP_SOURCES
    src/main.cpp
	src/opencv/opencv.cpp
	src/opencv/opencv.h
	src/opencv/framerate.cpp
	src/opencv/framerate.h
	src/data/haarcascade_frontalface_alt2.xml
    src/ui/widget.cpp
    src/ui/widget.h
    src/ui/widget.ui
)

set(APP_INCLUDE_DIRS
    src
    src/data
    src/ui
	src/opencv
)

include_directories(${APP_INCLUDE_DIRS})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/data/haarcascade_frontalface_alt2.xml
	DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Add an executable to the project and sources
add_executable(${PROJECT_NAME} ${APP_SOURCES})


# Use the Widgets module from Qt 5
target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} )
target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::PrintSupport)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} )


