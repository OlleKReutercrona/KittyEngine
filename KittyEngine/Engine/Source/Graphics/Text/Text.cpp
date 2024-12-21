#include "stdafx.h"
#include "Text.h"
#include "Engine/Source/Graphics/Sprite/Sprite.h"
#include <External/Include/nlohmann/json.hpp>

#include <External/Include/msdf-atlas-gen/msdf-atlas-gen.h>

#include "dxtex/DDSTextureLoader11.h"
#include "Graphics/Graphics.h"
#include "msdf-atlas-gen/core/edge-coloring.h"
#include "msdf-atlas-gen/ext/import-font.h"

#include <ft2build.h>

#include "imgui/imgui.h"
#include "Utility/BinaryIO.h"

#include FT_FREETYPE_H


using namespace msdf_atlas;


float KE::SpriteFont::GetKern(const std::string& aString, int aCharacterIndex)
{
	if (aString.size() - 1 > aCharacterIndex && fontData.kernPairs.contains({ aString[aCharacterIndex], aString[aCharacterIndex + 1] }))
	{
		return fontData.kernPairs.at({ aString[aCharacterIndex], aString[aCharacterIndex + 1] });
	}
	return 0.0f;
}

void KE::SpriteFont::AlignText(const std::vector<std::vector<Sprite*>>& aSpriteRowSet, TextAlign aHorizontalAlignType, TextAlign aVerticalAlignType)
{
	if (aSpriteRowSet.size() == 0) return;

	switch(aHorizontalAlignType)
	{
	case TextAlign::LeftOrTop:
	{
		// this is the default, so unless we want to correct indented text (which we might not) we don't need to do anything
		break;
	}
	case TextAlign::Center:
	{
		for (int i = 0; i < aSpriteRowSet.size(); i++)
		{
			if (aSpriteRowSet[i].size() == 0) continue;

			const auto& firstSpriteTrans = aSpriteRowSet[i][0]->myAttributes.myTransform;
			const float minX = firstSpriteTrans.GetPosition().x - firstSpriteTrans.GetScale().x;
			const auto& lastSpriteTrans = aSpriteRowSet[i][aSpriteRowSet[i].size() - 1]->myAttributes.myTransform;
			const float maxX = lastSpriteTrans.GetPosition().x + lastSpriteTrans.GetScale().x;
			const float rowWidth = maxX - minX;

			const float rowOffset = rowWidth / 2.0f;
			for (int j = 0; j < aSpriteRowSet[i].size(); j++)
			{
				aSpriteRowSet[i][j]->myAttributes.myTransform.SetPosition(
				{
					aSpriteRowSet[i][j]->myAttributes.myTransform.GetPosition().x - rowOffset,
					aSpriteRowSet[i][j]->myAttributes.myTransform.GetPosition().y,
					aSpriteRowSet[i][j]->myAttributes.myTransform.GetPosition().z
				});
			}
		}

		

		break;
	}
	case TextAlign::RightOrBottom:
	{
		break;
	}
	}
}

void KE::SpriteFont::CreateCharacterMap(const std::string& aCharacters)
{
	nlohmann::json characterData = nlohmann::json::parse(aCharacters);

	std::string fontName = characterData["name"];
	bool bold = characterData["bold"];
	bool italic = characterData["italic"];

	unsigned int size = characterData["size"];
	unsigned int width = characterData["width"];
	unsigned int height = characterData["height"];

	for (auto& character : characterData["characters"].items())
	{
		std::string keystr = character.key();
		char key = keystr[0];

		CharacterData data;

		data.myCharacter = key;
		/*data.x = character.value()["x"];
		data.y = character.value()["y"];
		data.width = character.value()["width"];
		data.height = character.value()["height"];
		data.xAdvance = character.value()["advance"];*/


		fontData.characters[key] = data;
	}


}


void KE::SpriteFont::PrepareSprites(Sprite** aSprites, const std::string& aString, const TextStyling& aStyling, const Transform& aOrigin)
{
	size_t charCount = aString.size();
	//const char* charArray = aString.c_str();

	const float scaleFactor = 1.0f;

	Matrix4x4f originMat = {}; //aOrigin.GetCUMatrix();

	const float fsScale = 1.0f / (fontData.metrics.ascendY - fontData.metrics.descendY);
	float x = 0.0f, y = 0.0f/*fsScale * fontData.metrics.ascendY*/;

	const float texelWidth  = 1.0f / (float)fontAtlas->myMetadata.myWidth;
	const float texelHeight = 1.0f / (float)fontAtlas->myMetadata.myHeight;

	std::vector<std::vector<KE::Sprite*>> spritesPerRow;
	spritesPerRow.push_back({});
	unsigned int currentRow = 0;

	for (int i = 0; i < aString.size(); i++)
	{
		char currentChar = aString[i];

		if (currentChar == '\n')
		{
			x = 0.0f;
			y -= fsScale * fontData.metrics.lineHeight;
			currentRow++;
			spritesPerRow.push_back({});
		}

		auto& charData = fontData.characters[currentChar];
		KE::Sprite& sprite = *aSprites[i];

		sprite.myAttributes.myTransform = aOrigin;
		Transform& sprTrans = sprite.myAttributes.myTransform;

		const float pLeft = charData.planeBounds.left	  * fsScale + x;
		const float pRight = charData.planeBounds.right	  * fsScale + x;
		const float pTop = charData.planeBounds.top		  * fsScale + y;
		const float pBottom = charData.planeBounds.bottom * fsScale + y;

		const float iLeft = charData.atlasBounds.left	  * texelWidth;
		const float iRight = charData.atlasBounds.right	  * texelWidth;
		const float iTop = charData.atlasBounds.top		  * texelHeight;
		const float iBottom = charData.atlasBounds.bottom * texelHeight;

		sprite.myAttributes.myUVs.u0 = iLeft;
		sprite.myAttributes.myUVs.v0 = 1.0f - iTop;
		sprite.myAttributes.myUVs.u1 = iRight;
		sprite.myAttributes.myUVs.v1 = 1.0f - iBottom;

		Vector3f cursorPos = { x, 0.0f, 0.0f };
		x += charData.xAdvance * fsScale;
		x += GetKern(aString, i) * fsScale;

		Vector4f p0 = originMat * Vector4f(pLeft, pTop, 0.0f, 1.0f);
		Vector4f p1 = originMat * Vector4f(pRight, pTop, 0.0f, 1.0f);
		Vector4f p2 = originMat * Vector4f(pRight, pBottom, 0.0f, 1.0f);
		Vector4f p3 = originMat * Vector4f(pLeft, pBottom, 0.0f, 1.0f);

		Vector4f pointCenter = (p0 + p1 + p2 + p3) * 0.25f;
		sprTrans.SetPosition(pointCenter.xyz());

		float xScale = (p1 - p0).Length() / 2.0f;
		float yScale = (p3 - p0).Length() / 2.0f;
		float zScale = 1.0f;

		sprTrans.SetScale({xScale, yScale, zScale});

		spritesPerRow[currentRow].push_back(&sprite);
	}

	//align the rows
	AlignText(spritesPerRow, aStyling.text.horizontalAlign, aStyling.text.verticalAlign);

	for (int i = 0; i < spritesPerRow.size(); i++)
	{
		for (int j = 0; j < spritesPerRow[i].size(); j++)
		{
			auto* spr = spritesPerRow[i][j];
			spr->myAttributes.myTransform *= aOrigin;
		}
	}
}
	
std::string KE::SpriteFont::CreateSpriteString(
	const int& aNumber, 
	const std::string& aPrefix,
	const std::string& aSuffix
)
{
	return aPrefix + std::to_string(aNumber) + aSuffix;
}


void KE::FontLoader::SaveGeneratedFont(const char* aFontPath, const MSDFPixel* aImageData, const int aWidth, const int aHeight, FontData& aFontData)
{
	BinaryWriter writer(aFontPath);

	writer.Write(aWidth);
	writer.Write(aHeight);

	size_t pixelDataSize = aWidth * aHeight * sizeof(MSDFPixel);
	writer.Write(pixelDataSize, (void*)aImageData);

	writer.Write(aFontData.metrics);
	const int charCount = static_cast<int>(aFontData.characters.size());
	writer.Write(charCount);
	for (auto& character : aFontData.characters)
	{
		writer.Write(character.second);
	}

	const int kernPairCount = static_cast<int>(aFontData.kernPairs.size());
	writer.Write(kernPairCount);
	for (auto& kernPair : aFontData.kernPairs)
	{
		writer.Write(kernPair.first);
		writer.Write(kernPair.second);
	}

}

KE::SpriteFont KE::FontLoader::LoadFont(const char* aFontPath)
{
	if (!std::filesystem::exists(aFontPath))
	{

		std::string ttfPath = aFontPath;
		ttfPath.replace(ttfPath.find_last_of('.'), ttfPath.size() - ttfPath.find_last_of('.'), ".ttf");

		GenerateMSDFFont(ttfPath.c_str(), 64.0f, 3.0f, 2.0f, 1.0f);
	}

	std::vector<MSDFPixel> pixels;

	int width, height;
	BinaryReader reader(aFontPath);
	reader.Read(width);
	reader.Read(height);

	pixels.resize(width * height);
	size_t pixelDataSize = width * height * sizeof(MSDFPixel);
	reader.Read(pixelDataSize, &pixels[0]);

	auto* graphics = KE_GLOBAL::blackboard.Get<Graphics>("graphics");
	auto& texLoader = graphics->GetTextureLoader();
	auto* tex = texLoader.CreateTextureFromData(std::format("{}_MSDF", aFontPath), (unsigned char*)&pixels[0], width, height);

	SpriteFont out;
	out.fontAtlas = tex;

	reader.Read(out.fontData.metrics);

	int characterCount;
	reader.Read(characterCount);

	for (int i = 0; i < characterCount; i++)
	{
		CharacterData charData;
		reader.Read(charData);
		out.fontData.characters[charData.myCharacter] = charData;
	}

	int kernPairCount;
	reader.Read(kernPairCount);

	for (int i = 0; i < kernPairCount; i++)
	{
		std::pair<char, char> kernCharPair;
		reader.Read(kernCharPair);

		float kernValue;
		reader.Read(kernValue);

		out.fontData.kernPairs[kernCharPair] = kernValue;
	}

	return out;
}

void KE::FontLoader::GenerateMSDFFont(const char* aFontPath, float aFontSize, float aMaxCornerAngle, float aPixelRange, float aMiterLimit)
{
	Timer fontTimer;

	msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
	if (!ft) { return; }
	msdfgen::FontHandle* font = msdfgen::loadFont(ft, aFontPath);
	if (!font) { return; }

	FT_Library library = (FT_Library)ft;

	struct FontHandleCheat
	{
		FT_Face face;
		bool ownership;
	};

	FT_Face face = ((FontHandleCheat*)font)->face;
	

	// Storage for glyph geometry and their coordinates in the atlas
	std::vector<GlyphGeometry> glyphs;
	// FontGeometry is a helper class that loads a set of glyphs from a single font.
	// It can also be used to get additional font metrics, kerning information, etc.
	FontGeometry fontGeometry(&glyphs);

	// Load a set of character glyphs:
	// The second argument can be ignored unless you mix different font sizes in one atlas.
	// In the last argument, you can specify a charset other than ASCII.
	// To load specific glyph indices, use loadGlyphs instead.
	fontGeometry.loadCharset(font, 1.0, Charset::ASCII);

	// Apply MSDF edge coloring. See edge-coloring.h for other coloring strategies.
	const double maxCornerAngle = static_cast<double>(aMaxCornerAngle);
	for (GlyphGeometry& glyph : glyphs)
	{
		glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, maxCornerAngle, 0);
	}

	// TightAtlasPacker class computes the layout of the atlas.
	TightAtlasPacker packer;
	// Set atlas parameters:
	// setDimensions or setDimensionsConstraint to find the best value
	packer.setDimensionsConstraint(DimensionsConstraint::POWER_OF_TWO_RECTANGLE);
	// setScale for a fixed size or setMinimumScale to use the largest that fits
	packer.setMinimumScale(static_cast<double>(aFontSize));
	// setPixelRange or setUnitRange
	packer.setPixelRange(static_cast<double>(aPixelRange));
	packer.setMiterLimit(static_cast<double>(aMiterLimit));
	// Compute atlas layout - pack glyphs
	packer.pack(glyphs.data(), (int)glyphs.size());
	// Get final atlas dimensions
	int width = 0, height = 0;
	packer.getDimensions(width, height);
	// The ImmediateAtlasGenerator class facilitates the generation of the atlas bitmap.

	constexpr auto func = &mtsdfGenerator;

	ImmediateAtlasGenerator<
		float, // pixel type of buffer for individual glyphs depends on generator function
		4, // number of atlas color channels
		func, // function to generate bitmaps for individual glyphs
		BitmapAtlasStorage<byte, 4> // class that stores the atlas bitmap
		// For example, a custom atlas storage class that stores it in VRAM can be used.
	> generator(width, height);

	// GeneratorAttributes can be modified to change the generator's default settings.
	GeneratorAttributes attributes;
	generator.setAttributes(attributes);
	generator.setThreadCount(4);
	// Generate atlas bitmap
	generator.generate(glyphs.data(), (int)glyphs.size());

	auto& storage = generator.atlasStorage();
	auto* nonConstStorage = const_cast<BitmapAtlasStorage<byte, 4>*>(&storage); //evil, but for now it's fine.. i think
	auto& bmp = nonConstStorage->getBitmap();

	std::vector<MSDFPixel> pixels(width* height);
	
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			unsigned int index = i + (height-1-j) * width;
			pixels[index].r = bmp(i, j)[0];
			pixels[index].g = bmp(i, j)[1];
			pixels[index].b = bmp(i, j)[2];
			pixels[index].a = bmp(i, j)[3];
		}
	}


	FontData fontInfo;

	for (const auto& kern: fontGeometry.getKerning())
	{
		std::pair kernPair = { static_cast<char>(kern.first.first), static_cast<char>(kern.first.second) };
		fontInfo.kernPairs[kernPair] = static_cast<float>(kern.second);
	}

	const auto& metrics = fontGeometry.getMetrics();
	fontInfo.metrics.ascendY			= static_cast<float>(metrics.ascenderY);
	fontInfo.metrics.descendY			= static_cast<float>(metrics.descenderY);
	fontInfo.metrics.emSize				= static_cast<float>(metrics.emSize);
	fontInfo.metrics.lineHeight			= static_cast<float>(metrics.lineHeight);
	fontInfo.metrics.underlineThickness = static_cast<float>(metrics.underlineThickness);
	fontInfo.metrics.underlineY			= static_cast<float>(metrics.underlineY);


	for (auto& glyph : glyphs)
	{
		unicode_t codepoint = glyph.getCodepoint();

		char asciiCode = static_cast<char>(codepoint);

		fontInfo.characters[asciiCode] = {};
		auto& charData = fontInfo.characters[asciiCode];

		charData.myCharacter = asciiCode;


		double l, r, t, b;
		{
			glyph.getQuadAtlasBounds(l, b, r, t);

			charData.atlasBounds.left	= static_cast<float>(l);
			charData.atlasBounds.right	= static_cast<float>(r);
			charData.atlasBounds.top	= static_cast<float>(t);
			charData.atlasBounds.bottom = static_cast<float>(b);
		}
		{
			glyph.getQuadPlaneBounds(l, b, r, t);

			charData.planeBounds.left	= static_cast<float>(l);
			charData.planeBounds.right	= static_cast<float>(r);
			charData.planeBounds.top	= static_cast<float>(t);
			charData.planeBounds.bottom = static_cast<float>(b);
		}

		charData.gScale = static_cast<float>(glyph.getGeometryScale());
		charData.xAdvance = static_cast<float>(glyph.getAdvance());
	}

	std::string outPath = aFontPath;
	auto pos = outPath.find_last_of('.');
	//change the extension to .ktf (kitty font)
	outPath.replace(pos, outPath.size() - pos, ".ktf");


	SaveGeneratedFont(outPath.c_str(), pixels.data(), width, height, fontInfo);

	msdfgen::destroyFont(font);
	msdfgen::deinitializeFreetype(ft);

	std::cout << "font took : " << fontTimer.UpdateDeltaTime() << std::endl;
}

