wdk_add_driver(driver main.cpp main.h)
target_include_directories(driver PRIVATE ${CMAKE_SOURCE_DIR}/shared)
#target_link_libraries(driver "$ENV{WDKContentRoot}\\Lib\\10.0.22000.0\\km\\x64\\wdmsec.lib")
