project(dice_chess)
set(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -Wall -Wshadow -Wextra -Og")
add_executable(main main.cpp board.cpp)
