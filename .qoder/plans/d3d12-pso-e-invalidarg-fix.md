# Fix Plan: D3D12 CreateGraphicsPipelineState E_INVALIDARG (0x80070057)

## Root Cause
Based on official Microsoft documentation and community solutions, the most likely causes are:

1. **Missing `D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT` flag** - The root signature does not allow input assembler input layout, but the PSO description includes an input layout with 3 elements (POSITION, NORMAL, TEXCOORD)
2. **Uninitialized PSO description structure** - Fields may contain garbage values
3. **Root signature and shader bytecode mismatch** - CBV registers or shader visibility may not align

## Solution Approach

Based on [DirectXTK12 Wiki](https://github.com/microsoft/DirectXTK12/wiki/PSOs,-Shaders,-and-Signatures) and [GitHub Issue #219](https://github.com/microsoft/DirectX-Graphics-Samples/issues/219), implement the following fixes:

### Step 1: Add ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT Flag to Root Signature
- **File**: Root signature creation code (likely where you create `D3D12_ROOT_SIGNATURE_DESC`)
- **Change**: Add the required flag when you have an input layout
- **Reference**: DirectXTK12 Wiki, GitHub Issue #219
- **Code**:
```cpp
// Find your root signature flags definition and update it:
D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = 
    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

// If you want to optimize by denying unused shader stages:
D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = 
    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
    D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
    D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
    D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
    D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
    D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;

// Use this flag when creating the root signature description:
D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
rootSignatureDesc.NumParameters = 4; // Your 4 CBV parameters
rootSignatureDesc.pParameters = rootParameters; // Your root parameters array
rootSignatureDesc.Flags = rootSignatureFlags; // <-- This is critical!
```

**If using HLSL root signature**, update the string:
```hlsl
#define RootSig \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | " \
"            DENY_DOMAIN_SHADER_ROOT_ACCESS | " \
"            DENY_GEOMETRY_SHADER_ROOT_ACCESS | " \
"            DENY_HULL_SHADER_ROOT_ACCESS), " \
"CBV(b0, visibility = SHADER_VISIBILITY_VERTEX), " \
"CBV(b1, visibility = SHADER_VISIBILITY_PIXEL), " \
"CBV(b2, visibility = SHADER_VISIBILITY_PIXEL), " \
"CBV(b3, visibility = SHADER_VISIBILITY_PIXEL)"

[RootSignature(RootSig)]
```

### Step 2: Verify Root Signature CBV Definitions Match Shaders
- **File**: Root signature creation code
- **Change**: Ensure CBV registers and shader visibility match exactly
- **Reference**: [Microsoft DirectX-Specs](https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html), [Adam Sawicki article](https://asawicki.info/news_1778_shapes_and_forms_of_dx12_root_signatures)
- **Code**:
```cpp
// Root parameter 0: CBV b0 for Vertex Shader (viewMatrix, projectionMatrix)
rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
rootParameters[0].Descriptor.ShaderRegister = 0; // b0
rootParameters[0].Descriptor.RegisterSpace = 0;
rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // VS only

// Root parameter 1: CBV b1 for Pixel Shader (PBRMaterial)
rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
rootParameters[1].Descriptor.ShaderRegister = 1; // b1
rootParameters[1].Descriptor.RegisterSpace = 0;
rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PS only

// Root parameter 2: CBV b2 for Pixel Shader (LightConstants)
rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
rootParameters[2].Descriptor.ShaderRegister = 2; // b2
rootParameters[2].Descriptor.RegisterSpace = 0;
rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PS only

// Root parameter 3: CBV b3 for Pixel Shader (CameraConstants)
rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
rootParameters[3].Descriptor.ShaderRegister = 3; // b3
rootParameters[3].Descriptor.RegisterSpace = 0;
rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PS only
```

**Verify your HLSL shaders declare matching registers:**
```hlsl
// Vertex Shader
cbuffer VSConstants : register(b0)
{
    matrix viewMatrix;
    matrix projectionMatrix;
};

// Pixel Shader
cbuffer PBRMaterial : register(b1)
{
    // Your material constants
};

cbuffer LightConstants : register(b2)
{
    // Your light constants
};

cbuffer CameraConstants : register(b3)
{
    // Your camera constants
};
```

### Step 3: Zero-Initialize PSO Description Structure
- **File**: Where you create `D3D12_GRAPHICS_PIPELINE_STATE_DESC`
- **Change**: Zero-initialize the structure
- **Reference**: [Stack Overflow #56577279](https://stackoverflow.com/questions/56577279/creategraphicspipelinestate-fails-with-e-invalidarg)
- **Code**:
```cpp
// CRITICAL: Zero-initialize the structure!
D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

// Now set all required fields:
psoDesc.pRootSignature = yourRootSignature.Get();
psoDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
psoDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };

// Input layout
psoDesc.InputLayout = { inputLayoutElements, numElements };

// Rasterizer state
psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

// Blend state
psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

// Depth/stencil state
psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
psoDesc.DepthStencilState.DepthEnable = TRUE;
psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;

// Sample mask
psoDesc.SampleMask = UINT_MAX;

// Primitive topology type
psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

// Render targets
psoDesc.NumRenderTargets = 1;
psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // Your back buffer format
psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT; // Your depth buffer format

// MSAA (if not using MSAA)
psoDesc.SampleDesc.Count = 1;
psoDesc.SampleDesc.Quality = 0;

// Now create PSO
HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));
if (FAILED(hr))
{
    // Check debug output for detailed error message
    // Make sure debug layer is enabled!
}
```

### Step 4: Enable Debug Layer for Detailed Error Messages
- **File**: Device initialization code (before creating D3D12 device)
- **Change**: Enable debug layer and GPU-based validation
- **Reference**: [Microsoft Learn - GPU-based validation](https://github.com/MicrosoftDocs/win32/blob/docs/desktop-src/direct3d12/using-d3d12-debug-layer-gpu-based-validation.md)
- **Code**:
```cpp
#if defined(DEBUG) || defined(_DEBUG)
{
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();
    }
    
    // Enable GPU-based validation for detailed shader validation
    ComPtr<ID3D12Debug1> debugController1;
    if (SUCCEEDED(debugController->QueryInterface(IID_PPV_ARGS(&debugController1))))
    {
        debugController1->SetEnableGPUBasedValidation(true);
    }
}
#endif

// Then create your device...
D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));
```

**Important**: Install "Graphics Tools" Windows optional feature:
1. Go to Settings > Apps > Optional Features
2. Click "Add a feature"
3. Search for "Graphics Tools" and install it

## Verification Steps

### 1. Check Visual Studio Output Window
After enabling debug layer, run your application with Visual Studio debugger attached. Look for detailed D3D12 ERROR messages in the Output window (Debug > Windows > Output). The message should tell you exactly which validation failed.

### 2. Verify Root Signature Flag
Check your root signature creation code and confirm `D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT` is present in the flags.

### 3. Verify Register Matching
Compare your root signature CBV definitions with shader bytecode:
- Root param 0 → b0 (VS) ✓
- Root param 1 → b1 (PS) ✓
- Root param 2 → b2 (PS) ✓
- Root param 3 → b3 (PS) ✓

### 4. Test with Minimal PSO
If still failing, create a minimal PSO with:
- No render targets (`NumRenderTargets = 0`)
- No depth buffer
- Simple pass-through shaders
- No input layout

If this works, gradually add features back to identify which field causes the failure.

### 5. Use PIX or RenderDoc
Use PIX for Windows (part of Windows SDK) or RenderDoc to capture a frame and inspect PSO creation. These tools provide detailed validation information.

### 6. Edge Cases to Check
- **Shader compilation**: Ensure shaders compiled successfully and bytecode is valid
- **dxil.dll**: Verify dxil.dll is present in your executable directory or system PATH
- **SDK version**: Make sure you're using a recent Windows SDK version (10.0.19041.0 or later)
- **Descriptor heap types**: If using descriptor tables (not root CBVs), ensure correct heap types are bound
- **Register space**: All CBVs should use the same register space (default is 0)

## References
- [DirectXTK12 Wiki - PSOs, Shaders, and Signatures](https://github.com/microsoft/DirectXTK12/wiki/PSOs,-Shaders,-and-Signatures)
- [GitHub Issue #219 - MiniEngine Simple Shader](https://github.com/microsoft/DirectX-Graphics-Samples/issues/219)
- [Microsoft DirectX-Specs - D3D12 Resource Binding](https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html)
- [Stack Overflow - CreateGraphicsPipelineState fails with E_INVALIDARG](https://stackoverflow.com/questions/56577279/creategraphicspipelinestate-fails-with-e-invalidarg)
- [Microsoft Learn - GPU-based validation and Debug Layer](https://github.com/MicrosoftDocs/win32/blob/docs/desktop-src/direct3d12/using-d3d12-debug-layer-gpu-based-validation.md)
- [Adam Sawicki - Shapes and forms of DX12 root signatures](https://asawicki.info/news_1778_shapes_and_forms_of_dx12_root_signatures)
