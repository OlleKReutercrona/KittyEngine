#pragma once
#include <string>

namespace KE
{
	class TextureLoader;
}

namespace KE_EDITOR
{
	struct EditorIcon;

	enum class EditorFileType
	{
		eFolder,
		eTexture,
		eCubemap,
		eVFXSequence,
		eScript,
		eModel,
		eLevel,
		eUnknown,

		Count
	};

	enum class EditorFileInteraction
	{
		eHovered,
		eLeftClick,
		eLeftDoubleClick,
		eRightClick,
		eRightDoubleClick,

		eNone
	};

	class EditorFile
	{
	private:
		EditorFileType myType;
		std::string myPath;
		std::string myName;
		std::string myIconPath;

	public:
		EditorFile() :myType(EditorFileType::eUnknown), myPath(""), myName(""), myIconPath("") {};

		EditorFile(const EditorFileType& aType, const std::string& aPath, const std::string& aName, const std::string& anIconPath)
		: myType(aType), myPath(aPath), myName(aName), myIconPath(anIconPath) { }

		EditorFile(const EditorFile& aFile);

		~EditorFile() = default;

		EditorFileInteraction Display(KE::TextureLoader* aTextureLoader) const;
		
		EditorFileInteraction BeginDisplayBox(KE::TextureLoader* aTextureLoader) const;
		EditorFileInteraction DisplayIcon(KE::TextureLoader* aTextureLoader) const;
		
		inline const std::string& GetPath() { return myPath; }
		inline const std::string& GetName() { return myName; }
		inline EditorFileType GetType() { return myType; }

		void DisplayName() const;
	};

}
