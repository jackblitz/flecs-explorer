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
