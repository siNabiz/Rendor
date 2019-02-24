//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include <iostream>

#if _DEBUG
#include "SimpleVertexShader_d.h"
#include "InstancedVertexShader_d.h"
#include "LightingPixelShader_d.h"
#include "SimplePixelShader_d.h"
#else
#include "SimpleVertexShader.h"
#include "InstancedVertexShader.h"
#include "LightingPixelShader.h"
#include "SimplePixelShader.h"
#endif

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

#define USE_INDEXES 0

typedef struct _lightingPixelCBufferStruct
{
    XMFLOAT4 LightColor;    // 16 bytes
    XMFLOAT4 ObjectColor;   // 16 bytes
    XMFLOAT3 LightPosition; // 12 bytes
    int UseConstantColor;   // 4 bytes
    XMFLOAT3 CameraPostion; // 12 bytes
    int UseTexture;         // 4 bytes
    //float Padding[3];       // 12 bytes
} LightingPixelCBufferStruct;

LightingPixelCBufferStruct g_lightingPixelCBuffer =
{
    XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
    XMFLOAT4(1.0f, 0.5f, 0.31f, 1.0f),
    XMFLOAT3(0.0f, 0.0f, 0.0f),
    1,
    XMFLOAT3(0.0f, 0.0f, 0.0f),
    0
};

typedef struct _simplePixelCBufferStruct
{
    XMFLOAT4 Color;     // 16 bytes
} SimplePixelCBufferStruct;

SimplePixelCBufferStruct g_simplePixelCBuffer =
{
    XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)
};

typedef struct _vertexCBufferStruct
{
    XMMATRIX MVPMatrix; // 64 bytes (4*16 bytes)
} VertexCBufferStruct;

VertexCBufferStruct g_vertexCBuffer =
{
    DirectX::XMMatrixIdentity()
};

typedef struct _vertexBufferStruct
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
    XMFLOAT3 Color;
    XMFLOAT2 TexCoord;
} VertexBufferStruct;

__declspec(align(16)) typedef struct _instanceVertexBufferStruct
{
    XMMATRIX WorldMatrix;
    XMMATRIX NormalMatrix;
} InstanceVertexBufferStruct;

int g_numInstances = 10;

#if USE_INDEXES
VertexBufferStruct g_vertices[8] =
{
    XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f),
    XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f),
    XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f),
    XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f),
    XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f),
    XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f),
    XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f),
    XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f)
};

WORD g_indices[36] =
{
    0, 1, 2, // side 1
    2, 3, 0,
    4, 0, 3, // side 2
    3, 7, 4,
    5, 4, 7, // side 3
    7, 6, 5,
    1, 5, 6, // side 4
    6, 2, 1,
    3, 2, 6, // side 5 (top)
    6, 7, 3,
    4, 5, 1, // side 6 (bottom)
    1, 0, 4
};
#else
VertexBufferStruct g_nonIndexedVertices[36] =
{
    XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT3( 0.0f,  0.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f), // 5
    XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3( 0.0f,  0.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f), // 4 
    XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3( 0.0f,  0.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), // 7
    XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3( 0.0f,  0.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), // 7
    XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3( 0.0f,  0.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), // 6
    XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT3( 0.0f,  0.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f), // 5
                                                                                                                    // 
    XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3( 0.0f,  0.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f), // 0
    XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3( 0.0f,  0.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f), // 1
    XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT3( 0.0f,  0.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), // 2
    XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT3( 0.0f,  0.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), // 2
    XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3( 0.0f,  0.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), // 3
    XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3( 0.0f,  0.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f), // 0
                                                                                                                    // 
    XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(-1.0f,  0.0f,  0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), // 3
    XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(-1.0f,  0.0f,  0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), // 7
    XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(-1.0f,  0.0f,  0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f), // 4
    XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(-1.0f,  0.0f,  0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f), // 4
    XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(-1.0f,  0.0f,  0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f), // 0
    XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(-1.0f,  0.0f,  0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), // 3
                                                                                                                    // 
    XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3( 1.0f,  0.0f,  0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), // 1
    XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT3( 1.0f,  0.0f,  0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), // 5
    XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3( 1.0f,  0.0f,  0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f), // 6
    XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3( 1.0f,  0.0f,  0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f), // 6
    XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT3( 1.0f,  0.0f,  0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f), // 2
    XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3( 1.0f,  0.0f,  0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), // 1
                                                                                                                    // 
    XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3( 0.0f, -1.0f,  0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f), // 4
    XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT3( 0.0f, -1.0f,  0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f), // 5
    XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3( 0.0f, -1.0f,  0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f), // 1
    XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3( 0.0f, -1.0f,  0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f), // 1
    XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3( 0.0f, -1.0f,  0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f), // 0
    XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3( 0.0f, -1.0f,  0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f), // 4
                                                                                                                    // 
    XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3( 0.0f,  1.0f,  0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), // 3
    XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT3( 0.0f,  1.0f,  0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), // 2
    XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3( 0.0f,  1.0f,  0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), // 6
    XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3( 0.0f,  1.0f,  0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), // 6
    XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3( 0.0f,  1.0f,  0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), // 7
    XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3( 0.0f,  1.0f,  0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)  // 3
};
#endif

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

    // for camera
    m_cameraPos = Vector3(0.0f, 0.0f, 7.0f);
    m_cameraPitch = 0.0f;
    m_cameraYaw = -90.0f;
    m_cameraFront = Vector3(XMScalarCos(XMConvertToRadians(m_cameraPitch))*XMScalarCos(XMConvertToRadians(m_cameraYaw)),
                            XMScalarSin(XMConvertToRadians(m_cameraPitch)), 
                            XMScalarCos(XMConvertToRadians(m_cameraPitch))*XMScalarSin(XMConvertToRadians(m_cameraYaw)));
    m_cameraFront.Normalize();
    m_cameraUp = Vector3(0.0f, 1.0f, 0.0f);
    m_cameraFOV = 100.0f;
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

        // update camera front
        {
            float rightPosX = state.thumbSticks.rightX;
            float rightPosY = state.thumbSticks.rightY;

            float speedAdjuster = 1.0f;
            m_cameraYaw += rightPosX * speedAdjuster;
            m_cameraPitch += rightPosY * speedAdjuster;

            if (m_cameraPitch > 89.0f)
                m_cameraPitch = 89.0f;
            else if (m_cameraPitch < -89.0f)
                m_cameraPitch = -89.0f;

            m_cameraFront = Vector3(XMScalarCos(XMConvertToRadians(m_cameraPitch))*XMScalarCos(XMConvertToRadians(m_cameraYaw)),
                                    XMScalarSin(XMConvertToRadians(m_cameraPitch)),
                                    XMScalarCos(XMConvertToRadians(m_cameraPitch))*XMScalarSin(XMConvertToRadians(m_cameraYaw)));
            m_cameraFront.Normalize();
        }

        // update camera position
        {
            float cameraSpeed = 0.05f;
            if (state.IsRightTriggerPressed())
            {
                cameraSpeed *= state.triggers.right * 2.0f;
            }
            if (state.IsLeftTriggerPressed())
            {
                cameraSpeed *= state.triggers.left * 0.5f;
            }
            if (state.IsDPadUpPressed())
            {
                m_cameraPos += cameraSpeed * m_cameraFront;
            }
            if (state.IsDPadDownPressed())
            {
                m_cameraPos -= cameraSpeed * m_cameraFront;
            }
            if (state.IsDPadRightPressed())
            {
                m_cameraPos += cameraSpeed * m_cameraFront.Cross(m_cameraUp);
            }
            if (state.IsDPadLeftPressed())
            {
                m_cameraPos -= cameraSpeed * m_cameraFront.Cross(m_cameraUp);
            }
        }

        // update FOV
        {
            float leftPosY = state.thumbSticks.leftY;
            m_cameraFOV -= leftPosY;
            if (m_cameraFOV > 100.0f)
                m_cameraFOV = 100.0f;
            else if (m_cameraFOV < 1.0f)
                m_cameraFOV = 1.0f;
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
#if USE_INDEXES
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
#endif
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Update camera information //
    Matrix viewMatrix = Matrix::CreateLookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);

    auto viewport = m_deviceResources->GetScreenViewport();
    Matrix projectionMatrix = SimpleMath::Matrix::CreatePerspectiveFieldOfView(XMConvertToRadians(m_cameraFOV), viewport.Width / viewport.Height, 0.1f, 100.0f);
    ///////////////////////////////

    context->VSSetConstantBuffers(0, 1, m_vertexCBuffer.GetAddressOf());

    ID3D11Buffer*const buffers[] = { m_vertexBuffer.Get(), m_instanceVertexBuffer.Get() };
    const UINT vertexStride[] = { sizeof(VertexBufferStruct), sizeof(InstanceVertexBufferStruct) };
    const UINT vertexOffset[] = { 0, 0 };

    // draw other objects
    {
        context->IASetVertexBuffers(0, 2, buffers, vertexStride, vertexOffset);
        context->IASetInputLayout(m_instancedInputLayout.Get());
        context->VSSetShader(m_instancedVertexShader.Get(), nullptr, 0);

        VertexCBufferStruct vertexCBuffer;
        vertexCBuffer.MVPMatrix = viewMatrix * projectionMatrix;
        context->UpdateSubresource(m_vertexCBuffer.Get(), 0, nullptr, &vertexCBuffer, 0, 0);

        context->PSSetShader(m_lightingPixelShader.Get(), nullptr, 0);
        context->PSSetConstantBuffers(0, 1, m_lightingPixelCBuffer.GetAddressOf());

        LightingPixelCBufferStruct lightingCBuffer;
        lightingCBuffer.CameraPostion = XMFLOAT3(m_cameraPos.x, m_cameraPos.y, m_cameraPos.z);
        lightingCBuffer.LightColor = g_lightingPixelCBuffer.LightColor;
        lightingCBuffer.LightPosition = g_lightingPixelCBuffer.LightPosition;
        lightingCBuffer.ObjectColor = g_lightingPixelCBuffer.ObjectColor;
        lightingCBuffer.UseConstantColor = g_lightingPixelCBuffer.UseConstantColor;
        lightingCBuffer.UseTexture = g_lightingPixelCBuffer.UseTexture;
        context->UpdateSubresource(m_lightingPixelCBuffer.Get(), 0, nullptr, &lightingCBuffer, 0, 0);

        context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

        ID3D11ShaderResourceView* textures[2] = { m_texture_0.Get(), m_texture_1.Get() };
        context->PSSetShaderResources(0, 2, textures);

#if USE_INDEXES
        context->DrawIndexedInstanced(ARRAYSIZE(g_indices), g_numInstances, 0, 0, 0);
#else
        context->DrawInstanced(ARRAYSIZE(g_nonIndexedVertices), g_numInstances, 0, 0);
#endif
    }

    // draw the lamp
    {
        context->IASetVertexBuffers(0, 1, buffers, vertexStride, vertexOffset);
        context->IASetInputLayout(m_simpleInputLayout.Get());
        context->VSSetShader(m_simpleVertexShader.Get(), nullptr, 0);

        Matrix modelMatrix = Matrix::CreateScale(0.2f);
        modelMatrix *= Matrix::CreateTranslation(0.0f, 0.0f, 0.0f);

        VertexCBufferStruct vertexCBuffer;
        vertexCBuffer.MVPMatrix = modelMatrix * viewMatrix * projectionMatrix;
        context->UpdateSubresource(m_vertexCBuffer.Get(), 0, nullptr, &vertexCBuffer, 0, 0);

        context->PSSetShader(m_simplePixelShader.Get(), nullptr, 0);
        context->PSSetConstantBuffers(0, 1, m_simplePixelCBuffer.GetAddressOf());

#if USE_INDEXES
        context->DrawIndexed(ARRAYSIZE(g_indices), 0, 0);
#else
        context->Draw(ARRAYSIZE(g_nonIndexedVertices), 0);
#endif
    }

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

    // Create instanced vertex shader
    DX::ThrowIfFailed(device->CreateVertexShader(g_InstancedVertexShader, sizeof(g_InstancedVertexShader), nullptr, &m_instancedVertexShader));

    // Create vertex shader input layout
    D3D11_INPUT_ELEMENT_DESC instancedVertexLayoutDesc[] =
    {
        // Per-vertex data
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0,   DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0,    DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        // Per-instance data
        { "MODELMATRIX", 0,  DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "MODELMATRIX", 1,  DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "MODELMATRIX", 2,  DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "MODELMATRIX", 3,  DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "NORMALMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "NORMALMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "NORMALMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "NORMALMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    };

    DX::ThrowIfFailed(device->CreateInputLayout(instancedVertexLayoutDesc, ARRAYSIZE(instancedVertexLayoutDesc), g_InstancedVertexShader, sizeof(g_InstancedVertexShader), &m_instancedInputLayout));

    // Create vertex shader
    DX::ThrowIfFailed(device->CreateVertexShader(g_SimpleVertexShader, sizeof(g_SimpleVertexShader), nullptr, &m_simpleVertexShader));

    // Create vertex shader input layout
    D3D11_INPUT_ELEMENT_DESC simpleVertexLayoutDesc[] =
    {
        // Per-vertex data
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0,   DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0,    DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    DX::ThrowIfFailed(device->CreateInputLayout(simpleVertexLayoutDesc, ARRAYSIZE(simpleVertexLayoutDesc), g_SimpleVertexShader, sizeof(g_SimpleVertexShader), &m_simpleInputLayout));

    // Setup VP constant buffer
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

    // Setup per-instance buffer
    {
        D3D11_BUFFER_DESC instanceBufferDesc;
        ZeroMemory(&instanceBufferDesc, sizeof(D3D11_BUFFER_DESC));
        instanceBufferDesc.ByteWidth = sizeof(InstanceVertexBufferStruct) * g_numInstances; // one element only
        instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        // Create and setup the per-instance buffer data
        InstanceVertexBufferStruct* instanceData = (InstanceVertexBufferStruct*)_aligned_malloc(sizeof(InstanceVertexBufferStruct) * g_numInstances, 16);
        ZeroMemory(instanceData, sizeof(instanceData));

        for (int i = 0; i < g_numInstances; i++)
        {
            float randVal = (float)rand() / RAND_MAX;

            Matrix modelMatrix = Matrix::CreateScale(0.5f);
            modelMatrix *= Matrix::CreateRotationX(randVal * XM_2PI);
            modelMatrix *= Matrix::CreateRotationY(randVal * XM_2PI);
            modelMatrix *= Matrix::CreateRotationZ(randVal * XM_2PI);
            float distX = ((2.0f * (float)rand() / RAND_MAX) - 1.0f) * 5.0f;
            float distY = ((2.0f * (float)rand() / RAND_MAX) - 1.0f) * 5.0f;
            float distZ = ((2.0f * (float)rand() / RAND_MAX) - 1.0f) * 5.0f;
            modelMatrix *= Matrix::CreateTranslation(distX, distY, distZ);

            instanceData[i].WorldMatrix = modelMatrix;
            instanceData[i].NormalMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, modelMatrix));
        }

        D3D11_SUBRESOURCE_DATA resourceData;
        ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
        resourceData.pSysMem = instanceData;

        DX::ThrowIfFailed(device->CreateBuffer(&instanceBufferDesc, &resourceData, &m_instanceVertexBuffer));

        _aligned_free(instanceData);
    }

    // Setup vertex buffer
    {
        D3D11_BUFFER_DESC vertexBufferDesc;
        ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
#if USE_INDEXES
        vertexBufferDesc.ByteWidth = sizeof(VertexBufferStruct) * ARRAYSIZE(g_vertices);
#else
        vertexBufferDesc.ByteWidth = sizeof(VertexBufferStruct) * ARRAYSIZE(g_nonIndexedVertices);
#endif
        vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA resourceData;
        ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
#if USE_INDEXES
        resourceData.pSysMem = g_vertices;
#else
        resourceData.pSysMem = g_nonIndexedVertices;
#endif
        
        DX::ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc, &resourceData, &m_vertexBuffer));
    }

#if USE_INDEXES
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
#endif

    // Setup rasterizer state
    {
        D3D11_RASTERIZER_DESC rasterizerStateDesc;
        ZeroMemory(&rasterizerStateDesc, sizeof(D3D11_RASTERIZER_DESC));
        rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
        rasterizerStateDesc.CullMode = D3D11_CULL_BACK;
        rasterizerStateDesc.FrontCounterClockwise = TRUE;
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

    // Setup pixel shaders
    DX::ThrowIfFailed(device->CreatePixelShader(g_LightingPixelShader, sizeof(g_LightingPixelShader), nullptr, &m_lightingPixelShader));
    DX::ThrowIfFailed(device->CreatePixelShader(g_SimplePixelShader, sizeof(g_SimplePixelShader), nullptr, &m_simplePixelShader));

    // Setup simple pixel shader constant buffer
    {
        D3D11_BUFFER_DESC constantBufferDesc;
        ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));
        constantBufferDesc.ByteWidth = sizeof(SimplePixelCBufferStruct); // one element only
        constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        D3D11_SUBRESOURCE_DATA resourceData;
        ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
        resourceData.pSysMem = &g_simplePixelCBuffer;

        DX::ThrowIfFailed(device->CreateBuffer(&constantBufferDesc, &resourceData, &m_simplePixelCBuffer));
    }

    // Setup lighting pixel shader constant buffer
    {
        D3D11_BUFFER_DESC constantBufferDesc;
        ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));
        constantBufferDesc.ByteWidth = sizeof(LightingPixelCBufferStruct); // one element only
        constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        D3D11_SUBRESOURCE_DATA resourceData;
        ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
        resourceData.pSysMem = &g_lightingPixelCBuffer;

        DX::ThrowIfFailed(device->CreateBuffer(&constantBufferDesc, &resourceData, &m_lightingPixelCBuffer));
    }

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
