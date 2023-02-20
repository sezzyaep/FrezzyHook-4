#include "DLL_MAIN.h"
typedef unsigned short ModelInstanceHandle_t;
struct ModelRenderInfo_t
{
	Vector origin;
	Vector angles;
	uint32_t pad;
	IClientRenderable* pRenderable;
	const model_t* pModel;
	matrix* pModelToWorld;
	const matrix* pLightingOffset;
	const Vector* pLightingOrigin;
	int flags;
	int entity_index;
	int skin;
	int body;
	int hitboxset;
	ModelInstanceHandle_t instance;
};
class c_bones {
public:
	c_bones(matrix* b, float time, float alpha) {
		memcpy(bones, b, sizeof(matrix[128]));
		this->time = time;
		this->alpha = alpha;
	}
	matrix bones[128];
	float time, alpha;
};
struct MatrixRecorded {
	void* ctx;
	void* state;
	ModelRenderInfo_t info;
	std::vector<c_bones> _info;
};
struct RenderableInfo_t {
	IClientRenderable* m_pRenderable;
	void* m_pAlphaProperty;
	int m_EnumCount;
	int m_nRenderFrame;
	unsigned short m_FirstShadow;
	unsigned short m_LeafList;
	short m_Area;
	uint16_t m_Flags;   // 0x0016
	uint16_t m_Flags2; // 0x0018
	Vector m_vecBloatedAbsMins;
	Vector m_vecBloatedAbsMaxs;
	Vector m_vecAbsMins;
	Vector m_vecAbsMaxs;
	int pad;
};

struct ColorMeshInfo_t;

struct LightDesc_t
{
	int	type;
	Vector		vecColor;
	Vector		vecPosition;
	Vector		vecDirection;
	float		flRange;
	float		flFalloff;
	float		flAttenuation0;
	float		flAttenuation1;
	float		flAttenuation2;
	float		flTheta;
	float		flPhi;
	float		flThetaDot;
	float		flPhiDot;
	float		flOneOverThetaDotMinusPhiDot;
	std::uint32_t fFlags;
protected:
	float		flRangeSquared;
};

struct MaterialLightingState_t
{
	Vector			vecAmbientCube[6];
	Vector			vecLightingOrigin;
	int				nLocalLightCount;
	LightDesc_t		localLightDesc[4];
};
typedef void* StudioDecalHandle_t;

struct studiohwdata_t;

struct StaticPropRenderInfo_t
{
	const matrix* pModelToWorld;
	const model_t* pModel;
	IClientRenderable* pRenderable;
	Vector* pLightingOrigin;
	short                   skin;
	ModelInstanceHandle_t   instance;
};
struct Texture_t
{
	std::byte			pad0[0xC];		// 0x0000
	IDirect3DTexture9* lpRawTexture;	// 0x000C
};


class ITexture
{
private:
	template <typename T, typename ... args_t>
	constexpr T CallVFunc(void* thisptr, std::size_t nIndex, args_t... argList)
	{
		using VirtualFns = T(__thiscall*)(void*, decltype(argList)...);
		return (*static_cast<VirtualFns**>(thisptr))[nIndex](thisptr, argList...);
	}
private:
	std::byte	pad0[0x50];		 // 0x0000
public:
	Texture_t** pTextureHandles; // 0x0050

	int GetActualWidth()
	{
		return CallVFunc<int>(this, 3);
	}

	int GetActualHeight()
	{
		return CallVFunc<int>(this, 4);
	}

	void IncrementReferenceCount()
	{
		CallVFunc<void>(this, 10);
	}

	void DecrementReferenceCount()
	{
		CallVFunc<void>(this, 11);
	}
};
typedef void* LightCacheHandle_t;

typedef int OverrideType_t;
/*
struct DrawModelInfo_t
{
	studiohdr_t* pStudioHdr;
	studiohwdata_t* pHardwareData;
	const matrix* m_pModelToWorld;
	StudioDecalHandle_t		hDecals;
	int						iSkin;
	int						iBody;
	int						iHitboxSet;
	IClientRenderable* pClientEntity;
	int						iLOD;
	ColorMeshInfo_t* pColorMeshes;
	bool					bStaticLighting;
	MaterialLightingState_t	LightingState;
	int						m_drawFlags;
};*/
class IRenderToRTHelperObject
{
public:
	virtual void Draw(const matrix& matRootToWorld) = 0;
	virtual bool BoundingSphere(Vector& vecCenter, float& flRadius) = 0;
	virtual ITexture* GetEnvCubeMap() = 0;
};

class CMDL
{
public:
	std::byte	pad[0x3C]; // 0x0000
	MDLHandle_t	hModelHandle; // 0x003C
	std::byte	pad0[0x8]; // 0x003E
	color_t		Color; // 0x0046
	std::byte	pad1[0x2]; // 0x004A
	int			nSkin; // 0x004C
	int			nBody; // 0x0050
	int			nSequence; // 0x0054
	int			nLOD; // 0x0058
	float		flPlaybackRate; // 0x005C
	float		flTime; // 0x0060
	float		flCurrentAnimEndTime; // 0x0064
	float		arrFlexControls[96 * 4]; // 0x0068
	Vector		vecViewTarget; // 0x0668
	bool		bWorldSpaceViewTarget; // 0x0674
	bool		bUseSequencePlaybackFPS; // 0x0675
	std::byte	pad2[0x2]; // 0x0676
	void* pProxyData; // 0x0678
	float		flTimeBasisAdjustment; // 0x067C
	std::byte	pad3[0x4]; // 0x0680 --isn't correct after this point iirc
	CUtlVector<int> arrSequenceFollowLoop; // 0x0684
	matrix	matModelToWorld; // 0x0698
	bool		bRequestBoneMergeTakeover; // 0x06C8
}; // Size: 0x06C9 // 0x6D0?
class CCustomMaterialOwner
{
public:
	virtual ~CCustomMaterialOwner() { }
	virtual void SetCustomMaterial(void* pCustomMaterial, int nIndex = 0) = 0;	// either replaces and existing material (releasing the old one), or adds one to the vector
	virtual void OnCustomMaterialsUpdated() {}
	virtual void DuplicateCustomMaterialsToOther(CCustomMaterialOwner* pOther) const = 0;

public:
	CUtlVector<void*> vecCustomMaterials;
}; // Size: 0x0014

struct MDLSquenceLayer_t
{
	int		nSequenceIndex;
	float	flWeight;
};
enum MorphFormatFlags_t
{
	MORPH_POSITION = 0x0001,	// 3D
	MORPH_NORMAL = 0x0002,	// 3D
	MORPH_WRINKLE = 0x0004,	// 1D
	MORPH_SPEED = 0x0008,	// 1D
	MORPH_SIDE = 0x0010,	// 1D
};
class CMergedMDL : public IRenderToRTHelperObject
{
public:
	virtual ~CMergedMDL() { }
	virtual void SetMDL(MDLHandle_t hModelHandle, CCustomMaterialOwner* pCustomMaterialOwner = nullptr, void* pProxyData = nullptr) = 0;
	virtual void SetMDL(const char* szModelPath, CCustomMaterialOwner* pCustomMaterialOwner = nullptr, void* pProxyData = nullptr) = 0;

	void SetMergedMDL(const char* szModelPath, CCustomMaterialOwner* pCustomMaterialOwner = nullptr, void* pProxyData = nullptr);

	void SetupBonesForAttachmentQueries();

	void SetSequence(const int nSequence, const bool bUseSequencePlaybackFPS)
	{
		this->RootMDL.nSequence = nSequence;
		this->RootMDL.bUseSequencePlaybackFPS = bUseSequencePlaybackFPS;
		this->RootMDL.flTimeBasisAdjustment = this->RootMDL.flTime;
	}

	void SetSkin(int nSkin)
	{
		this->RootMDL.nSkin = nSkin;
	}

public:
	CMDL RootMDL; // 0x0000
	CUtlVector<CMDL> vecMergedModels; // 0x069C
	float arrPoseParameters[24]; // 0x06E9
	int	nSequenceLayers; // 0x0749
	MDLSquenceLayer_t sequenceLayers[8]; // 0x074D -> // end: 0x78D
};

struct DrawModelState_t
{
	studiohdr_t* m_pStudioHdr;
	studiohwdata_t* m_pStudioHWData;
	IClientRenderable* m_pRenderable;
	const matrix* m_pModelToWorld;
	StudioDecalHandle_t		m_decals;
	int						m_drawFlags;
	int						m_lod;
};
struct DrawModelInfo_t
{
	studiohdr_t* pStudioHdr;
	studiohwdata_t* pHardwareData;
	StudioDecalHandle_t hDecals;
	int				iSkin;
	int				iBody;
	int				iHitboxSet;
	IClientRenderable* pClientEntity;
	int				iLOD;
	ColorMeshInfo_t* pColorMeshes;
	bool			bStaticLighting;
	Vector			m_vecAmbientCube[6];		// ambient, and lights that aren't in locallight[]
	int				m_nLocalLightCount;
	LightDesc_t		m_LocalLightDescs[4];
};

struct LightingQuery_t
{
	Vector                  LightingOrigin;
	ModelInstanceHandle_t   InstanceHandle;
	bool                    bAmbientBoost;
};

struct StaticLightingQuery_t : public LightingQuery_t
{
	IClientRenderable* pRenderable;
};

class DataCacheHandle_t;
class IRefCounted
{
public:
	virtual int AddReference() = 0;
	virtual int Release() = 0;
};

struct LightDesc_t;
class IMatRenderContext : public IRefCounted
{
private:
	template <typename T, typename ... args_t>
	constexpr T CallVFunc(void* thisptr, std::size_t nIndex, args_t... argList)
	{
		using VirtualFnz = T(__thiscall*)(void*, decltype(argList)...);
		return (*static_cast<VirtualFnz**>(thisptr))[nIndex](thisptr, argList...);
	}
public:
	void BeginRender()
	{
		CallVFunc<void>(this, 2);
	}

	void EndRender()
	{
		CallVFunc<void>(this, 3);
	}

	void BindLocalCubemap(ITexture* pTexture)
	{
		CallVFunc<void>(this, 5, pTexture);
	}

	void SetRenderTarget(ITexture* pTexture)
	{
		CallVFunc<void>(this, 6, pTexture);
	}

	ITexture* GetRenderTarget()
	{
		return CallVFunc<ITexture*>(this, 7);
	}

	void ClearBuffers(bool bClearColor, bool bClearDepth, bool bClearStencil = false)
	{
		CallVFunc<void>(this, 12, bClearColor, bClearDepth, bClearStencil);
	}

	void SetLights(int nCount, const LightDesc_t* pLights)
	{
		CallVFunc<void>(this, 17, nCount, pLights);
	}

	void SetAmbientLightCube(Vector4D vecCube[6])
	{
		CallVFunc<void>(this, 18, vecCube);
	}

	void SetIntRenderingParameter(int a1, int a2)
	{
		return CallVFunc < void(__thiscall*)(void*, int, int) >(this, 126)(this, a1, a2);
	}

	void Viewport(int x, int y, int iWidth, int iHeight)
	{
		CallVFunc<void>(this, 40, x, y, iWidth, iHeight);
	}

	void GetViewport(int& x, int& y, int& iWidth, int& iHeight)
	{
		CallVFunc<void, int&, int&, int&, int&>(this, 41, x, y, iWidth, iHeight);
	}

	void ClearColor3ub(unsigned char r, unsigned char g, unsigned char b)
	{
		CallVFunc<void>(this, 78, r, g, b);
	}

	void ClearColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		CallVFunc<void>(this, 79, r, g, b, a);
	}

	void DrawScreenSpaceRectangle(IMaterial* pMaterial, int iDestX, int iDestY, int iWidth, int iHeight, float flTextureX0, float flTextureY0, float flTextureX1, float flTextureY1, int iTextureWidth, int iTextureHeight, void* pClientRenderable = nullptr, int nXDice = 1, int nYDice = 1)
	{
		CallVFunc<void>(this, 114, pMaterial, iDestX, iDestY, iWidth, iHeight, flTextureX0, flTextureY0, flTextureX1, flTextureY1, iTextureWidth, iTextureHeight, pClientRenderable, nXDice, nYDice);
	}

	void PushRenderTargetAndViewport()
	{
		CallVFunc<void>(this, 119);
	}

	void PopRenderTargetAndViewport()
	{
		CallVFunc<void>(this, 120);
	}

	void SetLightingOrigin(/*Vector vecLightingOrigin*/float x, float y, float z)
	{
		CallVFunc<void>(this, 158, x, y, z);
	}

};
struct color_tMeshInfo_t;

class IVModelRender
{
public:
	virtual int                     DrawModel(int flags, IClientRenderable* pRenderable, ModelInstanceHandle_t instance, int entity_index, const model_t* model, Vector const& origin, Vector const& angles, int skin, int body, int hitboxset, const matrix* modelToWorld = NULL, const matrix* pLightingOffset = NULL) = 0;
	virtual void                    ForcedMaterialOverride(IMaterial* newMaterial, int nOverrideType = 0, int nOverrides = 0) = 0;
	virtual bool                    IsForcedMaterialOverride(void) = 0; // hey hey!!!!! this is used to fix glow being retarded in DrawModel / DrawModelExecute. I bet you didn't know that! ~alpha
	virtual void                    SetViewTarget(const studiohdr_t* pStudioHdr, int nBodyIndex, const Vector& target) = 0;
	virtual ModelInstanceHandle_t   CreateInstance(IClientRenderable* pRenderable, LightCacheHandle_t* pCache = NULL) = 0;
	virtual void                    DestroyInstance(ModelInstanceHandle_t handle) = 0;
	virtual void                    SetStaticLighting(ModelInstanceHandle_t handle, LightCacheHandle_t* pHandle) = 0;
	virtual LightCacheHandle_t      GetStaticLighting(ModelInstanceHandle_t handle) = 0;
	virtual bool                    ChangeInstance(ModelInstanceHandle_t handle, IClientRenderable* pRenderable) = 0;
	virtual void                    AddDecal(ModelInstanceHandle_t handle, Ray_t const& ray, Vector const& decalUp, int decalIndex, int body, bool noPokeThru, int maxLODToDecal) = 0;
	virtual void                    RemoveAllDecals(ModelInstanceHandle_t handle) = 0;
	virtual bool                    ModelHasDecals(ModelInstanceHandle_t handle) = 0;
	virtual void                    RemoveAllDecalsFromAllModels() = 0;
	virtual matrix* DrawModelShadowSetup(IClientRenderable* pRenderable, int body, int skin, ModelRenderInfo_t* pInfo, matrix* pCustomBoneToWorld = NULL) = 0;
	virtual void                    DrawModelShadow(IClientRenderable* pRenderable, const ModelRenderInfo_t& info, matrix* pCustomBoneToWorld = NULL) = 0;
	virtual bool                    RecomputeStaticLighting(ModelInstanceHandle_t handle) = 0;
	virtual void                    ReleaseAllStaticPropcolor_tData(void) = 0;
	virtual void                    RestoreAllStaticPropcolor_tData(void) = 0;
	virtual int                     DrawModelEx(ModelRenderInfo_t& pInfo) = 0;
	virtual int                     DrawModelExStaticProp(ModelRenderInfo_t& pInfo) = 0;
	virtual bool                    DrawModelSetup(ModelRenderInfo_t& pInfo, DrawModelInfo_t* pState, matrix** ppBoneToWorldOut) = 0;
	virtual void                    DrawModelExecute(IMatRenderContext* ctx, const DrawModelInfo_t& state, const ModelRenderInfo_t& pInfo, matrix* pCustomBoneToWorld = NULL) = 0;
	virtual void                    SetupLighting(const Vector& vecCenter) = 0;
	virtual int                     DrawStaticPropArrayFast(StaticPropRenderInfo_t* pProps, int count, bool bShadowDepth) = 0;
	virtual void                    SuppressEngineLighting(bool bSuppress) = 0;
	virtual void                    Setupcolor_tMeshes(int nTotalVerts) = 0;
	virtual void                    SetupLightingEx(const Vector& vecCenter, ModelInstanceHandle_t handle) = 0;
	virtual bool                    GetBrightestShadowingLightSource(const Vector& vecCenter, Vector& lightPos, Vector& lightBrightness, bool bAllowNonTaggedLights) = 0;
	virtual void                    ComputeLightingState(int nCount, const LightingQuery_t* pQuery, MaterialLightingState_t* pState, ITexture** ppEnvCubemapTexture) = 0;
	virtual void                    GetModelDecalHandles(StudioDecalHandle_t* pDecals, int nDecalStride, int nCount, const ModelInstanceHandle_t* pHandles) = 0;
	virtual void                    ComputeStaticLightingState(int nCount, const StaticLightingQuery_t* pQuery, MaterialLightingState_t* pState, MaterialLightingState_t* pDecalState, color_tMeshInfo_t** ppStaticLighting, ITexture** ppEnvCubemapTexture, DataCacheHandle_t* pcolor_tMeshHandles) = 0;
	virtual void                    CleanupStaticLightingState(int nCount, DataCacheHandle_t* pcolor_tMeshHandles) = 0;
};