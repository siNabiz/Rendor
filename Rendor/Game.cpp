//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include <iostream>

#if _DEBUG
#include "VertexShader_d.h"
#include "PixelShader_d.h"
#else
#include "VertexShader.h"
#include "PixelShader.h"
#endif

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

typedef struct _vertexCBufferStruct
{
    XMMATRIX MVPMatrix;
} VertexCBufferStruct;

VertexCBufferStruct g_vertexCBuffer =
{
    DirectX::XMMatrixIdentity()
};

typedef struct _vertexBufferStruct
{
    XMFLOAT3 Position;
    XMFLOAT3 Color;
    XMFLOAT2 TexCoord;
} VertexBufferStruct;

VertexBufferStruct g_triangleNDCVertices[4] =
{
    XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f),
    XMFLOAT3(0.5f, -0.5f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f),
    XMFLOAT3(0.5f, 0.5f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f),
    XMFLOAT3(-0.5f, 0.5f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f)
};

WORD g_indices[6] =
{
    0, 2, 1,
    0, 3, 2
};

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */

    // for gamepad controller
    m_gamePad = std::make_unique<GamePad>();
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.
    elapsedTime;

    // for gamepad controller
    auto state = m_gamePad->GetState(0);

    if (state.IsConnected())
    {
        if (state.IsViewPressed())
        {
            ExitGame();
        }

        m_buttons.Update(state);
        if (m_buttons.a == GamePad::ButtonStateTracker::PRESSED)
        {
            // A was up last frame, it just went down this frame
        }
        if (m_buttons.b == GamePad::ButtonStateTracker::RELEASED)
        {
            // B was down last frame, it just went up this frame
        }
    }
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO: Add your rendering code here.
    const UINT vertexStride = sizeof(VertexBufferStruct);
    const UINT vertexOffset = 0;

    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &vertexStride, &vertexOffset);
    context->IASetInputLayout(m_inputLayout.Get());
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->VSSetConstantBuffers(0, 1, m_vertexCBuffer.GetAddressOf());

    {
        Matrix viewMatrix = Matrix::CreateLookAt(Vector3(0.0f, -3.0f, 3.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));

        auto viewport = m_deviceResources->GetScreenViewport();
        Matrix projectionMatrix = SimpleMath::Matrix::CreatePerspectiveFieldOfView(XMConvertToRadians(45.0f), viewport.Width / viewport.Height, 0.1f, 100.0f);

        float timeSinVal = XMScalarSin(float(m_timer.GetTotalSeconds()));
        float timeCosVal = XMScalarCos(float(m_timer.GetTotalSeconds()));

        Matrix modelMatrix = Matrix::CreateScale(0.5f);
        modelMatrix *= Matrix::CreateRotationZ(timeSinVal * XM_2PI);
        modelMatrix *= Matrix::CreateTranslation(timeSinVal * -0.5f, timeCosVal * 0.5f, 0.0f);

        VertexCBufferStruct vertexCBuffer;
        vertexCBuffer.MVPMatrix = modelMatrix * viewMatrix * projectionMatrix;
        context->UpdateSubresource(m_vertexCBuffer.Get(), 0, nullptr, &vertexCBuffer, 0, 0);
    }

    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

    ID3D11ShaderResourceView* textures[2] = { m_texture_0.Get(), m_texture_1.Get() };
    context->PSSetShaderResources(0, 2, textures);

    context->DrawIndexed(ARRAYSIZE(g_indices), 0, 0);

    m_deviceResources->PIXEndEvent();

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::Black);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    
    // Set the depth/stencil state
    context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the rasterizer state
    context->RSSetState(m_rasterizerState.Get());

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.

    m_gamePad->Resume();
    m_buttons.Reset();
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).

    m_gamePad->Suspend();
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).

    m_buttons.Reset();
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // TODO: Initialize device dependent objects here (independent of window size).

    // Create vertex shader
    DX::ThrowIfFailed(device->CreateVertexShader(g_VertexShader, sizeof(g_VertexShader), nullptr, &m_vertexShader));

    // Create vertex shader input layout
    D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    DX::ThrowIfFailed(device->CreateInputLayout(vertexLayoutDesc, ARRAYSIZE(vertexLayoutDesc), g_VertexShader, sizeof(g_VertexShader), &m_inputLayout));

    // Setup MVP constant buffer
    {
        D3D11_BUFFER_DESC constantBufferDesc;
        ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));
        constantBufferDesc.ByteWidth = sizeof(VertexCBufferStruct); // one element only
        constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        D3D11_SUBRESOURCE_DATA resourceData;
        ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
        resourceData.pSysMem = &g_vertexCBuffer;

        DX::ThrowIfFailed(device->CreateBuffer(&constantBufferDesc, &resourceData, &m_vertexCBuffer));
    }

    // Setup vertex buffer
    {
        D3D11_BUFFER_DESC vertexBufferDesc;
        ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
        vertexBufferDesc.ByteWidth = sizeof(VertexBufferStruct) * ARRAYSIZE(g_triangleNDCVertices);
        vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA resourceData;
        ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
        resourceData.pSysMem = g_triangleNDCVertices;

        DX::ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc, &resourceData, &m_vertexBuffer));
    }

    // Setup index buffer
    {
        D3D11_BUFFER_DESC indexBufferDesc;
        ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));
        indexBufferDesc.ByteWidth = sizeof(WORD) * ARRAYSIZE(g_indices);
        indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA resourceData;
        ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
        resourceData.pSysMem = g_indices;

        DX::ThrowIfFailed(device->CreateBuffer(&indexBufferDesc, &resourceData, &m_indexBuffer));
    }

    // Setup rasterizer state
    {
        D3D11_RASTERIZER_DESC rasterizerStateDesc;
        ZeroMemory(&rasterizerStateDesc, sizeof(D3D11_RASTERIZER_DESC));
        rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
        rasterizerStateDesc.CullMode = D3D11_CULL_BACK;
        rasterizerStateDesc.FrontCounterClockwise = FALSE;
        rasterizerStateDesc.DepthBias = 0;
        rasterizerStateDesc.DepthBiasClamp = 0.0f;
        rasterizerStateDesc.SlopeScaledDepthBias = 0.0f;
        rasterizerStateDesc.DepthClipEnable = TRUE;
        rasterizerStateDesc.ScissorEnable = FALSE;
        rasterizerStateDesc.MultisampleEnable = FALSE;
        rasterizerStateDesc.AntialiasedLineEnable = FALSE;

        DX::ThrowIfFailed(device->CreateRasterizerState(&rasterizerStateDesc, &m_rasterizerState));
    }

    // Setup depth/stencil state
    {
        D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
        ZeroMemory(&depthStencilStateDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
        depthStencilStateDesc.DepthEnable = TRUE;
        depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
        depthStencilStateDesc.StencilEnable = FALSE;

        DX::ThrowIfFailed(device->CreateDepthStencilState(&depthStencilStateDesc, &m_depthStencilState));
    }

    // Setup pixel shader
    DX::ThrowIfFailed(device->CreatePixelShader(g_PixelShader, sizeof(g_PixelShader), nullptr, &m_pixelShader));

    // Setup sampler state
    {
        D3D11_SAMPLER_DESC samplerStateDesc;
        ZeroMemory(&samplerStateDesc, sizeof(D3D11_SAMPLER_DESC));
        samplerStateDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerStateDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerStateDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerStateDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerStateDesc.MipLODBias = 0.0f;
        samplerStateDesc.MaxAnisotropy = 1;
        samplerStateDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerStateDesc.BorderColor[0] = 1.0f;
        samplerStateDesc.BorderColor[1] = 1.0f;
        samplerStateDesc.BorderColor[2] = 1.0f;
        samplerStateDesc.BorderColor[3] = 1.0f;
        samplerStateDesc.MinLOD = -FLT_MAX;
        samplerStateDesc.MaxLOD = FLT_MAX;

        DX::ThrowIfFailed(device->CreateSamplerState(&samplerStateDesc, &m_samplerState));
    }

    // Import textures
    DX::ThrowIfFailed(CreateWICTextureFromFile(device, L"wall.jpg", nullptr, m_texture_0.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateWICTextureFromFile(device, L"awesomeface.png", nullptr, m_texture_1.ReleaseAndGetAddressOf()));
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
    m_texture_0.Reset();
    m_texture_1.Reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
