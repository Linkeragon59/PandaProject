cmake_minimum_required(VERSION 3.6.2)

add_subdirectory(Panda)
add_subdirectory(PandaExe)

set_target_properties(Panda PROPERTIES FOLDER "Game")
set_target_properties(PandaExe PROPERTIES FOLDER "Game")
