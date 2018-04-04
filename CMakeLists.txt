cmake_minimum_required (VERSION 3.1 FATAL_ERROR)

if (NOT DEFINED RUN_TEST_SUITE)
option (RUN_TEST_SUITE "run test suite after install" ON)
endif (NOT DEFINED RUN_TEST_SUITE)

add_subdirectory(Source/GmmLib)
