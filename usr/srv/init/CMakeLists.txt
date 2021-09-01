project(init C ASM)

set(SOURCES init.c syscall.S)

add_compile_options(-I../../../root/usr/include)

set(EXECUTABLE_OUTPUT_PATH ${init_SOURCE_DIR})
add_executable(init ${SOURCES})
