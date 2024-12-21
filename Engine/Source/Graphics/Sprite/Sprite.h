#pragma once
#include "Engine/Source/Math/Transform.h"
#include "Engine/Source/Math/KittyMath.h"
#include "Engine/Source/Graphics/GraphicsConstants.h"
#include "Engine/Source/Graphics/Text/TextStyling.h"

struct ID3D11ShaderResourceView;

namespace KE
{
	class PixelShader;
	class VertexShader;
	struct Texture;
	class SpriteManager;

#pragma region EffectType
	enum class eEffectType // Used together with min/max UVs in SpriteRenderBuffer to create effects
	{
		None,
		Bars,
		Circle,
		Cooldown,
		JamesBond,
		SpinningSquare,
		Resource,
	};

	inline const char* EnumToString(const eEffectType aType)
	{
		switch (aType)
		{
		case eEffectType::None: return "None";
		case eEffectType::Bars: return "Bars";
		case eEffectType::Circle: return "Circle";
		case eEffectType::Cooldown: return "Cooldown";
		case eEffectType::JamesBond: return "JamesBond";
		case eEffectType::SpinningSquare: return "SpinningSquare";
		case eEffectType::Resource: return "Resource";
		default: return "Unknown";
		}
	}
#pragma endregion

#pragma region ProgressionDirection
	enum class eProgressionDirection
	{
		None,
		LeftToRight,
		RightToLeft,
		TopToBottom,
		BottomToTop,
		DiagonalLeftUp,
		DiagonalLeftDown,
		DiagonalRightUp,
		DiagonalRightDown,
		Circle,
		Cooldown,
		JamesBond,
		SpinningSquare,
		Resource,
		Count
	};

	inline const char* EnumToString(const eProgressionDirection aDirection)
	{
		switch (aDirection)
		{
		case eProgressionDirection::None: return "None";
		case eProgressionDirection::LeftToRight: return "LeftToRight";
		case eProgressionDirection::RightToLeft: return "RightToLeft";
		case eProgressionDirection::TopToBottom: return "TopToBottom";
		case eProgressionDirection::BottomToTop: return "BottomToTop";
		case eProgressionDirection::DiagonalLeftUp: return "DiagonalLeftUp";
		case eProgressionDirection::DiagonalLeftDown: return "DiagonalLeftDown";
		case eProgressionDirection::DiagonalRightUp: return "DiagonalRightUp";
		case eProgressionDirection::DiagonalRightDown: return "DiagonalRightDown";
		case eProgressionDirection::Circle: return "Circle";
		case eProgressionDirection::Cooldown: return "Cooldown";
		case eProgressionDirection::JamesBond: return "JamesBond";
		case eProgressionDirection::SpinningSquare: return "SpinningSquare";
		case eProgressionDirection::Resource: return "Resource";
		default: return "Unknown";
		}
	}
	 
	inline eEffectType ProgressionDirectionToEffectType(const eProgressionDirection aDirection)
	{
		switch (aDirection)
		{
			case eProgressionDirection::None:
			case eProgressionDirection::LeftToRight:
			case eProgressionDirection::RightToLeft:
			case eProgressionDirection::TopToBottom:
			case eProgressionDirection::BottomToTop:
			case eProgressionDirection::DiagonalLeftUp:
			case eProgressionDirection::DiagonalLeftDown:
			case eProgressionDirection::DiagonalRightUp:
			case eProgressionDirection::DiagonalRightDown:
				return eEffectType::Bars;
			case eProgressionDirection::Circle:
				return eEffectType::Circle;
			case eProgressionDirection::Cooldown:
				return eEffectType::Cooldown;
			case eProgressionDirection::JamesBond:
				return eEffectType::JamesBond;
			case eProgressionDirection::SpinningSquare:
				return eEffectType::SpinningSquare;
			case eProgressionDirection::Resource:
				return eEffectType::Resource;
			default:
				return eEffectType::None;
		}
	}
#pragma endregion

	struct SpriteRenderAttributes
	{
		Transform myTransform;

		struct
		{
			float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
		} myColor;

		struct
		{
			float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
		} myUVs;

		struct
		{
			float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
		} myModelUvRegion; //used to specify a region of the model to texture

	};

	enum class SpriteBatchMode : int
	{
		Default,
		Billboard,
		Screen,
		ScreenText
	};

	struct SpriteData // Data that is shared between all sprites of the same type
	{
		Texture* myTexture = nullptr;

		struct
		{
			float uvMin[2] = {0.0f, 0.0f};
			float uvMax[2] = {1.0f, 1.0f};
		} myUVs;

		SpriteBatchMode myMode   = SpriteBatchMode::Default;
		eEffectType myEffectType = eEffectType::None;
		bool flipX = false;
		bool flipY = false;
	};

	struct Sprite
	{
		SpriteRenderAttributes myAttributes;
		void SetFillFactor(const float aFactor, eProgressionDirection aDirection);

		void RotateAroundCenter(const float aAngle)
		{
			float spriteWidth = myAttributes.myTransform.GetScale().x;
			float spriteHeight = myAttributes.myTransform.GetScale().y;

			//center of the sprite is position + scale / 2

			myAttributes.myTransform.SetPosition(myAttributes.myTransform.GetPosition() + Vector3f(spriteWidth / 2.0f, spriteHeight / 2.0f, 0.0f));
			//rotate around z
			Vector3f rotation = { 0.0f, 0.0f, aAngle };
			myAttributes.myTransform.RotateLocal(rotation);

			myAttributes.myTransform.SetPosition(myAttributes.myTransform.GetPosition() - Vector3f(spriteWidth / 2.0f, spriteHeight / 2.0f, 0.0f));
		}
	};

	struct SpriteBatch
	{
		eRenderLayers myRenderLayer = eRenderLayers::UI;
		SpriteData myData;
		std::vector<Sprite> myInstances;

		TextStyling myTextStyling = {};

		VertexShader* myCustomVS = nullptr;
		PixelShader* myCustomPS = nullptr;

		SpriteBatch() = default;
		SpriteBatch(SpriteData aData) : myData(aData), myInstances(0) {}
	};

	inline void Sprite::SetFillFactor(const float aFactor, eProgressionDirection aDirection = eProgressionDirection::LeftToRight)
	{
		const float factor = std::clamp(aFactor, 0.0f, 1.0f);

		switch (aDirection)
		{
			case eProgressionDirection::None:
			{
				myAttributes.myUVs =
				{
					0.0f, 0.0f,
					1.0f, 1.0f
				};
				break;
			}
			case eProgressionDirection::LeftToRight:
			{
				myAttributes.myUVs =
				{
					0.0f, 0.0f,
					factor, 1.0f
				};
				break;
			}
			case eProgressionDirection::RightToLeft:
			{
				myAttributes.myUVs =
				{
					1.0f - factor, 0.0f,
					1.0f, 1.0f
				};
				break;
			}
			case eProgressionDirection::TopToBottom:
			{
				myAttributes.myUVs =
				{
					0.0f, 0.0f,
					1.0f, factor
				};
				break;
			}
			case eProgressionDirection::BottomToTop:
			{
				myAttributes.myUVs =
				{
					0.0f, 1.0f - factor,
					1.0f, 1.0f
				};
				break;
			}
			case eProgressionDirection::DiagonalLeftUp:
			{
				myAttributes.myUVs =
				{
					0.0f, 1.0f - factor,
					factor, 1.0f
				};
				break;
			}
			case eProgressionDirection::DiagonalLeftDown:
			{
				myAttributes.myUVs =
				{
					0.0f, 0.0f,
					factor, factor
				};
				break;
			}
			case eProgressionDirection::DiagonalRightUp:
			{
				myAttributes.myUVs =
				{
					1.0f - factor, 1.0f - factor,
					1.0f, 1.0f
				};
				break;
			}
			case eProgressionDirection::DiagonalRightDown:
			{
				myAttributes.myUVs =
				{
					1.0f - factor, 0.0f,
					1.0f, factor
				};
				break;
			}
			case eProgressionDirection::Circle:
			{
				myAttributes.myUVs =
				{
					factor, factor,
					factor, factor
				};
				break;
			}
			case eProgressionDirection::Cooldown:
			{
				myAttributes.myUVs =
				{
					1.0f - factor, factor,
					factor, factor
				};
				break;
			}
			case eProgressionDirection::JamesBond:
			{
				float angle        = factor * 2.0f * PI;
				float x            = 0.5f + 0.5f * sin(angle);
				float y            = 0.5f - 0.5f * cos(angle);
				myAttributes.myUVs =
				{
					x, y,
					0.5f, 0.5f
				};
				break;
			}
			case eProgressionDirection::SpinningSquare:
			{
				float angle        = factor * 2.0f * PI;
				float x            = 0.5f + 0.5f * sin(angle);
				float y            = 0.5f - 0.5f * cos(angle);
				myAttributes.myUVs =
				{
					x, y,
					0.5f, 0.5f
				};
				break;
			}
			case eProgressionDirection::Resource:
			{
				myAttributes.myUVs =
				{
					0.0f, 1.0f - factor,
					1.0f, 1.0f
				};
				break;
			}
			default:
				;
		}
	}
}
