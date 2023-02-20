#pragma once
#include "DLL_MAIN.h"
//class IVRenderView
//{
//public:
//	void SetBlend(float alpha)
//	{
//		typedef void(__thiscall* oDrawModelExecute)(PVOID, float);
//		return getvfunc< oDrawModelExecute >(this, 4)(this, alpha);
//	}
//
//	void SetColorModulation(float const* colors)
//	{
//		typedef void(__thiscall* oDrawModelExecute)(PVOID, float const*);
//		return getvfunc< oDrawModelExecute >(this, 6)(this, colors);
//	}
//};
struct VisibleFogVolumeInfo_t
{
    int            m_nVisibleFogVolume;
    int            m_nVisibleFogVolumeLeaf;
    bool        m_bEyeInFogVolume;
    float       m_flDistanceToWater;
    float       m_flWaterHeight;
    IMaterial* m_pFogVolumeMaterial;
};
typedef unsigned short LeafIndex_t;


struct WorldListLeafData_t
{
    LeafIndex_t     leafIndex;    // 16 bits
    int16_t         waterData;
    uint16_t        firstTranslucentSurface;    // engine-internal list index
    uint16_t        translucentSurfaceCount;    // count of translucent surfaces+disps
};
struct WorldListInfo_t
{
    int                     m_ViewFogVolume;
    int                     m_LeafCount;
    bool                    m_bHasWater;
    WorldListLeafData_t* m_pLeafDataList;
};
struct VisOverrideData_t
{
    Vector        m_vecVisOrigin;                    // The point to to use as the viewpoint for area portal backface cull checks.
    float        m_fDistToAreaPortalTolerance;    // The distance from an area portal before using the full screen as the viewable portion.
};
class CViewSetup {
public:
    int            x, x_old;
    int            y, y_old;
    int            width, width_old;
    int            height, height_old;
    bool        m_bOrtho;
    float        m_OrthoLeft;
    float        m_OrthoTop;
    float        m_OrthoRight;
    float        m_OrthoBottom;
    bool        m_bCustomViewMatrix;
    matrix		m_matCustomViewMatrix;
    char        pad_0x68[0x48];
    float        fov;
    float        fovViewmodel;
    Vector        origin;
    Vector        angles;
    float		flNearZ;
    float		flFarZ;
    float		flNearViewmodelZ;
    float		flFarViewmodelZ;
    float		flAspectRatio;
    float		flNearBlurDepth;
    float		flNearFocusDepth;
    float		flFarFocusDepth;
    float		flFarBlurDepth;
    float		flNearBlurRadius;
    float		flFarBlurRadius;
    float		flDoFQuality;
    int			nMotionBlurMode;
    float		flShutterTime;
    Vector		vecShutterOpenPosition;
    Vector		vecShutterOpenAngles;
    Vector		vecShutterClosePosition;
    Vector		vecShutterCloseAngles;
    float		flOffCenterTop;
    float		flOffCenterBottom;
    float		flOffCenterLeft;
    float		flOffCenterRight;
    bool		bOffCenter;
    bool		bRenderToSubrectOfLargerScreen;
    bool		bDoBloomAndToneMapping;
    bool		bDoDepthOfField;
    bool		bHDRTarget;
    bool		bDrawWorldNormal;
    bool		bCullFontFaces;
    bool		bCacheFullSceneState;
    bool		bCSMView;
};

class IWorldRenderList;
class IVRenderView
{
public:

    virtual void      DrawBrushModel(void* pBaseEntity, model_t* pModel, const Vector& vecOrigin, const Vector& angView, bool bSort) = 0;
    virtual void      DrawIdentityBrushModel(IWorldRenderList* pList, model_t* pModel) = 0;
    virtual void      TouchLight(struct dlight_t* dLight) = 0;
    virtual void      Draw3DDebugOverlays() = 0;
    virtual void      SetBlend(float flBlend) = 0;
    virtual float      GetBlend() = 0;
    virtual void      SetColorModulation(float const* flBlend) = 0;
    inline void       SetColorModulation(float r, float g, float b)
    {
        float clr[3] = { r, g, b };
        SetColorModulation(clr);
    }

    virtual void      GetColorModulation(float* flBlend) = 0;
    virtual void      SceneBegin() = 0;
    virtual void      SceneEnd() = 0;
    virtual void      GetVisibleFogVolume(const Vector& vecEyePoint, VisibleFogVolumeInfo_t* pInfo) = 0;
    virtual IWorldRenderList* CreateWorldList() = 0;
    virtual void      BuildWorldLists_Epilogue(IWorldRenderList* pList, WorldListInfo_t* pInfo, bool bShadowDepth) = 0;
    virtual void      BuildWorldLists(IWorldRenderList* pList, WorldListInfo_t* pInfo, int iForceFViewLeaf, const VisOverrideData_t* pVisData = nullptr, bool bShadowDepth = false, float* pReflectionWaterHeight = nullptr) = 0;
    virtual void      DrawWorldLists(IWorldRenderList* pList, unsigned long flags, float flWaterZAdjust) = 0;
    virtual void      GetWorldListIndicesInfo(void* pIndicesInfoOut, IWorldRenderList* pList, unsigned long nFlags) = 0;
    virtual void      DrawTopView(bool bEnable) = 0;
    virtual void      TopViewNoBackfaceCulling(bool bDisable) = 0;
    virtual void      TopViewNoVisCheck(bool bDisable) = 0;
    virtual void      TopViewBounds(Vector2D const& vecMins, Vector2D const& vecMaxs) = 0;
    virtual void      SetTopViewVolumeCuller(const void* pVolumeCuller) = 0;
    virtual void      DrawLights() = 0;
    virtual void      DrawMaskEntities() = 0;
    virtual void      DrawTranslucentSurfaces(IWorldRenderList* pList, int* pSortList, int iSortCount, unsigned long fFlags) = 0;
    virtual void      DrawLineFile() = 0;
    virtual void      DrawLightmaps(IWorldRenderList* pList, int iPageID) = 0;
    virtual void      ViewSetupVis(bool bNoVis, int nOrigins, const Vector vecOrigin[]) = 0;
    virtual bool      AreAnyLeavesVisible(int* pLeafList, int nLeaves) = 0;
    virtual  void      VguiPaint() = 0;
    virtual void      ViewDrawFade(uint8_t* pColor, IMaterial* pMaterial) = 0;
    virtual void      OLD_SetProjectionMatrix(float flFov, float zNear, float zFar) = 0;
    virtual int    GetLightAtPoint(Vector& vecPosition) = 0;
    virtual int        GetViewEntity() = 0;
    virtual bool      IsViewEntity(int nEntityIndex) = 0;
    virtual float      GetFieldOfView() = 0;
    virtual unsigned char** GetAreaBits() = 0;
    virtual void      SetFogVolumeState(int nVisibleFogVolume, bool bUseHeightFog) = 0;
    virtual void      InstallBrushSurfaceRenderer(void* pBrushRenderer) = 0;
    virtual void      DrawBrushModelShadow(IClientRenderable* pRenderable) = 0;
    virtual  bool      LeafContainsTranslucentSurfaces(IWorldRenderList* pList, int nSortIndex, unsigned long fFlags) = 0;
    virtual bool      DoesBoxIntersectWaterVolume(const Vector& vecMins, const Vector& vecMaxs, int nLeafWaterDataID) = 0;
    virtual void      SetAreaState(unsigned char uAreaBits[32], unsigned char uAreaPortalBits[24]) = 0;
    virtual void      VGui_Paint(int nMode) = 0;
    virtual void      Push3DView(IMatRenderContext* pRenderContext, const CViewSetup& view, int nFlags, ITexture* pRenderTarget, Frustum_t frustumPlanes) = 0; //xd
    virtual void      Push2DView(IMatRenderContext* pRenderContext, const CViewSetup& view, int nFlags, ITexture* pRenderTarget, Frustum_t frustumPlanes) = 0;
    virtual void      PopView(IMatRenderContext* pRenderContext, Frustum_t frustumPlanes) = 0;
    virtual void      SetMainView(const Vector& vecOrigin, const Vector& angView) = 0;
    virtual void      ViewSetupVisEx(bool bNoVis, int nOrigins, const Vector arrOrigin[], unsigned int& uReturnFlags) = 0;
    virtual void      OverrideViewFrustum(Frustum_t custom) = 0;
    virtual void      DrawBrushModelShadowDepth(void* pEntity, model_t* pModel, const Vector& vecOrigin, const Vector& angView, int nDepthMode) = 0;
    virtual void      UpdateBrushModelLightmap(model_t* pModel, IClientRenderable* pRenderable) = 0;
    virtual void      BeginUpdateLightmaps() = 0;
    virtual void      EndUpdateLightmaps() = 0;
    virtual void      OLD_SetOffCenterProjectionMatrix(float flFOV, float flNearZ, float flFarZ, float flAspectRatio, float flBottom, float flTop, float flLeft, float flRight) = 0;
    virtual void      OLD_SetProjectionMatrixOrtho(float flLeft, float flTop, float flRight, float flBottom, float flNearZ, float flFarZ) = 0;
    virtual void      Push3DView(IMatRenderContext* pRenderContext, const CViewSetup& view, int nFlags, ITexture* pRenderTarget, Frustum_t frustumPlanes, ITexture* pDepthTexture) = 0;
    virtual void      GetMatricesForView(const CViewSetup& view, void* pWorldToView, void* pViewToProjection, void* pWorldToProjection, void* pWorldToPixels) = 0;
    virtual void      DrawBrushModelEx(void* pEntity, model_t* pModel, const Vector& vecOrigin, const Vector& angView, int nMode) = 0;
    virtual bool      DoesBrushModelNeedPowerOf2Framebuffer(const model_t* pModel) = 0;
    virtual void      DrawBrushModelArray(IMatRenderContext* pContext, int nCount, const void* pInstanceData, int nModelTypeFlags) = 0;

};