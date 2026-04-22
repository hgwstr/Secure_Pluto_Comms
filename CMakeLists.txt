cmake_minimum_required(VERSION 3.14)
project(pluto_chat CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFTW3 REQUIRED fftw3)

add_library(pluto_dsp STATIC
    src/dsp_utils.cpp
    src/pulse_shape.cpp
    src/oversample.cpp
    src/barker_generator.cpp
    src/symbol_conversion.cpp
    src/maximum_frequency.cpp
    src/interpolation_with_sinc.cpp
    src/generate_filter.cpp
    src/quantization.cpp
    src/symbol_correlation.cpp
    src/costas_loop.cpp
    src/clock_recovery.cpp
    src/frame_generator_tx.cpp
    src/signal_generator_tx.cpp
    src/frame_generator_rx.cpp
    src/message_handler.cpp
)

target_include_directories(pluto_dsp PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${FFTW3_INCLUDE_DIRS}
)

target_link_libraries(pluto_dsp PUBLIC
    ${FFTW3_LIBRARIES}
    m
)

target_link_directories(pluto_dsp PUBLIC
    ${FFTW3_LIBRARY_DIRS}
)

pkg_check_modules(LIBIIO libiio)

if(LIBIIO_FOUND)
    add_executable(pluto_chat
        src/main.cpp
        src/my_radio.cpp
        src/operation_tx.cpp
        src/operation_rx.cpp
    )

    target_include_directories(pluto_chat PRIVATE
        ${LIBIIO_INCLUDE_DIRS}
    )

    target_link_libraries(pluto_chat PRIVATE
        pluto_dsp
        ${LIBIIO_LIBRARIES}
        pthread
    )

    target_link_directories(pluto_chat PRIVATE
        ${LIBIIO_LIBRARY_DIRS}
    )
else()
    message(WARNING "libiio not found — skipping pluto_chat target. "
                    "Only test will be built.")
endif()

add_executable(test
    tests/test.cpp
)

target_link_libraries(test PRIVATE
    pluto_dsp
    pthread
)
