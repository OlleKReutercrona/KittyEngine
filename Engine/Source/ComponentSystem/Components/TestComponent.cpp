#include "stdafx.h"
#include "TestComponent.h"
#include "../GameObject.h"

#include <iostream>

KE::TestComponent::TestComponent(GameObject& aGameObject) : Component(aGameObject)
{
}

KE::TestComponent::~TestComponent()
{
	std::cout << "Test Component is removed" << std::endl;
}

void KE::TestComponent::Awake()
{
	std::cout << "Do once" << std::endl;
	std::cout << "TestBool = " << myTestBool << std::endl;
	std::cout << "TestInt = " << myTestInt << std::endl;
	std::cout << "TestFloat = " << myTestFloat << std::endl;
	myGameObject.RemoveComponent<TestComponent>();
}

void KE::TestComponent::LateUpdate()
{
}

void KE::TestComponent::Update()
{
}

void KE::TestComponent::OnDestroy()
{
	std::cout << "Running OnDestroy" << std::endl;
}

void KE::TestComponent::OnEnable()
{
}

void KE::TestComponent::OnDisable()
{
}

//void KE::TestComponent::OnTrigger()
//{
//}
//
//void KE::TestComponent::OnCollision()
//{
//}

void KE::TestComponent::SetData(void* aDataObject)
{
	if (!aDataObject) { return; }

	TestComponentData* data = (TestComponentData*)aDataObject;

	myTestBool = data->testBool;
	myTestInt = data->testInt;
	myTestFloat = data->testFloat;
}
