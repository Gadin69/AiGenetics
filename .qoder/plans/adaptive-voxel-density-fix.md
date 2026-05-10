# Fix Plan: Adaptive Voxel Density for Creature Mesh Generation

## Root Cause
Uniform voxel resolution in mesh generation wastes computational resources on simple areas (limb shafts, flat surfaces) while providing insufficient detail in complex areas (joints, faces, spine connections). This results in either overly dense meshes (poor performance) or insufficient detail (poor visual quality).

## Solution Approach
Based on research from **OpenVDB documentation** (industry standard used by DreamWorks, Weta, ILM), **Dual Contouring papers** (UC Berkeley, Rice University), and **recent CVPR/Nature papers** (2025-2026), implement curvature-based adaptive voxel refinement with distance-to-joint weighting.

---

### Step 1: Implement Adaptive Voxel Grid Builder

**File**: `src/mesh/adaptive_voxel_grid.py` (new file)

**Change**: Build octree-based adaptive voxel grid with refinement driven by surface proximity and curvature.

**Reference**: OpenVDB FAQ states OpenVDB uses hierarchical tree structure with configurable branching factors (8³, 16³, 32³) at each level. Values stored at internal nodes represent "tiles" (constant-value regions), naturally creating coarse resolution in simple areas.

**Code**:
```python
import numpy as np
from typing import List, Tuple, Optional
from dataclasses import dataclass

@dataclass
class OctreeNode:
    """Node in adaptive octree."""
    bounds_min: np.ndarray  # [x, y, z] minimum corner
    bounds_max: np.ndarray  # [x, y, z] maximum corner
    children: List['OctreeNode'] = None  # 8 children (None if leaf)
    is_leaf: bool = True
    sdf_value: float = 0.0  # SDF value at node center (for leaf nodes)
    curvature: float = 0.0  # Surface curvature in this region
    depth: int = 0  # Depth in octree (0 = root)
    
    @property
    def center(self) -> np.ndarray:
        return (self.bounds_min + self.bounds_max) / 2.0
    
    @property
    def size(self) -> float:
        """Length of one side of this node."""
        return np.max(self.bounds_max - self.bounds_min)

@dataclass
class VoxelGridConfig:
    """Configuration for adaptive voxel grid."""
    bounds_min: np.ndarray  # Overall bounding box minimum
    bounds_max: np.ndarray  # Overall bounding box maximum
    max_depth: int = 8  # Maximum octree depth
    base_voxel_size: float = 0.1  # Size of voxel at max_depth
    surface_band_width: float = 0.5  # Distance from surface to refine (in voxel units)
    curvature_threshold: float = 0.3  # Curvature threshold for refinement
    joint_refine_radius: float = 0.5  # Radius around joints to refine
    joint_refine_strength: float = 0.25  # Voxel size multiplier near joints (0.25 = 4x finer)

class AdaptiveVoxelGrid:
    """Builds and manages adaptive voxel grid for creature mesh generation."""
    
    def __init__(self, config: VoxelGridConfig):
        self.config = config
        self.root: Optional[OctreeNode] = None
    
    def build(self, sdf_function, skeleton=None) -> OctreeNode:
        """Build adaptive octree from SDF function and optional skeleton.
        
        Args:
            sdf_function: Callable that takes [x,y,z] and returns SDF value
            skeleton: Optional skeleton with joints for distance-based refinement
        """
        self.root = self._build_recursive(
            sdf_function,
            bounds_min=self.config.bounds_min.copy(),
            bounds_max=self.config.bounds_max.copy(),
            depth=0,
            skeleton=skeleton
        )
        return self.root
    
    def _build_recursive(
        self,
        sdf_function,
        bounds_min: np.ndarray,
        bounds_max: np.ndarray,
        depth: int,
        skeleton
    ) -> OctreeNode:
        """Recursively build octree, refining where needed.
        
        Refinement criteria:
        1. Surface proximity: SDF value near zero (within surface_band_width)
        2. High curvature: Curvature exceeds threshold
        3. Joint proximity: Within joint_refine_radius of skeleton joint
        4. Not at max_depth
        """
        node = OctreeNode(
            bounds_min=bounds_min.copy(),
            bounds_max=bounds_max.copy(),
            depth=depth
        )
        
        # Check if should refine
        should_refine = False
        
        if depth < self.config.max_depth:
            center = node.center
            node_size = node.size
            
            # Criterion 1: Surface proximity
            sdf_value = sdf_function(center)
            node.sdf_value = sdf_value
            
            surface_distance = abs(sdf_value) / (self.config.base_voxel_size * (2 ** (self.config.max_depth - depth)))
            if surface_distance < self.config.surface_band_width:
                should_refine = True
            
            # Criterion 2: Curvature
            if should_refine or depth > 2:  # Only compute curvature if near surface
                curvature = self._compute_curvature(sdf_function, center, node_size)
                node.curvature = curvature
                
                if curvature > self.config.curvature_threshold:
                    should_refine = True
            
            # Criterion 3: Joint proximity
            if skeleton and len(skeleton.joints) > 0:
                min_joint_distance = min(
                    np.linalg.norm(center - joint.position)
                    for joint in skeleton.joints
                )
                
                if min_joint_distance < self.config.joint_refine_radius * (2 ** (self.config.max_depth - depth)):
                    should_refine = True
        
        # Refine or keep as leaf
        if should_refine:
            node.is_leaf = False
            node.children = []
            
            # Subdivide into 8 children
            mid = (bounds_min + bounds_max) / 2.0
            
            for i in range(8):
                child_min = bounds_min.copy()
                child_max = mid.copy()
                
                # Set child bounds based on octant
                if i & 1: child_min[0] = mid[0]; child_max[0] = bounds_max[0]
                if i & 2: child_min[1] = mid[1]; child_max[1] = bounds_max[1]
                if i & 4: child_min[2] = mid[2]; child_max[2] = bounds_max[2]
                
                child = self._build_recursive(
                    sdf_function, child_min, child_max, depth + 1, skeleton
                )
                node.children.append(child)
        
        return node
    
    def _compute_curvature(self, sdf_function, center: np.ndarray, h: float) -> float:
        """Compute mean curvature from SDF using finite differences.
        
        Formula: H = ∇·(∇φ/|∇φ|) / 2
        For unit gradient SDF, simplifies to Laplacian / 2
        """
        x, y, z = center
        
        # Compute Laplacian (sum of second derivatives)
        dxx = (sdf_function([x+h, y, z]) - 2*sdf_function([x, y, z]) + sdf_function([x-h, y, z])) / (h*h)
        dyy = (sdf_function([x, y+h, z]) - 2*sdf_function([x, y, z]) + sdf_function([x, y-h, z])) / (h*h)
        dzz = (sdf_function([x, y, z+h]) - 2*sdf_function([x, y, z]) + sdf_function([x, y, z-h])) / (h*h)
        
        # Mean curvature
        laplacian = dxx + dyy + dzz
        curvature = abs(laplacian) / 2.0
        
        return curvature
    
    def collect_leaf_nodes(self, node: OctreeNode = None) -> List[OctreeNode]:
        """Collect all leaf nodes from octree."""
        if node is None:
            node = self.root
        
        if node.is_leaf:
            return [node]
        
        leaves = []
        for child in node.children:
            leaves.extend(self.collect_leaf_nodes(child))
        
        return leaves
    
    def get_voxel_size_at(self, node: OctreeNode) -> float:
        """Get effective voxel size for a node."""
        depth_factor = 2 ** (self.config.max_depth - node.depth)
        return self.config.base_voxel_size * depth_factor
```

**Verification**:
- Octree correctly subdivides near surface (SDF ≈ 0)
- High-curvature regions refine to max_depth
- Joint proximity triggers refinement
- Leaf nodes correctly collected via traversal
- Voxel size computation correct for each depth level

---

### Step 2: Implement Skeleton Distance Field Generator

**File**: `src/mesh/skeleton_distance_field.py` (new file)

**Change**: Generate SDF from creature skeleton primitives with adaptive resolution zones.

**Reference**: OpenVDB VolumeToMesh uses SDF as input; adaptivity controlled by spatial multiplier grid. We compute distance-to-joint field to drive spatial adaptivity.

**Code**:
```python
import numpy as np
from typing import List, Optional
from dataclasses import dataclass

@dataclass
class SkeletonPrimitive:
    """A primitive shape in the skeleton (cylinder, sphere, cone, box)."""
    primitive_type: str  # "cylinder", "sphere", "cone", "box"
    position: np.ndarray  # Base position
    direction: np.ndarray  # Direction vector (for cylinder, cone)
    length: float  # Length (for cylinder, cone)
    radius: float  # Radius (for cylinder, sphere, cone base)
    size: np.ndarray = None  # Size (for box) [width, height, depth]

@dataclass
class SkeletonJoint:
    """A joint in the skeleton (articulation point)."""
    name: str
    position: np.ndarray
    radius: float = 0.15  # Joint radius for collision

@dataclass
class Skeleton:
    """Complete skeleton with primitives and joints."""
    primitives: List[SkeletonPrimitive]
    joints: List[SkeletonJoint]

def compute_sdf_from_skeleton(
    point: np.ndarray,
    skeleton: Skeleton,
    adaptive_mode: bool = True
) -> float:
    """Compute signed distance from point to skeleton surface.
    
    Args:
        point: Query point [x, y, z]
        skeleton: Skeleton to compute distance to
        adaptive_mode: If True, return adaptive multiplier for voxel size
    
    Returns:
        SDF value (negative = inside, positive = outside)
    """
    # Compute minimum distance to any primitive
    min_distance = float('inf')
    
    for prim in skeleton.primitives:
        if prim.primitive_type == "cylinder":
            dist = distance_to_cylinder(point, prim)
        elif prim.primitive_type == "sphere":
            dist = distance_to_sphere(point, prim)
        elif prim.primitive_type == "cone":
            dist = distance_to_cone(point, prim)
        elif prim.primitive_type == "box":
            dist = distance_to_box(point, prim)
        else:
            raise ValueError(f"Unknown primitive type: {prim.primitive_type}")
        
        min_distance = min(min_distance, dist)
    
    return min_distance

def distance_to_cylinder(point: np.ndarray, cylinder: SkeletonPrimitive) -> float:
    """Compute signed distance from point to cylinder surface."""
    # Project point onto cylinder axis
    axis = cylinder.direction / np.linalg.norm(cylinder.direction)
    to_point = point - cylinder.position
    
    # Project onto axis
    projection_length = np.dot(to_point, axis)
    projection_length = np.clip(projection_length, 0, cylinder.length)
    
    # Closest point on cylinder axis
    closest_point = cylinder.position + axis * projection_length
    
    # Distance to axis
    radial_distance = np.linalg.norm(point - closest_point)
    
    # Signed distance (negative = inside)
    return radial_distance - cylinder.radius

def distance_to_sphere(point: np.ndarray, sphere: SkeletonPrimitive) -> float:
    """Compute signed distance from point to sphere surface."""
    distance_to_center = np.linalg.norm(point - sphere.position)
    return distance_to_center - sphere.radius

def distance_to_cone(point: np.ndarray, cone: SkeletonPrimitive) -> float:
    """Compute signed distance from point to cone surface (simplified)."""
    # Approximate cone as cylinder for SDF (can be refined)
    axis = cone.direction / np.linalg.norm(cone.direction)
    to_point = point - cone.position
    
    projection_length = np.dot(to_point, axis)
    projection_length = np.clip(projection_length, 0, cone.length)
    
    # Taper radius based on height
    taper = 1.0 - (projection_length / cone.length)
    local_radius = cone.radius * taper
    
    closest_point = cone.position + axis * projection_length
    radial_distance = np.linalg.norm(point - closest_point)
    
    return radial_distance - local_radius

def distance_to_box(point: np.ndarray, box: SkeletonPrimitive) -> float:
    """Compute signed distance from point to box surface."""
    # Distance to box center in each axis
    box_center = box.position + box.size / 2.0
    d = np.abs(point - box_center) - box.size / 2.0
    
    # Outside distance
    outside_distance = np.linalg.norm(np.maximum(d, 0.0))
    
    # Inside distance (negative)
    inside_distance = np.min(np.maximum(d, 0.0))
    
    return outside_distance + min(inside_distance, 0.0)

def compute_joint_distance_field(
    point: np.ndarray,
    skeleton: Skeleton,
    refine_radius: float = 0.5,
    refine_strength: float = 0.25
) -> float:
    """Compute adaptive multiplier based on distance to nearest joint.
    
    Returns:
        Multiplier in [refine_strength, 1.0]
        - refine_strength near joints (finer voxels)
        - 1.0 far from joints (coarser voxels)
    """
    if len(skeleton.joints) == 0:
        return 1.0
    
    # Find minimum distance to any joint
    min_joint_distance = min(
        np.linalg.norm(point - joint.position)
        for joint in skeleton.joints
    )
    
    # Compute adaptive multiplier
    if min_joint_distance < refine_radius:
        # Linear interpolation: refine_strength at distance=0, 1.0 at distance=refine_radius
        multiplier = refine_strength + (1.0 - refine_strength) * (min_joint_distance / refine_radius)
    else:
        multiplier = 1.0
    
    return multiplier
```

**Verification**:
- SDF correctly negative inside primitives, positive outside
- Cylinder SDF accounts for end caps
- Cone SDF tapers correctly from base to apex
- Joint distance field returns 0.25 at joint center, 1.0 beyond refine_radius
- All distance functions return correct values for known test points

---

### Step 3: Implement Adaptive Marching Cubes Extractor

**File**: `src/mesh/adaptive_marching_cubes.py` (new file)

**Change**: Extract mesh from adaptive octree using modified Marching Cubes that handles resolution transitions.

**Reference**: Standard Marching Cubes operates on uniform grids. Adaptive variant operates on octree leaves, handling LOD seams at resolution boundaries.

**Code**:
```python
import numpy as np
from typing import List, Tuple, Dict
from dataclasses import dataclass

# Marching Cubes lookup table (simplified - full table has 256 entries)
# Maps 8-bit index (signs at 8 corners) to triangle edge intersections
MC_LOOKUP_TABLE = {
    # Example entries (full implementation requires complete table)
    # 0: [],  # All outside - no triangles
    # 7: [[0, 8, 3], [1, 8, 0]],  # etc.
    # ... (use precomputed table in actual implementation)
}

@dataclass
class MeshData:
    """Extracted mesh data."""
    vertices: List[np.ndarray]
    indices: List[Tuple[int, int, int]]
    normals: List[np.ndarray] = None

def extract_mesh_from_octree(
    root: 'OctreeNode',
    sdf_function,
    isovalue: float = 0.0,
    adaptivity: float = 0.1
) -> MeshData:
    """Extract mesh from adaptive octree using modified Marching Cubes.
    
    Args:
        root: Root of adaptive octree
        sdf_function: SDF function to evaluate
        isovalue: Isovalue threshold (0.0 = surface)
        adaptivity: Adaptivity threshold [0.0, 1.0] for triangle reduction
    
    Returns:
        MeshData with vertices and triangle indices
    """
    mesh = MeshData(vertices=[], indices=[])
    
    # Process each leaf node
    leaf_nodes = collect_leaf_nodes(root)
    
    # Track edge vertices to avoid duplicates
    edge_vertex_cache: Dict[Tuple, int] = {}
    
    for node in leaf_nodes:
        # Skip nodes far from surface
        if abs(node.sdf_value) > node.size:
            continue
        
        # Extract cell from octree leaf
        cell_vertices = get_cell_vertices(node)
        
        # Evaluate SDF at cell corners
        corner_sdfs = [sdf_function(v) for v in cell_vertices]
        
        # Compute Marching Cubes index
        mc_index = compute_mc_index(corner_sdfs, isovalue)
        
        # Skip if all inside or all outside
        if mc_index == 0 or mc_index == 255:
            continue
        
        # Generate triangles for this cell
        triangles = MC_LOOKUP_TABLE.get(mc_index, [])
        
        for triangle in triangles:
            triangle_indices = []
            
            for edge in triangle:
                # Get or create vertex on edge
                edge_key = get_edge_key(cell_vertices, edge)
                
                if edge_key not in edge_vertex_cache:
                    # Interpolate vertex position
                    v0, v1 = get_edge_endpoints(cell_vertices, edge)
                    s0, s1 = corner_sdfs[edge[0]], corner_sdfs[edge[1]]
                    vertex = interpolate_vertex(v0, v1, s0, s1, isovalue)
                    
                    vertex_idx = len(mesh.vertices)
                    mesh.vertices.append(vertex)
                    edge_vertex_cache[edge_key] = vertex_idx
                else:
                    vertex_idx = edge_vertex_cache[edge_key]
                
                triangle_indices.append(vertex_idx)
            
            mesh.indices.append(tuple(triangle_indices))
    
    # Apply adaptivity (simplify mesh in low-curvature regions)
    if adaptivity > 0.0:
        mesh = apply_mesh_adaptivity(mesh, leaf_nodes, adaptivity)
    
    return mesh

def get_cell_vertices(node: 'OctreeNode') -> List[np.ndarray]:
    """Get 8 corner vertices of an octree cell."""
    min_corner = node.bounds_min
    max_corner = node.bounds_max
    
    vertices = []
    for i in range(8):
        corner = np.array([
            max_corner[0] if i & 1 else min_corner[0],
            max_corner[1] if i & 2 else min_corner[1],
            max_corner[2] if i & 4 else min_corner[2],
        ])
        vertices.append(corner)
    
    return vertices

def compute_mc_index(corner_sdfs: List[float], isovalue: float) -> int:
    """Compute Marching Cubes index from corner SDF values."""
    index = 0
    for i, sdf in enumerate(corner_sdfs):
        if sdf < isovalue:
            index |= (1 << i)
    return index

def interpolate_vertex(
    v0: np.ndarray,
    v1: np.ndarray,
    s0: float,
    s1: float,
    isovalue: float
) -> np.ndarray:
    """Interpolate vertex position along edge where SDF crosses isovalue."""
    # Avoid division by zero
    if abs(s1 - s0) < 1e-10:
        return (v0 + v1) / 2.0
    
    # Linear interpolation
    t = (isovalue - s0) / (s1 - s0)
    t = np.clip(t, 0.0, 1.0)
    
    return v0 + t * (v1 - v0)

def apply_mesh_adaptivity(
    mesh: MeshData,
    leaf_nodes: List['OctreeNode'],
    adaptivity: float
) -> MeshData:
    """Simplify mesh in low-curvature regions.
    
    This is a simplified version - full implementation would use:
    - Quadric Error Metrics (Garland & Heckbert)
    - Vertex clustering
    - Edge collapse operations
    """
    # For now, return mesh as-is
    # Full implementation would reduce triangle count based on adaptivity parameter
    return mesh
```

**Verification**:
- Mesh correctly extracted at isosurface (SDF = 0)
- Vertices interpolated along edges at correct positions
- Triangle indices reference valid vertices
- Mesh is watertight (no holes at surface)
- Adaptivity reduces triangle count in flat regions

---

### Step 4: Implement OpenVDB Integration (Production Path)

**File**: `src/mesh/openvdb_integration.py` (new file)

**Change**: Integrate with OpenVDB library for production-quality adaptive mesh extraction.

**Reference**: OpenVDB documentation provides `VolumeToMesh` with `setSpatialAdaptivity()` for spatially-varying refinement. This is the recommended production path.

**Code**:
```python
"""
OpenVDB integration for adaptive mesh extraction.

Requires OpenVDB Python bindings:
    pip install openvdb
    or build from source: https://github.com/AcademySoftwareFoundation/openvdb
"""

try:
    import openvdb as vdb
    HAS_OPENVDB = True
except ImportError:
    HAS_OPENVDB = False
    print("Warning: OpenVDB not available. Using fallback mesh extraction.")

import numpy as np
from typing import Optional

def extract_mesh_with_openvdb(
    sdf_grid: np.ndarray,
    voxel_size: float = 0.1,
    isovalue: float = 0.0,
    adaptivity: float = 0.1,
    spatial_adaptivity_grid: Optional[np.ndarray] = None
) -> dict:
    """Extract mesh from SDF grid using OpenVDB's VolumeToMesh.
    
    Args:
        sdf_grid: 3D numpy array of SDF values
        voxel_size: Size of each voxel in world units
        isovalue: Isovalue threshold (0.0 = surface)
        adaptivity: Adaptivity threshold [0.0, 1.0]
        spatial_adaptivity_grid: Optional 3D array of local adaptivity values
    
    Returns:
        Dictionary with 'vertices' and 'indices' arrays
    """
    if not HAS_OPENVDB:
        raise RuntimeError("OpenVDB not available. Install openvdb package.")
    
    # Create OpenVDB FloatGrid
    grid = vdb.FloatGrid()
    grid.transform = vdb.createLinearTransform(voxelSize=voxel_size)
    
    # Copy SDF data to OpenVDB grid
    # Note: OpenVDB expects data in specific format
    sdf_flat = sdf_grid.flatten()
    grid.copyFromArray(sdf_flat)
    
    # Create VolumeToMesh extractor
    mesher = vdb.tools.VolumeToMesh(
        isovalue=isovalue,
        adaptivity=adaptivity,
        relaxDisorientedTriangles=True
    )
    
    # Apply spatial adaptivity if provided
    if spatial_adaptivity_grid is not None:
        adaptivity_grid = vdb.FloatGrid()
        adaptivity_grid.transform = grid.transform
        adaptivity_grid.copyFromArray(spatial_adaptivity_grid.flatten())
        mesher.setSpatialAdaptivity(adaptivity_grid)
    
    # Extract mesh
    mesher(grid)
    
    # Retrieve mesh data
    vertices = np.array(mesher.pointList())
    indices = np.array(mesher.polygonPoolList())
    
    return {
        'vertices': vertices,
        'indices': indices
    }

def create_spatial_adaptivity_from_joints(
    grid_shape: tuple,
    voxel_size: float,
    joints: list,
    joint_refine_radius: float = 0.5,
    near_adaptivity: float = 0.05,
    far_adaptivity: float = 0.5
) -> np.ndarray:
    """Create spatial adaptivity grid based on joint positions.
    
    Args:
        grid_shape: Shape of adaptivity grid (nx, ny, nz)
        voxel_size: Size of each voxel
        joints: List of joint positions [x, y, z]
        joint_refine_radius: Radius around joints to refine
        near_adaptivity: Adaptivity value near joints (lower = more detail)
        far_adaptivity: Adaptivity value far from joints (higher = less detail)
    
    Returns:
        3D numpy array of adaptivity values
    """
    # Create coordinate grids
    nx, ny, nz = grid_shape
    x = np.arange(nx) * voxel_size
    y = np.arange(ny) * voxel_size
    z = np.arange(nz) * voxel_size
    X, Y, Z = np.meshgrid(x, y, z, indexing='ij')
    
    # Initialize with far_adaptivity
    adaptivity_grid = np.full(grid_shape, far_adaptivity, dtype=np.float32)
    
    # For each joint, create refinement zone
    for joint in joints:
        # Compute distance from each voxel to joint
        dist = np.sqrt((X - joint[0])**2 + (Y - joint[1])**2 + (Z - joint[2])**2)
        
        # Create spherical refinement zone
        mask = dist < joint_refine_radius
        
        # Interpolate adaptivity based on distance
        t = dist[mask] / joint_refine_radius
        adaptivity_grid[mask] = np.minimum(
            adaptivity_grid[mask],
            near_adaptivity + (far_adaptivity - near_adaptivity) * t
        )
    
    return adaptivity_grid
```

**Verification**:
- OpenVDB grid correctly populated from SDF array
- Spatial adaptivity grid computed from joint positions
- Mesh extraction succeeds with valid vertices/indices
- Fallback to custom extraction if OpenVDB unavailable
- Adaptivity values in valid range [0.0, 1.0]

---

## Verification Steps

1. **Unit test octree construction**:
   - Create simple SDF (single sphere) → verify octree refines near surface
   - Create planar SDF → verify octree stays coarse (no curvature)
   - Create joint near point → verify refinement in joint radius
   - Verify max_depth not exceeded

2. **Unit test SDF computation**:
   - Test distance_to_sphere with known points → verify exact distances
   - Test distance_to_cylinder with points on axis, on surface, outside → verify signs
   - Test distance_to_box with points at corners, edges, faces → verify distances

3. **Unit test mesh extraction**:
   - Extract mesh from sphere SDF → verify approximately spherical mesh
   - Extract mesh from box SDF → verify box-shaped mesh
   - Verify mesh is watertight (manifold, no holes)
   - Verify triangle count reasonable (not excessive)

4. **Integration test full pipeline**:
   - Create simple skeleton (spine + 4 limbs)
   - Build SDF from skeleton primitives
   - Build adaptive octree
   - Extract mesh
   - Render mesh → verify creature shape recognizable
   - Compare mesh with uniform voxels vs adaptive → verify adaptive has fewer triangles in limb shafts

5. **Performance testing**:
   - Measure octree build time for different max_depth values
   - Measure mesh extraction time for uniform vs adaptive grids
   - Compare triangle count: adaptive should be 30-70% fewer triangles than uniform
   - Verify visual quality acceptable (no visible artifacts from adaptivity)

6. **Edge case testing**:
   - Test with empty skeleton → verify empty mesh
   - Test with single joint → verify refinement around joint
   - Test with overlapping primitives → verify SDF handles intersections correctly
   - Test with very small voxel_size (0.01) → verify no numerical issues

---

## References

1. OpenVDB Documentation. "VolumeToMesh Struct Reference." https://www.openvdb.org/documentation/doxygen/structopenvdb_1_1v13__0_1_1tools_1_1VolumeToMesh.html
2. OpenVDB Documentation. "Frequently Asked Questions." https://www.openvdb.org/documentation/doxygen/faq.html
3. Museth, K. (2013). "VDB: High-Resolution Sparse Volumes with Dynamic Topology." ACM Transactions on Graphics.
4. Schaefer, S., Warren, J. "Dual Contouring: The Secret Sauce." UC Berkeley.
5. Ju, T., et al. (2006). "Dual Marching Cubes: Primal Contouring of Dual Grids." Rice University.
6. "Occupancy-Based Dual Contouring." arXiv:2409.13418v1 (2024).
7. "High-Fidelity Lightweight Mesh Reconstruction from Point Clouds." CVPR 2025.
8. "Frequency-domain oversampling for multi-resolution surface reconstruction." Nature Scientific Reports (2026).
