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

add_executable(tasks tst_tasks.cpp)
add_test(NAME Tasks COMMAND Tasks)
add_custom_command(TARGET tasks
    POST_BUILD
    COMMAND rm -rf ./task-repos && cp -r ${CMAKE_CURRENT_SOURCE_DIR}/task-repos .
)
target_link_libraries(tasks PRIVATE Qt5::Test LisaBackuplib)


SETUP_TESTS(
   triggering
)
