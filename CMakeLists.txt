cmake_minimum_required(VERSION 3.5)
project(lugand_j_Epiboy)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_modules")
find_package(SFML REQUIRED system window graphics network audio)
if (SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(SOURCE_FILES main.cpp src/Cpu/Cpu.cc src/Cpu/Cpu.hh src/Mmu/Mmu.cc src/Mmu/Mmu.hh src/Gpu/Gpu.cc src/Gpu/Gpu.hh)



add_executable(lugand_j_Epiboy ${SOURCE_FILES})
target_link_libraries(lugand_j_Epiboy ${SFML_LIBRARIES})