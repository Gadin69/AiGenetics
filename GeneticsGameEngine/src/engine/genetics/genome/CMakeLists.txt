# CMakeLists.txt for genetics genome module

# Add genome to the genetics library
add_library(genetics_genome INTERFACE)

target_include_directories(genetics_genome INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(genetics_genome INTERFACE
    WIN32
    _WINDOWS
    UNICODE
    _UNICODE
)

# Install targets
install(TARGETS genetics_genome
    EXPORT geneticsTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES "Chromosome.h" "Gene.h" "Genome.h"
    DESTINATION include/genetics/genome
)
