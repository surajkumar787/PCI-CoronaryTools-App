
set_property(GLOBAL PROPERTY USE_FOLDERS ON) 

# BxLib library
add_subdirectory(BxLib)

# CbdLib library
add_subdirectory(CbdLib)

# DiaLib library
add_subdirectory(DiaLib)

# ErrLib library
add_subdirectory(ErrLib)

# MkxLib library
add_subdirectory(MkxLib)

# OscLib library
add_subdirectory(OscLib)

# PrmLib library
add_subdirectory(PrmLib)

# RdgLib library
add_subdirectory(RdgLib)

# SwLib library
add_subdirectory(SwLib)


set_target_properties(Bxlib PROPERTIES FOLDER "Algorithms")
set_target_properties(CbdLib PROPERTIES FOLDER "Algorithms")
set_target_properties(DiaLib PROPERTIES FOLDER "Algorithms")
set_target_properties(ErrLib PROPERTIES FOLDER "Algorithms")
set_target_properties(MkxLib PROPERTIES FOLDER "Algorithms")
set_target_properties(OscLib PROPERTIES FOLDER "Algorithms")
set_target_properties(PrmLib PROPERTIES FOLDER "Algorithms")
set_target_properties(RdgLib PROPERTIES FOLDER "Algorithms")
set_target_properties(SwLib PROPERTIES FOLDER "Algorithms")
