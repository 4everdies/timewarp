cmake_minimum_required(VERSION 3.17)
project(Timewarp_Injector)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(injector main.cpp)

if (WIN32 OR MINGW)
    target_link_libraries(injector PRIVATE kernel32 user32)
    target_link_options(injector PRIVATE
        -static-libgcc
        -static-libstdc++
        -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive
    )
endif()
