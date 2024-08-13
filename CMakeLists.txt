cmake_minimum_required(VERSION 3.8)
project(zenoh_pico_ros)

set(CMAKE_C_STANDARD 11)
set(BUILD_SHARED_LIBS OFF)

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
find_package(zenohpico_vendor REQUIRED)
find_package(zenohpico REQUIRED)

# Hack to make zenohpico::static target work
get_target_property(ZENOHPICO_INCLUDES zenohpico::lib INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(__zenohpico_static INTERFACE ${ZENOHPICO_INCLUDES})
set_property(TARGET __zenohpico_static PROPERTY IMPORTED_LOCATION ${ZENOHPICO_INCLUDES}/../lib/libzenohpico.a)

add_executable(zenoh-pico-ros
  main.c
)

ament_target_dependencies(zenoh-pico-ros
  geometry_msgs
)

target_link_libraries(zenoh-pico-ros
  rosidl_runtime_c::rosidl_runtime_c
  rosidl_typesupport_microxrcedds_c::rosidl_typesupport_microxrcedds_c
  zenohpico::static
)
