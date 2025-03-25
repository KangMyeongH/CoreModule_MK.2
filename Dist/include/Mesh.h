#pragma once
#include "core_defines.h"
#include "Object.h"

namespace engine
{
    class COREMODULE_API Mesh : public Object
    {
    protected:
        Mesh();
        ~Mesh() override;

    public:
        size_t GetMaterialCount() const { return m_SubMeshes.size(); }

    	std::vector<SubMesh>& GetSubMeshes() { return m_SubMeshes; }
        void SetSubMesh(const std::vector<SubMesh>& subMeshes);

        static SharedPtr<Mesh> Create(const SharedPtr<VIBuffer>& viBuffer);
    	void Destroy() override;

        void Bind(const ComPtr<ID3D11DeviceContext>& context);
        void Render(const ComPtr<ID3D11DeviceContext>& context);

    private:
        SharedPtr<VIBuffer> m_VIBuffer;
        std::vector<SubMesh> m_SubMeshes;
    };
}
