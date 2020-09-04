#-------------------------------------------------------------------
# Flags common to all Linux based platforms
#-------------------------------------------------------------------

# Shell script extension
set(SHELL_SUFFIX "sh")

# This flag MUST be added to solve a bug related to shared lib dynamic loading
# (std::type_infos representing the same template type do not compare equal,
# introducing subtle bugs)
# IMPORTANT: DO NOT ADD THIS FLAG WITH STATIC LINKING
#add_flags(CMAKE_EXE_LINKER_FLAGS "-Wl,-E")

# Forbid undefined symbols at link time (shared libraries and executables)
add_flags(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
add_flags(CMAKE_EXE_LINKER_FLAGS "-Wl,--no-undefined")

# Link with the loader library
list(APPEND SYSLIBS dl)
