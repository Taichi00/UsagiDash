#include "RootSignature.h"
#include "Engine.h"
#include "d3dx12.h"

RootSignature::RootSignature(const int numParams, RootSignatureParameter params[])
{
	auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;	// �A�v���P�[�V�����̓��̓A�Z���u�����g�p����
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;			// �h���C���V�F�[�_�[�̃��[�g�V�O�l�`���ւ̃A�N�Z�X�����ۂ���
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;				// �n���V�F�[�_�[�̃��[�g�V�O�l�`���ւ̃A�N�Z�X�����ۂ���
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;			// �W�I���g���V�F�[�_�[�̃��[�g�V�O�l�`���ւ̃A�N�Z�X�����ۂ���

	//CD3DX12_DESCRIPTOR_RANGE diffuseTexRange;
	//diffuseTexRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);	// t0 : diffuse texture

	//CD3DX12_DESCRIPTOR_RANGE shadowTexRange;
	//shadowTexRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);		// t1 : shadow map

	auto descriptorRange = new CD3DX12_DESCRIPTOR_RANGE[numParams];
	auto rootParam = new CD3DX12_ROOT_PARAMETER[numParams];

	int bId = 0, tId = 0;
	for (int i = 0; i < numParams; i++)
	{
		switch (params[i])
		{
		case RSConstantBuffer:
			rootParam[i].InitAsConstantBufferView(bId++, 0, D3D12_SHADER_VISIBILITY_ALL);
			break;
		case RSTexture:
			descriptorRange[tId].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, tId);
			rootParam[i].InitAsDescriptorTable(1, &descriptorRange[tId++], D3D12_SHADER_VISIBILITY_ALL);
			break;
		}
	}

	//rootParam[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);	// b0 : transform
	//rootParam[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);	// b1 : scene
	//rootParam[2].InitAsConstantBufferView(2, 0, D3D12_SHADER_VISIBILITY_ALL);	// b2 : bone parameter
	//rootParam[3].InitAsConstantBufferView(3, 0, D3D12_SHADER_VISIBILITY_ALL);	// b3 : material parameter
	//rootParam[4].InitAsDescriptorTable(1, &diffuseTexRange, D3D12_SHADER_VISIBILITY_ALL);
	//rootParam[5].InitAsDescriptorTable(1, &shadowTexRange, D3D12_SHADER_VISIBILITY_ALL);

	// �X�^�e�B�b�N�T���v���[�̐ݒ�
	auto sampler = CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);
	//auto sampler = CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_POINT);
	
	// ���[�g�V�O�l�`���̐ݒ�i�ݒ肵�������[�g�p�����[�^�ƃX�^�e�B�b�N�T���v���[������j
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.NumParameters		= numParams;			// ���[�g�p�����[�^�̌�
	desc.NumStaticSamplers	= 1;					// �T���v���[�̌�
	desc.pParameters		= rootParam;			// ���[�g�p�����[�^�̃|�C���^	
	desc.pStaticSamplers	= &sampler;				// �T���v���[�̃|�C���^
	desc.Flags				= flag;					// �t���O��ݒ�

	ComPtr<ID3DBlob> pBlob;
	ComPtr<ID3DBlob> pErrorBlob;

	// �V���A���C�Y
	auto hr = D3D12SerializeRootSignature(
		&desc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		pBlob.GetAddressOf(),
		pErrorBlob.GetAddressOf()
	);
	if (FAILED(hr))
	{
		printf("���[�g�V�O�l�`���V���A���C�Y�Ɏ��s\n");
		return;
	}

	// ���[�g�V�O�l�`������
	hr = g_Engine->Device()->CreateRootSignature(
		0,							// GPU����������ꍇ�̃m�[�h�}�X�N
		pBlob->GetBufferPointer(),	// �V���A���C�Y�����f�[�^�̃|�C���^
		pBlob->GetBufferSize(),		// �V���A���C�Y�����f�[�^�̃T�C�Y
		IID_PPV_ARGS(m_pRootSignature.GetAddressOf())
	);
	if (FAILED(hr))
	{
		printf("���[�g�V�O�l�`���̐����Ɏ��s");
		return;
	}

	delete[] rootParam;
	delete[] descriptorRange;

	m_IsValid = true;
}

bool RootSignature::IsValid()
{
	return m_IsValid;
}

ID3D12RootSignature* RootSignature::Get()
{
	return m_pRootSignature.Get();
}

