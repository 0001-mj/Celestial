#pragma once
#include <Debug.h>

UEngine* GEngine;

UWorld* GetWorld()
{
	return GEngine->GameViewport->World;
}

template <typename ClassType>
ClassType* Cast(UObject* Obj)
{
	return Obj->IsA(ClassType::StaticClass()) ? static_cast<ClassType*>(Obj) : nullptr;
}

template <typename ObjectType>
ObjectType* DefaultObject(UClass* Class = ObjectType::StaticClass())
{
	return Cast<ObjectType>(Class->DefaultObject);
}