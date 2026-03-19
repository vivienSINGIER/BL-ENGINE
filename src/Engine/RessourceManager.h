#ifndef RESSOURCE_MANAGER_H_DEFINED
#define RESSOURCE_MANAGER_H_DEFINED

#include "define.h"
#include "../Render/Generic/Render.h"

class RessourceManager
{
public:
	RessourceManager();
	~RessourceManager();

	static uint32 AddGeometry(String _name, Geometry* _pGeo);
	static Geometry* GetGeometry(String _name);
	static Geometry* GetGeometry(uint32 _id) { return s_pInstance->m_vGeometries[_id]; }

	static uint32 AddMaterial(String _name, Material* pMat);
	static Material* GetMaterial(String _name);
	static Material* GetMaterial(uint32 _id) { return s_pInstance->m_vMaterials[_id]; }

	static uint32 AddShader(String _name, Shader* _pShader);
	static Shader* GetShader(String _name);
	static Shader* GetShader(uint32 _id) { return s_pInstance->m_vShaders[_id]; }

	static uint32 AddFont(String _name, RenderFont* _pFont);
	static RenderFont* GetFont(String _name);
	static RenderFont* GetFont(uint32 _id) { return s_pInstance->m_vFonts[_id]; }

	static uint32 AddTexture(String _name, Texture* _pTexture);
	static Texture* GetTexture(String _name);
	static Texture* GetTexture(uint32 _id) { return s_pInstance->m_vTextures[_id]; }

	static uint32 AddCamera(String _name);
	static Camera* GetCamera(String _name);
	static Camera* GetCamera(uint32 _id) { return s_pInstance->m_vCameras[_id]; }

private:
	static RessourceManager* s_pInstance;

	Vector<Geometry*> m_vGeometries;
	Vector<Shader*> m_vShaders;
	Vector<Material*> m_vMaterials;
	Vector<RenderFont*> m_vFonts;
	Vector<Texture*> m_vTextures;
	Vector<Camera*> m_vCameras;
	
	UnorderedMap<String, uint32> m_mGeometryIndices;
	UnorderedMap<String, uint32> m_mShaderIndices;
	UnorderedMap<String, uint32> m_mMaterialIndices;
	UnorderedMap<String, uint32> m_mFontIndices;
	UnorderedMap<String, uint32> m_mTextureIndices;
	UnorderedMap<String, uint32> m_mCameraIndices;
};

#endif