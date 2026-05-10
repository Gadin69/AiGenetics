# Research Summary: Procedural Creature Generation

## Overview
This document summarizes formal technical research conducted on two critical topics for procedural creature generation in the 3dGenetics project.

---

## Topic 1: Creature Body Plan Generation Rules

**Research File**: `.qoder/research/creature-body-plan-rules.md`  
**Fix Plan**: `.qoder/plans/creature-body-plan-rules-fix.md`

### Key Findings

#### 1. Spine Orientation Rules
- **Quadrupeds**: Horizontal spine (parallel to ground), runs anterior→posterior
- **Bipeds**: Vertical spine (perpendicular to ground), runs superior→inferior
- **Swimming creatures**: Horizontal spine for undulation-based propulsion
- **Flying creatures**: Horizontal spine during flight for streamlined body

**Source**: Biology references (LibreTexts, Lumen Learning) confirm spine always runs along anterior-posterior axis. Spore SIGGRAPH 2008 paper confirms tag-based morphological classification.

#### 2. Limb Attachment Hierarchy
- **Forelimbs** (arms/wings): Attach at ~35% of spine length (thoracic vertebrae)
- **Hindlimbs** (legs): Attach at ~65% of spine length (lumbar/sacral vertebrae)
- **Wings**: Attach dorsally (top) to front half of spine
- **Tail**: Continues from last spine segment

**Source**: Bournemouth University MSc thesis, Rune Skovbo Johansen's procedural creature blog, Anatomy DSL GitHub project.

#### 3. Limb Orientation
- **Legs/arms**: Perpendicular to spine (90°), pointing ventrally (downward)
- **Wings**: Opposite direction to legs, pointing dorsally (upward) or laterally
- **Dorsal fins**: Dorsal (upward from spine)
- **Tail**: Continuation of spine axis (posterior)

**Source**: Biology (bilateral symmetry), Spore SIGGRAPH paper (semantic tagging system).

#### 4. Bone Primitive Placement
- **Cylinders**: Limb shafts (upper arm, lower leg), spine segments
- **Spheres**: Joints (shoulder, hip, knee), head, eyes
- **Cones**: Tails (tapering), horns, claws, teeth
- **Boxes**: Torso segments, flat bones (scapula, pelvis)

**Source**: Runevision's blog (extruded rectangles surprisingly effective), Kamil VFX rigging tutorial (start with primitives).

#### 5. Species Variation Rules
- **Vertebrates**: Internal skeleton, bilateral symmetry, spine-based body plan
- **Arthropods**: Exoskeleton, segmented tagmata, jointed appendages (no spine)
- **Mollusks**: Soft body, mantle/foot/visceral mass, no skeleton

**Source**: Eurographics CGF review (Lai et al., 2021), biology curriculum resources.

### Implementation Plan
**5-step approach**:
1. Define body plan templates (vertebrate, arthropod, mollusk)
2. Implement spine hierarchy generator (horizontal/vertical orientation)
3. Implement limb attachment system (anatomically correct positions)
4. Implement high-level semantic parameters (bulkiness, tallness, etc.)
5. Implement primitive shape renderer (cylinders, spheres, cones, boxes)

**Key Design Decision**: Use **tag-based semantic system** from Spore's SIGGRAPH paper. Tag each body part (spine, limb, wing, tail, head) to enable procedural animation retargeting.

---

## Topic 2: Adaptive Voxel Density for Mesh Generation

**Research File**: `.qoder/research/adaptive-voxel-density.md`  
**Fix Plan**: `.qoder/plans/adaptive-voxel-density-fix.md`

### Key Findings

#### 1. Adaptive Voxel Grids (OpenVDB)
- **Industry standard**: Used by DreamWorks, Weta Digital, ILM
- **Hierarchical tree structure**: Configurable branching factors (8³, 16³, 32³)
- **Tile compression**: Uniform regions collapse into tiles at higher tree levels
- **Spatial adaptivity**: `setSpatialAdaptivity()` controls local refinement

**Source**: OpenVDB official documentation v13.0.1, OpenVDB FAQ.

**Key Quote**: *"OpenVDB stores both data values and child nodes at each level of the tree, it is adaptive only in the first sense [adaptive voxel sizes], not the second [multiple grids with different fixed voxel sizes]."* — OpenVDB FAQ

#### 2. Octree-Based Voxelization
- **Refinement criteria**: Surface proximity, curvature, joint proximity, feature proximity
- **2:1 balance constraint**: Limit adjacent cells to 1 level difference to avoid T-junctions
- **Refinement near joints**: 2-4× finer resolution at shoulders, hips, knees

**Source**: arXiv 2024 (Dual Contouring), McNeel Forum (Grasshopper/OpenVDB practitioners).

#### 3. Multi-Resolution Marching Cubes
| Variant | Handles Adaptive Resolution? | Preserves Sharp Features? | LOD Seams? |
|---|---|---|---|
| Standard Marching Cubes | No | No | N/A (uniform) |
| Octree Marching Cubes | Yes | No | Yes (T-junctions) |
| Dual Contouring | Yes | Yes | No |
| Dual Marching Cubes | Yes | Yes | No |

**Source**: UC Berkeley (Schaefer & Warren), Rice University (Warren et al.).

**Recommendation**: Use **Dual Contouring** if implementing from scratch — handles adaptive resolution naturally without LOD seams, preserves sharp features.

#### 4. Distance-Based Refinement
- **High-priority zones**: Joints (shoulders, hips, knees), head/face, spine connections
- **Low-priority zones**: Limb shafts, tail mid-sections, flat torso sides
- **Curvature as key metric**: Both academic papers (Nature 2026, CVPR 2025) and production tools use surface curvature to drive adaptive refinement

**Source**: Nature Scientific Reports (2026), CVPR 2025, OpenVDB VolumeToMesh documentation.

**Adaptivity threshold recommendations**:
- `0.0`: No simplification (full resolution)
- `0.1-0.3`: Mild simplification (recommended starting point)
- `0.5+`: Aggressive simplification (may lose fine details)

### Implementation Plan
**4-step approach**:
1. Implement adaptive octree builder (refine near surface, high curvature, joints)
2. Implement skeleton distance field generator (SDF from primitives + joint proximity)
3. Implement adaptive Marching Cubes extractor (handles resolution transitions)
4. Implement OpenVDB integration (production path with `setSpatialAdaptivity()`)

**Key Design Decision**: Use **OpenVDB for production** (industry standard, battle-tested), but implement custom octree + Dual Contouring as fallback/educational option.

---

## Source Credibility Summary

| Source | Type | Credibility | Key Contribution |
|---|---|---|---|
| Spore SIGGRAPH 2008 Paper | Peer-reviewed conference paper | **VERY HIGH** | Tag-based animation retargeting |
| OpenVDB Documentation | Official library docs | **VERY HIGH** | Adaptive voxel grid implementation |
| Eurographics CGF Review (2021) | Peer-reviewed journal | **VERY HIGH** | Comprehensive morphology survey |
| Rune Skovbo Johansen Blog | Professional dev blog | **HIGH** | Practical parametrization approach |
| Bournemouth MSc Thesis (2022) | Academic thesis | **HIGH** | Limb attachment algorithm |
| Nature Scientific Reports (2026) | Peer-reviewed journal | **VERY HIGH** | Curvature-based octree refinement |
| CVPR 2025 Poster | Top-tier conference | **VERY HIGH** | Adaptive mesh extraction |
| Anatomy DSL (GitHub) | Open-source project | **MEDIUM** | Practical DSL for skeleton specification |
| Biology LibreTexts | Educational resource | **HIGH** | Body plan classification |

---

## Next Steps

1. **Review fix plans** with team to validate approach
2. **Prioritize implementation** — recommend starting with:
   - Creature body plan templates (Step 1 of body plan fix)
   - Adaptive octree builder (Step 1 of voxel density fix)
3. **Set up testing framework** for unit tests outlined in fix plans
4. **Create prototype** generating simple quadruped creature with adaptive mesh
5. **Benchmark performance** comparing uniform vs adaptive voxel resolution
6. **Iterate on semantic parameters** based on visual feedback from generated creatures

---

## File Locations

```
.qoder/
├── research/
│   ├── creature-body-plan-rules.md      # Detailed research findings (Topic 1)
│   ├── adaptive-voxel-density.md        # Detailed research findings (Topic 2)
│   └── RESEARCH-SUMMARY.md              # This file
└── plans/
    ├── creature-body-plan-rules-fix.md  # Step-by-step implementation plan (Topic 1)
    └── adaptive-voxel-density-fix.md    # Step-by-step implementation plan (Topic 2)
```

---

**Research conducted**: May 9, 2026  
**Researcher**: AI Technical Research Assistant  
**Project**: 3dGenetics — Procedural Creature Generation
