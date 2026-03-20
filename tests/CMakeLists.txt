include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# For Windows: prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(scheduler_tests
  test_scheduler.cpp
)

target_link_libraries(scheduler_tests
  PRIVATE
    telecom_scheduler
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(scheduler_tests)
