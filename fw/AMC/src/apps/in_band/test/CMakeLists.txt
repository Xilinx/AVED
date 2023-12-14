cmake_minimum_required( VERSION 3.5.0 )

project( amc )

include( CTest )
enable_testing()

#test setup - repeatable

# add_executable( <testName> <testFileName> <testFilePath> )

# target_link_libraries( <testName>
#                         cmocka 
#                         -Wl,--wrap=<wrapperFunctionName>
#                         ...         
# )

# add_test( NAME <testName>
#           COMMAND <testName>
# )
