# CMakeLists.txt for genetics subsystem

# Add genetics subdirectories
add_subdirectory(genome)
add_subdirectory(taxonomy)
add_subdirectory(expression)
add_subdirectory(breeding)

# Create genetics library that combines all subsystems
add_library(genetics INTERFACE)

target_link_libraries(genetics INTERFACE
    genetics_genome
    genetics_taxonomy
    genetics_expression
    genetics_breeding
)

target_include_directories(genetics INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(genetics INTERFACE
    WIN32
    _WINDOWS
    UNICODE
    _UNICODE
)

# Install targets
install(TARGETS genetics
    EXPORT geneticsTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION include/genetics
    FILES_MATCHING PATTERN "*.h"
    PATTERN "CMakeLists.txt" EXCLUDE
    PATTERN "breeding/*.cpp" EXCLUDE
    PATTERN "expression/*.cpp" EXCLUDE
)
