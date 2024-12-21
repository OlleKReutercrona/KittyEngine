#pragma once

namespace KE
{
	enum class TextAlign
	{
		LeftOrTop,
		Center,
		RightOrBottom,

		Count
	};

	inline const char* EnumToString(const TextAlign& aAlign)
	{
		switch (aAlign)
		{
		case TextAlign::LeftOrTop: return "LeftOrTop";
		case TextAlign::Center: return "Center";
		case TextAlign::RightOrBottom: return "RightOrBottom";
		}
		return "Unknown";
	}

	enum class StrokeAlign
	{
		Inside,
		Outside,
		Center,

		Count
	};

	inline const char* EnumToString(const StrokeAlign& aAlign)
	{
		switch (aAlign)
		{
		case StrokeAlign::Inside: return "Inside";
		case StrokeAlign::Outside: return "Outside";
		case StrokeAlign::Center: return "Center";
		}
		return "Unknown";
	}

	struct TextStyling
	{
		//main
		struct
		{
			Vector4f colour = { 1.0f, 1.0f, 1.0f, 1.0f };
			TextAlign horizontalAlign = TextAlign::LeftOrTop;
			TextAlign verticalAlign = TextAlign::LeftOrTop;
		} text;

		//stroke
		struct
		{
			Vector4f colour;
			float width = -1.0f;
			float softness;
			StrokeAlign align;
		} stroke;

		//drop shadow
		struct
		{
			Vector4f colour;
			float distance = -1.0f;
			float softness;
			float angle;
		} shadow;

		//glow
		struct
		{
			Vector4f colour;
			float innerWidth = -1.0f;
			float outerWidth = -1.0f;
			float softness;
		} glow;
	};

	struct TextStylingBufferData
	{
		Vector4f textColour;
		Vector4f strokeColour;
		float strokeSize;
		float strokeSoftness;

		float padding[2];

	};
}