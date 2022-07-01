cmake_minimum_required(VERSION 3.0.0)
project(web-server VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")



include(CTest)
enable_testing()

file(GLOB TARGET_SRC "./src/*.cpp" )

add_executable(${PROJECT_NAME} ${TARGET_SRC})

set (BOOST_COMPONENTS
    thread 
    filesystem
    unit_test_framework
    json
    system
    )
find_path(HIREDIS_HEADER hiredis)
target_include_directories(${PROJECT_NAME} PUBLIC ${HIREDIS_HEADER})
find_library(HIREDIS_LIB hiredis)
target_link_libraries(${PROJECT_NAME} ${HIREDIS_LIB})
find_path(REDIS_PLUS_PLUS_HEADER sw)
target_include_directories(${PROJECT_NAME} PUBLIC ${REDIS_PLUS_PLUS_HEADER})
message(${HIREDIS_LIB})

find_library(REDIS_PLUS_PLUS_LIB redis++)
target_link_libraries(${PROJECT_NAME} ${REDIS_PLUS_PLUS_LIB})
message(${REDIS_PLUS_PLUS_LIB})


set (Boost_USE_STATIC_LIBS ON)

set (Boost_USE_MULTITHREADED ON)

include_directories(${Boost_INCLUDE_DIRS})

find_package(Boost COMPONENTS ${BOOST_COMPONENTS} REQUIRED) 

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} pthread hiredis redis++)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

target_include_directories(${PROJECT_NAME} PRIVATE include)