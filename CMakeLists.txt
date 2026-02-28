
cmake_minimum_required(VERSION 3.0)

project(simple)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (opm-common_DIR "${CMAKE_SOURCE_DIR}/../opm-common/build" CACHE LOCATION "Build tree of opm-common")

find_package (opm-common)

include_directories(${CMAKE_SOURCE_DIR}
                    ${opm-common_INCLUDE_DIRS})

find_package(OpenMP REQUIRED)

find_package(Boost COMPONENTS system REQUIRED)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets Charts Test REQUIRED)

add_library(smry_appl STATIC
   appl/smry_appl.cpp
   appl/smry_xaxis.cpp
   appl/xaxis_ticks.cpp
   appl/smry_yaxis.cpp
   appl/smry_series.cpp
   appl/chartview.cpp
   appl/point_info.cpp
   appl/qsum_cmdf.cpp
   appl/derived_smry.cpp
   appl/qsum_func_lib.cpp
   appl/keyword_panel.cpp
   appl/main_window.cpp
  )

add_executable(qsummary main.cpp)

if(TARGET fmt::fmt)
  target_link_libraries(qsummary PRIVATE fmt::fmt)
endif()

target_link_libraries(smry_appl PRIVATE
    opmcommon
    Qt6::Widgets
    Qt6::Core
    Qt6::Charts
    stdc++fs
    OpenMP::OpenMP_CXX
)

target_link_libraries(qsummary PRIVATE
    smry_appl
    Boost::system
    Qt6::Widgets
    Qt6::Core
    Qt6::Charts
    OpenMP::OpenMP_CXX
)

add_executable(update_ref_define ./tests/update_ref_define.cpp)

target_link_libraries(update_ref_define smry_appl Boost::system Qt6::Widgets Qt6::Core Qt6::Charts OpenMP::OpenMP_CXX)


install(TARGETS qsummary DESTINATION bin)
