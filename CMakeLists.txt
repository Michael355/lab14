cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 11)
include(cmake/HunterGate.cmake)
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.19.174.tar.gz"
    SHA1 "712898528fb6dc8532454e2e323404d93a76d8ec"
)

project(builder)

hunter_add_package(Boost)
find_package(Boost CONFIG REQUIRED COMPONENTS program_options system)

add_executable(builder ${CMAKE_CURRENT_SOURCE_DIR}/sources/builder.cpp)
target_link_libraries(builder Boost::program_options Boost::system)
install(TARGETS builder RUNTIME DESTINATION bin)
