file(GLOB MAIN_SOURCES "*.cpp" "*.h")
file(GLOB DEBUG_SOURCES "*.cpp" "*.h")

list(REMOVE_ITEM MAIN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/debug.cpp")
list(REMOVE_ITEM DEBUG_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

add_executable(main ${MAIN_SOURCES})
add_executable(debug ${DEBUG_SOURCES})

# Enable LTO
set_property(TARGET main PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
