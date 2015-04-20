cmake_minimum_required(VERSION 3.1)
project(Pcon_TNG)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    Pcon-BBB/char_fsm.c
    Pcon-BBB/char_fsm.h
    Pcon-BBB/cmd_fsm.c
    Pcon-BBB/cmd_fsm.h
    Pcon-BBB/LICENSE
    Pcon-BBB/Pcon.c
    Pcon-BBB/Pcon.h
    Pcon-BBB/schedule.c
    Pcon-BBB/schedule.h
    Pcon-BBB/sd_card.c
    Pcon-BBB/sd_card.h
    Pcon-BBB/serial_io.c
    Pcon-BBB/serial_io.h
    Pcon-BBB/trace.c
    Pcon-BBB/trace.h
    Pcon-BBB/typedefs.h
    Pcon-C3/fds.c
    Pcon-C3/fds.h
    Pcon-C3/Pcon.c
    Pcon-C3/Pcon.h
    Pcon-C3/serial_io.c
    Pcon-C3/serial_io.h
    Pcon-C3/test.h
    Pcon-share/bitlit.h
    Pcon-share/Pcon.h
    Pcon-share/schedule.c
    Pcon-share/schedule.h
    Pcon-share/shared.h
    Pcon-share/typedefs.h
    Pcon-SimpleIDe/Pcon.c
    serial-C3/fds.c
    serial-C3/fds.h
    serial-C3/test.c
    serial-C3/test.h
    stest.c
    test_sd_card.c)

add_executable(Pcon_TNG ${SOURCE_FILES})