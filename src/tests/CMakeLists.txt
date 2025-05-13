add_module(Tests
        TestServerClient.cpp
)

# Link Tests with Google Test
target_link_libraries(Tests PRIVATE GTest::gtest GTest::gtest_main)
