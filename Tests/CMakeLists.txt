enable_testing()

# These commented out lines is the equivalense to the loop below.
add_executable(strings tst_strings.cpp)
add_test(NAME Strings COMMAND Strings)
target_link_libraries(strings PRIVATE Qt5::Test LisaBackuplib)

add_executable(triggering tst_triggering.cpp)
add_test(NAME Triggering COMMAND Triggering)
target_link_libraries(triggering PRIVATE Qt5::Test LisaBackuplib)

# add_executable(Gadgets tst_gadgets.cpp)
# add_test(NAME Gadgets COMMAND Gadgets)
# target_link_libraries(Gadgets PRIVATE Qt5::Test QWAMLib)


#function(SETUP_TESTS)
#       foreach(_testname ${ARGN})
#           add_executable(${_testname} tst_${_testname}.cpp )
#           add_test(NAME ${_testname} COMMAND ${_testname})
#           target_link_libraries(${_testname} Qt${QT_MAJOR_VERISION}::Test)
#       endforeach()
#endfunction()

#SETUP_TESTS(
#   strings
#)
