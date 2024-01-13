#pragma once
#define NOMINMAX
#include "engine/comptr.h"
#include "engine/descriptor_heap.h"
#include "engine/shared_struct.h"
#include "engine/texture2d.h"
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#pragma comment(lib, "d3d12.lib")	// d3d12���C�u�����������N����
#pragma comment(lib, "dxgi.lib")	// dxgi���C�u�����������N����

class Window;
class GBufferManager;
class ShadowMap;
class Engine2D;
class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;

class Engine
{
public:
	enum { FRAME_BUFFER_COUNT = 2 }; // �_�u���o�b�t�@�����O

public:
	Engine();
	~Engine();

public:
	/*static Engine* Get() {
		static Engine instance;
		return &instance;
	}*/

	bool Init(Window* window); // �G���W��������

	void InitRender();
	void BeginRender(); // �`��̊J�n����
	void EndRender(); // �`��̏I������
	void BeginRenderMSAA();
	void EndRenderMSAA();
	void EndRenderD3D();

	void BeginDeferredRender();
	void DepthPrePath();
	void GBufferPath();
	void LightingPath();
	void SSAOPath();
	void BlurHorizontalPath();
	void BlurVerticalPath();
	void PostProcessPath();
	void FXAAPath();
	void EndDeferredRender();

	void WaitRender(); // �`�抮����҂���

public:
	ID3D12Device6* Device();
	ID3D12GraphicsCommandList* CommandList();
	ID3D12CommandAllocator* Allocator();
	ID3D12CommandQueue* Queue();
	ID3D12CommandQueue** QueueAddress();
	ID3D12Resource* RenderTarget(int index);
	std::shared_ptr<DescriptorHeap> RtvHeap();
	std::shared_ptr<DescriptorHeap> DsvHeap();
	std::shared_ptr<DescriptorHeap> GBufferHeap();
	GBufferManager* GetGBufferManager();
	ShadowMap* GetShadowMap();
	UINT CurrentBackBufferIndex();
	Window* GetWindow();
	UINT FrameBufferWidth();
	UINT FrameBufferHeight();
	Engine2D* GetEngine2D();
	ID3D12DebugDevice* DebugDevice();

	float AspectRate();

public:
	bool UploadTexture(
		ID3D12Resource* textureData, std::vector<D3D12_SUBRESOURCE_DATA> subresources);

	std::unique_ptr<VertexBuffer> CreateVertexBuffer(const std::vector<Vertex>& vertices); // ���_�o�b�t�@�𐶐�
	std::unique_ptr<IndexBuffer> CreateIndexBuffer(const std::vector<uint32_t>& indices); // �C���f�b�N�X�o�b�t�@�𐶐�
	std::unique_ptr<ConstantBuffer> CreateConstantBuffer(const unsigned long long size); // �萔�o�b�t�@�𐶐�
	void CreateShaderResourceView(const Texture2D& texture, const DescriptorHandle& handle); // SRV�𐶐�
	std::unique_ptr<DescriptorHeap> CreateDescriptorHeap(const unsigned int num); // �f�B�X�N���v�^�q�[�v�𐶐�

private: // DirectX12�������Ɏg���֐�
	bool EnableDebugLayer();	// �f�o�b�O�f�o�C�X�𐶐�
	bool CreateDevice();		// �f�o�C�X�𐶐�
	bool CreateCommandQueue();	// �R�}���h�L���[�𐶐�
	bool CreateSwapChain();		// �X���b�v�`�F�C���𐶐�
	bool CreateCommandList();	// �R�}���h���X�g�ƃR�}���h�A���P�[�^�[�𐶐�
	bool CreateFence();			// �t�F���X�𐶐�
	void CreateViewPort();		// �r���[�|�[�g�𐶐�
	void CreateScissorRect();	// �V�U�[��`�𐶐�
	
private:
	Window* window_ = nullptr;
	HWND hwnd_;
	UINT frame_buffer_width_ = 0;
	UINT frame_buffer_height_ = 0;
	UINT current_back_buffer_index_ = 0;

	ComPtr<ID3D12Device6> device_; // �f�o�C�X
	ComPtr<ID3D12CommandQueue> queue_; // �R�}���h�L���[
	ComPtr<IDXGISwapChain3> swapchain_; // �X���b�v�`�F�C��
	ComPtr<ID3D12CommandAllocator> allocator_[FRAME_BUFFER_COUNT]; // �R�}���h�A���P�[�^�[
	ComPtr<ID3D12CommandAllocator> oneshot_allocator_;
	ComPtr<ID3D12GraphicsCommandList> command_list_; // �R�}���h���X�g
	HANDLE fence_event_; // �t�F���X�Ŏg���C�x���g
	ComPtr<ID3D12Fence> fence_; // �t�F���X
	UINT64 fence_value_[FRAME_BUFFER_COUNT]; // �t�F���X�̒l
	D3D12_VIEWPORT viewport_; // �r���[�|�[�g
	D3D12_RECT scissor_; // �V�U�[��`
	ComPtr<ID3D12DebugDevice> debug_device_;

private:
	bool CreateRenderTarget();	// �����_�[�^�[�Q�b�g�𐶐�
	bool CreateDepthStencil();	// �[�x�X�e���V���o�b�t�@�𐶐�
	bool CreateMSAA();
	bool CreateGBuffer();		// �f�B�t�@�[�h�����_�����O�p��G-Buffer�𐶐�

	UINT rtv_descriptor_size_ = 0; // �����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�T�C�Y
	std::shared_ptr<DescriptorHeap> rtv_heap_; // �����_�[�^�[�Q�b�g�̃f�B�X�N���v�^�q�[�v
	DescriptorHandle rtv_handles_[FRAME_BUFFER_COUNT];
	ComPtr<ID3D12Resource> render_targets_[FRAME_BUFFER_COUNT];	// �����_�[�^�[�Q�b�g

	UINT dsv_descriptor_size_ = 0; // �[�x�X�e���V���̃f�B�X�N���v�^�T�C�Y
	std::shared_ptr<DescriptorHeap> dsv_heap_; // �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v
	DescriptorHandle dsv_handle_;
	ComPtr<ID3D12Resource> depth_stencil_buffer_; // �[�x�X�e���V���o�b�t�@

	UINT sample_count_;
	ComPtr<ID3D12Resource> msaa_color_target_;
	ComPtr<ID3D12Resource> msaa_depth_target_;
	DescriptorHandle msaa_rtv_handle_;
	DescriptorHandle msaa_dsv_handle_;

	std::unique_ptr<ShadowMap> shadowmap_;

	std::unique_ptr<GBufferManager> gbuffer_manager_;
	std::shared_ptr<DescriptorHeap> gbuffer_heap_;

	std::unique_ptr<Engine2D> engine2d_; // 2D�`��G���W���ւ̃|�C���^

private:
	ID3D12Resource* current_render_target_ = nullptr; // ���݃t���[���̃����_�[�^�[�Q�b�g���ꎞ�I�ɕۑ����Ă����ϐ�
	

};

//extern std::unique_ptr<Engine> g_Engine; // �ǂ�����ł��Q�Ƃ������̂ŃO���[�o���ɂ���