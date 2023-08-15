cmake_minimum_required(VERSION 3.15)

project(DockerSample CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

set(SOURCES
	src/AppComponent.h
	src/main.cpp

	src/controller/SearchController.cpp
	src/controller/SearchController.h

	src/dto/SearchResult.cpp
	src/dto/SearchResult.h
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src)

find_package(oatpp)
target_link_libraries(${PROJECT_NAME} PUBLIC oatpp::oatpp)

find_package(sssl)
target_link_libraries(${PROJECT_NAME} PUBLIC sssl::sssl)

find_package(syl)
target_link_libraries(${PROJECT_NAME} PUBLIC syl::syl)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

install(TARGETS ${PROJECT_NAME})
