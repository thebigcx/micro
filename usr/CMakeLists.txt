add_compile_options(-Wall -Wextra -ffreestanding -g -O2 -fno-stack-protector -mno-red-zone -nostdinc)
add_link_options(-T${CMAKE_SOURCE_DIR}/linker.ld -nostdlib -static -Bsymbolic)

add_subdirectory(srv)
