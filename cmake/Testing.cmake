if(NOT BUILD_TESTING)
    return()
endif()

include(CTest)
find_package(GTest REQUIRED)

file(GLOB TEST_SRC ${CMAKE_SOURCE_DIR}/src/tests/*.cpp)

if(ENABLE_VALGRIND)
    find_program(VALGRIND valgrind)
    if(VALGRIND)
        set(MEMCHECK_CMD ${VALGRIND} --leak-check=full --error-exitcode=123)
    endif()
endif()

foreach(T ${TEST_SRC})
    get_filename_component(NAME ${T} NAME_WE)
    add_executable(${NAME} ${T})
    target_link_libraries(${NAME} PRIVATE sigilvm GTest::gtest GTest::gtest_main)

    if(ENABLE_VALGRIND AND MEMCHECK_CMD)
        add_test(NAME ${NAME} COMMAND ${MEMCHECK_CMD} $<TARGET_FILE:${NAME}>)
    else()
        add_test(NAME ${NAME} COMMAND ${NAME})
    endif()
endforeach()
