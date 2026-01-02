#pragma once

//put E infront of characterstate (before name of enum)
//convention for UE is to add ECS prefix
// uint8 to make enums small numbers
UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_Unequipped UMETA(DisplayName = "Unequipped"),
	ECS_EquippedOneHandWeapon  UMETA(DisplayName = "Equipped One Hand Weapon"),
	ECS_EquippedTwoHandWeapon  UMETA(DisplayName = "Equipped Two Hand Weapon")


};

//action state
UENUM(BlueprintType)
enum class EActionState : uint8
{
	EAS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	EAS_Attacking UMETA(DisplayName = "Attacking"),
	EAS_EquippingWeapon UMETA(DisplayName = "Equipping Weapon")
};