#pragma once
#include "TextStyling.h"

namespace KE
{
	struct Texture;
	struct Sprite;


	struct MSDFPixel
	{
		unsigned char r, g, b, a;
	};
	
	struct CharacterData
	{
		char myCharacter;

		struct
		{
			float left, right, top, bottom;
		} planeBounds;

		struct
		{
			float left, right, top, bottom;
		} atlasBounds;

		float xAdvance;

		float gScale;

	};

	struct FontData
	{
		std::map<std::pair<char, char>, float> kernPairs;
		std::unordered_map<char, CharacterData> characters;

		struct
		{
			float ascendY;
			float descendY;
			float emSize;
			float lineHeight;
			float underlineThickness;
			float underlineY;

		} metrics;

	};

	class SpriteFont
	{
		friend class FontLoader;
	private:
		KE::Texture* fontAtlas;
		FontData fontData;

		float GetKern(const std::string& aString, int aCharacterIndex);
		void AlignText(
			const std::vector<std::vector<Sprite*>>& aSpriteRowSet,
			TextAlign aHorizontalAlignType,
			TextAlign aVerticalAlignType
		);
		
	public:
		SpriteFont() = default;

		void CreateCharacterMap(const std::string& aCharacters);
		
		void Init(
			KE::Texture* aFontAtlas,
			const std::string& aCharacterDefinitions
		)
		{
			fontAtlas = aFontAtlas;
			CreateCharacterMap(aCharacterDefinitions);
		}


		void PrepareSprites(
			Sprite** aSprites,
			const std::string& aString,
			const TextStyling& aStyling = TextStyling(),
			const Transform& aOrigin = Transform()
		);


		static std::string CreateSpriteString(
			const int& aNumber,
			const std::string& aPrefix = "",
			const std::string& aSuffix = ""
		);

		KE::Texture* GetFontAtlas() const { return fontAtlas; }
		const FontData& GetFontData() { return fontData; }
	};

	class FontLoader
	{
	private:
		static void SaveGeneratedFont(
			const char* aFontPath,
			const MSDFPixel* aImageData,
			const int aWidth,
			const int aHeight,
			FontData& aFontData
		);
	public:
		static SpriteFont LoadFont(
			const char* aFontPath
		);

		static void GenerateMSDFFont(
			const char* aFontPath,
			float aFontSize,
			float aMaxCornerAngle,
			float aPixelRange,
			float aMiterLimit
		);
	};

}
