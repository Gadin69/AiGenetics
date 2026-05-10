#include "NeuralSystemManager.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <functional>
#include <cmath>

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
    
    // Execute batch forward pass with CREATURE-SPECIFIC sensory inputs
    // Each creature gets different inputs based on its properties for varied activation patterns
    std::vector<std::vector<float>> inputs;
    inputs.reserve(m_networks.size());
    
    for (size_t i = 0; i < m_networks.size(); ++i) {
        const auto& context = m_networks[i];
        
        // Generate diverse sensory inputs per creature
        // Use network index and time to create varying sensor readings
        float timePhase = static_cast<float>(m_frameCounter) * 0.01f;
        float creatureOffset = static_cast<float>(i) * 0.37f; // Golden ratio offset
        
        // Simulated sensory inputs that vary per creature and over time
        std::vector<float> sensorInputs;
        
        // Input 0: Environmental stimulus (varies per creature)
        sensorInputs.push_back(0.5f + 0.3f * std::sin(timePhase + creatureOffset));
        
        // Input 1: Internal state (based on genome expression)
        if (context.genome) {
            float avgExpression = 0.5f; // Default
            int activeCount = 0;
            for (const auto& chromosome : context.genome->GetChromosomes()) {
                for (const auto& gene : chromosome.GetGenes()) {
                    if (gene.alleleValue > 0.1f) {
                        avgExpression += gene.alleleValue;
                        activeCount++;
                    }
                }
            }
            if (activeCount > 0) avgExpression /= activeCount;
            sensorInputs.push_back(avgExpression + 0.2f * std::sin(timePhase * 1.3f + creatureOffset * 2.0f));
        } else {
            sensorInputs.push_back(0.5f + 0.3f * std::cos(timePhase * 0.7f + creatureOffset));
        }
        
        // Input 2: Neural feedback (oscillates differently per creature)
        sensorInputs.push_back(0.5f + 0.4f * std::sin(timePhase * 1.7f + creatureOffset * 3.0f));
        
        // Clamp to 0-1 range
        for (auto& val : sensorInputs) {
            val = std::max(0.0f, std::min(1.0f, val));
        }
        
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
    // CRITICAL FIX: Use genome to determine neural network structure
    // Each creature's genome creates a UNIQUE neural network
    const auto& genome = *context.genome;
    
    // Calculate genome complexity (number of active genes)
    int activeGeneCount = 0;
    float totalExpression = 0.0f;
    for (const auto& chromosome : genome.GetChromosomes()) {
        for (const auto& gene : chromosome.GetGenes()) {
            // Consider genes with alleleValue > 0.1 as "active"
            if (gene.alleleValue > 0.1f) {
                activeGeneCount++;
                totalExpression += gene.alleleValue;
            }
        }
    }
    
    // Genome-driven network sizing
    // More active genes = more complex neural network
    int sensoryCount = std::max(3, activeGeneCount / 10);  // 1 sensory per 10 active genes (min 3)
    int motorCount = std::max(2, activeGeneCount / 15);    // 1 motor per 15 active genes (min 2)
    int hiddenCount = std::max(5, activeGeneCount / 5);    // 1 hidden per 5 active genes (min 5)
    
    // Clamp to reasonable limits
    sensoryCount = std::min(sensoryCount, 10);
    motorCount = std::min(motorCount, 8);
    hiddenCount = std::min(hiddenCount, 30);
    
    std::cout << "  [NN Init] Genome-driven network: " << sensoryCount << " sensory, "
              << hiddenCount << " hidden, " << motorCount << " motor neurons" << std::endl;
    
    // Use genome ID hash as seed for random number generator
    // This ensures each creature gets deterministic but unique connections
    std::hash<std::string> genomeHash;
    size_t hashValue = genomeHash(genome.GetID());
    std::mt19937 rng(static_cast<unsigned int>(hashValue));
    std::uniform_real_distribution<float> weightDist(-0.5f, 0.5f);
    
    // Create sensory neurons
    for (int i = 0; i < sensoryCount; ++i) {
        Neuron sensoryNeuron(NeuronType::Sensory);
        sensoryNeuron.activation = 0.5f;
        context.network->AddNeuron(std::move(sensoryNeuron));
    }
    
    // Create hidden/general neurons
    for (int i = 0; i < hiddenCount; ++i) {
        Neuron hiddenNeuron(NeuronType::General);
        // Set threshold based on genome expression
        hiddenNeuron.threshold = 0.2f + (totalExpression / std::max(activeGeneCount, 1)) * 0.3f;
        context.network->AddNeuron(std::move(hiddenNeuron));
    }
    
    // Create motor neurons
    for (int i = 0; i < motorCount; ++i) {
        Neuron motorNeuron(NeuronType::Motor);
        context.network->AddNeuron(std::move(motorNeuron));
    }
    
    // Create genome-driven connections
    uint32_t sensoryStart = 0;
    uint32_t hiddenStart = static_cast<uint32_t>(sensoryCount);
    uint32_t motorStart = static_cast<uint32_t>(sensoryCount + hiddenCount);
    
    // Connect sensory to hidden neurons (with genome-influenced density)
    float connectionDensity = std::min(0.8f, totalExpression / std::max(activeGeneCount, 1));
    std::uniform_real_distribution<float> densityDist(0.0f, 1.0f);
    
    for (uint32_t sensoryIdx = sensoryStart; sensoryIdx < sensoryStart + sensoryCount; ++sensoryIdx) {
        for (uint32_t hiddenIdx = hiddenStart; hiddenIdx < hiddenStart + hiddenCount; ++hiddenIdx) {
            if (densityDist(rng) < connectionDensity) {
                float weight = weightDist(rng);
                context.network->AddSynapse(sensoryIdx, hiddenIdx, weight);
            }
        }
    }
    
    // Connect hidden to motor neurons
    for (uint32_t hiddenIdx = hiddenStart; hiddenIdx < hiddenStart + hiddenCount; ++hiddenIdx) {
        for (uint32_t motorIdx = motorStart; motorIdx < motorStart + motorCount; ++motorIdx) {
            if (densityDist(rng) < connectionDensity) {
                float weight = weightDist(rng);
                context.network->AddSynapse(hiddenIdx, motorIdx, weight);
            }
        }
    }
    
    // Add some random hidden-to-hidden connections for recurrent behavior
    int recurrentConnections = hiddenCount / 3;
    std::uniform_int_distribution<uint32_t> hiddenDist(hiddenStart, hiddenStart + hiddenCount - 1);
    for (int i = 0; i < recurrentConnections; ++i) {
        uint32_t from = hiddenDist(rng);
        uint32_t to = hiddenDist(rng);
        if (from != to) {
            float weight = weightDist(rng) * 0.3f; // Weaker recurrent connections
            context.network->AddSynapse(from, to, weight);
        }
    }
    
    std::cout << "  [NN Init] Created network with " 
              << context.network->GetNeurons().size() << " neurons, "
              << context.network->GetSynapses().size() << " synapses" << std::endl;
}

} // namespace Neural
} // namespace Engine
