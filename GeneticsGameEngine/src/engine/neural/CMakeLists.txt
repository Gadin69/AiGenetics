# Neural network subsystem CMake configuration

# Core module
set(NEURAL_CORE_SOURCES
    core/NeuralNetwork.cpp
)

set(NEURAL_CORE_HEADERS
    core/Neuron.h
    core/Synapse.h
    core/NeuralNetwork.h
)

# Growth module
set(NEURAL_GROWTH_SOURCES
    growth/GrowthController.cpp
    growth/PruningEngine.cpp
)

set(NEURAL_GROWTH_HEADERS
    growth/GrowthController.h
    growth/PruningEngine.h
)

# Learning module
set(NEURAL_LEARNING_SOURCES
    learning/ReinforcementLearner.cpp
    learning/FitnessEvaluator.cpp
)

set(NEURAL_LEARNING_HEADERS
    learning/ReinforcementLearner.h
    learning/FitnessEvaluator.h
)

# Optimization module
set(NEURAL_OPTIMIZATION_SOURCES
    optimization/BatchProcessor.cpp
)

set(NEURAL_OPTIMIZATION_HEADERS
    optimization/MemoryPool.h
    optimization/BatchProcessor.h
)

# Manager
set(NEURAL_MANAGER_SOURCES
    NeuralSystemManager.cpp
)

set(NEURAL_MANAGER_HEADERS
    NeuralSystemManager.h
)

# Create neural library
add_library(Neural STATIC
    ${NEURAL_CORE_SOURCES}
    ${NEURAL_CORE_HEADERS}
    ${NEURAL_GROWTH_SOURCES}
    ${NEURAL_GROWTH_HEADERS}
    ${NEURAL_LEARNING_SOURCES}
    ${NEURAL_LEARNING_HEADERS}
    ${NEURAL_OPTIMIZATION_SOURCES}
    ${NEURAL_OPTIMIZATION_HEADERS}
    ${NEURAL_MANAGER_SOURCES}
    ${NEURAL_MANAGER_HEADERS}
)

# Set MSVC runtime library
if(MSVC)
    set_target_properties(Neural PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    )
endif()

# Set compile options
set_target_properties(Neural PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Add /FS flag for PDB file sharing (MSVC parallel compilation fix)
if(MSVC)
    target_compile_options(Neural PRIVATE "/FS")
endif()

# Include directories
target_include_directories(Neural PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add compile definitions for Windows
target_compile_definitions(Neural PRIVATE
    WIN32
    _WINDOWS
    UNICODE
    _UNICODE
    NOMINMAX
)

# Link dependencies (none required for neural system)
# target_link_libraries(Neural PRIVATE
#     ...
# )
