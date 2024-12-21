#pragma once
#include <string>

namespace KE
{
	namespace Collision
	{
		enum class Layers
		{
			Default = 1 << 0, //1
			Player = 1 << 1, //2
			Enemy = 1 << 2, //4
			Ground = 1 << 3, //8
			Projectile = 1 << 4, //16
			Wall = 1 << 5, //32
			Trigger = 1 << 6, //64
			MovingObject = 1 << 7, //128
			PlayerAttack = 1 << 8, //256

			Count = 8,


			All = INT_MAX
		};

		//static void SetLayer(int& aLayerMask, const Layers aLayerToSet)
		//{
		//	aLayerMask << static_cast<int>(aLayerToSet);
		//}

		static bool IsOnLayer(const Layers aLayer, const int aLayerMask)
		{
			return static_cast<int>(aLayer) & aLayerMask;
		}

		static bool IsOnLayer(const Layers aFirst, const Layers aSecond)
		{
			return static_cast<int>(aFirst) & static_cast<int>(aSecond);
		}

		static std::string GetLayerAsString(const Layers aLayer)
		{
			switch (aLayer)
			{
			case KE::Collision::Layers::Default:
			{
				return "Default";
			}
			case KE::Collision::Layers::Player:
			{
				return "Player";
			}
			case KE::Collision::Layers::Enemy:
			{
				return "Enemy";
			}
			case KE::Collision::Layers::Ground:
			{
				return "Ground";
			}
			case KE::Collision::Layers::Projectile:
			{
				return "Projectile";
			}
			case KE::Collision::Layers::Wall:
			{
				return "Wall";
			}
			default:
				std::string str("LAYER STRING IS NOT IMPLEMENTED %i", (int)aLayer);
				return str;
			}
		}
	}
}