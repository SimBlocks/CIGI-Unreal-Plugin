//Copyright SimBlocks LLC 2016-2026

#include "UnrealCoordinates.h"

using namespace sbio::unrealcigi;

FUEWorldCoordinates::FUEWorldCoordinates(const FVector& InVector) : Value(InVector)
{
}

FUEWorldCoordinates::FUEWorldCoordinates(float InX, float InY, float InZ) : Value(InX, InY, InZ)
{
}

FVector FUEWorldCoordinates::ToFVector() const
{
  return Value;
}

FUEWorldRotation::FUEWorldRotation(const FQuat& InRotation) : Value(InRotation)
{
}

FUEWorldRotation::FUEWorldRotation(const FRotator& InRotation) : Value(InRotation.Quaternion())
{
}

FQuat FUEWorldRotation::ToFQuat() const
{
  return Value;
}

FRotator FUEWorldRotation::ToFRotator() const
{
  return Value.Rotator();
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026