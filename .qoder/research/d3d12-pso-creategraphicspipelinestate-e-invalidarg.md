# Research: DirectX 12 CreateGraphicsPipelineState E_INVALIDARG (0x80070057)

## Problem Statement
CreateGraphicsPipelineState is failing with HRESULT 0x80070057 (E_INVALIDARG) during PSO creation. The context involves:
- Root signature with 4 CBV parameters: b0 (VS), b1 (PS), b2 (PS), b3 (PS)
- Vertex shader expects 2 matrices in b0 (viewMatrix, projectionMatrix)
- Pixel shader expects: b1 (PBRMaterial), b2 (LightConstants), b3 (CameraConstants)
- Input layout has 3 elements: POSITION, NORMAL, TEXCOORD
- Error occurs during PSO creation, not during rendering

## Official Sources Found

### Source 1: Microsoft DirectX-Specs - D3D12 Resource Binding Functional Spec
- **URL**: https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html
- **Status**: Official Microsoft specification
- **Key Finding**: "The root signature maps descriptor tables, root descriptors, and root constants to this virtual register space."
- **Key Validation Rule**: Root signature must match shader bytecode register bindings exactly. Mismatches between root signature parameters and shader declarations cause validation failures.

### Source 2: Microsoft Learn - Creating a Root Signature
- **URL**: https://learn.microsoft.com/en-us/windows/win32/direct3d12/creating-a-root-signature
- **Section**: Root Signature in Pipeline State
- **Key Finding**: PSO creation validates that the root signature is compatible with the shader bytecode. The root signature passed in the PSO description must match what the shaders expect.

### Source 3: Stack Overflow - CreateGraphicsPipelineState fails with E_INVALIDARG
- **URL**: https://stackoverflow.com/questions/56577279/creategraphicspipelinestate-fails-with-e-invalidarg
- **Status**: 1 upvote, community solution
- **Key Finding**: Common causes include:
  1. Uninitialized PSO description structure (garbage values like 0xcccccccc)
  2. Missing dxil.dll next to executable
  3. SDK version mismatches
  4. **Solution**: Zero-initialize `D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};`

### Source 4: DirectXTK12 Wiki - PSOs, Shaders, and Signatures
- **URL**: https://github.com/microsoft/DirectXTK12/wiki/PSOs,-Shaders,-and-Signatures
- **Status**: Official Microsoft library documentation
- **Key Finding**: Root signature must include `D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT` flag when using input layouts with vertex buffers.
- **Code Example**:
```cpp
D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
    D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
    D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
    D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
```

### Source 5: GitHub Issue - MiniEngine Simple Shader #219
- **URL**: https://github.com/microsoft/DirectX-Graphics-Samples/issues/219
- **Status**: Closed, official Microsoft repository
- **Key Finding**: Error message: "D3D12 ERROR: ID3D12Device::CreateGraphicsPipelineState: Graphics pipeline state object uses input-assembler, but the root signature does not [allow it]"
- **Solution**: Root signature must have `ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT` flag when PSO has input layout defined.

### Source 6: Microsoft Learn - GPU-based validation and Debug Layer
- **URL**: https://github.com/MicrosoftDocs/win32/blob/docs/desktop-src/direct3d12/using-d3d12-debug-layer-gpu-based-validation.md
- **Key Finding**: Debug layer must be enabled AND "Graphics Tools" Windows feature must be installed to get detailed validation messages.

### Source 7: Braynzar Soft - DirectX 12 Constant Buffers Tutorial
- **URL**: https://www.braynzarsoft.net/viewtutorial/q16390-directx-12-constant-buffers-root-descriptor-tables
- **Key Finding**: "You can specify D3D12_SHADER_VISIBILITY_ALL to allow all shaders access, or 'OR' (|) the others together for each that have access. Only give access to the shaders that need it."

### Source 8: Adam Sawicki - Shapes and forms of DX12 root signatures
- **URL**: https://asawicki.info/news_1778_shapes_and_forms_of_dx12_root_signatures
- **Key Finding**: Comprehensive guide showing that root parameters must match shader register bindings exactly. Shader visibility must match which shader stages actually use the resources.

## Verified Solutions

### Solution 1: Missing ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT Flag (HIGH PROBABILITY)
- **Source**: DirectXTK12 Wiki, GitHub Issue #219
- **Effectiveness**: HIGH - This is a common cause of E_INVALIDARG when using input layouts
- **Complexity**: Simple - Add one flag to root signature
- **Code**:
```cpp
D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = 
    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
```
- **Validation Rule**: If PSO description has `InputLayout` with elements defined, the root signature MUST have this flag set, otherwise PSO creation fails with E_INVALIDARG.

### Solution 2: Uninitialized PSO Description Structure
- **Source**: Stack Overflow #56577279
- **Effectiveness**: HIGH - Common beginner mistake
- **Complexity**: Simple
- **Code**:
```cpp
D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // Zero-initialize!
// OR
ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
```
- **Validation Rule**: All fields must be explicitly set. Uninitialized fields contain garbage values that fail validation.

### Solution 3: Root Signature / Shader Bytecode Mismatch
- **Source**: Microsoft DirectX-Specs, Adam Sawicki article
- **Effectiveness**: HIGH - Must match exactly
- **Complexity**: Moderate - Requires checking all register bindings
- **Validation Rules**:
  1. Root signature CBV/SRV/UAV registers must match shader `register(bX)` declarations
  2. Shader visibility must match which stages use the resource
  3. Register space must be consistent (default is space0)
  4. Number of root parameters and their types must be compatible with shader bytecode

### Solution 4: Debug Layer Not Providing Detailed Errors
- **Source**: Microsoft Learn, Computer Graphics Stack Exchange
- **Effectiveness**: Medium - Helps diagnose the actual issue
- **Complexity**: Simple
- **Requirements**:
  1. Enable debug layer before device creation
  2. Install "Graphics Tools" optional Windows feature
  3. Enable GPU-based validation for detailed shader validation
- **Code**:
```cpp
#if defined(DEBUG) || defined(_DEBUG)
{
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();
    }
    
    ComPtr<ID3D12Debug1> debugController1;
    if (SUCCEEDED(debugController->QueryInterface(IID_PPV_ARGS(&debugController1))))
    {
        debugController1->SetEnableGPUBasedValidation(true);
    }
}
#endif
```

### Solution 5: Missing dxil.dll
- **Source**: Stack Overflow #76327523
- **Effectiveness**: Low for this case (usually affects shader compilation, not PSO creation)
- **Complexity**: Simple
- **Note**: dxil.dll must be present next to executable or in PATH

## Critical Validation Rules for PSO Creation

### Rule 1: Input Layout and Root Signature Compatibility
- If `psoDesc.InputLayout.NumElements > 0`, root signature MUST have `D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT`
- **Error**: "Graphics pipeline state object uses input-assembler, but the root signature does not allow input layout"

### Rule 2: Shader Bytecode and Root Signature Match
- Root signature parameters must align with shader register declarations
- For CBVs: `register(b0)`, `register(b1)`, etc. must match root signature CBV definitions
- Shader visibility flags must match which shader stages access each resource

### Rule 3: Complete PSO Description
- ALL fields in `D3D12_GRAPHICS_PIPELINE_STATE_DESC` must be explicitly initialized
- Critical fields:
  - `pRootSignature`
  - `VS` and `PS` (shader bytecode)
  - `InputLayout`
  - `PrimitiveTopologyType`
  - `NumRenderTargets` and `RTVFormats[]`
  - `DSVFormat` (if using depth buffer)
  - `SampleDesc` (MSAA settings)
  - `SampleMask`
  - `RasterizerState`
  - `BlendState`
  - `DepthStencilState`

### Rule 4: Root Signature Shader Visibility
- `D3D12_SHADER_VISIBILITY_VERTEX` - Only visible to vertex shader
- `D3D12_SHADER_VISIBILITY_PIXEL` - Only visible to pixel shader
- `D3D12_SHADER_VISIBILITY_ALL` - Visible to all shader stages
- **Best Practice**: Use most restrictive visibility to avoid validation issues

### Rule 5: Register Space Consistency
- All resources should use the same register space (typically space0)
- Root signature and shaders must agree on register space

## Recommendations

Based on official sources, the **most likely cause** is:

1. **Missing `D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT` flag** in root signature (Source: DirectXTK12 Wiki, GitHub Issue #219)
   - The input layout has 3 elements (POSITION, NORMAL, TEXCOORD)
   - If root signature doesn't have this flag, PSO creation will fail with E_INVALIDARG

2. **Uninitialized PSO description structure** (Source: Stack Overflow)
   - Ensure all fields are zero-initialized before setting values

3. **Root signature / shader mismatch** (Source: Microsoft DirectX-Specs)
   - Verify CBV registers b0, b1, b2, b3 match shader declarations exactly
   - Verify shader visibility settings match which stages use each CBV

## Debugging Steps

1. Enable debug layer with GPU-based validation
2. Install "Graphics Tools" Windows optional feature
3. Check Visual Studio Output window for detailed D3D12 ERROR messages
4. Verify root signature serialization is correct
5. Use PIX or RenderDoc to inspect PSO creation
6. Zero-initialize all structures
7. Validate each PSO description field systematically
