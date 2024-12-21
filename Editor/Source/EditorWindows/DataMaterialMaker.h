#pragma once


#ifndef KITTYENGINE_NO_EDITOR

namespace KE
{
	struct DataMaterial;
	struct Texture;
}

namespace KE_EDITOR
{
	class MaterialEditor : public EditorWindowBase
	{
		KE_EDITOR_FRIEND
	private:
		struct
		{
			Vector3f albedo = { 1.0f, 1.0f, 1.0f };
			float metallic = 0.0f;
			float roughness = 0.0f;
			float ao = 1.0f; //not used yet
			float emission = 0.0f;

			char materialName[64] = "CustomMaterial";
		} materialParameters;

		KE::DataMaterial* madeMaterial = nullptr;
		KE::Texture* thumbnailTexture = nullptr;

	public:
		MaterialEditor(EditorWindowInput aStartupData = {}) : EditorWindowBase(aStartupData) {}

		const char* GetWindowName() const override { return "Material Editor"; }
		void Init() override;
		void Update() override;
		void Render() override;
	};
}

#endif