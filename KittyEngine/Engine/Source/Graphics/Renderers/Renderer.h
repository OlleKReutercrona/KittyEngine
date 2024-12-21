#pragma once

namespace KE
{
	class Graphics;

	class Renderer
	{
	protected:
		Graphics* myGraphics;
	public:
		void Init(Graphics* aGraphics) { myGraphics = aGraphics; };
	};
}