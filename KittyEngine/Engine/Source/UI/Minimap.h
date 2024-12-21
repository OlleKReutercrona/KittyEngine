//#pragma once
//#include "Engine/Source/Utility/EventSystem.h"
//
//namespace KE
//{
//	class GUIScene;
//	struct GUIElement;
//
//	class Minimap : public ES::IObserver
//	{
//		friend class GUIHandler;
//	public:
//		Minimap() = default;
//		~Minimap() override;
//
//		void Init(GUIScene* aScene);
//		void Update();
//
//		// IObserver
//		void OnReceiveEvent(ES::Event& aEvent) override;
//		void OnInit() override;
//		void OnDestroy() override;
//
//	private:
//
//		void ResetMinimap();
//
//		GUIScene* myScene = nullptr;
//		GUIElement* myMap = nullptr;
//		GUIElement* myPlayer = nullptr;
//		GUIElement* myEnemy = nullptr;
//		GUIElement* mySpawnpoint = nullptr;
//		Vector3f myPlayerPosition = Vector3f(0.0f, 0.0f, 0.0f);
//		Vector3f myPlayerDirection = Vector3f(0.0f, 0.0f, 0.0f);
//		std::vector<Vector3f> myEnemyPositions = {};
//		std::vector<Vector3f> mySpawnpointPositions = {};
//
//		float xFactor = 0.65f;
//		float yFactor = 0.06f;
//		float xScale = 146.0f;
//		float yScale = 80.0f;
//
//		Vector2f mapZeroPos = {};
//		Vector2f scale = {};
//		Vector2f mapOffset = {};
//	};
//}
//
