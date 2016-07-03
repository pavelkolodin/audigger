cmake_minimum_required (VERSION 2.8)
project (box-resize)
message("Generator: " ${CMAKE_GENERATOR})

set (SRC
    main.cpp
    WinWin.cpp
    WinWin.h
    ../../src/gui/Selection.h
    ../../src/gui/Selection.cpp
    )

set (FORMS
	)

set (UI_HEADERS
    WinWin.h
    )

set (RC
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


find_package(Qt4 4.7.0 COMPONENTS QtCore QtGui REQUIRED)
include(${QT_USE_FILE})

QT4_WRAP_UI(GEN_UI_HDR ${FORMS})
QT4_WRAP_CPP(GEN_UI_HRD_MOC ${UI_HEADERS} )
QT4_ADD_RESOURCES(GEN_SRC_RC ${RC})

add_definitions(-DNOMINMAX) # to force Visual Studio to not generate its own std::min, std::max

include_directories( "../../src/gui/" )
include_directories( ${Boost_INCLUDE_DIRS})
include_directories( ${CMAKE_CURRENT_BINARY_DIR})

set (QT_DEFINITIONS STATIC)

add_executable(test ${GUI_TYPE} ${SRC} ${GEN_SRC_RC} ${UI_HEADERS} ${GEN_UI_HRD_MOC} ${GEN_UI_HEADERS} ${GEN_UI_HDR})

if(UNIX)			
	set_target_properties(test PROPERTIES LINK_FLAGS -Wl,--allow-multiple-definition)
	target_link_libraries(test ${QT_LIBRARIES} ${QTMAINLIB}
    ${Boost_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY})
	
else()
    target_link_libraries(audigger ${QT_LIBRARIES} ${Boost_LIBRARIES}
    ${QTMAINLIB})
endif()

