// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//#include <stdbool.h>

#include "MyGrid.h"
#include "Components/ActorComponent.h"
#include "AStar.generated.h"



class AFGAI_2GameModeBase;
class UGridNode;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FGAI_2_API UAStar : public UActorComponent
{
	GENERATED_BODY()
public:	
	UAStar();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void DrawPath();
	void FindPath(FVector startPos, FVector targetPos);
	void RetracePath(FGridNode* startNode, FGridNode* endNode);
	void PathFailed();

	int GetDistance(FGridNode* nodeA, FGridNode* nodeB);
	TArray<FGridNode*> SearchDirection(TArray<FGridNode*> SearchPath);

public:
	
	TArray<FGridNode*> path;
	UPROPERTY(EditAnywhere)
	AMyGrid* grid = nullptr;
	UPROPERTY(EditAnywhere)
	AFGAI_2GameModeBase* Instance = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* StartActor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* EndActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Start;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector End;
	FVector Velocity;

	UPROPERTY(EditAnywhere)
	bool jumpPoint = true;
	UPROPERTY(EditAnywhere)
	bool ShowValue = true;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool Move = false;
	UPROPERTY(EditAnywhere)
	float MoveSpeed = 300;
	
	UPROPERTY(EditAnywhere)
	FColor Color = FColor::Red;

	int TargetIndex;
};
