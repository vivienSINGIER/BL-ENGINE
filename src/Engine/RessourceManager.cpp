#ifndef RESSOURCE_MANAGER_CPP_DEFINED
#define RESSOURCE_MANAGER_CPP_DEFINED

#include "RessourceManager.h"

RessourceManager* RessourceManager::s_pInstance = nullptr;

RessourceManager::RessourceManager()
{
    s_pInstance = this;
}

RessourceManager::~RessourceManager()
{
}

void RessourceManager::AddGeometry(String _name, Geometry* _pGeo)
{
    if (_pGeo == nullptr) return;

    s_pInstance->m_mGeometries[_name] = _pGeo;
}

Geometry* RessourceManager::GetGeometry(String _name)
{
    if (s_pInstance->m_mGeometries.contains(_name) == false) return nullptr;

    return s_pInstance->m_mGeometries[_name];
}

void RessourceManager::AddShader(String _name, Shader* _pShader)
{
    if (_pShader == nullptr) return;

    s_pInstance->m_mShaders[_name] = _pShader;
}

Shader* RessourceManager::GetShader(String _name)
{
    if (s_pInstance->m_mShaders.contains(_name) == false) return nullptr;

    return s_pInstance->m_mShaders[_name];
}

void RessourceManager::AddFont(String _name, RenderFont* _pFont)
{
    if (_pFont == nullptr) return;

    s_pInstance->m_mFonts[_name] = _pFont;
}

RenderFont* RessourceManager::GetFont(String _name)
{
    if (s_pInstance->m_mFonts.contains(_name) == false) return nullptr;

    return s_pInstance->m_mFonts[_name];
}

void RessourceManager::AddMaterial(String _name, Material* pMat)
{
    if (pMat == nullptr) return;

    //if (m_mMaterials.contains(name)) return;

    s_pInstance->m_mMaterials[_name] = pMat;
}

Material* RessourceManager::GetMaterial(String _name)
{
    if (s_pInstance->m_mMaterials.contains(_name) == false) return nullptr;

    return s_pInstance->m_mMaterials[_name];
}

#endif