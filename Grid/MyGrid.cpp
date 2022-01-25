#include "MyGrid.h"
#include "CostComponent.h"
//#include "Node.h"
#include "FGAI_2/FGAI_2GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

AMyGrid::AMyGrid()
{
	PrimaryActorTick.bCanEverTick = true;
	SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
}

void AMyGrid::BeginPlay()
{
	Super::BeginPlay();

	Instance = Cast<AFGAI_2GameModeBase>(UGameplayStatics::GetGameMode(this));// Cast<AFGAI_2GameModeBase>(UGameplayStatics::GetGameState(this));

	if (Instance)
	{
		Instance->grid = this;
	}	else
	{
		UE_LOG(LogTemp,Warning,TEXT("AMyGrid, Did not find AFGAI_2GameModeBase"));
	}	
	
	gridWorldSize.X = (GridXWorldSizeInMeters * 100);
	gridWorldSize.Y = (GridYWorldSizeInMeters * 100);
	
	nodeDiameter = nodeRadius * 2;
	gridSizeX = FMath::RoundToInt(gridWorldSize.X / nodeDiameter);
	gridSizeY = FMath::RoundToInt(gridWorldSize.Y / nodeDiameter);
	grid.SetNum(gridSizeX * gridSizeY, true);
	CreateGrid();
}

// Called every frame
void AMyGrid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (updateGrid)
	{
		gridWorldSize.X = (GridXWorldSizeInMeters * 100);
		gridWorldSize.Y = (GridYWorldSizeInMeters * 100);
	
		nodeDiameter = nodeRadius * 2;
		gridSizeX = FMath::RoundToInt(gridWorldSize.X / nodeDiameter);
		gridSizeY = FMath::RoundToInt(gridWorldSize.Y / nodeDiameter);
		CreateGrid();
		updateGrid = false;
	}

	Draw();
}

TArray<FGridNode*> AMyGrid::GetNeighbours(FGridNode* node)
{
	TArray<FGridNode*> neighbours;

	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			/*
			X = -1 Y =1		X = 0 Y = 1		X = 1 Y =1
			X = -1 Y =0		X = 0 Y = 0		X = 1 Y =0
			X = -1 Y =-1	X = 0 Y = -1	X = 1 Y =-1

			if x and y is 0 then you are not on a Neighbour
			*/
			
			if (x == 0 && y == 0)		
				continue;

			int checkX = node->gridX + x; 
			int checkY = node->gridY + y;			

			if (checkX >= 0 && checkX < gridSizeX && checkY >= 0 && checkY < gridSizeY)
			{
				neighbours.Add(grid[ (gridSizeY * checkX) + checkY]);
			}
		}
	}

	return neighbours;
}

FGridNode* AMyGrid::NodeFromWorldPoint(FVector worldPosition)
{
	float percentX = (worldPosition.X + gridWorldSize.X / 2) / gridWorldSize.X;
	float percentY = (worldPosition.Y + gridWorldSize.Y / 2) / gridWorldSize.Y;
	
	percentX = FMath::Clamp(percentX, 0.f,1.f);
	percentY = FMath::Clamp(percentY, 0.f,1.f);

	int x = FMath::RoundToInt((gridSizeX - 1) * percentX);
	int y = FMath::RoundToInt((gridSizeY - 1) * percentY);

	return grid[(y * gridSizeX) + x];
}

FGridNode* AMyGrid::GetNodeFromXY(int X, int Y)
{
	return grid[(X * gridSizeX) + Y];
}

bool AMyGrid::WorldLocationToGridNodeLocation(const FVector& WorldLocation, FVector& NodeLocation)
{
	FGridNode* worldNode = NodeFromWorldPoint(WorldLocation);

	if (worldNode)
	{
		NodeLocation = worldNode->worldPosition;
		return true;
	}
	
	return false;
}

void AMyGrid::CreateGrid()
{
	grid.Empty();
	grid.SetNum(gridSizeX * gridSizeY, false);
	const FVector worldBottomLeft = GetActorLocation() - FVector::RightVector * gridWorldSize.X / 2 - FVector::ForwardVector * gridWorldSize.Y / 2;
	FCollisionQueryParams QueryParams;
	
	for (int x = 0; x < gridSizeX; x++)
	{
		for (int y = 0; y < gridSizeY; y++)
		{
			int gridIndex = (gridSizeX * x) + y;

			int OCost = 0;
			
			FVector worldPoint = worldBottomLeft + FVector::RightVector * (x * nodeDiameter + nodeRadius) +
				FVector::ForwardVector * (y * nodeDiameter + nodeRadius);

			TArray<FHitResult> BoxResult;
			TArray<AActor* > ActorsToIgnore;
			bool walkable = true;
			FName Name = "";

			GetWorld()->LineTraceMultiByChannel(BoxResult, worldPoint + FVector::UpVector * 200, worldPoint, ECC_Pawn, QueryParams);
			
			for (int i = 0; i < BoxResult.Num(); i++)
			{
				if(BoxResult[i].bBlockingHit)
				{					
					//UE_LOG(LogTemp,Warning, TEXT("AMyGrid, BoxResult"),walkable );
					AActor* hitActor = Cast<AActor>(BoxResult[i].Actor);

					if (!hitActor)
						continue;

					UCostComponent* CostComponent = hitActor->FindComponentByClass<UCostComponent>();
					
					if (CostComponent)
					{
						walkable = CostComponent->walkable;
						OCost = CostComponent->ObstacleCost;
					}
				}
			}

			if (grid.Num() <= gridIndex)
			{
				UE_LOG(LogTemp,Warning, TEXT("gridIndex is bigger than grid"));
				break;
			}

			if (grid[gridIndex] == nullptr)
			{
				grid[gridIndex] = new FGridNode();
				grid[gridIndex]->init(walkable, worldPoint, x,y, OCost, gridIndex);	
			}
		}
	}
}

void AMyGrid::Draw()
{
	for (FGridNode* Node : grid)
	{
		if (Node == nullptr)
			continue;
		
		FColor color = !Node->walkable ? FColor::Red : FColor::Green; //
		float boxSize = nodeRadius - (nodeRadius / 10);
		
		color = FColor::Red;

		colorScale = Node->ObstacleCost / color.R;
			
		if (Node->walkable)
		{
			if (Node->ObstacleCost > 0.6)
			{					
				color = FColor::Orange;
			}
			else if(Node->ObstacleCost > 0.3)
			{
				color = FColor::Yellow;
			}
			else
			{
				color = FColor::Green;
			}
		}

		if (drawBox)
		{
			UKismetSystemLibrary::DrawDebugBox(this, Node->worldPosition,
		FVector::OneVector * boxSize, color, GetActorRotation(), 0.f,3);			
		}
	}
}