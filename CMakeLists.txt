cmake_minimum_required(VERSION 3.8)
project(zenoh_pico_ros)

set(CMAKE_C_STANDARD 11)
set(BUILD_SHARED_LIBS OFF)

include(FetchContent)

FetchContent_declare(zenoh-pico 
  GIT_REPOSITORY "https://github.com/eclipse-zenoh/zenoh-pico.git" 
  GIT_TAG e837b672f3412f361873ba4b780a2d01360465f6
)

set(Z_FEATURE_MULTI_THREAD 1)
set(Z_FEATURE_PUBLICATION 1)
set(Z_FEATURE_SUBSCRIPTION 1)
set(Z_FEATURE_QUERY 0)
set(Z_FEATURE_QUERYABLE 0)
set(Z_FEATURE_RAWETH_TRANSPORT 0)
set(Z_FEATURE_ATTACHMENT 1)

FetchContent_MakeAvailable(zenoh-pico)

target_compile_definitions(zenohpico
  PUBLIC
    Z_FEATURE_SCOUTING_UDP=0
    Z_FEATURE_LINK_UDP_MULTICAST=0
    Z_FEATURE_LINK_UDP_UNICAST=0
)

set (ENV{VIRTUAL_ENV} "${CMAKE_CURRENT_SOURCE_DIR}/.venv")
set (Python3_FIND_VIRTUALENV FIRST)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(PYTHON_PREFIX_PATH "")

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/colcon_ws/install")
list(APPEND PYTHON_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/colcon_ws/install/lib/python3.${Python3_VERSION_MINOR}/site-packages")

list(JOIN CMAKE_PREFIX_PATH ":" AMENT_PREFIX_PATH)
list(JOIN PYTHON_PREFIX_PATH ":" PYTHONPATH)

set(ENV{AMENT_PREFIX_PATH} "${AMENT_PREFIX_PATH}")
set(ENV{PYTHONPATH} "${PYTHONPATH}")

find_package(ament_cmake REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rosidl_runtime_c REQUIRED)
find_package(rosidl_typesupport_microxrcedds_c REQUIRED)

add_executable(zenoh-pico-ros
  main.c
)

ament_target_dependencies(zenoh-pico-ros
  geometry_msgs
)

target_link_libraries(zenoh-pico-ros
  rosidl_runtime_c::rosidl_runtime_c
  rosidl_typesupport_microxrcedds_c::rosidl_typesupport_microxrcedds_c
  zenohpico
)
