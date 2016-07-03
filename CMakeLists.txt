cmake_minimum_required (VERSION 2.8)
project (audigger)
message("Generator: " ${CMAKE_GENERATOR})
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/fir")

# Flags:
# DISABLE_CONSOLE
# SUPPORT_MP3
# AUDIO_VIRT
# DEBUG

if (DISABLE_CONSOLE)
	add_definitions(-DDISABLE_CONSOLE)
endif()

if (SUPPORT_MP3)
	add_definitions(-DSUPPORT_MP3)
endif()

if (AUDIO_VIRT)
	add_definitions(-DAUDIO_VIRT)
endif()

add_definitions(-DNOMINMAX) # to force Visual Studio to not generate its own std::min, std::max

set (SRC
    src/main.cpp
    src/defines.h
    src/common.h
    src/common.cpp
    src/audio_common.h
    src/audio_common.cpp

    src/AudioParams.h
    src/AudioParams.cpp	
    src/AudioDevice.h
    src/AudioDevice.cpp
    src/AudioFile.h
    src/AudioFile.cpp
    src/AudioBlock.h
    src/AudioBlock.cpp
    src/IProcessorClient.h
	src/DSP.h
    src/DSP.cpp
	src/Processor.h
    src/Processor.cpp
    src/ProcessorTasks.h
    src/ProcessorTasks.cpp
    
    src/Controller.h
    src/Controller.cpp
    
    src/TrackAbstract.h
    src/TrackAudio.h
    src/TrackAudio.cpp
    src/TrackAudioHistory.h
    src/TrackAudioHistory.cpp
	src/TrackMarks.h
    src/TrackMarks.cpp
	src/TrackTimeLine.h
	src/TrackTimeLine.cpp
	src/TrackInfo.h
	src/TrackInfo.cpp
	src/TracksGroup.h
	src/TracksGroup.cpp
	#src/MutexRent.h
	#src/MutexRent.cpp
	src/Settings.h
	src/Settings.cpp
	src/TracksBase.h
    src/TracksBase.cpp
    src/VectorSimple.h
    
    src/EventDistributor.h
    
    
	src/gui/Dragging.h
	src/gui/CommonGui.h
	src/gui/CommonGui.cpp
    src/gui/ITracksGroupClient.h
    src/gui/ProjectManager.h
    src/gui/ProjectManager.cpp
    src/gui/Selection.h
    src/gui/Selection.cpp
    src/gui/ToolBar.h
    src/gui/ToolBar.cpp

    src/gui/WidgetHeaders.h
    src/gui/WidgetHeaders.cpp
    src/gui/WidgetToolBar.h
    src/gui/WidgetToolBar.cpp
    src/gui/WindowAbout.h
    src/gui/WindowAbout.cpp
    src/gui/WidgetAudioParams.h
    src/gui/WidgetAudioParams.cpp
    src/gui/WindowMain.h
    src/gui/WindowMain.cpp
    src/gui/WidgetToolBank.h
    src/gui/WidgetToolBank.cpp
	src/gui/WidgetDial.h
    src/gui/WidgetDial.cpp
    src/gui/WidgetsCommon.h
    src/gui/WidgetsCommon.cpp
    src/gui/WidgetTrackAbstract.h
    src/gui/WidgetTracksGroup.h
    src/gui/WidgetTracksGroup.cpp
    src/gui/WidgetTrackMarks.h
    src/gui/WidgetTrackMarks.cpp
    src/gui/BlockTable.h
    src/gui/WidgetTrackSonogram.h
    src/gui/WidgetTrackSonogram.cpp
    src/gui/WidgetTrackSonogramUtils.h
    src/gui/WidgetTrackSonogramUtils.cpp
    src/gui/WidgetTrackTimeLine.h
    src/gui/WidgetTrackTimeLine.cpp
    src/gui/WidgetTrackWaveForm.h
    src/gui/WidgetTrackWaveForm.cpp

    
    /ar/forge/3rdparty/FFT_CODE/complex.cpp
    /ar/forge/3rdparty/FFT_CODE/fft.cpp    
    )

if(AUDIO_VIRT)
	set(SRC ${SRC}
		src/AudioDeviceVirt.h
		src/AudioDeviceVirt.cpp
		)
else(AUDIO_VIRT)
	if (WIN32)
		set (SRC ${SRC} 
			src/AudioDeviceWinAPI.cpp
			src/AudioDeviceWinAPI.h
			)
	else(WIN32)
		set (SRC ${SRC} 
			src/AudioDeviceALSA.cpp
			src/AudioDeviceALSA.h
			)
	endif(WIN32)
endif(AUDIO_VIRT)

set (SRC_UNITTEST_AUDIOTRACK
	src/audio_common.h
	src/audio_common.cpp
	src/TrackAudio.h
	src/TrackAudio.cpp
	src/AudioFile.h
	src/AudioFile.cpp
	test/unittest/test-audiotrack.cpp
	)

set (FORMS
	#src/WidgetMain.ui
	#src/WidgetToolsMain.ui
	#src/WidgetToolsPaint.ui
	)

set (UI_HEADERS
	src/gui/WindowAbout.h
    src/gui/WindowMain.h
    src/gui/WidgetTrackAbstract.h
    src/gui/WidgetTracksGroup.h
	src/gui/WidgetTrackSonogram.h
	src/gui/WidgetTrackMarks.h
	src/gui/WidgetTrackTimeLine.h
	src/gui/WidgetTrackWaveForm.h
	src/gui/WidgetHeaders.h
	src/gui/WidgetToolBar.h
	src/gui/WidgetToolBank.h
	src/gui/WidgetDial.h
	src/gui/WidgetTextEditEnter.h
	src/gui/WidgetsCommon.h
	src/gui/WidgetAudioParams.h
    )

set (RC
    src/gui/resources.qrc
	)


set (SRC_samplebank
	samplebank/main.cpp
	samplebank/SampleBank.h
	samplebank/SampleBank.cpp
	src/common.h
    src/common.cpp
	src/audio_common.h
	src/audio_common.cpp
    src/AudioFile.h
    src/AudioFile.cpp
    src/AudioBlock.h
    src/AudioBlock.cpp
	src/TracksGroup.h
	src/TracksGroup.cpp
    src/TrackAbstract.h
    src/TrackAudio.h
    src/TrackAudio.cpp
    src/TrackAudioHistory.h
    src/TrackAudioHistory.cpp
	src/TrackMarks.h
    src/TrackMarks.cpp
    src/TrackTimeLine.h
    src/TrackTimeLine.cpp
	)



set(Boost_DEBUG TRUE)
set(Boost_USE_STATIC_RUNTIME ON)
set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS ON)
#set(USE_STATIC_Boost ON)
add_definitions(-DBOOST_FILESYSTEM_VERSION=3)
set(Boost_USE_STATIC_LIBS ON)
add_definitions(-DBOOST_THREAD_USE_LIB)
add_definitions(-DBOOST_ALL_NO_LIB)

find_package(Boost 1.48.0 COMPONENTS system thread random chrono filesystem REQUIRED)



#find_package(Qt4 4.7.0 COMPONENTS QtCore QtGui REQUIRED)
#include(${QT_USE_FILE})
#QT4_WRAP_UI(GEN_UI_HDR ${FORMS})
#QT4_WRAP_CPP(GEN_UI_HRD_MOC ${UI_HEADERS} )
#QT4_ADD_RESOURCES(GEN_SRC_RC ${RC})


find_package(Qt5Widgets)
qt5_wrap_ui(GEN_UI_HDR ${FORMS})
qt5_wrap_cpp(GEN_UI_HRD_MOC ${UI_HEADERS} )



#if (UNIX)
#	find_package(ALSA)
#	include_directories ( ${ALSA_INCLUDE_DIR} )
#endif()

#####################################################
# Compilation
#####################################################
message (":) ALSA:               ${ALSA_LIBRARY}" )
message (":) Qt5Widgets_INCLUDE_DIRS: ${Qt5Widgets_INCLUDE_DIRS}" )

message (":) ${GEN_UI_HDR}")
message (":) ${GEN_UI_HRD_MOC}")
#message (":) ${GEN_SRC_RC}")
#message (":) ${QT_LIBRARIES}")
message (":) Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")
message (":) Boost_LIBRARIES:    ${Boost_LIBRARIES}")
message (":) Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message (":) Boost_INCLUDE_DIR:  ${Boost_INCLUDE_DIR}" )


include_directories( "src/")
include_directories( "src/gui")
include_directories( "/ar/forge/3rdparty/FFT_CODE")
include_directories( "/ar/forge/3rdparty")
include_directories( "/ar/forge")
include_directories( ${Qt5Widgets_INCLUDE_DIRS} )
include_directories( ${Boost_INCLUDE_DIRS})
include_directories( ${CMAKE_CURRENT_SOURCE_DIR})
include_directories( ${CMAKE_CURRENT_BINARY_DIR})

link_directories   ( ${CMAKE_CURRENT_BINARY_DIR})
link_directories   ( ${Boost_LIBRARY_DIRS})

#set (QT_IS_STATIC ON)
set (QT_DEFINITIONS STATIC)
#set (CMAKE_EXE_LINKER_FLAGS "-static")


#
# add jsoncpp
#

add_subdirectory(/ar/forge/3rdparty/jsoncpp-src-0.5.0 jsoncpp)
include_directories(/ar/forge/3rdparty/jsoncpp-src-0.5.0/include)



################
# Optimization
################

if (CMAKE_GENERATOR STREQUAL "Visual Studio 10")
	set (PLATFORM "win32-vs2010")
	# /Ot - time optimization
	# /MP - multi-processor build (faster)
	# /EHsc - Enable C++ Exceptions
	# /MT - static linking with CRT
	set(CMAKE_CXX_FLAGS_RELEASE "/D WIN32 /MP /Ot /arch:SSE2 /EHsc /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "/D WIN32 /MP /Ot /arch:SSE2 /EHsc /MT")
elseif (CMAKE_GENERATOR STREQUAL "Visual Studio 10 Win64")
	set (PLATFORM "win64-vs2010")
	set(CMAKE_CXX_FLAGS_RELEASE "/D WIN32 /MP /Ot /arch:SSE2 /EHsc /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "/D WIN32 /MP /Ot /arch:SSE2 /EHsc /MT")
elseif (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
	set (PLATFORM "win32-mingw") # for libsndfile only
	set(CMAKE_CXX_FLAGS "-fPIC -static -O2")
else()
	set(CMAKE_CXX_FLAGS_RELEASE "-fPIC -Wall -O2 -std=c++11 -fprofile-arcs -ftest-coverage ")
	set(CMAKE_CXX_FLAGS_DEBUG "-fPIC -Wall -g -O0 -pg -std=c++11 -fprofile-arcs -ftest-coverage")
endif()

if (CMAKE_GENERATOR MATCHES "NMake Makefiles")
	set(QTMAINLIB
			optimized ${QT_LIBRARY_DIR}/qtmain.lib
			debug ${QT_LIBRARY_DIR}/qtmaind.lib)
elseif (CMAKE_GENERATOR MATCHES "Visual Studio")
	set(QTMAINLIB
			optimized ${QT_LIBRARY_DIR}/qtmain.lib
			debug ${QT_LIBRARY_DIR}/qtmaind.lib)
elseif (CMAKE_GENERATOR MATCHES "MinGW Makefiles")
	# Nothing!
endif()

IF(WIN32)
	message(":) WIN32")
	if (SUPPORT_MP3)
		set (PATH_MPG123 "C:/dev/mpg123/${PLATFORM}")
		set (LIBMPG123 ${PATH_MPG123}/libmpg123.dll )
		include_directories( ${PATH_MPG123} )
		link_directories( ${PATH_MPG123} )
	endif()
	
	set (PATH_SNDFILE "C:/work/lib/libsndfile/${PLATFORM}")
	include_directories( ${PATH_SNDFILE} )
	link_directories( ${PATH_SNDFILE} )
	
	file(COPY ${PATH_SNDFILE}/libsndfile-1.dll DESTINATION ${PROJECT_BINARY_DIR}/Release)
	file(COPY ${PATH_SNDFILE}/libsndfile-1.dll DESTINATION ${PROJECT_BINARY_DIR}/Debug)

	#set(GUI_TYPE "WIN32")
	IF(DISABLE_CONSOLE)
		SET(GUI_TYPE WIN32)
	ENDIF()
	
ELSE(WIN32)
	if (SUPPORT_MP3)
		set (LIBMPG123 mpg123)
	endif()
	find_package(ALSA REQUIRED)
	include_directories( ${ALSA_INCLUDE_DIR} )
	#message("ALSA_INCLUDE_DIR" ${ALSA_INCLUDE_DIR} )
ENDIF(WIN32)


## yaml-cpp
## add_subdirectory( 3rdparty/yaml-cpp-0.5.0 )
## include_directories( 3rdparty/yaml-cpp-0.5.0/include )






add_executable(audigger
        ${GUI_TYPE}
        ${SRC}
        ${GEN_SRC_RC}
        ${UI_HEADERS}
        ${GEN_UI_HRD_MOC}
        ${GEN_UI_HEADERS}
        ${GEN_UI_HDR})
        
add_executable(samplebank
        ${SRC_samplebank})


#
# Linking
#
	
if(UNIX)			
	set_target_properties(audigger PROPERTIES LINK_FLAGS -Wl,--allow-multiple-definition)
	target_link_libraries(audigger
	   Qt5::Widgets
	   ${QTMAINLIB}
	   ${Boost_LIBRARIES}
	   ${Boost_FILESYSTEM_LIBRARY}
	   sndfile
	   ${ALSA_LIBRARY}
	   ${LIBMPG123}
	   jsoncpp
	   fir)
	qt5_use_modules(audigger Widgets)
	
	target_link_libraries(samplebank
	   ${Boost_LIBRARIES}
	   ${Boost_FILESYSTEM_LIBRARY}
	   sndfile
	   ${LIBMPG123}
	   jsoncpp
	   rt
	   fir)
	
#	target_link_libraries(unittest-audiotrack sndfile mpg123)
#	add_executable(alsatestapp1 src/alsa-test-app/main.cpp)
#	target_link_libraries(alsatestapp1 ${ALSA_LIBRARY})
#	add_executable(alsatestapp2 src/alsa-test-app-2/main.cpp)
#	target_link_libraries(alsatestapp2 ${ALSA_LIBRARY})
else()
    target_link_libraries(audigger ${QT_LIBRARIES} ${QTMAINLIB} ${Boost_LIBRARIES} libsndfile-1 winmm ${LIBMPG123} jsoncpp)
    target_link_libraries(samplebank ${Boost_LIBRARIES} sndfile ${LIBMPG123} jsoncpp)
	#target_link_libraries(unittest-audiotrack libsndfile-1 ${LIBMPG123})
endif()








install(TARGETS audigger DESTINATION bin)

#
# Test
#

#QT4_WRAP_CPP(GEN_UI_HRD_MOC_test1 "test/test1_qt.h")
#add_executable(test_TrackMarks "test/test_TrackMarks.cpp" "src/TrackMarks.cpp")


#
# DEB
#

#

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_DEBIAN_PACKAGE_NAME "audigger")
#set(CPACK_DEBIAN_PACKAGE_VERSION "0.0.1")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsndfile1, libasound2, libstdc++6, libqtgui4")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "pavelkolodin@gmail.com")
#set(CPACK_PACKAGE_CONTACT "pavelkolodin@gmail.com")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Audio editor + text labeling.")
include(CPack)


#
