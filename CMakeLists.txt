cmake_minimum_required(VERSION 3.0)
project(sylar)

include (cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -g -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined") 

set(LIB_SRC
    sylar/config.cc
    sylar/fiber.cc
    sylar/log.cc
    sylar/mutex.cc
    sylar/scheduler.cc
    sylar/util.cc
    sylar/thread.cc
    )
    
add_library(sylar SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(sylar)  # __FILE__

# 查找并链接库
find_package(yaml-cpp REQUIRED)
find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)  # 强制使用-pthread而非-lpthread

# 链接库文件
target_link_libraries(sylar
	PRIVATE
	${YAML_CPP_LIBRARIES}
	Threads::Threads
)

add_executable(test tests/test.cc)
add_dependencies(test sylar)
force_redefine_file_macro_for_sources(test)  # __FILE__
target_link_libraries(test sylar Threads::Threads)

add_executable(test_config tests/test_config.cc)
add_dependencies(test_config sylar)
force_redefine_file_macro_for_sources(test_config)  # __FILE__
target_link_libraries(test_config PRIVATE sylar yaml-cpp Threads::Threads)

add_executable(test_thread tests/test_thread.cc)
add_dependencies(test_thread sylar)
force_redefine_file_macro_for_sources(test_thread)  # __FILE__
target_link_libraries(test_thread PRIVATE sylar Threads::Threads)

add_executable(test_util tests/test_util.cc)
add_dependencies(test_util sylar)
force_redefine_file_macro_for_sources(test_util)  # __FILE__
target_link_libraries(test_util PRIVATE sylar Threads::Threads)

add_executable(test_fiber tests/test_fiber.cc)
add_dependencies(test_fiber sylar)
force_redefine_file_macro_for_sources(test_fiber)  # __FILE__
target_link_libraries(test_fiber PRIVATE sylar Threads::Threads)

add_executable(test_scheduler tests/test_scheduler.cc)
add_dependencies(test_scheduler sylar)
force_redefine_file_macro_for_sources(test_scheduler)  # __FILE__
target_link_libraries(test_scheduler PRIVATE sylar Threads::Threads)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)


