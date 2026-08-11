add_executable(test_main
    test_main.c
)

target_include_directories(test_main PRIVATE
    ${CMAKE_SOURCE_DIR}/vendor/cJSON
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_main PRIVATE
    cjson
)

add_test(NAME test_main COMMAND test_main)

add_executable(test_terminal
    test_terminal.c
    ${CMAKE_SOURCE_DIR}/src/terminal.c
)

target_include_directories(test_terminal PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${NCURSES_INCLUDE_DIRS}
)

target_link_libraries(test_terminal PRIVATE
    ${NCURSES_LIBRARIES}
)

add_test(NAME test_terminal COMMAND test_terminal)
