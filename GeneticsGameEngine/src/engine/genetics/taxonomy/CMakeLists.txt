# CMakeLists.txt for genetics taxonomy module

# Add taxonomy to the genetics library
add_library(genetics_taxonomy INTERFACE)

target_include_directories(genetics_taxonomy INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(genetics_taxonomy INTERFACE
    WIN32
    _WINDOWS
    UNICODE
    _UNICODE
)

target_link_libraries(genetics_taxonomy INTERFACE
    genetics_genome
)

# Install targets
install(TARGETS genetics_taxonomy
    EXPORT geneticsTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES "Organism.h" "Animal.h" "Chordata.h" "Arthropoda.h" "Mollusca.h"
    DESTINATION include/genetics/taxonomy
)
