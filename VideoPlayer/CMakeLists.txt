cmake_minimum_required(VERSION 3.10)

project(player VERSION 1.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_CONSTANT_MACROS -std=c++11")

set(OTHER_LIBS_FLAGS "-lbz2 -lz -lpthread -llzma")

#set_source_files_properties(src/AVLog.cpp PROPERTIES COMPILE_DEFINITIONS DEBUG_LOG=1)

if(APPLE) 
    set(MAC_OS_FLAGS "-framework CoreVideo -framework Security  -framework VideoToolbox -framework CoreMedia -framework CoreServices -framework CoreFoundation -framework AudioUnit -framework AudioToolbox -framework CoreAudio")
endif()

set(ffmpeg_DIR ${CMAKE_SOURCE_DIR}/ffmpeg/ffmpeg_1.0_static_dbg)
set(ffmpeg_LIB ${ffmpeg_DIR}/lib)

include_directories(${ffmpeg_DIR}/include)

add_library(avcodec STATIC IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION ${ffmpeg_LIB}/libavcodec.a)

add_library(avformat STATIC IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION ${ffmpeg_LIB}/libavformat.a)

add_library(avutils STATIC IMPORTED)
set_target_properties(avutils PROPERTIES IMPORTED_LOCATION ${ffmpeg_LIB}/libavutil.a)

add_library(swscale STATIC IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION ${ffmpeg_LIB}/libswscale.a)

add_library(swresample STATIC IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION ${ffmpeg_LIB}/libswresample.a)


include_directories(include)

file(GLOB SOURCES "src/*.cpp")

# add_executable (${PROJECT_NAME} src/main.cpp ${SOURCES})
add_library(${PROJECT_NAME} SHARED src/PlayerManager.cpp ${SOURCES})

target_link_libraries(${PROJECT_NAME} avformat avcodec avutils swscale swresample ${OTHER_LIBS_FLAGS} ${MAC_OS_FLAGS} )
