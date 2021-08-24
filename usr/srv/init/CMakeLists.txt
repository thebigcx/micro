project(init C ASM)

set(SOURCES init.c)

set(EXECUTABLE_OUTPUT_PATH ${init_SOURCE_DIR})
add_executable(init ${SOURCES})
