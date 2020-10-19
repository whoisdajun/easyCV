CMAKE_MINIMUM_REQUIRED(VERSION 3.15)

project(easyCV)
set(CMAKE_CXX_STANDARD 14)

include_directories(
        ${PROJECT_SOURCE_DIR}/include/
)
file(GLOB USER_LIBS_PATH src/*.cpp)
message(${USER_LIBS_PATH})
add_library(${PROJECT_NAME} SHARED
        ${USER_LIBS_PATH}
)
#add_subdirectory(src)

