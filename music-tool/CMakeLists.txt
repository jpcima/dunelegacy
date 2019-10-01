cmake_minimum_required(VERSION 3.3)

project(dunemusic)

set(CMAKE_CXX_STANDARD 14)

find_package(Threads)

option(USE_MIXER_X "User SDL Mixer X fork instead of original SDL Mixer" OFF)

if(USE_MIXER_X)
	set(SDL_MIXER_LIB SDL2_mixer_ext)
else()
	set(SDL_MIXER_LIB SDL2_mixer)
endif()

add_executable(dunemusic
  main.cpp
  globals.cpp
  vgmrecorder.cpp
  realtime/ring_buffer.cpp
  ../src/FileClasses/adl/sound_adlib.cpp
  ../src/FileClasses/adl/nukedopl3.c
  ../src/FileClasses/adl/surroundopl.cpp
  ../src/FileClasses/music/ADLPlayer.cpp
  ../src/FileClasses/FileManager.cpp
  ../src/FileClasses/TextManager.cpp
  ../src/FileClasses/Pakfile.cpp
  ../src/FileClasses/IndexedTextFile.cpp
  ../src/FileClasses/MentatTextFile.cpp
  ../src/FileClasses/POFile.cpp
  ../src/FileClasses/INIFile.cpp
  ../src/misc/FileSystem.cpp
  ../src/misc/fnkdat.cpp
  ../src/misc/format.cpp
  ../src/misc/md5.cpp
  ../src/misc/string_util.cpp
  ../src/mmath.cpp)

target_compile_definitions(dunemusic
  PRIVATE "DUNELEGACY_DATADIR=\"${CMAKE_INSTALL_PREFIX}/share/dunelegacy\"")

if(USE_MIXER_X)
	target_compile_definitions(dunemusic PRIVATE -DUSE_SDL_MIXER_X)
endif()

target_include_directories(dunemusic
  PRIVATE ../include
  PRIVATE ../src)

target_link_libraries(dunemusic
  PRIVATE ${SDL_MIXER_LIB}
  PRIVATE SDL2
  PRIVATE ${CMAKE_THREAD_LIBS_INIT})
