

file( GLOB UNIT_TEST_SRCS test_*.cpp )

gtest_begin( camhd_motion_analysis_unit )

  fips_src( . test_*.cpp )

  fips_deps(camhd)

gtest_end()


# include_directories( ${GTEST_INCLUDE_DIRS}
#                      ${CMAKE_SOURCE_DIR}/lib )
#
#
#
# add_executable( camhd_motrack_unit_test
#               ${UNIT_TEST_SRCS} )
#
# link_directories( ${GTEST_LIBRARY_DIR} )
#
# target_link_libraries( camhd_motrack_unit_test
#           ${COMMON_LIBRARIES}
#         camhd )
#
# # Don't use ctest, instead just execture the Google test binaries
# add_custom_target( unit_test
#                   COMMAND ./camhd_motrack_unit_test
#                   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/bin/
#                   DEPENDS camhd_motrack_unit_test )
