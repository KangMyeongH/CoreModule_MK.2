#pragma once
#include "core_defines.h"


namespace engine
{
	class Material;
}

namespace engine
{
	namespace editor
	{
		class EditorCamera;

		class COREMODULE_API Grid
        {
            //======================================//
            //				constructor				//
            //======================================//
        public:
            Grid();
            ~Grid();

            //======================================//
			//				 property				//
			//======================================//
		public:
            SharedPtr<Material> GetMaterial() const { return m_Material; }


            //======================================//
            //				  method				//
            //======================================//
        public:
            void InitGrid(const ComPtr<ID3D11Device>& device, int maxGridRange);
            void UpdateGridVertices(ComPtr<ID3D11DeviceContext> context, Vector3 cameraPos, _float gridStep, _int gridRange);
            void Bind(const ComPtr<ID3D11DeviceContext>& context, const EditorCamera& camera);
        	void RenderGird(const ComPtr<ID3D11DeviceContext>& context);

            //======================================//
            //				  fields				//
            //======================================//
        private:
            ComPtr<ID3D11Buffer> m_GirdVB;
            SharedPtr<Material> m_Material;

            _int m_MaxGirdVerts = 0;
            _int m_CurrentVerts = 0;

            _float m_PrevSnappedX = 99999999.0f;
            _float m_PrevSnappedZ = 99999999.0f;
        };
	}
}
