#include "ChordataSkeletonGenerator.h"
#include <cmath>
#include <iostream>
#include <random>
#include <algorithm>

using namespace DirectX;

namespace Engine {
namespace Animation {

Skeleton ChordataSkeletonGenerator::GenerateSkeleton(
    const Engine::Genetics::Genome& genome,
    const Engine::Procedural::Generation::CreatureParams& params)
{
    Skeleton skeleton;
    
    // Extract genetic parameters
    float bodyLength = 2.0f * params.scaleFactor;
    int vertebraCount = 8 + (static_cast<int>(genome.GetGeneValue(0x1001) * 100.0f) % 8); // 8-15 vertebrae (gene-driven!)
    float headSize = 0.4f * params.scaleFactor;
    
    // STEP 1: Generate spine (central axis)
    GenerateSpine(skeleton, bodyLength, vertebraCount);
    
    // STEP 2: Generate head at top of spine
    GenerateHead(skeleton, vertebraCount - 1, headSize);
    
    // STEP 3: Generate attachments for EACH vertebra (gene-driven!)
    // This is where genetic diversity explodes - each vertebra independently decides
    // what to grow based on genes, not hardcoded templates
    for (int i = 0; i < vertebraCount; ++i)
    {
        GenerateAttachmentsForVertebra(skeleton, i, genome, i, vertebraCount);
    }
    
    // Compute final world transforms
    skeleton.ComputeWorldTransforms();
    
    return skeleton;
}

void ChordataSkeletonGenerator::GenerateSpine(Skeleton& skeleton, float bodyLength, int vertebraCount)
{
    float vertebraSpacing = bodyLength / vertebraCount;
    
    // Create vertebrae from tail to head (along X axis - horizontal spine for quadrupeds)
    // Based on biological research: quadrupeds have horizontal spines (anterior→posterior = X-axis)
    for (int i = 0; i < vertebraCount; ++i)
    {
        std::string name = "Vertebra_" + std::to_string(i);
        
        // First vertebra is root - use absolute position
        // Subsequent vertebrae use RELATIVE offset from parent
        XMFLOAT3 position;
        if (i == 0)
        {
            position = {-bodyLength / 2.0f, 0.0f, 0.0f}; // Start at tail (negative X)
        }
        else
        {
            // Relative offset from parent (move forward along X axis)
            position = {vertebraSpacing, 0.0f, 0.0f};
        }
        
        XMFLOAT3 length = {vertebraSpacing, 0.3f, 0.3f}; // X is the length axis (horizontal spine)
        
        int parentIndex = (i > 0) ? (i - 1) : -1; // First vertebra is root
        skeleton.AddBone(CreateBone(name, position, length, parentIndex), parentIndex);
        
        // Set anatomical axes for proper voxel growth direction
        Bone& newBone = const_cast<Bone&>(skeleton.GetBones().back());
        if (i == 0)
        {
            // Root vertebra: forward = +X (along spine), up = +Y (dorsal), right = +Z (lateral)
            newBone.forwardAxis = {1.0f, 0.0f, 0.0f};
            newBone.upAxis = {0.0f, 1.0f, 0.0f};
            newBone.rightAxis = {0.0f, 0.0f, 1.0f};
        }
        else
        {
            // Subsequent vertebrae: inherit from parent
            const Bone& parent = skeleton.GetBones()[parentIndex];
            newBone.forwardAxis = parent.forwardAxis;
            newBone.upAxis = parent.upAxis;
            newBone.rightAxis = parent.rightAxis;
        }
    }
    
    // Add rib connections for thoracic vertebrae (middle section)
    // Connect left and right lateral attachment points to simulate ribcage
    int ribStart = vertebraCount / 4;      // Start ribs at 25% along spine
    int ribEnd = vertebraCount * 3 / 4;    // End ribs at 75% along spine
    
    for (int i = ribStart; i < ribEnd; ++i)
    {
        // Find left and right lateral attachment bones for this vertebra
        int vertebraBoneIndex = i;
        int leftRibIndex = -1;
        int rightRibIndex = -1;
        
        // Search for lateral attachment bones (added by GenerateAttachmentsForVertebra)
        const auto& bones = skeleton.GetBones();
        for (size_t b = 0; b < bones.size(); ++b)
        {
            if (bones[b].parentIndex == vertebraBoneIndex)
            {
                // Check if this is a lateral attachment based on position (now Z axis instead of X)
                if (bones[b].localPosition.z < -0.1f)
                    leftRibIndex = static_cast<int>(b);
                else if (bones[b].localPosition.z > 0.1f)
                    rightRibIndex = static_cast<int>(b);
            }
        }
        
        // Connect left and right ribs if both exist
        if (leftRibIndex != -1 && rightRibIndex != -1)
        {
            skeleton.AddStructuralConnection(leftRibIndex, rightRibIndex,
                                             StructuralConnectionType::RIB);
        }
    }
}

void ChordataSkeletonGenerator::GenerateHead(Skeleton& skeleton, int spineTopIndex, float headSize)
{
    XMFLOAT3 position = {headSize * 0.5f, 0.0f, 0.0f}; // In front of spine end (positive X axis)
    XMFLOAT3 length = {headSize * 1.0f, headSize * 1.2f, headSize * 1.2f}; // Larger head
    
    skeleton.AddBone(CreateBone("Head", position, length, spineTopIndex), spineTopIndex);
    
    // Set anatomical axes for head
    Bone& headBone = const_cast<Bone&>(skeleton.GetBones().back());
    headBone.forwardAxis = {1.0f, 0.0f, 0.0f}; // Forward along spine (X axis)
    headBone.upAxis = {0.0f, 1.0f, 0.0f};      // Up (Y axis)
    headBone.rightAxis = {0.0f, 0.0f, 1.0f};   // Right (Z axis)
}

// NEW: Flexible attachment system - each vertebra can grow up to 5 appendages
void ChordataSkeletonGenerator::GenerateAttachmentsForVertebra(
    Skeleton& skeleton, int vertebraIndex,
    const Engine::Genetics::Genome& genome,
    int vertebraPosition, int totalVertebrae)
{
    // DEBUG: Confirm this function is being called
    if (vertebraIndex == 0) {
        std::cout << "  [ATTACHMENT_SYSTEM] Starting attachment generation for " << totalVertebrae 
                  << " vertebrae, 5 slots each = " << (totalVertebrae * 5) << " total checks" << std::endl;
    }
    
    // Find the vertebra bone index (head is at spineTopIndex+1, so vertebrae are 0..totalVertebrae-1)
    int vertebraBoneIndex = vertebraIndex;
    
    // Get vertebra's world position for length calculations
    float normalizedPos = static_cast<float>(vertebraPosition) / totalVertebrae;
    float baseLength = 0.5f + normalizedPos * 1.0f; // Varies along spine
    
    // Check each of the 5 attachment slots
    for (int slotInt = 0; slotInt < static_cast<int>(AttachmentSlot::COUNT); ++slotInt)
    {
        AttachmentSlot slot = static_cast<AttachmentSlot>(slotInt);
        
        // RELAXED ANATOMICAL RULE: Allow limbs on more vertebrae
        // - VENTRAL slots (legs): Allow on 10-90% of spine (very permissive)
        // - LATERAL slots (arms/wings): Allow on 10-80% of spine
        // - DORSAL slots: Allow anywhere
        bool shouldGrow = true;
        
        if (slot == AttachmentSlot::VENTRAL_LEFT || slot == AttachmentSlot::VENTRAL_RIGHT)
        {
            // Legs: Allow on most of spine (10-90%)
            bool inValidRegion = (normalizedPos >= 0.10f && normalizedPos <= 0.90f);
            
            if (!inValidRegion)
            {
                shouldGrow = false; // Skip only at very beginning/end of spine
            }
        }
        else if (slot == AttachmentSlot::LEFT_LATERAL || slot == AttachmentSlot::RIGHT_LATERAL)
        {
            // Lateral limbs: Allow on 10-80% of spine
            bool inValidRegion = (normalizedPos >= 0.10f && normalizedPos <= 0.80f);
            if (!inValidRegion)
            {
                shouldGrow = false;
            }
        }
        
        if (!shouldGrow)
            continue;
        
        // Determine what grows here based on genes
        AppendageType appendageType = DetermineAppendageType(genome, vertebraIndex, slot);
        
        if (appendageType != AppendageType::NONE)
        {
            GenerateAppendage(skeleton, vertebraBoneIndex, appendageType, slot, 
                            baseLength, vertebraIndex, genome);
        }
    }
}

// Gene-driven appendage determination - THIS IS WHERE EVOLUTION HAPPENS
AppendageType ChordataSkeletonGenerator::DetermineAppendageType(
    const Engine::Genetics::Genome& genome,
    int vertebraIndex, AttachmentSlot slot) const
{
    // Each slot uses a different gene locus to determine appendage type
    // This allows independent evolution of each attachment point
    int geneLocus = 0x2000 + (vertebraIndex * 10) + static_cast<int>(slot);
    
    // Gene value 0-9 determines appendage type
    int geneValue = static_cast<int>(genome.GetGeneValue(geneLocus) * 100.0f); // 0-99
    
    // DEBUG: Log first gene check for each vertebra
    if (vertebraIndex == 0 && static_cast<int>(slot) == 0) {
        std::cout << "  [APPENDAGE DEBUG] Vertebra 0, Slot 0 (locus 0x" << std::hex << geneLocus << std::dec 
                  << "): geneValue=" << geneValue << " (raw=" << genome.GetGeneValue(geneLocus) << ")" << std::endl;
    }
    
    if (vertebraIndex < 2 && geneValue >= 30) {
        std::cout << "  [APPENDAGE] Vertebra " << vertebraIndex << ", Slot " << static_cast<int>(slot) 
                  << " (locus 0x" << std::hex << geneLocus << std::dec << "): geneValue=" << geneValue << " -> "
                  << (geneValue < 50 ? "LEG" : geneValue < 60 ? "ARM" : "OTHER") << std::endl;
    }
    
    // Probability distribution (MODIFIED for testing - more limbs!):
    // 0-9: NONE (10% chance - reduced from 30%)
    // 10-39: LEG (30% - increased)
    // 40-54: ARM (15%)
    // 55-64: WING (10%)
    // 65-69: EYE (5%)
    // 70-74: ANTENNA (5%)
    // 75-79: FIN (5%)
    // 80-84: CLAW (5%)
    // 85-89: TAIL_EXT (5%)
    // 90-99: ORGAN (10%)
    
    if (geneValue < 10) {
        // DEBUG: Show NONE results for first few vertebrae
        if (vertebraIndex < 3 && static_cast<int>(slot) < 2) {
            std::cout << "  [APPENDAGE_NONE] V" << vertebraIndex << " Slot" << static_cast<int>(slot) 
                      << " geneValue=" << geneValue << std::endl;
        }
        return AppendageType::NONE;
    }
    if (geneValue < 40) {
        std::cout << "  [LIMB_CREATED] LEG at V" << vertebraIndex << " Slot" << static_cast<int>(slot) 
                  << " geneValue=" << geneValue << std::endl;
        return AppendageType::LEG;
    }
    if (geneValue < 55) {
        std::cout << "  [LIMB_CREATED] ARM at V" << vertebraIndex << " Slot" << static_cast<int>(slot) 
                  << " geneValue=" << geneValue << std::endl;
        return AppendageType::ARM;
    }
    if (geneValue < 65) {
        std::cout << "  [LIMB_CREATED] WING at V" << vertebraIndex << " Slot" << static_cast<int>(slot) 
                  << " geneValue=" << geneValue << std::endl;
        return AppendageType::WING;
    }
    if (geneValue < 70) return AppendageType::EYE;
    if (geneValue < 75) return AppendageType::ANTENNA;
    if (geneValue < 80) return AppendageType::FIN;
    if (geneValue < 85) return AppendageType::CLAW;
    if (geneValue < 90) return AppendageType::TAIL_EXT;
    return AppendageType::ORGAN;
}

// Get segment count for a limb type from genome (uniform across all limbs of same type)
int ChordataSkeletonGenerator::GetLimbSegmentCount(
    const Engine::Genetics::Genome& genome, AppendageType type) const
{
    // Each appendage type has a dedicated gene locus for segment count
    uint16_t segmentGeneLocus = 0;
    
    switch (type) {
        case AppendageType::LEG:  segmentGeneLocus = 0x3000; break; // Leg segments
        case AppendageType::ARM:  segmentGeneLocus = 0x3001; break; // Arm segments
        case AppendageType::WING: segmentGeneLocus = 0x3002; break; // Wing segments
        case AppendageType::FIN:  segmentGeneLocus = 0x3003; break; // Fin segments
        case AppendageType::ANTENNA: segmentGeneLocus = 0x3004; break; // Antenna segments
        case AppendageType::TAIL_EXT: segmentGeneLocus = 0x3005; break; // Tail segments
        default: return 1; // Single segment for other types
    }
    
    // Gene value maps to segment count: 0.0-1.0 -> 1-5 segments
    float geneValue = genome.GetGeneValue(segmentGeneLocus);
    int segmentCount = 1 + static_cast<int>(geneValue * 100.0f) % 5; // 1-5 segments
    
    return segmentCount;
}

void ChordataSkeletonGenerator::GenerateAppendage(
    Skeleton& skeleton, int parentBoneIndex,
    AppendageType type, AttachmentSlot slot,
    float length, int vertebraIndex,
    const Engine::Genetics::Genome& genome)
{
    std::string typeName;
    XMFLOAT3 position = {0.0f, 0.0f, 0.0f};
    XMFLOAT3 boneLength = {0.15f, 0.15f, length * 0.8f}; // Thicker limbs
    XMFLOAT3 growthDirection = {0.0f, 0.0f, 0.0f}; // Direction limb should grow
    float rotation = 0.0f;
    
    // CRITICAL FIX: Position limb at attachment point ON THE SURFACE of the vertebra,
    // not at the center. The vertebra has dimensions ~0.25 x 0.30 x 0.30, so offset
    // by half the vertebra dimension in the attachment direction.
    float vertebraHalfWidth = 0.15f;  // Half of 0.30 (Y dimension)
    float vertebraHalfDepth = 0.15f;  // Half of 0.30 (Z dimension)
    
    // Position and growth direction based on attachment slot
    // ANATOMICAL ORIENTATION: Spine is horizontal (X-axis), so:
    // - DORSAL = UP (positive Y) - from top surface of vertebra
    // - VENTRAL = DOWN (negative Y) - from bottom surface of vertebra
    // - LATERAL = SIDEWAYS (±Z axis) - from side surfaces of vertebra
    switch (slot)
    {
    case AttachmentSlot::DORSAL:
        position = {0.0f, vertebraHalfWidth, 0.0f}; // Start at TOP surface
        growthDirection = {0.0f, 1.0f, 0.0f}; // Grow UP (dorsal)
        rotation = 0.0f;
        break;
    case AttachmentSlot::LEFT_LATERAL:
        position = {0.0f, 0.0f, vertebraHalfDepth}; // Start at LEFT surface
        growthDirection = {0.0f, 0.0f, 1.0f}; // Grow LEFT (positive Z)
        rotation = 0.0f;
        break;
    case AttachmentSlot::RIGHT_LATERAL:
        position = {0.0f, 0.0f, -vertebraHalfDepth}; // Start at RIGHT surface
        growthDirection = {0.0f, 0.0f, -1.0f}; // Grow RIGHT (negative Z)
        rotation = 0.0f;
        break;
    case AttachmentSlot::VENTRAL_LEFT:
        position = {0.0f, -vertebraHalfWidth, 0.0f}; // Start at BOTTOM surface
        growthDirection = {0.0f, -1.0f, 0.0f}; // Grow DOWN (ventral) for legs
        rotation = 0.0f;
        break;
    case AttachmentSlot::VENTRAL_RIGHT:
        position = {0.0f, -vertebraHalfWidth, 0.0f}; // Start at BOTTOM surface
        growthDirection = {0.0f, -1.0f, 0.0f}; // Grow DOWN (ventral) for legs
        rotation = 0.0f;
        break;
    }
    
    // Generate based on type
    switch (type)
    {
    case AppendageType::LEG:
    case AppendageType::ARM:
        typeName = (type == AppendageType::LEG) ? "Leg" : "Arm";
        {
            std::string name = typeName + "_V" + std::to_string(vertebraIndex) + 
                             "_S" + std::to_string(static_cast<int>(slot));
            
            // Get uniform segment count for this limb type
            int segmentCount = GetLimbSegmentCount(genome, type);
            
            // 1% mutation chance for this specific limb to have different segment count
            static thread_local std::mt19937 mutationRng(std::random_device{}());
            std::uniform_real_distribution<float> mutationDist(0.0f, 1.0f);
            if (mutationDist(mutationRng) < 0.01f) {
                // Mutate: random segment count 1-5
                segmentCount = 1 + (mutationRng() % 5);
                std::cout << "  [MUTATION] " << name << " mutated to " << segmentCount << " segments" << std::endl;
            }
            
            std::cout << "  [LIMB CREATED] " << name << " (" << segmentCount << " segments, parent=" << parentBoneIndex << ")" << std::endl;
            
            // Generate multi-segment limb
            GenerateMultiSegmentLimb(skeleton, name, parentBoneIndex, position, boneLength, rotation,
                                    type, segmentCount, vertebraIndex, static_cast<int>(slot), growthDirection);
        }
        break;
        
    case AppendageType::WING:
        {
            std::string name = "Wing_V" + std::to_string(vertebraIndex) + 
                             "_S" + std::to_string(static_cast<int>(slot));
            // Wings are longer and thinner
            boneLength = {0.05f, 0.05f, length * 2.0f};
            
            // Get uniform segment count for wings
            int segmentCount = GetLimbSegmentCount(genome, type);
            
            // 1% mutation chance
            static thread_local std::mt19937 mutationRng(std::random_device{}());
            std::uniform_real_distribution<float> mutationDist(0.0f, 1.0f);
            if (mutationDist(mutationRng) < 0.01f) {
                segmentCount = 1 + (mutationRng() % 5);
                std::cout << "  [MUTATION] " << name << " mutated to " << segmentCount << " segments" << std::endl;
            }
            
            std::cout << "  [LIMB CREATED] " << name << " (" << segmentCount << " segments, parent=" << parentBoneIndex << ")" << std::endl;
            GenerateMultiSegmentLimb(skeleton, name, parentBoneIndex, position, boneLength, rotation,
                                    type, segmentCount, vertebraIndex, static_cast<int>(slot), growthDirection);
        }
        break;
        
    case AppendageType::EYE:
        {
            std::string name = "Eye_V" + std::to_string(vertebraIndex) + 
                             "_S" + std::to_string(static_cast<int>(slot));
            XMFLOAT3 size = {0.1f, 0.1f, 0.1f};
            GenerateSensoryOrgan(skeleton, name, parentBoneIndex, position, size);
        }
        break;
        
    case AppendageType::ANTENNA:
        {
            std::string name = "Antenna_V" + std::to_string(vertebraIndex) + 
                             "_S" + std::to_string(static_cast<int>(slot));
            // Antennae are thin and long
            boneLength = {0.02f, 0.02f, length * 1.5f};
            GenerateLimbBone(skeleton, name, parentBoneIndex, position, boneLength, rotation);
        }
        break;
        
    case AppendageType::FIN:
        {
            std::string name = "Fin_V" + std::to_string(vertebraIndex) + 
                             "_S" + std::to_string(static_cast<int>(slot));
            // Fins are flat and wide
            boneLength = {0.3f, 0.05f, length * 0.8f};
            GenerateLimbBone(skeleton, name, parentBoneIndex, position, boneLength, rotation);
        }
        break;
        
    case AppendageType::CLAW:
        {
            std::string name = "Claw_V" + std::to_string(vertebraIndex) + 
                             "_S" + std::to_string(static_cast<int>(slot));
            boneLength = {0.08f, 0.08f, length * 0.6f};
            GenerateLimbBone(skeleton, name, parentBoneIndex, position, boneLength, rotation);
        }
        break;
        
    case AppendageType::TAIL_EXT:
        {
            std::string name = "TailExt_V" + std::to_string(vertebraIndex) + 
                             "_S" + std::to_string(static_cast<int>(slot));
            boneLength = {0.08f, 0.08f, length * 1.2f};
            GenerateLimbBone(skeleton, name, parentBoneIndex, position, boneLength, rotation);
        }
        break;
        
    case AppendageType::ORGAN:
        {
            std::string name = "Organ_V" + std::to_string(vertebraIndex) + 
                             "_S" + std::to_string(static_cast<int>(slot));
            XMFLOAT3 size = {0.2f, 0.2f, 0.2f};
            GenerateSensoryOrgan(skeleton, name, parentBoneIndex, position, size);
        }
        break;
        
    default:
        break;
    }
}

void ChordataSkeletonGenerator::GenerateLimbBone(
    Skeleton& skeleton, const std::string& name,
    int parentIndex, DirectX::XMFLOAT3 position,
    DirectX::XMFLOAT3 length, float rotationOffset)
{
    Bone bone;
    bone.name = name;
    bone.localPosition = position;
    bone.boneLength = length;
    bone.localRotation = {rotationOffset, 0.0f, 0.0f};
    bone.parentIndex = parentIndex;
    bone.mass = length.x * length.y * length.z;
    
    skeleton.AddBone(bone, parentIndex);
    
    // Set anatomical axes for limb (grows outward from attachment point)
    Bone& limbBone = const_cast<Bone&>(skeleton.GetBones().back());
    limbBone.forwardAxis = position; // Direction of growth
    
    // Calculate orthogonal basis
    XMVECTOR fwd = XMLoadFloat3(&limbBone.forwardAxis);
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMVECTOR right = XMVector3Cross(fwd, up);
    
    // If forward is parallel to up, use different up vector
    float rightLength = XMVectorGetX(XMVector3Length(right));
    if (rightLength < 0.01f)
    {
        up = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
        right = XMVector3Cross(fwd, up);
    }
    
    right = XMVector3Normalize(right);
    up = XMVector3Normalize(XMVector3Cross(right, fwd));
    
    XMStoreFloat3(&limbBone.rightAxis, right);
    XMStoreFloat3(&limbBone.upAxis, up);
}

// Generate multi-segment limb with proper bone chaining
void ChordataSkeletonGenerator::GenerateMultiSegmentLimb(
    Skeleton& skeleton, const std::string& baseName,
    int parentIndex, DirectX::XMFLOAT3 startPosition,
    DirectX::XMFLOAT3 boneLength, float rotation,
    AppendageType type, int segmentCount, int vertebraIndex, int slotIndex,
    DirectX::XMFLOAT3 growthDirection)
{
    // Normalize the growth direction
    float dirLength = std::sqrt(
        growthDirection.x * growthDirection.x +
        growthDirection.y * growthDirection.y +
        growthDirection.z * growthDirection.z
    );
    if (dirLength > 0.01f) {
        growthDirection.x /= dirLength;
        growthDirection.y /= dirLength;
        growthDirection.z /= dirLength;
    }
    
    // Total limb length is the MAXIMUM dimension of boneLength
    float totalLength = (std::max)(boneLength.x, (std::max)(boneLength.y, boneLength.z));
    
    // Each segment gets an equal portion of the total length
    float segmentLength = totalLength / segmentCount;
    
    // Base thickness is the MINIMUM dimension (cross-section)
    float baseThickness = (std::min)(boneLength.x, (std::min)(boneLength.y, boneLength.z));
    
    int currentParentIndex = parentIndex;
    
    for (int seg = 0; seg < segmentCount; ++seg) {
        // Segment name: Leg_V0_S3_Seg0, Leg_V0_S3_Seg1, etc.
        std::string segName = baseName + "_Seg" + std::to_string(seg);
        
        // Taper thickness: each segment slightly thinner
        float taperFactor = 1.0f - (seg * 0.15f); // 15% thinner per segment
        taperFactor = (std::max)(0.4f, taperFactor); // Minimum 40% thickness
        
        // Segment thickness (perpendicular to growth direction)
        float segThickness = baseThickness * taperFactor;
        
        // Segment dimensions: long along growth axis, thick perpendicular
        // We'll set the dimension parallel to growthDirection to segmentLength
        // and the other two dimensions to segThickness
        XMFLOAT3 segDimensions;
        
        // Determine which axis is the growth axis
        float absX = std::abs(growthDirection.x);
        float absY = std::abs(growthDirection.y);
        float absZ = std::abs(growthDirection.z);
        
        if (absX >= absY && absX >= absZ) {
            // Growing along X axis
            segDimensions = {segmentLength * taperFactor, segThickness, segThickness};
        } else if (absY >= absX && absY >= absZ) {
            // Growing along Y axis
            segDimensions = {segThickness, segmentLength * taperFactor, segThickness};
        } else {
            // Growing along Z axis
            segDimensions = {segThickness, segThickness, segmentLength * taperFactor};
        }
        
        // Slight bend for more natural look (except first segment)
        float segRotation = rotation;
        if (seg > 0) {
            segRotation += seg * 0.08f; // 0.08 radian bend per segment
        }
        
        // Position: each segment in its PARENT's local space
        XMFLOAT3 segPosition;
        if (seg == 0) {
            // First segment: position relative to vertebra (original parent)
            // Just use the attachment point offset (usually {0,0,0} now)
            segPosition = startPosition;
        } else {
            // Subsequent segments: position relative to previous segment's endpoint
            // The previous segment extended segmentLength along growthDirection
            segPosition = {
                growthDirection.x * segmentLength * taperFactor,
                growthDirection.y * segmentLength * taperFactor,
                growthDirection.z * segmentLength * taperFactor
            };
        }
        
        GenerateLimbBone(skeleton, segName, currentParentIndex, segPosition, segDimensions, segRotation);
        
        // DEBUG: Log segment creation with parent info
        std::cout << "  [SEGMENT CREATED] " << segName << " parent=" << currentParentIndex 
                  << " localPos=(" << segPosition.x << ", " << segPosition.y << ", " << segPosition.z 
                  << ") boneLength=(" << segDimensions.x << ", " << segDimensions.y << ", " << segDimensions.z << ")" << std::endl;
        
        // Next segment attaches to this one
        currentParentIndex = static_cast<int>(skeleton.GetBones().size()) - 1;
    }
}

void ChordataSkeletonGenerator::GenerateSensoryOrgan(
    Skeleton& skeleton, const std::string& name,
    int parentIndex, DirectX::XMFLOAT3 position,
    DirectX::XMFLOAT3 size)
{
    Bone bone;
    bone.name = name;
    bone.localPosition = position;
    bone.boneLength = size;
    bone.localRotation = {0.0f, 0.0f, 0.0f};
    bone.parentIndex = parentIndex;
    bone.mass = size.x * size.y * size.z * 0.5f; // Organs lighter
    
    skeleton.AddBone(bone, parentIndex);
}

} // namespace Animation
} // namespace Engine
