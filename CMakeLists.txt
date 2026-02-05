cmake_minimum_required(VERSION 3.15)
project(ZISA-Emulator VERSION 1.0 LANGUAGES C)

# Set C standard to C99
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

SET(ENABLE_DEBUGGER FALSE)
SET(SHOW_FPS FALSE)

if(SHOW_FPS)
list(APPEND DEFINITIONS CONFIG_SHOW_FPS)
endif()

if(APPLE)
list(APPEND DEFINITIONS _DARWIN_C_SOURCE)
endif()

set(HARDWARE_SOURCES
    hw/flash.c
    hw/keyboard.c
    hw/main.c
    hw/mmu.c
    hw/pio.c
    hw/ram.c
    hw/uart.c
    hw/z80.c
    hw/zeal.c
    hw/i2c.c
    hw/i2c/ds1307.c
    hw/i2c/at24c512.c
    hw/compactflash.c
    # video backend
    hw/zvb/default_font.c
    hw/zvb/zvb.c
    hw/zvb/zvb_font.c
    hw/zvb/zvb_palette.c
    hw/zvb/zvb_sprites.c
    hw/zvb/zvb_text.c
    hw/zvb/zvb_spi.c
    hw/zvb/zvb_tilemap.c
    hw/zvb/zvb_tileset.c
    hw/zvb/zvb_crc32.c
    hw/zvb/zvb_dma.c
    hw/zvb/zvb_sound.c
)

set(UTILS_SOURCES
    utils/fifo.c
    utils/paths.c
    utils/config.c
)

# only include debugger sources if debugger is enabled
if(ENABLE_DEBUGGER)
set(DEBUGGER_SOURCES
    hw/zeal_debugger.c
    hw/zeal_input.c
    hw/debugger/debugger.c
    hw/debugger/debugger_ui.c
    hw/debugger/disassembler_z80.c
    hw/debugger/raylib-nuklear.c
    hw/debugger/panels/breakpoints.c
    hw/debugger/panels/cpu.c
    hw/debugger/panels/disassembler.c
    hw/debugger/panels/display.c
    hw/debugger/panels/memory.c
    hw/debugger/panels/menubar.c
    hw/debugger/panels/mmu.c
    hw/debugger/panels/vram.c
)
list(APPEND DEFINITIONS CONFIG_ENABLE_DEBUGGER)
endif()

# shaders/CMakeLists.txt will generate shader headers
add_subdirectory(assets/shaders)

add_executable(zisaemu
    ${HARDWARE_SOURCES}
    ${UTILS_SOURCES}
    ${DEBUGGER_SOURCES})

# seperate for windows/linux
set(RAYLIB_DIR)
if(WIN32)
    set(RAYLIB_DIR ${CMAKE_SOURCE_DIR}/raylib/win64_msvc)
else()
    set(RAYLIB_DIR ${CMAKE_SOURCE_DIR}/raylib/linux64)
endif()

target_compile_definitions(zisaemu PRIVATE 
    ${DEFINITIONS}
    _CRT_SECURE_NO_WARNINGS)
target_include_directories(zisaemu PRIVATE 
    ${RAYLIB_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/assets)
target_link_directories(zisaemu PRIVATE ${RAYLIB_DIR}/lib)
target_compile_options(zisaemu PRIVATE
  -Wall
  -Wextra
  -Werror
  -pedantic
  -Wno-unused-function)
target_link_libraries(zisaemu raylib winmm gdi32)
add_dependencies(zisaemu generate_shaders)
