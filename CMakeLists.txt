CMAKE_MINIMUM_REQUIRED(VERSION 3.9)
PROJECT(njson)

INCLUDE(FindPkgConfig)
INCLUDE(GNUInstallDirs)
INCLUDE(FetchContent)

# C++ standard: c++11 (not gnu++11)
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_EXTENSIONS OFF)

pkg_check_modules(pkgs REQUIRED RapidJSON)
FOREACH(flag ${pkgs_CFLAGS})
	ADD_COMPILE_OPTIONS(${flag})
ENDFOREACH(flag)

ENABLE_TESTING()

# Google test
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

# Global include directories
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(test)
