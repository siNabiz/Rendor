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

    // Shared for all vertex shaders
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_planeVertexBuffer;

    // For instanced vertex shader
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_instancedVertexShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_instancedInputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_instanceVertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_planeInstanceVertexBuffer;

    // For simple vertex shader
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_simpleVertexShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_simpleInputLayout;

    // For simple pixel shader
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_simplePixelShader;

    // For lighting pixel shader
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_lightingPixelShader;

    // Constant buffers
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexCBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_simplePixelCBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_lightingPixelCBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_lightingPixelLightCBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_lightingPixelMaterialCBuffer;

    // For rasterizer state
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;

    // For depth/stencil state
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;

    // For sampler state
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

    // For textures imported
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_boxTexture_0;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_boxTexture_1;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_planeTexture;

    // For controlling the camera
    DirectX::SimpleMath::Vector3 m_cameraUp;
    DirectX::SimpleMath::Vector3 m_cameraPos;
    DirectX::SimpleMath::Vector3 m_cameraFront;
    float m_cameraPitch;
    float m_cameraYaw;
    float m_cameraFOV;
};