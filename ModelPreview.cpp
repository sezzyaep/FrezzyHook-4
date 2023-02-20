#include "ModelPreview.h"

std::array < Vector4D, 6 > aWhiteArray =
{
	Vector4D(0.4f, 0.4f, 0.4f, 1.0f),
	Vector4D(0.4f, 0.4f, 0.4f, 1.0f),
	Vector4D(0.4f, 0.4f, 0.4f, 1.0f),
	Vector4D(0.4f, 0.4f, 0.4f, 1.0f),
	Vector4D(0.4f, 0.4f, 0.4f, 1.0f),
	Vector4D(0.4f, 0.4f, 0.4f, 1.0f),
};

std::array < float_t, 3 > aColorModulation =
{
	1.0f,
	1.0f,
	1.0f
};

void C_DrawModel::Instance()
{
	if (!vars.menu.open)
		return;

	if (!m_PreviewTexture)
	{
		interfaces.material_system->BeginRenderTargetAllocation();

		m_PreviewTexture = interfaces.material_system->CreateNamedRenderTargetTextureEx(
			"Preview",
			350, 575,
			RT_SIZE_NO_CHANGE,
			interfaces.material_system->GetBackBufferFormat(),
			MATERIAL_RT_DEPTH_SHARED,
			TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
			CREATERENDERTARGETFLAGS_HDR
		);

		interfaces.material_system->FinishRenderTargetAllocation();
	}

	if (!m_CubemapTexture)
		m_CubemapTexture = interfaces.material_system->FindTexture("editor/cubemap.hdr", TEXTURE_GROUP_CUBE_MAP);

	static auto CreateModelAddress = csgo->Utils.FindPatternIDA(GetModuleHandleA("client.dll"), "53 8B D9 56 57 8D 4B 04 C7 03 ? ? ? ? E8 ? ? ? ? 6A");
	auto CreateModel = reinterpret_cast<void(__thiscall*)(void*)>(CreateModelAddress);
	if (!m_PreviewModel)
	{
		m_PreviewModel = static_cast<C_MergedMDL*>(interfaces.memalloc->Alloc(0x75C));
		CreateModel(m_PreviewModel);

		m_PreviewModel->SetMDL("models/player/custom_player/uiplayer/animset_uiplayer.mdl");
		m_PreviewModel->SetMergedMDL("models/player/custom_player/legacy/ctm_fbi_variantb.mdl");
		m_PreviewModel->SetMergedMDL("models/weapons/w_pist_elite.mdl");

		m_PreviewModel->SetSequence(32, false);
		m_PreviewModel->SetupBonesForAttachmentQueries();
	}

	m_PreviewModel->RootMDL.flTime += interfaces.global_vars->frametime / 2.0f;

	m_ViewSetup.x = 0;
	m_ViewSetup.y = 0;
	m_ViewSetup.width = 350;
	m_ViewSetup.height = 575;
	m_ViewSetup.m_bOrtho = false;
	m_ViewSetup.fov = 70.f;
	m_ViewSetup.origin = Vector(-65.0f, 2.0f, 50);
	m_ViewSetup.angles = Vector(0, 0, 0);
	m_ViewSetup.zNear = 7.0f;
	m_ViewSetup.zFar = 1000.f;
	m_ViewSetup.bDoBloomAndToneMapping = true;

	CMatRenderContextPtr pRenderContext(interfaces.material_system);

	pRenderContext->PushRenderTargetAndViewport();
	pRenderContext->SetRenderTarget(m_PreviewTexture);

	pRenderContext->BindLocalCubemap(m_CubemapTexture);
	pRenderContext->SetLightingOrigin(-65.0f, 2.0f, 50.0f);
	pRenderContext->SetIntRenderingParameter(10, 0);

	Frustum dummyFrustum;
	interfaces.render_view->Push3DView(pRenderContext, m_ViewSetup, VIEW_CLEAR_COLOR | VIEW_CLEAR_DEPTH | VIEW_CLEAR_STENCIL, m_PreviewTexture, dummyFrustum);

	pRenderContext->ClearColor4ub(false, false, false, false);
	pRenderContext->ClearBuffers(true, true, true);
	pRenderContext->SetAmbientLightCube(aWhiteArray.data());

	interfaces.studio_render->SetAmbientLightColors(aWhiteArray.data());
	interfaces.studio_render->SetLocalLights(0, nullptr);

	matrix matPlayerView = { };
	Math::AngleMatrix(Vector(0, m_model.m_flModelRotation, 0), Vector(0, 0, 0), matPlayerView);

	interfaces.models.model_render->SuppressEngineLighting(true);


	m_model.m_bDrawModel = true;

	m_PreviewModel->Draw(matPlayerView);
	
		

	m_model.m_bDrawModel = false;

	interfaces.models.model_render->SuppressEngineLighting(false);

	interfaces.render_view->PopView(pRenderContext, dummyFrustum);
	pRenderContext->BindLocalCubemap(nullptr);

	pRenderContext->PopRenderTargetAndViewport();
	pRenderContext->Release();
}

void C_MergedMDL::SetupBonesForAttachmentQueries()
{
	static auto ptr = csgo->Utils.FindPatternIDA(GetModuleHandleA("client.dll"),
		"55 8B EC 83 EC 14 83 3D ? ? ? ? ? 53");
	return ((void(__thiscall*)(void*))(ptr))(this);
}

bool& IMaterialSystem::DisableRenderTargetAllocationForever()
{
	static auto ptr = csgo->Utils.FindPatternIDA(GetModuleHandleA("client.dll"),
		"80 B9 ? ? ? ? ? 74 0F") + 0x2;
	return *reinterpret_cast<bool*>(reinterpret_cast<std::uintptr_t>(this) + ptr);
}

void C_MergedMDL::SetMergedMDL(const char* szModelPath, CCustomMaterialOwner* pCustomMaterialOwner, void* pProxyData)
{
	static auto ptr = csgo->Utils.FindPatternIDA(GetModuleHandleA("client.dll"),
		"55 8B EC 57 8B F9 8B 0D ? ? ? ? 85 C9 75");
	return ((void(__thiscall*)(void*, const char*, CCustomMaterialOwner*, void*, bool))(ptr))(this, szModelPath, pCustomMaterialOwner, pProxyData, false);
}