cmake_minimum_required (VERSION 3.1 FATAL_ERROR)

if (NOT DEFINED MEDIA_RUN_TEST_SUITE)
option (MEDIA_RUN_TEST_SUITE "run test suite after install" ON)
endif (NOT DEFINED MEDIA_RUN_TEST_SUITE)

add_subdirectory(Source/GmmLib)
