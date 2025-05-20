cmake_minimum_required (VERSION 3.5)

if (NOT DEFINED RUN_TEST_SUITE)
option (RUN_TEST_SUITE "run test suite after install" ON)
endif (NOT DEFINED RUN_TEST_SUITE)

project(igfx_gmmumd)

enable_testing()

add_subdirectory(Source/GmmLib)
