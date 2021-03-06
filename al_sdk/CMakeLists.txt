project(altexo_sdk)
cmake_minimum_required(VERSION 2.8)

file(GLOB SRC_LIST
  src/*.cpp
  src/*.h
  src/*.cc
  src/**/*.cpp
  src/**/*.h
  src/**/*.cc
)

INCLUDE_DIRECTORIES(./src/)

##
# Compiler flags
#
if(UNIX)
  set(CMAKE_C_COMPILER "/usr/bin/clang")
  set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
endif()

if (APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g  -pipe -fno-rtti -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 -O0 -std=c++11 -Wall -W -D_REENTRANT")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined dynamic_lookup")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -undefined dynamic_lookup")
elseif(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -m64 -pipe -fno-rtti -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 -O0 -std=c++11 -Wall -W -D_REENTRANT")
endif()

##
# USEFUL VARS
#
if (APPLE)
  set(WEBRTCTRUNK "/Volumes/MAC/workspace/WEBRTC/webrtc/src")
elseif(UNIX)
  # set(WEBRTCTRUNK "/home/xors/workspace/ALTEXO/webrtc/src")
  set(WEBRTCTRUNK "/home/xors/workspace/lib/webrtc-checkout/src")

elseif (WIN32)
  set(WEBRTCTRUNK "D:\\WORKSPACE\\webrtc\\src")
endif()
# set(WEBRTCBUILD "${WEBRTCTRUNK}/out/Release")
set(WEBRTCBUILD "${WEBRTCTRUNK}/out/Default")

##
# DEFINES
#
if (WIN32)
  add_definitions(-DWEBRTC_WIN)
  add_definitions(-DPOSIX)
  add_definitions(-DNOMINMAX)
else()
  add_definitions(-DUNIX)
  if(UNIX)
    add_definitions(-DLINUX)
  endif()
  add_definitions(-DWEBRTC_POSIX)
  add_definitions(-DPOSIX)
endif()

if (UNIX)
  if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -std=c++11")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O2")
  elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
      string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
  endif()
endif()

if (MSVC)
  # Enable the static libraries on Windows
  foreach (flag_var
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
    CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
    CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
    # update to the static version of the run time library
    string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
  endforeach()

  set(CMAKE_C_STANDARD_LIBRARIES "")
  set(CMAKE_CXX_STANDARD_LIBRARIES "")
endif()

# enable boost static flag
unset(Boost_LIBRARIES)
set(Boost_USE_STATIC ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)



if (UNIX)
  add_library(${PROJECT_NAME} SHARED ${SRC_LIST})
else()
  # seems like only static is allowed for win
  # add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
  add_library(${PROJECT_NAME} SHARED ${SRC_LIST})
endif()
# add_executable(${PROJECT_NAME} ${SRC_LIST})

if (APPLE)
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-m64" LINK_FLAGS "-m64")
elseif(UNIX)
  # compile and link for 64 bit mode
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-m64" LINK_FLAGS "-m64")
endif()

if (UNIX)
  set(CMAKE_AUTOMOC ON)
endif()

##
# INCLUDE
#
include_directories(.)
include_directories(${WEBRTCTRUNK}/)
include_directories(${WEBRTCTRUNK}/chromium/src/third_party/jsoncpp/source/include/)
include_directories(${WEBRTCTRUNK}/third_party/)
include_directories(${WEBRTCTRUNK}/third_party/webrtc/)
include_directories(${WEBRTCTRUNK}/webrtc/net/third_party/nss/ssl)
include_directories(${WEBRTCTRUNK}/third_party/jsoncpp/overrides/include)
include_directories(${WEBRTCTRUNK}/third_party/jsoncpp/source/include)
include_directories(${WEBRTCTRUNK}/chromium/src/third_party/libyuv/include)
include_directories(../include/)

##
# LIBRARIES
#
if (APPLE)
  include(al-webrtc-deps-mac.cmake)
elseif(UNIX)
  include(al-webrtc-deps.cmake)
elseif (WIN32)
  include(al-webrtc-deps-win.cmake)
endif()

if(UNIX)
  FIND_PACKAGE( Boost COMPONENTS system filesystem thread REQUIRED )
  INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
else()
  INCLUDE_DIRECTORIES( "C:\\local\\boost_1_61_0" )
endif()


if (APPLE)
  TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC
    ${WEBRTC_LIBS}
    -lpthread
    -ldl
    -lexpat
    -lm -ldl
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
  )
elseif(UNIX)
  TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC
    -Wl,--start-group
    ${WEBRTC_LIBS}
    -Wl,--end-group
    # -Wl,--start-group
    # ${WEBRTC_VENDOR_LIBS}
    # -Wl,--end-group
    -lpthread
    -lX11 -lXfixes -lXi -lXcomposite -lXext -lXrender -ldl -lrt -lexpat -lXrandr -lm -ldl
    boost_thread boost_system
  )
elseif (WIN32)
  TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC
    ${WEBRTC_LIBS}

    "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.10586.0\\um\\x86\\strmiids.lib"
    "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.10586.0\\um\\x86\\WS2_32.Lib"
    "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.10586.0\\um\\x86\\Secur32.Lib"
    "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.10586.0\\um\\x86\\Crypt32.Lib"
    "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.10586.0\\um\\x86\\msdmo.lib"
    "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.10586.0\\um\\x86\\WinMM.Lib"
    "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.10586.0\\um\\x86\\dmoguids.lib"
    "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.10586.0\\um\\x86\\wmcodecdspuuid.lib"
    "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.10586.0\\um\\x86\\AdvAPI32.Lib"

    # "C:\\local\\boost_1_61_0\\lib32-msvc-14.0\\libboost_system-vc140-mt-1_61.lib"
    # "C:\\local\\boost_1_61_0\\lib32-msvc-14.0\\libboost_filesystem-vc140-mt-1_61.lib"
    # "C:\\local\\boost_1_61_0\\lib32-msvc-14.0\\libboost_thread-vc140-mt-1_61.lib"
    # "C:\\local\\boost_1_61_0\\lib32-msvc-14.0\\libboost_date_time-vc140-mt-1_61.lib"
    # "C:\\local\\boost_1_61_0\\lib32-msvc-14.0\\libboost_chrono-vc140-mt-1_61.lib"
    "C:\\local\\boost_1_61_0\\lib32-msvc-14.0\\libboost_system-vc140-mt-s-1_61.lib"
    "C:\\local\\boost_1_61_0\\lib32-msvc-14.0\\libboost_filesystem-vc140-mt-s-1_61.lib"
    "C:\\local\\boost_1_61_0\\lib32-msvc-14.0\\libboost_thread-vc140-mt-s-1_61.lib"
    "C:\\local\\boost_1_61_0\\lib32-msvc-14.0\\libboost_date_time-vc140-mt-s-1_61.lib"
    "C:\\local\\boost_1_61_0\\lib32-msvc-14.0\\libboost_chrono-vc140-mt-s-1_61.lib"
  )
endif()
