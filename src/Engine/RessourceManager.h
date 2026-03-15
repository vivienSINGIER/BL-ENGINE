#ifndef RESSOURCE_MANAGER_H_DEFINED
#define RESSOURCE_MANAGER_H_DEFINED

#include "define.h"
#include "../Render/Generic/Render.h"

class RessourceManager
{
public:
	RessourceManager();
	~RessourceManager();

	static void AddGeometry(String _name, Geometry* _pGeo);
	static Geometry* GetGeometry(String _name);

	static void AddMaterial(String _name, Material* pMat);
	static Material* GetMaterial(String _name);

	static void AddShader(String _name, Shader* _pShader);
	static Shader* GetShader(String _name);

	static void AddFont(String _name, RenderFont* _pFont);
	static RenderFont* GetFont(String _name);

private:
	static RessourceManager* s_pInstance;

	UnorderedMap<String, Geometry*> m_mGeometries;
	UnorderedMap<String, Shader*> m_mShaders;
	UnorderedMap<String, Material*> m_mMaterials;
	UnorderedMap<String, RenderFont*> m_mFonts;
	UnorderedMap<String, Brush*> m_mBrushs;
};

#endif