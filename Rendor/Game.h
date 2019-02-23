//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"


// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;
     
    // For gamepad controller
    std::unique_ptr<DirectX::GamePad> m_gamePad;
    DirectX::GamePad::ButtonStateTracker m_buttons;

    // For vertex shader
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_instanceVertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

    // Constant buffers
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexCBuffer;

    // For pixel shader
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pixelShaderInputLayout;

    // For rasterizer state
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;

    // For depth/stencil state
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;

    // For sampler state
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

    // For textures imported
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture_0;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture_1;

    // For controlling the camera
    DirectX::SimpleMath::Vector3 m_cameraUp;
    DirectX::SimpleMath::Vector3 m_cameraPos;
    DirectX::SimpleMath::Vector3 m_cameraFront;
    float m_cameraPitch;
    float m_cameraYaw;
    float m_cameraFOV;
};