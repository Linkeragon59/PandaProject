cmake_minimum_required(VERSION 3.16)

add_subdirectory(Panda)
add_subdirectory(PandaExe)

add_subdirectory(RhythmShooter)
add_subdirectory(RhythmShooterExe)

set_target_properties(Panda PROPERTIES FOLDER "Games")
set_target_properties(PandaExe PROPERTIES FOLDER "Games")

set_target_properties(RhythmShooter PROPERTIES FOLDER "Games")
set_target_properties(RhythmShooterExe PROPERTIES FOLDER "Games")
