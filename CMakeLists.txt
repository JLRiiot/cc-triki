cmake_minimum_required(VERSION 3.15)
project(triqui CXX)


find_package(raylib)


add_executable(triqui src/triqui.cpp src/main.cpp)
target_link_libraries(triqui raylib)



install(TARGETS triqui DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        )
