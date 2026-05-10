# Fix Plan: Creature Body Plan Generation System

## Root Cause
Procedural creature generation lacks concrete anatomical rules, resulting in creatures that appear anatomically incorrect or implausible. Without structured rules for spine orientation, limb attachment, limb orientation, bone primitive selection, and species variation, generated creatures fail to resemble believable animals.

## Solution Approach
Based on research from **Spore's SIGGRAPH 2008 paper** (Hecker et al.), **Rune Skovbo Johansen's procedural creature development blog**, **Eurographics Virtual Creature Morphology Review**, and **biology body plan references**, implement a tag-based hierarchical skeleton generation system with semantic parameters.

---

### Step 1: Define Creature Body Plan Templates

**File**: `src/creature/body_plans.py` (new file)

**Change**: Create enumeration and data structures for different body plan types (vertebrate, arthropod, mollusk).

**Reference**: Biology references establish that different animal phyla follow fundamentally different structural norms.

**Code**:
```python
from enum import Enum
from dataclasses import dataclass
from typing import List, Optional

class BodyPlanType(Enum):
    VERTEBRATE = "vertebrate"      # Internal skeleton, bilateral symmetry
    ARTHROPOD = "arthropod"        # Exoskeleton, segmented tagmata
    MOLLUSK = "mollusk"            # Soft body, mantle/foot/visceral mass

@dataclass
class BodyPlanTemplate:
    """Defines the structural template for a creature type."""
    body_type: BodyPlanType
    symmetry: str = "bilateral"    # bilateral, radial, asymmetrical
    skeleton_type: str = "endoskeleton"  # endoskeleton, exoskeleton, hydrostatic
    primary_axis: str = "spine"    # spine (vertebrates), ventral nerve cord (arthropods)
    limb_pairs: int = 2            # Number of paired appendages (0 for mollusks)
    segmentation: bool = True      # Whether body is segmented
    
    # Spine configuration (vertebrates only)
    spine_segments: int = 20       # Total vertebrae
    cervical_segments: int = 7     # Neck vertebrae (fixed at 7 for most mammals)
    thoracic_segments: int = 12    # Chest vertebrae (rib attachment)
    lumbar_segments: int = 5       # Lower back vertebrae
    sacral_segments: int = 5       # Pelvic attachment (fused)
    caudal_segments: int = 10      # Tail vertebrae (variable)

# Predefined templates
VERTEBRATE_TEMPLATE = BodyPlanTemplate(
    body_type=BodyPlanType.VERTEBRATE,
    limb_pairs=2,  # Forelimbs + hindlimbs
    spine_segments=39
)

ARTHROPOD_TEMPLATE = BodyPlanTemplate(
    body_type=BodyPlanType.ARTHROPOD,
    skeleton_type="exoskeleton",
    primary_axis="ventral_nerve_cord",
    limb_pairs=4,  # Insects: 3 pairs legs + optional wings
    segmentation=True
)
```

**Verification**:
- Templates correctly encode biological distinctions
- Each template has appropriate default values
- Can instantiate all three body plan types

---

### Step 2: Implement Spine Hierarchy Generator

**File**: `src/creature/spine_generator.py` (new file)

**Change**: Generate spine bone chain based on body plan template and orientation parameters.

**Reference**: Runevision's research uses spine as primary axis; all body parts positioned relative to spine segments. Biology confirms spine runs anterior→posterior in quadrupeds, superior→inferior in bipeds.

**Code**:
```python
import numpy as np
from dataclasses import dataclass
from typing import List, Tuple

@dataclass
class Bone:
    """Represents a single bone in the skeleton hierarchy."""
    name: str
    parent: Optional[str]  # Parent bone name (None for root)
    position: np.ndarray   # 3D position (world space)
    direction: np.ndarray  # Normalized direction vector
    length: float
    thickness: float
    primitive_type: str    # "cylinder", "sphere", "cone", "box"
    tags: List[str]        # Semantic tags: ["spine", "thoracic", "limb_attachment"]

@dataclass  
class SpineConfig:
    """Configuration for spine generation."""
    total_segments: int
    orientation: str = "horizontal"  # "horizontal" (quadruped) or "vertical" (biped)
    curvature: float = 0.0           # S-curve amount (negative = C-curve)
    segment_length: float = 1.0      # Base length per segment
    thickness_profile: List[float] = None  # Thickness multiplier per segment
    
    def __post_init__(self):
        if self.thickness_profile is None:
            # Default: thicker in middle, taper at ends
            self.thickness_profile = [
                0.6 + 0.4 * np.sin(i / self.total_segments * np.pi)
                for i in range(self.total_segments)
            ]

def generate_spine(config: SpineConfig) -> List[Bone]:
    """Generate a chain of spine bones.
    
    Rules:
    - Horizontal spines run along X-axis (anterior→posterior)
    - Vertical spines run along Y-axis (superior→inferior)
    - Thickness varies: thicker in thoracic region, thinner at neck/tail
    - Each spine segment tagged with region (cervical, thoracic, lumbar, etc.)
    """
    bones = []
    
    # Determine spine axis based on orientation
    if config.orientation == "horizontal":
        axis = np.array([1.0, 0.0, 0.0])  # X-axis
    elif config.orientation == "vertical":
        axis = np.array([0.0, 1.0, 0.0])  # Y-axis
    else:
        raise ValueError(f"Unknown orientation: {config.orientation}")
    
    current_position = np.array([0.0, 0.0, 0.0])
    
    for i in range(config.total_segments):
        # Determine region tag
        region = classify_spine_region(i, config.total_segments)
        
        # Create bone
        bone = Bone(
            name=f"spine_{region}_{i}",
            parent=f"spine_{region}_{i-1}" if i > 0 else None,
            position=current_position.copy(),
            direction=axis.copy(),
            length=config.segment_length * (1.0 + config.curvature * np.sin(i / config.total_segments * np.pi)),
            thickness=config.thickness_profile[i],
            primitive_type="cylinder",
            tags=["spine", region, "limb_attachment"]
        )
        
        bones.append(bone)
        
        # Advance position
        current_position += axis * bone.length
    
    return bones

def classify_spine_region(segment_index: int, total_segments: int) -> str:
    """Classify spine segment into anatomical region.
    
    Rules (from biology):
    - Cervical (neck): First 7 segments (fixed in most mammals)
    - Thoracic (chest): Next ~12 segments (rib attachment)
    - Lumbar (lower back): Next ~5 segments (flexible)
    - Sacral (pelvic): Next ~5 segments (fused)
    - Caudal (tail): Remaining segments
    """
    cervical_end = 7
    thoracic_end = cervical_end + 12
    lumbar_end = thoracic_end + 5
    sacral_end = lumbar_end + 5
    
    if segment_index < cervical_end:
        return "cervical"
    elif segment_index < thoracic_end:
        return "thoracic"
    elif segment_index < lumbar_end:
        return "lumbar"
    elif segment_index < sacral_end:
        return "sacral"
    else:
        return "caudal"
```

**Verification**:
- Horizontal spine generates along X-axis
- Vertical spine generates along Y-axis
- Segments correctly classified into anatomical regions
- Thickness profile thicker in middle, tapers at ends
- All bones tagged with semantic labels

---

### Step 3: Implement Limb Attachment System

**File**: `src/creature/limb_attachment.py` (new file)

**Change**: Attach limbs to spine at anatomically correct positions with proper orientation.

**Reference**: 
- Spore SIGGRAPH paper: Parts tagged as limbs, wings, etc. attach to spine based on semantic tags
- Bournemouth thesis: "Attach wings to the top attachment point of a random bone in the front half of the spine"
- Biology: Forelimbs attach at thoracic vertebrae (shoulders), hindlimbs at lumbar/sacral (hips)

**Code**:
```python
@dataclass
class LimbConfig:
    """Configuration for a limb attachment."""
    limb_type: str            # "forelimb", "hindlimb", "wing", "dorsal_fin", "tail"
    side: str                 # "left", "right", "center" (for tail/dorsal fin)
    attachment_region: str    # Spine region to attach to
    attachment_percentage: float  # Position within region (0.0-1.0)
    orientation: str          # "ventral" (down), "dorsal" (up), "lateral" (sideways)
    num_segments: int         # Number of bone segments in limb
    segment_lengths: List[float]  # Length of each segment

def attach_limbs_to_spine(spine_bones: List[Bone], limbs: List[LimbConfig]) -> List[Bone]:
    """Attach limbs to spine bones at correct positions.
    
    Rules:
    - Forelimbs (arms/legs): Attach to thoracic spine (~35% of spine length)
    - Hindlimbs (legs): Attach to lumbar/sacral spine (~65% of spine length)
    - Wings: Attach dorsally to front half of spine
    - Dorsal fins: Attach dorsally along spine
    - Tail: Continues from last spine segment
    - Limbs are perpendicular to spine axis
    """
    all_bones = list(spine_bones)
    
    for limb in limbs:
        # Find attachment spine bone
        attach_bone = find_attachment_bone(spine_bones, limb)
        
        # Compute attachment point
        attachment_point = attach_bone.position + attach_bone.direction * (
            attach_bone.length * limb.attachment_percentage
        )
        
        # Determine limb direction based on orientation
        limb_direction = compute_limb_direction(attach_bone.direction, limb.orientation, limb.side)
        
        # Generate limb bone chain
        limb_bones = generate_limb_chain(
            limb=limb,
            start_position=attachment_point,
            direction=limb_direction,
            parent_bone_name=attach_bone.name
        )
        
        all_bones.extend(limb_bones)
    
    return all_bones

def find_attachment_bone(spine_bones: List[Bone], limb: LimbConfig) -> Bone:
    """Find the spine bone to attach a limb to.
    
    Rules from research:
    - Forelimbs: Front half of spine, thoracic region
    - Hindlimbs: Rear half of spine, lumbar/sacral region
    - Wings: Top attachment point of bone in front half
    """
    if limb.limb_type in ["forelimb", "wing"]:
        # Attach to front half of spine
        target_index = int(len(spine_bones) * 0.35)
    elif limb.limb_type == "hindlimb":
        # Attach to rear half of spine
        target_index = int(len(spine_bones) * 0.65)
    elif limb.limb_type == "tail":
        # Attach to last spine segment
        target_index = len(spine_bones) - 1
    else:
        target_index = int(len(spine_bones) * limb.attachment_percentage)
    
    return spine_bones[target_index]

def compute_limb_direction(
    spine_direction: np.ndarray,
    orientation: str,
    side: str
) -> np.ndarray:
    """Compute limb direction relative to spine.
    
    Rules:
    - Legs/arms: Perpendicular to spine, pointing ventrally (down)
    - Wings: Opposite direction to legs, pointing dorsally (up)
    - Dorsal fins: Dorsal (up from spine)
    - Tail: Continuation of spine axis
    """
    if orientation == "ventral":
        # Perpendicular to spine, pointing down (negative Y or Z)
        limb_dir = np.array([0.0, -1.0, 0.0])
    elif orientation == "dorsal":
        # Opposite to legs, pointing up
        limb_dir = np.array([0.0, 1.0, 0.0])
    elif orientation == "lateral":
        # Pointing sideways
        sign = 1.0 if side == "right" else -1.0
        limb_dir = np.array([0.0, 0.0, sign])
    else:
        # Continuation of spine
        limb_dir = spine_direction.copy()
    
    return limb_dir / np.linalg.norm(limb_dir)

def generate_limb_chain(
    limb: LimbConfig,
    start_position: np.ndarray,
    direction: np.ndarray,
    parent_bone_name: str
) -> List[Bone]:
    """Generate a chain of limb bones.
    
    Structure:
    - Joint sphere (shoulder/hip)
    - Upper limb cylinder (humerus/femur)
    - Joint sphere (elbow/knee)
    - Lower limb cylinder (radius-ulna/tibia-fibula)
    - Joint sphere (wrist/ankle)
    - Hand/foot box or spheres
    """
    bones = []
    current_position = start_position.copy()
    
    for i in range(limb.num_segments):
        # Alternate between joint spheres and limb cylinders
        if i % 2 == 0:
            # Joint sphere
            bone_type = "sphere"
            thickness = 0.15  # Joint radius
        else:
            # Limb cylinder
            bone_type = "cylinder"
            thickness = 0.10  # Limb radius
        
        segment_length = limb.segment_lengths[i] if i < len(limb.segment_lengths) else 1.0
        
        bone = Bone(
            name=f"{limb.limb_type}_{limb.side}_segment_{i}",
            parent=parent_bone_name if i == 0 else bones[-1].name,
            position=current_position.copy(),
            direction=direction.copy(),
            length=segment_length,
            thickness=thickness,
            primitive_type=bone_type,
            tags=[limb.limb_type, limb.side, bone_type]
        )
        
        bones.append(bone)
        current_position += direction * segment_length
    
    # Add terminal cone for claws/hooves if specified
    if limb.limb_type in ["forelimb", "hindlimb"]:
        terminal_bone = Bone(
            name=f"{limb.limb_type}_{limb.side}_terminal",
            parent=bones[-1].name,
            position=current_position.copy(),
            direction=direction.copy(),
            length=0.2,
            thickness=0.05,
            primitive_type="cone",
            tags=[limb.limb_type, limb.side, "claw"]
        )
        bones.append(terminal_bone)
    
    return bones
```

**Verification**:
- Forelimbs attach at ~35% of spine length
- Hindlimbs attach at ~65% of spine length
- Limbs are perpendicular to spine axis
- Wings attach dorsally, legs attach ventrally
- Joint spheres alternate with limb cylinders
- Terminal cones for claws/hooves

---

### Step 4: Implement High-Level Semantic Parameter System

**File**: `src/creature/semantic_parameters.py` (new file)

**Change**: Create user-friendly parameters that map to low-level bone configurations.

**Reference**: Runevision's research shows that high-level parameters (bulkiness, tallness, tail thickness) are more useful than raw bone lengths/rotations.

**Code**:
```python
@dataclass
class CreatureParameters:
    """High-level semantic parameters for creature generation.
    
    All parameters normalized to [0.0, 1.0] range.
    """
    # Body shape
    bulkiness: float = 0.5          # 0.0 = slender (greyhound), 1.0 = stocky (bear)
    tallness: float = 0.5           # 0.0 = short-legged (dachshund), 1.0 = long-legged (deer)
    
    # Head
    head_length: float = 0.5        # 0.0 = short snout (cat), 1.0 = long snout (wolf)
    ear_pointiness: float = 0.5     # 0.0 = round ears (bear), 1.0 = pointy ears (fox)
    
    # Tail
    tail_length: float = 0.5        # 0.0 = no tail, 1.0 = long tail
    tail_base_thickness: float = 0.3  # 0.0 = minimal (cow), 1.0 = as thick as torso (crocodile)
    
    # Limbs
    limb_thickness: float = 0.5     # 0.0 = thin limbs, 1.0 = thick limbs
    forelimb_length: float = 0.5    # Relative to spine length
    hindlimb_length: float = 0.5    # Relative to spine length
    
    # Neck
    neck_length: float = 0.5        # 0.0 = no neck, 1.0 = long neck (giraffe)
    
    # Spine
    spine_curvature: float = 0.0    # -1.0 = C-curve, 0.0 = straight, 1.0 = S-curve
    spine_orientation: str = "horizontal"  # "horizontal" or "vertical"

def apply_semantic_parameters(
    template: BodyPlanTemplate,
    params: CreatureParameters
) -> SpineConfig:
    """Convert high-level semantic parameters to low-level spine configuration.
    
    This ensures meaningful parameter ranges and valid creature generation.
    """
    # Compute spine configuration from semantic parameters
    spine_config = SpineConfig(
        total_segments=template.spine_segments,
        orientation=params.spine_orientation,
        curvature=params.spine_curvature,
        segment_length=1.0 + params.tallness * 0.5,  # Taller creatures have longer segments
        thickness_profile=compute_thickness_profile(
            num_segments=template.spine_segments,
            bulkiness=params.bulkiness,
            neck_length=params.neck_length,
            tail_base_thickness=params.tail_base_thickness
        )
    )
    
    return spine_config

def compute_thickness_profile(
    num_segments: int,
    bulkiness: float,
    neck_length: float,
    tail_base_thickness: float
) -> List[float]:
    """Compute thickness multiplier for each spine segment.
    
    Rules:
    - Bulkiness increases overall thickness
    - Neck region tapers (controlled by neck_length)
    - Tail region tapers (controlled by tail_base_thickness)
    - Thoracic region (chest) is thickest
    """
    profile = []
    
    for i in range(num_segments):
        region = classify_spine_region(i, num_segments)
        
        if region == "cervical":
            # Neck: taper based on neck_length parameter
            taper = 1.0 - (neck_length * 0.5)
            thickness = (0.4 + 0.3 * bulkiness) * taper
        elif region == "thoracic":
            # Chest: thickest region
            thickness = 0.8 + 0.2 * bulkiness
        elif region == "lumbar":
            # Lower back: medium thickness
            thickness = 0.6 + 0.2 * bulkiness
        elif region == "sacral":
            # Pelvic: thick
            thickness = 0.7 + 0.2 * bulkiness
        else:  # caudal
            # Tail: taper from tail_base_thickness to 0
            tail_progress = (i - 29) / (num_segments - 29)  # 0.0 to 1.0
            thickness = tail_base_thickness * (1.0 - tail_progress)
        
        profile.append(thickness)
    
    return profile
```

**Verification**:
- All parameters constrained to [0.0, 1.0] range
- Bulkiness increases torso thickness uniformly
- Tallness increases spine segment length
- Tail base thickness maps to biologically valid range (cow to crocodile)
- Neck length controls cervical region taper

---

### Step 5: Implement Primitive Shape Renderer

**File**: `src/creature/primitive_renderer.py` (new file)

**Change**: Convert bone hierarchy with primitives into mesh geometry.

**Reference**: Runevision uses extruded rectangles; biology and game dev sources recommend cylinders for limbs, spheres for joints, cones for tapering structures.

**Code**:
```python
import numpy as np
from typing import List, Tuple

def generate_primitive_mesh(bone: Bone, resolution: int = 16) -> Tuple[np.ndarray, np.ndarray]:
    """Generate mesh for a single bone primitive.
    
    Returns:
        vertices: Nx3 array of vertex positions
        indices: Mx3 array of triangle indices
    """
    if bone.primitive_type == "cylinder":
        return generate_cylinder(bone, resolution)
    elif bone.primitive_type == "sphere":
        return generate_sphere(bone, resolution)
    elif bone.primitive_type == "cone":
        return generate_cone(bone, resolution)
    elif bone.primitive_type == "box":
        return generate_box(bone, resolution)
    else:
        raise ValueError(f"Unknown primitive type: {bone.primitive_type}")

def generate_cylinder(bone: Bone, resolution: int) -> Tuple[np.ndarray, np.ndarray]:
    """Generate cylinder mesh aligned with bone direction.
    
    Used for: Limb shafts, spine segments
    """
    vertices = []
    indices = []
    
    radius = bone.thickness
    length = bone.length
    direction = bone.direction / np.linalg.norm(bone.direction)
    
    # Compute perpendicular basis for cylinder cross-section
    perp1, perp2 = compute_orthogonal_basis(direction)
    
    # Generate vertices
    for ring in range(2):  # Top and bottom rings
        z = ring * length
        for i in range(resolution):
            angle = 2 * np.pi * i / resolution
            x = radius * np.cos(angle)
            y = radius * np.sin(angle)
            
            # Transform to bone's local coordinate system
            vertex = bone.position + x * perp1 + y * perp2 + z * direction
            vertices.append(vertex)
    
    # Generate indices (triangles)
    for i in range(resolution):
        next_i = (i + 1) % resolution
        # Two triangles per segment
        indices.append([i, next_i, i + resolution])
        indices.append([next_i, next_i + resolution, i + resolution])
    
    # Add end caps (circles)
    cap_center_bottom = bone.position
    cap_center_top = bone.position + direction * length
    
    bottom_center_idx = len(vertices)
    vertices.append(cap_center_bottom)
    
    top_center_idx = len(vertices)
    vertices.append(cap_center_top)
    
    for i in range(resolution):
        next_i = (i + 1) % resolution
        # Bottom cap
        indices.append([bottom_center_idx, i, next_i])
        # Top cap
        indices.append([top_center_idx, next_i + resolution, i + resolution])
    
    return np.array(vertices), np.array(indices)

def generate_sphere(bone: Bone, resolution: int) -> Tuple[np.ndarray, np.ndarray]:
    """Generate sphere mesh for joint/head.
    
    Used for: Joints (shoulder, hip, knee), head, eyes
    """
    vertices = []
    indices = []
    
    radius = bone.thickness
    rings = resolution // 2
    segments = resolution
    
    for ring in range(rings + 1):
        phi = np.pi * ring / rings
        for seg in range(segments + 1):
            theta = 2 * np.pi * seg / segments
            
            x = radius * np.sin(phi) * np.cos(theta)
            y = radius * np.sin(phi) * np.sin(theta)
            z = radius * np.cos(phi)
            
            vertex = bone.position + np.array([x, y, z])
            vertices.append(vertex)
    
    # Generate indices
    for ring in range(rings):
        for seg in range(segments):
            i0 = ring * (segments + 1) + seg
            i1 = i0 + 1
            i2 = i0 + (segments + 1)
            i3 = i2 + 1
            
            indices.append([i0, i2, i1])
            indices.append([i1, i2, i3])
    
    return np.array(vertices), np.array(indices)

def generate_cone(bone: Bone, resolution: int) -> Tuple[np.ndarray, np.ndarray]:
    """Generate cone mesh for tail/horn/claw.
    
    Used for: Tails (tapering), horns, claws, teeth
    """
    vertices = []
    indices = []
    
    base_radius = bone.thickness
    length = bone.length
    direction = bone.direction / np.linalg.norm(bone.direction)
    perp1, perp2 = compute_orthogonal_basis(direction)
    
    # Base ring
    for i in range(resolution):
        angle = 2 * np.pi * i / resolution
        x = base_radius * np.cos(angle)
        y = base_radius * np.sin(angle)
        vertex = bone.position + x * perp1 + y * perp2
        vertices.append(vertex)
    
    # Apex
    apex_idx = len(vertices)
    vertices.append(bone.position + direction * length)
    
    # Side triangles
    for i in range(resolution):
        next_i = (i + 1) % resolution
        indices.append([i, next_i, apex_idx])
    
    # Base cap
    base_center_idx = len(vertices)
    vertices.append(bone.position)
    
    for i in range(resolution):
        next_i = (i + 1) % resolution
        indices.append([base_center_idx, i, next_i])
    
    return np.array(vertices), np.array(indices)

def compute_orthogonal_basis(direction: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
    """Compute two perpendicular vectors to the given direction.
    
    Used to orient cylinders, cones perpendicular to bone axis.
    """
    # Normalize input
    direction = direction / np.linalg.norm(direction)
    
    # Pick arbitrary vector not parallel to direction
    if abs(direction[0]) < 0.9:
        temp = np.array([1.0, 0.0, 0.0])
    else:
        temp = np.array([0.0, 1.0, 0.0])
    
    # Compute perpendicular vectors using cross product
    perp1 = np.cross(direction, temp)
    perp1 = perp1 / np.linalg.norm(perp1)
    perp2 = np.cross(direction, perp1)
    perp2 = perp2 / np.linalg.norm(perp2)
    
    return perp1, perp2
```

**Verification**:
- Cylinders aligned with bone direction
- Spheres centered at joint positions
- Cones taper from base to apex
- All primitives properly oriented using orthogonal basis
- Mesh watertight (end caps included)

---

## Verification Steps

1. **Unit test spine generation**:
   - Create horizontal spine → verify bones aligned along X-axis
   - Create vertical spine → verify bones aligned along Y-axis
   - Verify segment count matches configuration
   - Verify region classification correct (cervical=0-6, thoracic=7-18, etc.)

2. **Unit test limb attachment**:
   - Attach forelimb → verify attaches at ~35% of spine
   - Attach hindlimb → verify attaches at ~65% of spine
   - Verify limbs perpendicular to spine axis
   - Verify left/right limbs on opposite sides

3. **Integration test full creature generation**:
   - Generate vertebrate with default parameters → verify plausible quadruped shape
   - Adjust bulkiness to 0.9 → verify thicker torso
   - Adjust tallness to 0.9 → verify longer legs
   - Adjust tail_length to 0.0 → verify no tail generated

4. **Visual validation**:
   - Render generated creatures in 3D viewport
   - Compare to reference animals (fox, bear, deer from Runevision's blog)
   - Verify anatomical correctness (joints at correct positions, proportions reasonable)

5. **Edge case testing**:
   - Test with extreme parameter values (all 0.0, all 1.0)
   - Verify no invalid configurations (negative lengths, NaN positions)
   - Test with minimal spine (5 segments) and maximal spine (100 segments)

---

## References

1. Hecker, C., et al. (2008). "Real-time Motion Retargeting to Highly Varied User-Created Morphologies." SIGGRAPH 2008.
2. Johansen, R.S. (2025). "Procedural creature progress 2021-2024." https://blog.runevision.com/2025/01/procedural-creature-progress-2021-2024.html
3. Lai, et al. (2021). "Virtual Creature Morphology - A Review." Computer Graphics Forum, 40(2).
4. Bournemouth University MSc Thesis (2022). "Procedural Creature Generation and Animation for Games."
5. THISISDINOSAUR. "Anatomy: A DSL capable of specifying arbitrary skeletons." GitHub.
6. Biology LibreTexts. "Animal Form and Function."
