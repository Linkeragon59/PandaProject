cmake_minimum_required(VERSION 3.16)

add_subdirectory(Panda)
add_subdirectory(RhythmShooter)

set_target_properties(Panda PROPERTIES FOLDER "Games")
set_target_properties(RhythmShooter PROPERTIES FOLDER "Games")
