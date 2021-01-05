#include "CrescentPCH.h"
#include "BoneMapper.h"

namespace CrescentEngine
{
    BoneMapper::BoneMapper()
    {
        Clear();
    }

    uint32_t BoneMapper::Name(const std::string& name)
    {
        if (m_BoneLibrary.count(name)) //If the bone exists in our library, we simply return it. 
        {
            return m_BoneLibrary[name];
        }

        return m_BoneLibrary[name] = m_TotalBones++; //Else, we create a new entry in our library, set its value to a unique index, return and then increment it. 
    }

    void BoneMapper::Clear()
    {
        m_BoneLibrary.clear();
        m_TotalBones = 0;
    }
}
