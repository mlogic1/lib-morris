cmake_minimum_required(VERSION 3.10.2)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(lib-morris CXX)

SET (MORRIS_INCLUDE_DIR ./include/MorrisGame/)
SET (MORRIS_SRC_DIR ./src/)

SET (MORRIS_HEADER_FILES 
	${MORRIS_INCLUDE_DIR}IMorrisEventListener.h
	${MORRIS_INCLUDE_DIR}IMorrisLogger.h
	${MORRIS_INCLUDE_DIR}MorrisMarkerColor.h
	${MORRIS_INCLUDE_DIR}MorrisPlayer.h
	${MORRIS_INCLUDE_DIR}MorrisField.h
	${MORRIS_INCLUDE_DIR}MorrisGame.h
	${MORRIS_INCLUDE_DIR}MorrisGameState.h
	${MORRIS_INCLUDE_DIR}MorrisMarker.h
)
SET (MORRIS_SRC_FILES 
	${MORRIS_SRC_DIR}MorrisGame.cpp	
	${MORRIS_SRC_DIR}MorrisField.cpp
	${MORRIS_SRC_DIR}MorrisMarker.cpp
)

add_library(libMorris STATIC ${MORRIS_HEADER_FILES} ${MORRIS_SRC_FILES})

target_include_directories(libMorris INTERFACE ./include)
target_include_directories(libMorris PRIVATE ${MORRIS_INCLUDE_DIR})