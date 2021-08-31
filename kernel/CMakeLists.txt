project(vmkernel C ASM)

set(ARCHSRC ${vmkernel_SOURCE_DIR}/arch/x86_64)
add_subdirectory(arch/x86_64)

set(KERNSRC ${vmkernel_SOURCE_DIR}/kern)
add_subdirectory(kern)

include_directories(arch/x86_64/include)
include_directories(include/kern)

add_compile_options(-Wall -Wextra -ffreestanding -g -DDEBUG -fno-stack-protector -fno-pic -fpie -mno-red-zone -nostdinc -fsanitize=undefined)
#add_compile_options(-Wall -Wextra -ffreestanding -g -fno-stack-protector -fno-pic -fpie -mno-red-zone -nostdinc)
add_link_options(-T${vmkernel_SOURCE_DIR}/kernel.ld -nostdlib)

set(EXECUTABLE_OUTPUT_PATH ${vmkernel_SOURCE_DIR}/dist)
add_executable(vmkernel ${SOURCES})
