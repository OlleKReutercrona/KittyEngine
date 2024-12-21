#include "stdafx.h"
//#include "Minimap.h"
//
//#include "GUIElement.h"
//#include "GUIScene.h"
//#include "External/Include/imgui/imgui.h"
//#include "Project/Source/GameEvents/GameEvents.h"
//
//namespace KE
//{
//	Minimap::~Minimap()
//	{
//		Minimap::OnDestroy();
//	}
//	void Minimap::Init(GUIScene* aScene)
//	{
//		myScene = aScene;
//
//		for (auto& element : myScene->GetGUIElements())
//		{
//			if (element.myName == "Map")
//			{
//				myMap = &element;
//			}
//			if (element.myName == "Player")
//			{
//				myPlayer = &element;
//			}
//			if (element.myName == "Enemy")
//			{
//				myEnemy = &element;
//				myEnemy->mySpriteBatch.myInstances.clear();
//			}
//			if (element.myName == "Spawnpoint")
//			{
//				mySpawnpoint = &element;
//				mySpawnpoint->mySpriteBatch.myInstances.clear();
//			}
//		}
//
//		myMap->mySpriteBatch.myData.flipX = true;
//		myMap->mySpriteBatch.myData.flipY = true;
//
//		OnInit();
//	}
//	void Minimap::Update()
//	{
//		if (myMap == nullptr)
//		{
//			return;
//		}
//
//		//ImGui::Begin("Minimap");
//		//ImGui::DragFloat("X Factor", &xFactor, 0.01f, 0.0f, 1.0f);
//		//ImGui::DragFloat("Y Factor", &yFactor, 0.01f, 0.0f, 1.0f);
//		//ImGui::DragFloat("X Scale", &xScale, 1.0f, 0.0f, 1000.0f);
//		//ImGui::DragFloat("Y Scale", &yScale, 1.0f, 0.0f, 1000.0f);
//		//ImGui::End();
//
//		mapZeroPos = { myMap->myBox.myWidth * (1.0f - xFactor), -myMap->myBox.myHeight * (1.0f - yFactor) };
//		scale = { myMap->myBox.myWidth / xScale, myMap->myBox.myHeight / yScale };
//		mapOffset = mapZeroPos + myMap->myBox.myScreenPosition;
//
//		const Vector3f scaledPlayerPosition =
//		{
//			1.0f - myPlayerPosition.x * -scale.x + mapOffset.x,
//			1.0f - myPlayerPosition.z * scale.y + mapOffset.y,
//			0.0f,
//		};
//
//		myPlayer->mySpriteBatch.myInstances[0].myAttributes.myTransform.SetPosition(scaledPlayerPosition);
//	}
//	void Minimap::OnReceiveEvent(ES::Event& aEvent)
//	{
//		if (P7::PlayerMovementEvent* event = dynamic_cast<P7::PlayerMovementEvent*>(&aEvent))
//		{
//			myPlayerPosition = event->myPosition;
//			myPlayerDirection = event->myDirection;
//		}
//
//		if (P7::EnemiesMovementEvent* event = dynamic_cast<P7::EnemiesMovementEvent*>(&aEvent))
//		{
//			myEnemyPositions = event->myPositions;
//
//			if (myEnemyPositions.empty())
//			{
//				myEnemy->mySpriteBatch.myInstances.clear();
//				return;
//			}
//
//			myEnemy->mySpriteBatch.myInstances.clear();
//			myEnemy->mySpriteBatch.myInstances.resize(myEnemyPositions.size());
//
//			for (size_t i = 0; i < myEnemyPositions.size(); ++i)
//			{
//				Vector3f scaledEnemyPosition =
//				{
//					1.0f - myEnemyPositions[i].x * -scale.x + mapOffset.x,
//					1.0f - myEnemyPositions[i].z * scale.y + mapOffset.y,
//					0.0f,
//				};
//
//				if (i < myEnemy->mySpriteBatch.myInstances.size())
//				{
//					myEnemy->mySpriteBatch.myInstances[i].myAttributes.myTransform.SetPosition(scaledEnemyPosition);
//					myEnemy->mySpriteBatch.myInstances[i].myAttributes.myTransform.SetScale({ myEnemy->myBox.myWidth, myEnemy->myBox.myHeight, 1.0f });
//				}
//			}
//		}
//
//		if (P7::SpawnPointEvent* event = dynamic_cast<P7::SpawnPointEvent*>(&aEvent))
//		{
//			mySpawnpointPositions = event->myPositions;
//
//			mySpawnpoint->mySpriteBatch.myInstances.clear();
//			mySpawnpoint->mySpriteBatch.myInstances.resize(mySpawnpointPositions.size());
//
//			for (size_t i = 0; i < mySpawnpointPositions.size(); ++i)
//			{
//				Vector3f scaledSpawnpointPosition =
//				{
//					1.0f - mySpawnpointPositions[i].x * -scale.x + mapOffset.x,
//					1.0f - mySpawnpointPositions[i].z * scale.y + mapOffset.y,
//					0.0f,
//				};
//				mySpawnpoint->mySpriteBatch.myInstances[i].myAttributes.myTransform.SetPosition(scaledSpawnpointPosition);
//				mySpawnpoint->mySpriteBatch.myInstances[i].myAttributes.myTransform.SetScale({ mySpawnpoint->myBox.myWidth, mySpawnpoint->myBox.myHeight, 1.0f });
//			}
//
//		}
//	}
//	void Minimap::OnInit()
//	{
//		ES::EventSystem::GetInstance().Attach<P7::PlayerMovementEvent>(this);
//		ES::EventSystem::GetInstance().Attach<P7::EnemiesMovementEvent>(this);
//		ES::EventSystem::GetInstance().Attach<P7::SpawnPointEvent>(this);
//	}
//
//	void Minimap::OnDestroy()
//	{
//		ES::EventSystem::GetInstance().Detach<P7::PlayerMovementEvent>(this);
//		ES::EventSystem::GetInstance().Detach<P7::EnemiesMovementEvent>(this);
//		ES::EventSystem::GetInstance().Detach<P7::SpawnPointEvent>(this);
//	}
//
//	void Minimap::ResetMinimap()
//	{
//		myEnemyPositions.clear();
//		myEnemy->mySpriteBatch.myInstances.clear();
//	}
//}
