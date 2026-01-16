project(dice_chess)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -ggdb -fno-omit-frame-pointer -Wall -Wextra -fsanitize=undefined -D_GLIBCXX_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -flto")

add_library(board OBJECT board.cpp)

add_executable(main main.cpp board)
add_executable(move_generation_test unit-tests/move_generation_test.cpp unit-tests/test_utils.cpp board)
add_executable(monte-carlo monte-carlo.cpp board)
