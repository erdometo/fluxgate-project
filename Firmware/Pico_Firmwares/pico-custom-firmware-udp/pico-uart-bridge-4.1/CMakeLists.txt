# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.13)

include(pico-sdk/pico_sdk_init.cmake)

project(uart_bridge C CXX ASM)
set(PICO_BOARD pico_w)

pico_sdk_init()

add_executable(uart_bridge uart-bridge.c 
usb-descriptors.c 
)

pico_generate_pio_header(uart_bridge ${CMAKE_CURRENT_LIST_DIR}/rx.pio)

target_include_directories(uart_bridge PUBLIC
	./
	pico-sdk/lib/tinyusb/src

	)

target_link_libraries(uart_bridge
	hardware_flash
	pico_multicore
	pico_stdlib
	tinyusb_device
	hardware_pio
	pico_cyw43_arch_lwip_threadsafe_background
)


pico_add_extra_outputs(uart_bridge)
