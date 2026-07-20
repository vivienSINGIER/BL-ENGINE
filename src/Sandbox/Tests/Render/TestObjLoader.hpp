// #ifndef TEST_OBJ_LOADER_HPP_DEFINED
// #define TEST_OBJ_LOADER_HPP_DEFINED
//
// #include "Camera.h"
// #include "D3D12Context.h"
// #include "Utils/Structs.h"
// #include "ShaderManager.h"
// #include "Test.h"
//
// #include "Render/Window.h"
// #include "Render/RenderWindow.h"
// #include "Render/PipelineStateManager.h"
// #include "Render/GeometryFactory.hpp"
//
// class TestObjLoader : public Test
// {
// public:
//     static void Run()
//     {
//         D3D12Context::GetInstance();
//
//         char buffer[256];
//         sprintf_s(buffer, "MSAA Quality: %d\n", D3D12Context::Get4xMsaaQuality());
//         OutputDebugStringA(buffer);
//         
//         RenderWindow window(1280, 720, L"My app");
//         window.Create();
//
//         XMFLOAT4X4 cWorld = MathHelper::Identity4x4();
//         cWorld._43 = -10.0f;  // Recule la caméra de 5 unités
//         Camera cam;
//         cam.SetWorld(cWorld);
//         XMFLOAT3 target = {0.0f, 0.0f, 0.0f};
//         cam.LookAt(target);
//         window.SetMainCamera(&cam);
//         
//         ShaderManager::GetInstance()->CompileShader(L"default-vs", "main", "vs_5_0");
//         ShaderManager::GetInstance()->CompileShader(L"default-ps", "main", "ps_5_0");
//         ShaderManager::GetInstance()->CompileRootSig(L"default-rs");
//         PipelineStateObject* pso = PipelineStateManager::GetDefaultPSO();
//
//         D3D12_SHADER_BYTECODE vs = ShaderManager::GetInstance()->GetCompiledShader(L"default-vs");
//         D3D12_SHADER_BYTECODE ps = ShaderManager::GetInstance()->GetCompiledShader(L"default-ps");
//         
//         sprintf_s(buffer, "VS bytecode size: %llu\n", vs.BytecodeLength);
//         OutputDebugStringA(buffer);
//         sprintf_s(buffer, "PS bytecode size: %llu\n", ps.BytecodeLength);
//         OutputDebugStringA(buffer);
//
//         if (vs.BytecodeLength == 0)
//             OutputDebugStringA("ERROR: Vertex shader is empty!\n");
//         if (ps.BytecodeLength == 0)
//             OutputDebugStringA("ERROR: Pixel shader is empty!\n");
//         
//         // window.SetClearColor({1.0f, 1.0f, 0.5f});
//
//         UploadBuffer<ObjectData>* uploadBuffer = new UploadBuffer<ObjectData>(1);
//         
//         RenderItem item;
//
//         D3D12Context::GetCommandAllocator()->Reset();
//         D3D12Context::GetCommandList()->Reset(D3D12Context::GetInstance()->GetCommandAllocator(), nullptr);
//         
//         item.data.world = MathHelper::Identity4x4();
//
//         item.objectCB = uploadBuffer;
//         item.bufferIndex = uploadBuffer->GetFirstAvailable();
//         
//         item.pGeo = GeometryFactory::LoadGeometry("../../res/ak47.obj");
//         
//         XMMATRIX world = XMMatrixIdentity();
//         XMStoreFloat4x4(&item.data.world, XMMatrixTranspose(world));
//
//         item.objectCB->CopyData(item.bufferIndex, item.data);
//         
//         item.pPso = pso;
//
//         ThrowIfFailed(D3D12Context::GetCommandList()->Close());
//         ID3D12CommandList* cmdsLists[] = { D3D12Context::GetCommandList() };
//         D3D12Context::GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
//         
//         D3D12Context::GetInstance()->FlushCommandQueue();
//         
//         while (window.IsOpen())
//         {
//             window.Update();
//             window.BeginDraw();
//
//             // draw
//             window.Draw(item);
//             
//             window.EndDraw();
//         }
//     }
// };
//
// #endif
