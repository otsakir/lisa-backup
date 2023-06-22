enable_testing()

# These commented out lines is the equivalent to the loop below.
#add_executable(triggering tst_triggering.cpp)
#add_test(NAME Triggering COMMAND Triggering)
#target_link_libraries(triggering PRIVATE Qt5::Test LisaBackuplib)


function(SETUP_TESTS)
       foreach(_testname ${ARGN})
           add_executable(${_testname} tst_${_testname}.cpp )
           add_test(NAME ${_testname} COMMAND ${_testname})
           target_link_libraries(${_testname} PRIVATE Qt5::Test LisaBackuplib)
       endforeach()
endfunction()

SETUP_TESTS(
   triggering
)
