add_library(core_config INTERFACE)

include(${CMAKE_CURRENT_LIST_DIR}/config-build.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/config-debug.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/config-memory.cmake)
