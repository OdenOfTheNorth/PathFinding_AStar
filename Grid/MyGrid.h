#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyGrid.generated.h"

class AFGAI_2GameModeBase;

USTRUCT(BlueprintType)
struct FGridNode
{
	GENERATED_BODY()
	
	FVector2D Direction;
	FVector worldPosition;
	
	void init(bool _walkable, FVector _worldPos, int _gridX, int _gridY, int _ObstacleCost, int index, int pIndex = -1)
	{
		walkable = _walkable;
		worldPosition = _worldPos;
		gridX = _gridX;
		gridY = _gridY;
		ObstacleCost = _ObstacleCost;
		fCost = gCost + hCost;
		gridIndex = index;
		pathIndex = pIndex;
	}
	
	bool walkable;
	
	int ObstacleCost;
	int gridX;
	int gridY;	
	int gCost;	
	int hCost;
	int gridIndex;// =  (gridSizeX * x) + y;;
	int pathIndex;
	int fCost = gCost + hCost;
	
	FGridNode* parent;
};


UCLASS()
class FGAI_2_API AMyGrid : public AActor
{
	GENERATED_BODY()	
public:	
	AMyGrid();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	TArray<FGridNode*> GetNeighbours(FGridNode* node);
	FGridNode* NodeFromWorldPoint(FVector worldPosition);
	FGridNode* GetNodeFromXY(int X, int Y);

	UFUNCTION(BlueprintCallable)
	bool WorldLocationToGridNodeLocation(const FVector& WorldLocation, FVector& NodeLocation);

	UPROPERTY(EditAnywhere)
	bool updateGrid = false;
	
	void CreateGrid();
	void Draw();	
	
public:
	UPROPERTY(EditAnywhere)
	USceneComponent* SceneComponent = nullptr;

	UPROPERTY(EditAnywhere)
	AFGAI_2GameModeBase* Instance;
	
	TArray<FGridNode*> grid;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float colorScale = 0;
	
	UPROPERTY(EditAnywhere)
	bool drawBox = true;
	
	UPROPERTY(EditAnywhere)
	float GridXWorldSizeInMeters = 25;
	UPROPERTY(EditAnywhere)
	float GridYWorldSizeInMeters = 25;
	
	UPROPERTY(EditAnywhere)
	float nodeRadius = 50;
	UPROPERTY(EditAnywhere)
	float DrawTime = 500;
	FVector gridWorldSize = FVector::OneVector;

	float nodeDiameter;
	int gridSizeX, gridSizeY;
};
