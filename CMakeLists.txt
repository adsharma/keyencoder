cmake_minimum_required(VERSION 2.6)
project(keyencoder)
# Sync this with ROCKSDB_VERSION
set(KEYENCODER_VERSION "4.8.0")
set(KEYENCODER_VERSION_MAJOR "4")

if (UNIX)
set(CMAKE_CXX_FLAGS "-DOS_LINUX -DROCKSDB_PLATFORM_POSIX -DGFLAGS=google")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DROCKSDB_LIB_IO_POSIX")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DZLIB -DBZIP2 -DSNAPPY -DLZ4")

if (CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin-memcmp")
endif (CMAKE_COMPILER_IS_GNUCXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse -msse4.2 -Woverloaded-virtual")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-field-initializers")

list(APPEND CMAKE_CXX_FLAGS_RELEASE
     "-fno-omit-frame-pointer -momit-leaf-frame-pointer -DNDEBUG")

endif (UNIX)

set(BUILD_VERSION_CC ${CMAKE_CURRENT_SOURCE_DIR}/util/build_version.cc)
add_custom_command(OUTPUT ${BUILD_VERSION_CC}
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/build_tools/create_version_cc.sh
      ${BUILD_VERSION_CC}
)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

set(EXTRA_LIBRARIES "")
find_package(Threads)
find_package(JeMalloc)
find_package(Rocksdb)
find_package(Gtest)

if (JEMALLOC_FOUND)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DROCKSDB_JEMALLOC")
list(APPEND EXTRA_LIBRARIES ${JEMALLOC_LIBRARIES})
endif (JEMALLOC_FOUND)

if (ROCKSDB_FOUND)
include_directories(${ROCKSDB_INCLUDE_DIR})
list(APPEND EXTRA_LIBRARIES ${ROCKSDB_LIBRARIES})
endif (ROCKSDB_FOUND)

if (GTEST_FOUND)
include_directories(${GTEST_INCLUDE_DIR})
list(APPEND EXTRA_LIBRARIES ${GTEST_LIBRARIES})
endif (GTEST_FOUND)

# Main library source code
set(SOURCES
       utilities/table/serializer.cc
       #utilities/table/ldb_table_cmd.cc
)

add_library(keyencoder-static STATIC ${SOURCES})
add_library(keyencoder-shared SHARED ${SOURCES})
set_target_properties(keyencoder-shared PROPERTIES
                      VERSION ${KEYENCODER_VERSION}
                      SOVERSION ${KEYENCODER_VERSION_MAJOR})

set_target_properties(keyencoder-static PROPERTIES
                      LINKER_LANGUAGE CXX
                      OUTPUT_NAME "keyencoder")
set_target_properties(keyencoder-shared PROPERTIES
                      LINKER_LANGUAGE CXX
                      OUTPUT_NAME "keyencoder")

set_property(TARGET keyencoder-static PROPERTY CXX_STANDARD 11)
set_property(TARGET keyencoder-shared PROPERTY CXX_STANDARD 11)

set(APPS
        utilities/table/ldb_table.cc
)

set(TESTS
        utilities/table/table_ordering_test.cc
)

set(BINS ${APPS} ${TESTS})

foreach(sourcefile ${BINS})
    string(REPLACE ".cc" "" exename ${sourcefile})
    string(REGEX REPLACE "^((.+)/)+" "" exename ${exename})
    add_executable(${exename}${ARTIFACT_SUFFIX} ${sourcefile})
    list(APPEND EXES ${exename}${ARTIFACT_SUFFIX})
    target_link_libraries(${exename}${ARTIFACT_SUFFIX} keyencoder-shared
                          ${EXTRA_LIBRARIES}
                          gflags rt snappy bz2 lz4 z
                          ${CMAKE_THREAD_LIBS_INIT})
    set_property(TARGET ${exename}${ARTIFACT_SUFFIX} PROPERTY CXX_STANDARD 11)
endforeach(sourcefile ${APPS})

install(TARGETS keyencoder-static COMPONENT devel ARCHIVE DESTINATION lib)
install(TARGETS keyencoder-shared COMPONENT runtime DESTINATION lib)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/rocksdb/"
        COMPONENT devel
        DESTINATION include/rocksdb)

install(TARGETS ${EXES} RUNTIME
        DESTINATION bin
        COMPONENT tools)
