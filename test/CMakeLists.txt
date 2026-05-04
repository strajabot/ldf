find_package(Catch2 3 REQUIRED)
include(Catch)

file(GLOB TEST_SOURCES "test_*.cpp")

if(TEST_SOURCES)
    add_executable(tests ${TEST_SOURCES})
    target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/ldf ${CMAKE_SOURCE_DIR}/proposal/include/ldf)
    target_compile_options(tests PRIVATE -fno-pie)
    target_link_options(tests PRIVATE -no-pie)
    target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
    catch_discover_tests(tests)
endif()
