cmake_minimum_required(VERSION 3.15)

project(DockerSample CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

set(SOURCES
	src/main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

find_package(oatpp)
target_link_libraries(${PROJECT_NAME} oatpp::oatpp)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

install(TARGETS ${PROJECT_NAME})
