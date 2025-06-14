cmake_minimum_required (VERSION 3.8)

project ("cmdkit" VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Header library
include_directories("include")
file(GLOB_RECURSE HEADERS CMAKE_CONFIGURE_DEPENDS "include/*.hpp")
add_library(CMDKIT INTERFACE ${HEADERS})

# examples executable
add_executable(use_result "example/use_result.cpp")
target_link_libraries(use_result PRIVATE CMDKIT)

add_executable(use_command "example/use_command.cpp")
target_link_libraries(use_command PRIVATE CMDKIT)

add_executable(use_terminal "example/use_terminal.cpp")
target_link_libraries(use_terminal PRIVATE CMDKIT)

add_executable(use_cmdkit "example/use_cmdkit.cpp")
target_link_libraries(use_cmdkit PRIVATE CMDKIT)