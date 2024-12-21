#include "stdafx.h"
#include "CollisionHandler.h"

#include "Collider.h"
#include "Collision/Intersection.h"
#include "ComponentSystem/Components/Component.h"
#include "ComponentSystem/GameObject.h"
#include "Engine/Source/ComponentSystem/GameObject.h"
#include <Engine/Source/AI/Pathfinding/Navmesh.h>
#include <Engine/Source/Graphics/DebugRenderer.h>
#include <Math/Matrix3x3.h>

#include "Graphics/ModelData.h"
#include "omnipvd/PxOmniPvd.h"

#define USE_PHYSX



KE::CollisionHandler::CollisionHandler() : 
	myDispatcher(nullptr),
	myMaterial(nullptr),
	myPhysics(nullptr),
	myScene(nullptr),
	myFoundation(nullptr),
	myControllerManager(nullptr),
	myPvd(nullptr)
{
	myPhysicsObjects.reserve(2048);
}

KE::CollisionHandler::~CollisionHandler()
{
	Unload();
}

void KE::CollisionHandler::Unload()
{
	//// Clear Kitty Colliders
	//for (int i = 0; i < myColliders.size(); i++)
	//{
	//	delete myColliders[i];
	//	myColliders[i] = nullptr;
	//}

	//myColliders.clear();

#ifdef USE_PHYSX

	// Clear physics objects 
	for (int i = 0; i < myPhysicsObjects.size(); i++)
	{
		myPhysicsObjects[i].myActor->release();
	}
	myPhysicsObjects.clear();
	myPhysicsObjects.reserve(2048);


	//for (auto& physObj : myPhysObjects)
	//{
	//	physObj.second->myActor->release();
	//}
	myPhysObjects.clear();

	// Releases the physx stuff
	if (myFoundation != NULL)
	{
		myControllerManager->release();
		myPvd->release();
		myScene->release();
		myDispatcher->release();
		myMaterial->release();
		myPhysics->release();
		myFoundation->release();


		myPvd = NULL;
		myControllerManager = NULL;
		myScene = NULL;
		myDispatcher = NULL;
		myMaterial = NULL;
		myPhysics = NULL;
		myFoundation = NULL;
	}

#endif
	myDbg = nullptr;
}

void KE::CollisionHandler::Init()
{
	myDbg = KE_GLOBAL::blackboard.Get<KE::DebugRenderer>("debugRenderer");

#ifdef USE_PHYSX
	InitPhysX();
#endif // USE_PHYSX
}

void KE::CollisionHandler::InitPhysX()
{
	using namespace physx;

	myFoundation = PxCreateFoundation(PX_PHYSICS_FOUNDATION_VERSION, myDefaultAllocatorCallback, myDefaultErrorCallback);
	if (!myFoundation) throw("PxCreateFoundation failed!");

	myPvd = PxCreatePvd(*myFoundation);
	if (!myPvd)
	{
		KE_LOG_CHANNEL("PhysX", "Physx PVD failed to be created");
	}

	if(!myPvd->connect(*PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10), PxPvdInstrumentationFlag::eALL))
	{
		KE_LOG_CHANNEL("PhysX", "Physx PVD failed to connect");
	}

	myToleranceScale.length = 1;
	myToleranceScale.speed = 981;
	myPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *myFoundation, myToleranceScale, true, myPvd, 0);
	myDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
	myMaterial = myPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	// Initialize scene
	physx::PxSceneDesc sceneDesc(myPhysics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = myDispatcher;
	sceneDesc.filterShader = CustomFilterShader;
	sceneDesc.kineKineFilteringMode = physx::PxPairFilteringMode::eKEEP;
	sceneDesc.staticKineFilteringMode = physx::PxPairFilteringMode::eKEEP;                 
	sceneDesc.simulationEventCallback = this; 

	myScene = myPhysics->createScene(sceneDesc);
	myControllerManager = PxCreateControllerManager(*myScene);
	myControllerManager->setOverlapRecoveryModule(true);
	myControllerManager->setTessellation(true, 1.0f);
	//myControllerManager->setPreciseSweeps(false);
	//if (!PxInitExtensions(*myPhysics, myPvd)) {
	//
	//}

	// PhysX Visual Debugging = PVD
	PxPvdSceneClient* pvdClient = myScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		pvdClient->updateCamera("test", PxVec3(0, 10, 10), PxVec3(0, 1, 0), PxVec3(0, 0, 0));
	}
	else
	{
		KE_LOG_CHANNEL("PhysX", "PVD failed to be created");
	}

	myScene->setSimulationEventCallback(this);

	
}

void KE::CollisionHandler::UpdateGameObjects()
{
	for (auto& physobj : myPhysObjects)
	{
		if (!physobj.second->myKECollider.isStatic)
		{
			if (myPlayersID == physobj.first) continue;
			if (!physobj.second->myKECollider.myComponent) { continue; }
			if (physobj.second->disableColliderMovementPrecedence) { continue; }


			physx::PxTransform trans = physobj.second->myActor->getGlobalPose();

			// Remove offset from transform since the physx transform is already moved by the offset
			trans.p.x -= physobj.second->myKECollider.myOffset.x;
			trans.p.y -= physobj.second->myKECollider.myOffset.y;
			trans.p.z -= physobj.second->myKECollider.myOffset.z;
			
			Vector3f newpos({ trans.p.x, trans.p.y,trans.p.z });

			physobj.second->myKECollider.myPosition = newpos;

			//physobj.second->myKECollider.myComponent->GetGameObject().myTransform = trans;
			if (!physobj.second->myKECollider.myComponent)
			{
				std::cout << "FIX THIS ISSUE\n";
			}

			physobj.second->myKECollider.myComponent->GetGameObject().myTransform.SetPosition(newpos);
		}
	}
}

void KE::CollisionHandler::Update()
{
#ifndef USE_PHYSX
	CheckCollision();
#else

	static float timeStep = 0.0f;

	timeStep += KE_GLOBAL::deltaTime;
	if (timeStep > 1.0f / 90.0f)
	{
		myScene->simulate(timeStep);

		myScene->fetchResults(true);

		UpdateGameObjects();

		timeStep = 0.0f;
	}


#endif
}

KE::Collider* KE::CollisionHandler::RayCast(const Rayf& aRay, Vector3f& anOutPoint, const float aDistance, const int aLayerMask, Vector3f* aOutNormal)
{
	Vector3f origin(aRay.GetOrigin());
	Vector3f direction(aRay.GetDirection());

	const u_int bufferSize = 128;
	physx::PxRaycastHit hitBuffer[bufferSize];

	physx::PxRaycastBuffer hitData(hitBuffer, bufferSize);

	physx::PxQueryFilterData filterdata;

	filterdata.data.word3 = aLayerMask;

	float shortestDist = FLT_MAX;

	KE::Collider* hit = nullptr;

	if (myScene->raycast(
		{ origin.x, origin.y, origin.z },
		{ direction.x, direction.y, direction.z },
		aDistance,
		hitData,
		physx::PxHitFlag::eDEFAULT))
	{
		for (int i = 0; i < (int)hitData.getNbTouches(); i++)
		{
			int ID = *(int*)(hitData.getTouch(i).actor->userData);
			PhysicsObject& obj = *myPhysObjects.at(ID);

			// Ignore if not active
			if (!obj.myKECollider.isActive) { continue; }

			// Check if on layermask
			if (aLayerMask == 0 || (int)obj.myKECollider.myComponent->GetGameObject().myLayer & aLayerMask) 
			{
				Vector3f hitPosition = *(Vector3f*)&hitData.getTouch(i).position;

				const float dist = Vector3f(hitPosition - origin).Length();

				if (dist < shortestDist) 
				{
					shortestDist = dist;
					hit = &obj.myKECollider;

					anOutPoint = hitPosition;

					if (aOutNormal)
					{
						*aOutNormal = *(Vector3f*)&hitData.getTouch(i).normal;
					}
				}

			}
			//hits.push_back(&obj.myKECollider);
		}


		//return &obj.myKECollider;
	}

	return hit;
}

void KE::CollisionHandler::RegisterToBlackboard()
{
	KE_GLOBAL::blackboard.Register("collisionHandler", this);
}

void KE::CollisionHandler::AddNavmesh(Navmesh& aNavmesh)
{
	myNavmesh = &aNavmesh;
}

void KE::CollisionHandler::NavmeshRayCast(Rayf& aRay, Vector3f& aOutPoint, Vector3f aUserPosition)
{
	if (!myNavmesh) return;

	Vector3f intersectPoint;
	Vector3f v1;
	Vector3f v2;
	Vector3f v3;
	Plane plane;
	float distance = FLT_MAX;
	float userHeight = aUserPosition.y;

	for (NavNode& node : myNavmesh->myNodes)
	{
		v1 = *node.vertices[0];
		v2 = *node.vertices[1];
		v3 = *node.vertices[2];

		plane.InitWith3Points(v3, v2, v1);

		// If ray never intersected plane -> Check Next
		if (!IntersectionPlaneRay(plane, aRay, intersectPoint)) continue;

		// If IntersectPoint is in triangle -> return true (Position is valid on Navmesh)

		if (InsideTriangle(intersectPoint, node.vertices))
		{
			float tempDist = (intersectPoint - aRay.GetOrigin()).Length();

			if (tempDist > distance) continue;
			//if (abs(intersectPoint.y) - abs(userHeight) > 5.0f) continue;

			distance = tempDist;
			aOutPoint = intersectPoint;
		}
	}
}

std::vector<KE::Collider*> KE::CollisionHandler::BoxCast(const KE::Box& aBox, int aLayerMask)
{
	std::vector<KE::Collider*> hits;

	const u_int bufferSize = 256;
	physx::PxOverlapHit hitBuffer[bufferSize];

	physx::PxOverlapBuffer buf(hitBuffer, bufferSize);

	physx::PxVec3 size;
	Vector3f kesize = aBox.myMax * 2.0f;

	memcpy(&size, &kesize, sizeof(Vector3f));

	physx::PxBoxGeometry box(size);

	physx::PxVec3 vecPos({ aBox.myPosition.x + aBox.myOffset.x,aBox.myPosition.y + aBox.myOffset.y, aBox.myPosition.z + aBox.myOffset.z });
	physx::PxTransform position(vecPos);

	physx::PxQueryFilterData filter(physx::PxQueryFlag::eNO_BLOCK);


	// https://nvidia-omniverse.github.io/PhysX/physx/5.3.1/_api_build/class_px_scene.html#_CPPv4NK7PxScene7overlapERK10PxGeometryRK11PxTransformR17PxOverlapCallbackRK17PxQueryFilterDataP21PxQueryFilterCallbackPK12PxQueryCache20PxGeometryQueryFlags
	if (myScene->overlap(box, position, buf))
	{
		int nTouches = (int)buf.getNbTouches();

		for (int i = 0; i < nTouches; i++)
		{
			if (buf.getTouch(i).actor)
			{
				int ID = *(int*)(buf.getTouch(i).actor->userData);
				if (!myPhysObjects.contains(ID)) { continue; }
				PhysicsObject& obj = *myPhysObjects.at(ID);
				if (aLayerMask == 0 || (int)obj.myKECollider.myComponent->GetGameObject().myLayer & aLayerMask)
				{
					hits.push_back(&obj.myKECollider);
				}
			}
		}
	}

	return hits;
}


KE::Collider* KE::CollisionHandler::CreateBox(const Transform& aTransform, const Vector3f& anOffset, const Vector3f& aSize, const Collision::Layers aLayer, const bool aIsStatic, const bool aIsKinematic)
{
#ifndef USE_PHYSX
	return nullptr;
#else
	//PhysicsObject& physObj = myPhysicsObjects.emplace_back();

	const int ID = GenerateID();


	PhysicsObject& physObj = myPhysicsObjects.emplace_back();
	myPhysObjects.insert(std::pair(ID, &physObj));

	physObj.ID = ID;
	physObj.myKECollider.isStatic = aIsStatic;
	physObj.myKECollider.myOffset = anOffset;
	physObj.myKECollider.myBaseSize = aSize;


	Vector3f transformedSize = aSize;
	//Matrix3x3f matrix3 = aTransform.GetCUMatrix();
	//transformedSize = matrix3 * transformedSize;
	transformedSize.x *= aTransform.GetScale().x;
	transformedSize.y *= aTransform.GetScale().y;
	transformedSize.z *= aTransform.GetScale().z;

	transformedSize /= 2.0f;

	physx::PxShape* aBoxShape = myPhysics->createShape(physx::PxBoxGeometry(transformedSize.x, transformedSize.y, transformedSize.z), * myMaterial);

	aBoxShape->setRestOffset(0.0f);
	aBoxShape->setContactOffset(0.0f);



	physx::PxFilterData filterData;
	filterData.word0 |= static_cast<int>(aLayer);
	filterData.word1 = INT_MAX;

	aBoxShape->setQueryFilterData(filterData);
	aBoxShape->setSimulationFilterData(filterData);

	Matrix4x4f matrix = aTransform.GetCUMatrix();

	Transform transform(matrix);

	transform.SetScale({1.0f, 1.0f, 1.0f});

	matrix = transform.GetCUMatrix();

	//Matrix4x4f::Transpose(matrix);

	physx::PxMat44T<float> mat;

	memcpy(&mat, &matrix, sizeof(Matrix4x4f));

	physx::PxTransform t(mat);

	if (aIsStatic)
	{
		physx::PxRigidStatic* actor = myPhysics->createRigidStatic(t);
		physObj.myActor = actor;

		physObj.myActor->userData = (void*)&physObj.ID;

		actor->attachShape(*aBoxShape);
		myScene->addActor(*actor);
	}
	else
	{
		physx::PxRigidDynamic* actor = myPhysics->createRigidDynamic(t);
		physObj.myActor = actor;

		physObj.myActor->userData = (void*)&physObj.ID;

		actor->attachShape(*aBoxShape);
		

		physx::PxRigidBodyExt::updateMassAndInertia(*actor, 1000000.0f);

		myScene->addActor(*actor);
	}

	if (aIsKinematic)
	{
		physObj.myKECollider.SetKinematic(aIsKinematic);
	}

	return &physObj.myKECollider;
#endif // !USE_PHYSX
}

KE::Collider* KE::CollisionHandler::CreateSphere(const Transform& aTransform, const Vector3f& anOffset, const float aRadius, const Collision::Layers aLayer, const bool aIsStatic, const bool aIsKinematic)
{
#ifndef USE_PHYSX
	return nullptr;
#else
	const int ID = GenerateID();
	PhysicsObject& physObj = myPhysicsObjects.emplace_back();
	myPhysObjects.insert(std::pair(ID, &physObj));

	physObj.ID = ID;

	physObj.myKECollider.isStatic = aIsStatic;

	physObj.myKECollider.myOffset = anOffset;

	physx::PxShape* sphere = myPhysics->createShape(physx::PxSphereGeometry(aRadius), *myMaterial);

	physx::PxFilterData filterData;
	filterData.word0 |= static_cast<int>(aLayer);

	sphere->setQueryFilterData(filterData);
	sphere->setSimulationFilterData(filterData);


	Matrix4x4f matrix = aTransform.GetCUMatrix();

	physx::PxMat44T<float> mat;

	memcpy(&mat, &matrix, sizeof(Matrix4x4f));

	physx::PxTransform t(mat);

	if (aIsStatic)
	{
		physx::PxRigidStatic* actor = myPhysics->createRigidStatic(t);
		physObj.myActor = actor;

		physObj.myActor->userData = (void*)&physObj.ID;

		actor->attachShape(*sphere);
		myScene->addActor(*actor);
	}
	else
	{
		physx::PxRigidDynamic* actor = myPhysics->createRigidDynamic(t);
		physObj.myActor = actor;

		physObj.myActor->userData = (void*)&physObj.ID;

		actor->attachShape(*sphere);

		physx::PxRigidBodyExt::updateMassAndInertia(*actor, 1.0f);
		myScene->addActor(*actor);
	}

	if (aIsKinematic)
	{
		physObj.myKECollider.SetKinematic(aIsKinematic);
	}

	return &physObj.myKECollider;
#endif // !USE_PHYSX
}

KE::Collider* KE::CollisionHandler::CreateCapsule(const Transform& aTransform, const Vector3f& anOffset, const float aRadius, const float aHalfHeight, const Collision::Layers aLayer, const bool aIsKinematic, const bool aIsStatic)
{
#ifndef USE_PHYSX
	return nullptr;
#else
	const int ID = GenerateID();
	PhysicsObject& physObj = myPhysicsObjects.emplace_back();
	myPhysObjects.insert(std::pair(ID, &physObj));

	physObj.ID = ID;

	physObj.myKECollider.myOffset = anOffset;


	Matrix4x4f matrix = aTransform.GetCUMatrix();

	physx::PxMat44T<float> mat;

	memcpy(&mat, &matrix, sizeof(Matrix4x4f));

	physx::PxTransform t(mat);

	//physx::PxRigidStatic* actor = myPhysics->createRigidStatic(t);
	//physObj.myActor = actor;

	//physObj.myActor->userData = (void*)&physObj.ID;

	// 	physx::PxShape* aBoxShape = myPhysics->createShape(physx::PxBoxGeometry(transformedSize.x, transformedSize.y, transformedSize.z), * myMaterial);

	physx::PxShape* capsule = myPhysics->createShape(physx::PxCapsuleGeometry(aRadius, aHalfHeight), *myMaterial);

	//physx::PxShape* capsule = physx::PxRigidActorExt::createExclusiveShape(
	//	*actor, 
	//	physx::PxCapsuleGeometry(aRadius, aHalfHeight), 
	//	*myMaterial);

	physx::PxFilterData filterData;
	filterData.word0 |= static_cast<int>(aLayer);
	filterData.word1 = INT_MAX;

	capsule->setQueryFilterData(filterData);
	capsule->setSimulationFilterData(filterData);


	physx::PxTransform relativePose(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1)));
	capsule->setLocalPose(relativePose);
	//physx::PxRigidBodyExt::updateMassAndInertia(*actor, 1.0f);

	//actor->attachShape(*capsule);

	if (aIsStatic)
	{
		physx::PxRigidStatic* actor = myPhysics->createRigidStatic(t);
		physObj.myActor = actor;

		physObj.myActor->userData = (void*)&physObj.ID;

		actor->attachShape(*capsule);
		myScene->addActor(*actor);
	}
	else
	{
		physx::PxRigidDynamic* actor = myPhysics->createRigidDynamic(t);
		physObj.myActor = actor;

		physObj.myActor->userData = (void*)&physObj.ID;

		actor->attachShape(*capsule);


		physx::PxRigidBodyExt::updateMassAndInertia(*actor, 1000000.0f);

		myScene->addActor(*actor);
	}


	if (aIsKinematic)
	{
		physObj.myKECollider.SetKinematic(aIsKinematic);
	}

	return &physObj.myKECollider;
#endif // !USE_PHYSX
}

KE::Collider* KE::CollisionHandler::CreateMeshCollider(
	const Transform& aTransform, 
	const std::vector<Vertex>& someVertices,
	const std::vector<unsigned int>& someIndices, 
	const Collision::Layers aLayer, 
	const bool aIsStatic, 
	const bool aIsKinematic)
{
#ifndef USE_PHYSX
	return nullptr;
#else

	const int ID = GenerateID();
	PhysicsObject& physObj = myPhysicsObjects.emplace_back();
	myPhysObjects.insert(std::pair(ID, &physObj));

	physObj.ID = ID;

	Matrix4x4f matrix = aTransform.GetCUMatrix();
	physx::PxMat44T<float> mat;
	memcpy(&mat, &matrix, sizeof(Matrix4x4f));
	physx::PxTransform t(mat);
	physx::PxRigidDynamic* actor = myPhysics->createRigidDynamic(t);
	physObj.myActor = actor;
	physObj.myActor->userData = (void*)&physObj.ID;
	physObj.myActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, true);
	
	

	physx::PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = static_cast<physx::PxU32>(someVertices.size());
	meshDesc.points.stride = sizeof(Vertex);
	meshDesc.points.data = someVertices.data();

	meshDesc.triangles.count = static_cast<physx::PxU32>(someIndices.size() / 3);
	meshDesc.triangles.stride = 3 * sizeof(unsigned int);
	meshDesc.triangles.data = someIndices.data();

	physx::PxDefaultMemoryOutputStream writeBuffer;
	//physx::PxTriangleMeshCookingResult::Enum* result;


	physx::PxCookingParams param22(myToleranceScale);
	bool status = PxCookTriangleMesh(param22, meshDesc, writeBuffer);
	if (!status) {
		return NULL;
	}

	physx::PxTriangleMesh* triangleMesh = PxCreateTriangleMesh(param22, meshDesc, myPhysics->getPhysicsInsertionCallback());

	physx::PxFilterData filterData;
	filterData.word0 |= static_cast<int>(aLayer);

	
	physx::PxTriangleMeshGeometry geom(triangleMesh) ;

	physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor, geom, *myMaterial);
	physObj.myActor->attachShape(*shape);


	shape->setQueryFilterData(filterData);
	shape->setSimulationFilterData(filterData);

	shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
	shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);

	physx::PxTransform relativePose(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1)));
	shape->setLocalPose(relativePose);

	physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
	
	return &physObj.myKECollider;


#endif // !USE_PHYSX
}

KE::PhysicsCharacterController* KE::CollisionHandler::CreateController(const PhysxControllerData& aPhysxData, const CharacterControllerUserData& aControllerUserData)
{
	PhysicsCharacterController* controller = new PhysicsCharacterController();
	auto characterController = controller->Create(myPhysics, myControllerManager, aPhysxData, aControllerUserData);

	myPhysObjects.insert(characterController);

	return controller;
}

void KE::CollisionHandler::RegisterPhysicsObject(const std::pair<int, PhysicsObject*>& anObject)
{
	myPhysObjects.insert(anObject);
}

void KE::CollisionHandler::DebugDraw(KE::DebugRenderer& aDrawer)
{
	//auto playerActor = myPlayerController.myPhysicsObject.myActor;
	//
	//if(playerActor)
	//{
	//
	//	physx::PxTransform tm = playerActor->getGlobalPose();
	//
	//	tm.q.z = 0.0f;
	//	tm.q.w = 0.0f;
	//
	//	Transform transform(tm);
	//
	//	physx::PxShape* shape = nullptr;
	//	playerActor->getShapes(&shape, 1, 0);
	//	physx::PxGeometryHolder geom(shape->getGeometry());
	//
	//	//const float radius = geom.capsule().radius;
	//	const float radius = myPlayerController.myCapsuleController->getRadius();
	//	float length = myPlayerController.myCapsuleController->getHeight();
	//
	//	length += radius * 2.0f;
	//
	//	Vector3f pos = transform.GetPosition();
	//	pos.y -= length / 2.0f;
	//	pos.y -= myPlayerController.myCapsuleController->getContactOffset();
	//	transform.SetPosition(pos);
	//
	//	aDrawer.RenderCapsule(transform, radius, length, {0.0f, 1.0f, 1.0f, 1.0f});
	//}

	for (auto& physObj : myPhysObjects)
	{
		auto actor = physObj.second->myActor;

		physx::PxU32 nShapes = actor->getNbShapes();

		for (physx::PxU32 j = 0; j < nShapes; j++)
		{
			physx::PxShape* shape = nullptr;
			actor->getShapes(&shape, 1, j);
			physx::PxTransform tm = actor->getGlobalPose();
			physx::PxGeometryHolder geom(shape->getGeometry());

			switch (geom.getType())
			{
			case physx::PxGeometryType::eSPHERE: {
				Vector3f pos = Vector3f(tm.p.x, tm.p.y, tm.p.z);
				float radius = geom.sphere().radius;

				aDrawer.RenderSphere(pos, radius);
				break;
			}
			case physx::PxGeometryType::eBOX: {
				Vector3f pos = Vector3f(tm.p.x, tm.p.y, tm.p.z);
				Vector3f size = Vector3f(geom.box().halfExtents.x * 2, geom.box().halfExtents.y * 2, geom.box().halfExtents.z * 2);

				Transform kTrans(tm);

				//kTrans.SetScale(
				//	physObj.second->myKECollider.myComponent->GetGameObject().myWorldSpaceTransform.GetScale()
				//);

				aDrawer.RenderCube(kTrans, size);
				break;
			}
			case physx::PxGeometryType::eCAPSULE:
			{
				Transform transform(tm);

				const float radius = geom.capsule().radius;
				const float length = geom.capsule().halfHeight * 2.0f;

				aDrawer.RenderCapsule(transform, radius, length);
			}
			default:
				break;
			}
		}
	}
}

int KE::CollisionHandler::GenerateID()
{
	int id = -1;
	while (true)
	{
		id = rand() % 10000;
		if (myPhysObjects.count(id) == 0)
		{
			break;
		}
	}

	return id;
}

void KE::CollisionHandler::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
	physx::PxActor* actor1 = pairHeader.actors[0];
	physx::PxActor* actor2 = pairHeader.actors[1];

	int colID1 = *(int*)(actor1->userData);
	int colID2 = *(int*)(actor2->userData);

	if (!myPhysObjects.contains(colID1) || !myPhysObjects.contains(colID2))
	{
		std::cout << "on contact trigger but couldnt find a matching actor [" << colID1 << "] & [" << colID2 << "]" << std::endl;
		return;
	}
	PhysicsObject& col1 = *myPhysObjects.at(colID1);
	PhysicsObject& col2 = *myPhysObjects.at(colID2);

	//temporary ish, make sure both col1 and col2 have a KECollider

	if (!col1.myKECollider.myComponent || !col2.myKECollider.myComponent)
	{
		physx::PxShape* shape1 = nullptr;
		physx::PxShape* shape2 = nullptr;
		col1.myActor->getShapes(&shape1, 1, 0);
		col2.myActor->getShapes(&shape2, 1, 0);
		
		auto* testData = (CharacterControllerUserData*)(col1.myKECollider.myComponent) ? col1.myActor->userData : col2.myActor->userData;

		if (testData)
		{
			CharacterControllerUserData* data = static_cast<CharacterControllerUserData*>(testData);

			std::cout << "Player\n";
		}

		PhysxShapeUserData* userData1 = static_cast<PhysxShapeUserData*>(shape1->userData);
		PhysxShapeUserData* userData2 = static_cast<PhysxShapeUserData*>(shape2->userData);

		if (userData1 && userData2)
		{
			//std::cout << "User: " << userData1->myID << " Collided with: " << userData2->myID << std::endl;
			//std::cout << "COLLIDING WITH PLAYER" << std::endl;
			// [TODO] Set up a Common PhysxUserData as seen in Collider.h for all objects, including CCT's.
			// [TODO] Player's CCT's physxObject's component is null, this should probably be set to some kind of component
			// to avoid handling stuff in this IF statement.
			// Ultimately we want to trigger each components OnCollisionEnter / OnTriggerEnter.
		}

		return;
	}

	CollisionData data1(col1.myKECollider, col1.myKECollider.myComponent->GetGameObject());
	CollisionData data2(col2.myKECollider, col2.myKECollider.myComponent->GetGameObject());

	if (!col1.myKECollider.isStatic)
	{
		physx::PxRigidDynamic* actor = actor1->is<physx::PxRigidDynamic>();

		//actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, false);

		physx::PxTransform shapeTransform = actor->getGlobalPose();

		col1.myKECollider.myComponent->GetGameObject().myTransform.SetPosition({ shapeTransform.p.x, shapeTransform.p.y, shapeTransform.p.z });
	}

	if (!col2.myKECollider.isStatic)
	{
		physx::PxRigidDynamic* actor = actor2->is<physx::PxRigidDynamic>();

		physx::PxTransform shapeTransform = actor->getGlobalPose();

		col2.myKECollider.myComponent->GetGameObject().myTransform.SetPosition({ shapeTransform.p.x, shapeTransform.p.y, shapeTransform.p.z });
	}


	// https://nvidia-omniverse.github.io/PhysX/physx/5.1.0/_build/physx/latest/struct_px_contact_pair.html#_CPPv413PxContactPair

	//std::cout << "COLLISION DETECTED IN PHYSX!!!!" << std::endl;
	if (pairs->events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
	{
		data1.state |= 1 << 0;
		data2.state |= 1 << 0;

	}
	else if (pairs->events & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
	{
		data1.state |= 1 << 1;
		data2.state |= 1 << 1;

		//std::cout << "Contact!" << std::endl;
	}
	else if (pairs->events & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
	{
		data1.state |= 1 << 2;
		data2.state |= 1 << 2;

		//std::cout << "Lost contact" << std::endl;
	}

	col2.myKECollider.myCollisionData.push_back(data1);
	col1.myKECollider.myCollisionData.push_back(data2);
}

void KE::CollisionHandler::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
	std::cout << "TRIGGER DETECTED IN PHYSX!!!!" << std::endl;
}

physx::PxFilterFlags KE::CollisionHandler::CustomFilterShader(physx::PxFilterObjectAttributes attribute0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attribute1, physx::PxFilterData filterData1, physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
{
	// https://nvidia-omniverse.github.io/PhysX/physx/5.1.0/docs/RigidBodyCollision.html#collisionfiltering

	// Let Triggers through if set to: physx::PxPairFlag::eTRIGGER_DEFAULT
	//if (physx::PxFilterObjectIsTrigger(attribute0) || physx::PxFilterObjectIsTrigger(attribute1))
	//{
	//	pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
	//	return physx::PxFilterFlag::eDEFAULT;
	//}

	physx::PxPairFlag::Enum hitTypeFlag = physx::PxPairFlag::eCONTACT_DEFAULT;

	if (
	(
		filterData0.word0 & static_cast<int>(KE::Collision::Layers::Player) && 
		filterData1.word0 & static_cast<int>(KE::Collision::Layers::Projectile)
	) ||
	(
		filterData1.word0 & static_cast<int>(KE::Collision::Layers::Player) &&
		filterData0.word0 & static_cast<int>(KE::Collision::Layers::Projectile)
	) ||
	(
		filterData1.word0 & static_cast<int>(KE::Collision::Layers::Projectile) &&
		filterData0.word0 & static_cast<int>(KE::Collision::Layers::Projectile)
	)
	)
	{
		return physx::PxFilterFlag::eSUPPRESS;
	}

	if (filterData0.word3 != 0) 
	{
		if((filterData0.word3 >> filterData1.word0) & 1)
		{
			return physx::PxFilterFlag::eDEFAULT;

		}
	}
	else if (filterData1.word3 != 0) 
	{
		if ((filterData1.word3 >> filterData0.word0) & 1)
		{
			return physx::PxFilterFlag::eDEFAULT;
		}
	}

	if (!(filterData0.word0 & filterData1.word1))
	{
		return physx::PxFilterFlag::eSUPPRESS;
	}
	if (!(filterData0.word1 & filterData1.word0))
	{
		return physx::PxFilterFlag::eSUPPRESS;
	}

	// Generate contacts for all that were not filtered above
	pairFlags = hitTypeFlag;

	pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
	pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS;
	pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_LOST;


	// The filterData is data set by the user
	// The words can be layers or IDs that can be used to compare with other objects.

	// Trigger the contact callback for pairs (A, B) where
	// the filtermask of A contains the ID of B and vice versa.
	//if ((filterData0.word0 & filterData1.word1) || (filterData1.word0 & filterData0.word1))
	//if ((filterData0.word1 & filterData1.word0))
	//{
	//	pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
	//}

	return physx::PxFilterFlag::eDEFAULT;
}

#pragma region OLD COLLISION
//void KE::CollisionHandler::CheckCollision()
//{
//	// [NOTE] This needs to be optimized for future project. Prefferd with a [QuadTree] & [LayerMasks].
//
//	for (int i = 0; i < myColliders.size(); i++)
//	{
//		if (!myColliders[i]->isActive) continue;
//
//		Collider* firstCol = myColliders[i];
//
//		for (int j = i + 1; j < myColliders.size(); j++)
//		{
//			if (!myColliders[j]->isActive) continue;
//
//			Collider* secondCol = myColliders[j];
//
//			if (Intersect(firstCol->myShape, secondCol->myShape))
//			{
//				// Pack struct with collision info here
//				CollisionData colData1(*secondCol, secondCol->myComponent->GetGameObject());
//				firstCol->myCollisionData.push_back(colData1);
//
//				CollisionData colData2(*firstCol, firstCol->myComponent->GetGameObject());
//				secondCol->myCollisionData.push_back(colData2);
//			}
//		}
//	}
//}
//
//KE::Collider& KE::CollisionHandler::AddCollider()
//{
//	myColliders.emplace_back(new KE::Collider());
//
//	return *myColliders.back();
//}
//
//KE::Collider* KE::CollisionHandler::RayCast(Rayf& aRay, Vector3f& anOutPoint)
//{
//	for (size_t i = 0; i < myColliders.size(); i++)
//	{
//		if (!myColliders[i]->isActive) continue;
//
//
//		if (Intersect(myColliders[i]->myShape, aRay, anOutPoint))
//		{
//			return myColliders[i];
//		}
//	}
//
//	return nullptr;
//}
//
//KE::Collider* KE::CollisionHandler::RayCast(Rayf& aRay, Vector3f& anOutPoint, const int aLayer)
//{
//	for (size_t i = 0; i < myColliders.size(); i++)
//	{
//		if (!myColliders[i]->isActive) continue;
//
//		if ((static_cast<int>(myColliders[i]->myComponent->GetGameObject().myLayer) & aLayer) == 0) continue;
//
//		if (Intersect(myColliders[i]->myShape, aRay, anOutPoint))
//		{
//			return myColliders[i];
//		}
//	}
//
//	return nullptr;
//}
//
//// [TODO] - Make it possible to target a specific layermask in RayCast.
//KE::Collider* KE::CollisionHandler::RayCast(Rayf& aRay, const bool isTrigger)
//{
//	for (size_t i = 0; i < myColliders.size(); i++)
//	{
//		if (!myColliders[i]->isActive) continue;
//
//		// Added this so we can hit objects within the phone! Might have to change this later? Makes sense to target triggers though!
//		if (myColliders[i]->isTrigger != isTrigger) continue;
//
//		if (Intersect(myColliders[i]->myShape, aRay))
//		{
//			return myColliders[i];
//		}
//	}
//
//	return nullptr;
//}
//

//
//bool KE::CollisionHandler::PlayerRayCast(Rayf& aRay, Vector3f& aOutPoint, Vector3f aPlayerPosition)
//{
//	Vector3f intersectPoint;
//	Vector3f v1 = { aPlayerPosition.x + 10.f, aPlayerPosition.y, aPlayerPosition.z };
//	Vector3f v2 = { aPlayerPosition.x, aPlayerPosition.y, aPlayerPosition.z + 10.f };
//	Vector3f v3 = { aPlayerPosition.x - 10.f, aPlayerPosition.y, aPlayerPosition.z };
//	Plane plane;
//
//	plane.InitWith3Points(v3, v2, v1);
//
//	if (!IntersectionPlaneRay(plane, aRay, intersectPoint))
//	{
//		return false;
//	}
//	else
//	{
//		aOutPoint = intersectPoint;
//		return true;
//	}
//}
//
//std::vector<KE::Collider*> KE::CollisionHandler::ColliderCast(const KE::Shape& aShape, const int aLayerMask)
//{
//	std::vector<Collider*> collisions;
//
//	for (size_t i = 0; i < myColliders.size(); i++)
//	{
//		if ((static_cast<int>(myColliders[i]->myComponent->GetGameObject().myLayer) & aLayerMask) == 0 ||
//			!myColliders[i]->isActive) continue;
//
//		if (Intersect(&aShape, myColliders[i]->myShape))
//		{
//			collisions.push_back(myColliders[i]);
//		}
//	}
//
//	return collisions;
//}
//
//std::vector<KE::Collider*> KE::CollisionHandler::BoxCast(const KE::Box& aBox, const int aLayerMask)
//{
//	std::vector<Collider*> collisions;
//
//	for (size_t i = 0; i < myColliders.size(); i++)
//	{
//		if ((static_cast<int>(myColliders[i]->myComponent->GetGameObject().myLayer) & aLayerMask) == 0) continue;
//
//		if (Intersect(&aBox, myColliders[i]->myShape))
//		{
//			collisions.push_back(myColliders[i]);
//		}
//	}
//	return collisions;
//}
//
//std::vector<KE::Collider*> KE::CollisionHandler::SphereCast(const KE::Sphere& aSphere, const int aLayerMask)
//{
//	std::vector<Collider*> collisions;
//
//	for (size_t i = 0; i < myColliders.size(); i++)
//	{
//		if ((static_cast<int>(myColliders[i]->myComponent->GetGameObject().myLayer) & aLayerMask) == 0) continue;
//
//		if (Intersect(&aSphere, myColliders[i]->myShape))
//		{
//			collisions.push_back(myColliders[i]);
//		}
//	}
//
//	return collisions;
//}
#pragma endregion