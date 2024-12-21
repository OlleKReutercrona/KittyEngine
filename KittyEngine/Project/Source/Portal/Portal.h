#pragma once
#include "Engine/Source/ComponentSystem/Components/Component.h"
#include "Engine/Source/Math/Matrix3x3.h"
#include "Engine/Source/Utility/EventSystem.h"

namespace P8
{
	class Portal;
	class BallManager;

	struct PortalData
	{
		int linkedPortalID = -1;
	};


	struct PortedObject
	{
		enum eType
		{
			Undefined,
			Player,
			Ball
		};

		eType type = Undefined;
		KE::GameObject* object;
		float time;
	};

	class Portal : public KE::Component, public ES::IObserver
	{
	public:
		Portal(KE::GameObject& aGameObject);
		~Portal();

		void Awake() override;
		void SetData(void* aData = nullptr) override;
		void Update() override;

		void Teleport(KE::GameObject& aGo);
		bool IsInPortal(const Vector3f& aPortalPos, const Vector3f& aPortalDir, KE::GameObject& aGo, const Vector3f& aVelocity);
		void CollectPortableObjects();

		static Vector3f GetTransformedDirection(const Portal* aFromPortal, const Portal* aToPortal, const Vector3f& aDirection);
		static Vector3f GetTransformedPosition(const Portal* aFromPortal, const Portal* aToPortal, const Vector3f& aPosition);


	protected:
		void DrawDebug(KE::DebugRenderer& aDbg) override;

		// Inherited via IObserver
		void OnReceiveEvent(ES::Event& aEvent) override;
		void OnInit() override;
		void OnDestroy() override;

	private:
		BallManager* myBallManager = nullptr;
		Portal* myLinkedPortal = nullptr;

		int myConnectedPortalID = -1;
		float myPortalOffset = 2.0f;
		std::unordered_map<int, PortedObject> myPortedObjects;
	};


}
