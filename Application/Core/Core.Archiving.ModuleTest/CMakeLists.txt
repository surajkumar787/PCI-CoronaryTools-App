# Core.Archiving.ModuleTest executable

include(AddGoogleTest)

add_executable(Core.Archiving.ModuleTest src/Main.cpp
						src/MainControllerUnderTest.cpp
						src/PACSPushSpy.cpp)

target_include_directories(Core.Archiving.ModuleTest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(Core.Archiving.ModuleTest PUBLIC ../include/)
target_include_directories(Core.Archiving.ModuleTest PUBLIC ../TestMocks/include/)
target_include_directories(Core.Archiving.ModuleTest PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sense.1.13.1.104054/include/)
#target_include_directories(Core.Archiving.ModuleTest PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sense.1.13.1.104054/include/Sense/Graphics/)

target_link_libraries(Core.Archiving.ModuleTest PUBLIC GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(Core.Archiving.ModuleTest)

