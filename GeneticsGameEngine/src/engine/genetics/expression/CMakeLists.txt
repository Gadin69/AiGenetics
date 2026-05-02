# CMakeLists.txt for genetics expression module

# Add expression to the genetics library
add_library(genetics_expression INTERFACE)

target_include_directories(genetics_expression INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(genetics_expression INTERFACE
    WIN32
    _WINDOWS
    UNICODE
    _UNICODE
)

target_link_libraries(genetics_expression INTERFACE
    genetics_genome
    genetics_taxonomy
)

# Install targets
install(TARGETS genetics_expression
    EXPORT geneticsTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES "ExpressionRules.h" "GeneticExpression.h"
    DESTINATION include/genetics/expression
)
