cmake_minimum_required (VERSION 2.8)

project(viewer)

set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)

# ------ Select the mode (Video Capture or Image) -------
option (TEST_VIDEO_CAPTURE "Visualize Video Capture" ON)
option (TEST_IAMGE "Visualize Image" ON)


# ------ Optional, but will speed up the performance -------
option(OPENMP "enable openmp" ON)

# ------ O3 optimzation -------
SET(CMAKE_C_FLAGS "$ENV{CFLAGS} -O3 -Wall") 

# ------ find OpenMP -------
if (OPENMP)
  FIND_PACKAGE(OpenMP REQUIRED)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

# ------ Find OpenCV -------
#set(OpenCV_DIR D:/Opencv4.2/opencv/build)  # set where OpenCVConfig.cmake is located
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV library status:")
message(STATUS "version: ${OpenCV_VERSION}")


#------- Include Sources -------
#include_directories ("${PROJECT_SOURCE_DIR}/src/eunit/emat")

# Declare the executable target built from your sources
file(GLOB LIB_HEADERS
    src/eunit/emat/*.hpp
)

#set(LIBS_SOURCES **)


if (TEST_VIDEO_CAPTURE)
	# ------ Link your application with OpenCV libraries -------
	add_executable(video_capture_viewer test/viewer/test_video_capture.cpp)
	target_link_libraries(video_capture_viewer ${OpenCV_LIBS})
	# ------ set compile options -------
	if  (MSVC)
	else()
			target_compile_options(video_capture_viewer PUBLIC -Wall $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++11>)
	endif()

	# ------ install -------
	install (TARGETS video_capture_viewer DESTINATION .)
endif (TEST_VIDEO_CAPTURE)

if (TEST_IAMGE)
	# ------ Link your application with OpenCV libraries -------
	add_executable(image_viewer test/viewer/test_image.cpp)
	target_link_libraries(image_viewer ${OpenCV_LIBS})
	# ------ set compile options -------
	if  (MSVC)
	else()
			target_compile_options(image_viewer PUBLIC -Wall $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++11>)
	endif()
	# ------ install -------
	install (TARGETS image_viewer DESTINATION .)
endif (TEST_IAMGE)


