# CMakeLists.txt for genetics breeding module

# Add breeding to the genetics library
add_library(genetics_breeding INTERFACE)

target_include_directories(genetics_breeding INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(genetics_breeding INTERFACE
    WIN32
    _WINDOWS
    UNICODE
    _UNICODE
)

target_link_libraries(genetics_breeding INTERFACE
    genetics_genome
    genetics_taxonomy
)

# Install targets
install(TARGETS genetics_breeding
    EXPORT geneticsTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES "BreedingSystem.h" "MutationEngine.h"
    DESTINATION include/genetics/breeding
)
