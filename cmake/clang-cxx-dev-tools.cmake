# Additional targets to perform clang-format/clang-tidy
# Get all project files
file(GLOB_RECURSE
     ALL_CXX_SOURCE_FILES
     ${CMAKE_CURRENT_SOURCE_DIR}/src/*.[chi]pp
     ${CMAKE_CURRENT_SOURCE_DIR}/src/*.[chi]xx
     ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc
     ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hh
     ${CMAKE_CURRENT_SOURCE_DIR}/src/*.ii
     ${CMAKE_CURRENT_SOURCE_DIR}/src/*.[CHI]

     ${CMAKE_CURRENT_SOURCE_DIR}/include/*.[chi]pp
     ${CMAKE_CURRENT_SOURCE_DIR}/include/*.[chi]xx
     ${CMAKE_CURRENT_SOURCE_DIR}/include/*.cc
     ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hh
     ${CMAKE_CURRENT_SOURCE_DIR}/include/*.ii
     ${CMAKE_CURRENT_SOURCE_DIR}/include/*.[CHI]
     )

# Adding clang-format target if executable is found
find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
  get_target_property(include_dirs_list ${PROJECT_NAME} INCLUDE_DIRECTORIES)
  add_custom_target(
    clang-format
    COMMAND ${CLANG_FORMAT}
    -i
    -style=file
    ${ALL_CXX_SOURCE_FILES}
    # A bit of sed action to transform the include directories list into proper compiler flags
    # --
    # "-I\"$$(echo" "\"${include_dirs_list}\"" "|" "sed" "\"s/[^\\]" "/\\\"" "-I/g;" "s/;//g;" "s/\\\\ /" "/g\")\""
    )
endif()

# Adding clang-tidy target if executable is found
find_program(CLANG_TIDY "clang-tidy")
if(CLANG_TIDY)
  get_target_property(include_dirs_list ${PROJECT_NAME} INCLUDE_DIRECTORIES)
  add_custom_target(
    clang-tidy
    COMMAND ${CLANG_TIDY}
    ${ALL_CXX_SOURCE_FILES}
    --
    -std=c++${CMAKE_CXX_STANDARD}
    "$$(echo" "\"-extra-arg=-I\\\"${include_dirs_list}\\\"\"" "|" "sed" "\"s/;\\(.\\)/\\\"" "-extra-arg=-I\\\"\\1/g;" "s/;//g;" "s/\\\\ /" "/g\")"
    )
endif()

find_program(CPPCHECK "cppcheck")
if(CLANG_FORMAT)
  get_target_property(include_dirs_list ${PROJECT_NAME} INCLUDE_DIRECTORIES)
  add_custom_target(
   cppcheck 
   COMMAND ${CPPCHECK}
    --enable=all
    --suppress=missingIncludeSystem
    --suppress=unmatchedSuppression
    ${ALL_CXX_SOURCE_FILES}
    # --
    # "-I\"\\\"$$(echo" "\"${include_dirs_list}\"" "|" "sed" "\"s/;./\\\"" "" "-I\\\"/g;;s/;//g;s/\\\\\\\\" "/" "/g\")\\\"\""
    )
endif()
