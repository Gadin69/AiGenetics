#include "NeuralSystemManager.h"
#include <iostream>
#include <algorithm>

namespace Engine {
namespace Neural {

NeuralSystemManager& NeuralSystemManager::GetInstance()
{
    static NeuralSystemManager instance;
    return instance;
}

NeuralSystemManager::NeuralSystemManager()
    : m_pruningInterval(300)  // Prune every 300 frames (~5 seconds at 60 FPS)
    , m_frameCounter(0)
{
}

NeuralSystemManager::~NeuralSystemManager()
{
    Clear();
}

NeuralSystemManager::NetworkContext::NetworkContext()
    : genome(nullptr)
    , survivalTime(0.0f)
    , movementEfficiency(0.0f)
    , goalAchievement(0.0f)
    , energyConsumption(0.0f)
{
}

void NeuralSystemManager::Initialize()
{
    std::cout << "Initializing Neural System Manager..." << std::endl;
    
    // Reserve capacity for performance
    m_networks.reserve(1000);
    
    std::cout << "Neural System Manager initialized successfully." << std::endl;
}

size_t NeuralSystemManager::CreateNetworkForOrganism(const Engine::Genetics::Genome& genome)
{
    // Create new network context
    NetworkContext context;
    context.network = std::make_unique<NeuralNetwork>();
    
    // Make a PERSISTENT copy of the genome to avoid dangling references
    context.genome = std::make_unique<Engine::Genetics::Genome>(genome);
    
    // Initialize subsystems with pointer to the persistent genome
    context.growthController = std::make_unique<GrowthController>(*context.network, context.genome.get());
    context.pruningEngine = std::make_unique<PruningEngine>(*context.network);
    context.learner = std::make_unique<ReinforcementLearner>(*context.network);
    context.fitnessEvaluator = std::make_unique<FitnessEvaluator>();
    
    // Initialize network with default neurons
    InitializeNetwork(context);
    
    // Add to network list
    size_t networkIndex = m_networks.size();
    m_networks.push_back(std::move(context));
    
    // Add to batch processor
    m_batchProcessor.AddNetwork(m_networks.back().network.get());
    
    return networkIndex;
}

void NeuralSystemManager::UpdateAllNetworks(float deltaTime)
{
    m_frameCounter++;
    
    // Update survival time for all networks
    for (auto& context : m_networks) {
        context.survivalTime += deltaTime;
    }
    
    // Execute batch forward pass with default inputs (environmental sensors)
    // In a full implementation, you'd gather actual sensor data
    std::vector<std::vector<float>> inputs;
    inputs.reserve(m_networks.size());
    
    for (const auto& context : m_networks) {
        // Default: 3 sensory inputs (simplified)
        std::vector<float> sensorInputs = {0.5f, 0.5f, 0.5f};
        inputs.push_back(std::move(sensorInputs));
    }
    
    if (!inputs.empty()) {
        m_batchProcessor.ExecuteBatchForwardPass(inputs);
    }
    
    // Periodic pruning to prevent memory bloat
    if (m_frameCounter % m_pruningInterval == 0) {
        PruneAllNetworks();
    }
}

int NeuralSystemManager::TriggerGrowth(size_t networkIndex)
{
    if (networkIndex >= m_networks.size()) {
        return 0;
    }
    
    auto& context = m_networks[networkIndex];
    
    // Execute hybrid growth trigger
    int neuronsAdded = context.growthController->ExecuteHybridGrowthTrigger();
    
    // Rebuild batches if network topology changed
    if (neuronsAdded > 0) {
        m_batchProcessor.Clear();
        for (auto& ctx : m_networks) {
            m_batchProcessor.AddNetwork(ctx.network.get());
        }
    }
    
    return neuronsAdded;
}

void NeuralSystemManager::ApplyLearning(size_t networkIndex, float reward)
{
    if (networkIndex >= m_networks.size()) {
        return;
    }
    
    auto& context = m_networks[networkIndex];
    
    // Record fitness
    float fitness = context.fitnessEvaluator->EvaluateFitness(
        context.survivalTime,
        context.movementEfficiency,
        context.goalAchievement,
        context.energyConsumption
    );
    
    context.fitnessEvaluator->RecordFitness(fitness);
    
    // Apply learning with reward
    context.learner->Learn(reward, context.learner->GetLearningRate());
    
    // Decay learning rate over time
    context.learner->DecayLearningRate(0.999f);
}

void NeuralSystemManager::ExecuteForwardPass(size_t networkIndex, const std::vector<float>& inputs)
{
    if (networkIndex < m_networks.size()) {
        m_networks[networkIndex].network->ForwardPass(inputs);
    }
}

std::vector<float> NeuralSystemManager::GetNetworkOutputs(size_t networkIndex) const
{
    if (networkIndex < m_networks.size()) {
        return m_networks[networkIndex].network->GetOutputs();
    }
    return std::vector<float>();
}

NeuralNetwork* NeuralSystemManager::GetNetwork(size_t index)
{
    if (index < m_networks.size()) {
        return m_networks[index].network.get();
    }
    return nullptr;
}

int NeuralSystemManager::PruneAllNetworks(float weightThreshold)
{
    int totalPruned = 0;
    
    for (auto& context : m_networks) {
        totalPruned += context.pruningEngine->ExecutePruning(weightThreshold);
    }
    
    return totalPruned;
}

void NeuralSystemManager::Clear()
{
    m_networks.clear();
    m_batchProcessor.Clear();
    m_frameCounter = 0;
}

void NeuralSystemManager::InitializeNetwork(NetworkContext& context)
{
    // Create default sensory neurons (3 inputs)
    for (int i = 0; i < 3; ++i) {
        Neuron sensoryNeuron(NeuronType::Sensory);
        sensoryNeuron.activation = 0.5f;
        context.network->AddNeuron(std::move(sensoryNeuron));
    }
    
    // Create default motor neurons (2 outputs)
    for (int i = 0; i < 2; ++i) {
        Neuron motorNeuron(NeuronType::Motor);
        context.network->AddNeuron(std::move(motorNeuron));
    }
    
    // Create some initial general neurons
    for (int i = 0; i < 5; ++i) {
        Neuron generalNeuron(NeuronType::General);
        context.network->AddNeuron(std::move(generalNeuron));
    }
    
    // Create initial random connections
    // Connect sensory to general neurons
    for (uint32_t sensoryIdx = 0; sensoryIdx < 3; ++sensoryIdx) {
        for (uint32_t generalIdx = 3; generalIdx < 8; ++generalIdx) {
            float weight = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.5f;
            context.network->AddSynapse(sensoryIdx, generalIdx, weight);
        }
    }
    
    // Connect general to motor neurons
    for (uint32_t generalIdx = 3; generalIdx < 8; ++generalIdx) {
        for (uint32_t motorIdx = 8; motorIdx < 10; ++motorIdx) {
            float weight = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.5f;
            context.network->AddSynapse(generalIdx, motorIdx, weight);
        }
    }
}

} // namespace Neural
} // namespace Engine
