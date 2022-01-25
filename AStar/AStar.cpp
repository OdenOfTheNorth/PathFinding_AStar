#include "AStar.h"
#include "FGAI_2/FGAI_2GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

UAStar::UAStar()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAStar::BeginPlay()
{
	Super::BeginPlay();

	Instance = Cast<AFGAI_2GameModeBase>(UGameplayStatics::GetGameMode(this));// Cast<AFGAI_2GameModeBase>(UGameplayStatics::GetGameState(this));
	if (!Instance)
	{
		UE_LOG(LogTemp,Warning,TEXT("UAStar, AFGAI_2GameModeBase is nullptr"));
		return;
	}

	Instance->AStarComponents.Add(this);
	
	UE_LOG(LogTemp,Warning,TEXT("UAStar, AFGAI_2GameModeBase"));
	
	if (!grid)
	{
		UE_LOG(LogTemp,Warning,TEXT("UAStar, grid is nullptr"));
	}

	StartActor = GetOwner();
}

void UAStar::PathFailed()
{
	UE_LOG(LogTemp,Warning,TEXT("UAStar, Did not find path"));
}

void UAStar::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!grid)
	{
		if (!Instance)
    	{
    		UE_LOG(LogTemp,Warning,TEXT("UAStar, AFGAI_2GameModeBase is nullptr"));
    		return;
    	}
		grid = Instance->grid;		
	}	

	if (grid)
	{
		Start = StartActor->GetActorLocation();
		End = EndActor->GetActorLocation();
		
		FindPath( Start, End);

		if (Move)
		{
			if (path.Num() == 0)
				return;			

			TargetIndex  = path.Num() - 1;
			
			Velocity = ( path[TargetIndex]->worldPosition - Start ).GetSafeNormal() * MoveSpeed * DeltaTime;
	
			StartActor->AddActorWorldOffset(Velocity, false);
		}			
	}
	
	DrawPath();
}

void UAStar::DrawPath()
{
	for (FGridNode* Node : path)
	{
		float boxSize = grid->nodeRadius - (grid->nodeRadius / 10);
		if (!jumpPoint)
		{
			UKismetSystemLibrary::DrawDebugBox(this, Node->worldPosition,
		FVector::OneVector * boxSize, Color, GetOwner()->GetActorRotation(), 0.f,3);
		}
		
		if (ShowValue)
		{
			FString values = "fCost " + FString::FromInt(Node->fCost) +
			" hCost " +	FString::FromInt(Node->hCost) +
			" gCost " + FString::FromInt(Node->gCost);

			UKismetSystemLibrary::DrawDebugString(this, Node->worldPosition, values, nullptr, Color);
		}
	}
}

void UAStar::FindPath(FVector startPos, FVector targetPos)
{			
	FGridNode* startNode = grid->NodeFromWorldPoint(startPos);
	FGridNode* targetNode = grid->NodeFromWorldPoint(targetPos);
	
	UKismetSystemLibrary::DrawDebugBox(this, startNode->worldPosition,	
    FVector::OneVector * grid->nodeRadius / 2, FColor::Black, FVector::ZeroVector.Rotation(), 0.f,3);

	UKismetSystemLibrary::DrawDebugBox(this, targetNode->worldPosition,	
	FVector::OneVector * grid->nodeRadius / 2, FColor::Black, FVector::ZeroVector.Rotation(), 0.f,3);

	if (startNode == nullptr || targetNode == nullptr || startNode == targetNode)	
		return;
	
	TArray<FGridNode*> openSet;
	TArray<FGridNode*> closedSet;
	
	openSet.Add(startNode);
	
	while (openSet.Num() > 0)
	{		
		FGridNode* node = openSet[0];

		for (int i = 1; i < openSet.Num(); i++)
		{		
		    if (openSet[i]->fCost < node->fCost || openSet[i]->fCost == node->fCost)
            {
	            if (openSet[i]->hCost < node->hCost)
	            {
	            	node = openSet[i];	
	            }		    	
            }
		}

		openSet.Remove(node);
		closedSet.Add(node);
		int pathIndex = 0;

		if (node == targetNode)
		{
			RetracePath(startNode, targetNode);
			return;
		}
		
		for (FGridNode* neighbour : grid->GetNeighbours(node))
		{
			if (!neighbour->walkable || closedSet.Contains(neighbour))
			{
				continue;
			}

			int newCostToNeighbour = node->gCost + GetDistance(node, neighbour) + neighbour->ObstacleCost;

			if (newCostToNeighbour < neighbour->gCost || !openSet.Contains(neighbour))
			{
				neighbour->gCost = newCostToNeighbour;
				neighbour->hCost = GetDistance(neighbour, targetNode);
				neighbour->parent = node;			

				neighbour->init(
					neighbour->walkable,
					neighbour->worldPosition,
					neighbour->gridX,
					neighbour->gridY,
					neighbour->ObstacleCost,
					neighbour->gridIndex,
					pathIndex
					);

				if (!openSet.Contains(neighbour))
				{
					openSet.Add(neighbour);
				}					
				else
				{
					openSet[neighbour->pathIndex] = neighbour;
				}					
			}

			pathIndex++;
		}
	}

	if (startNode != targetNode)
	{
#if WITH_EDITOR 
		PathFailed();
#endif
		
		Velocity = FVector::ZeroVector;
		return;
	}

	for (int i = 1; i < closedSet.Num(); i++)
	{
		if (closedSet[i] != nullptr)
		{
			float boxSize = grid->nodeRadius - (grid->nodeRadius / 10);
			
			UKismetSystemLibrary::DrawDebugBox(this, closedSet[i]->worldPosition,
		FVector::OneVector * boxSize, FColor::Yellow, GetOwner()->GetActorRotation(), 0.f,1);
		}
	}
}

void UAStar::RetracePath(FGridNode* startNode, FGridNode* endNode)
{
	path.Empty();
	path.SetNum(0,true);
	
	FGridNode* currentNode = endNode;

	while (currentNode != startNode)
	{
		if (!path.Contains(currentNode)){}
		path.Add(currentNode);

		int dirX = (currentNode->gridX - currentNode->parent->gridX);
		int dirY = (currentNode->gridY - currentNode->parent->gridY);

		currentNode->Direction = FVector2D(dirX, dirY);
		
		currentNode = currentNode->parent;
	}

	if (jumpPoint)
	{
		path = SearchDirection(path);
	}

	TargetIndex = path.Num() - 1;
}

int UAStar::GetDistance(FGridNode* nodeA, FGridNode* nodeB)
{
	int dstX = FMath::Abs(nodeA->gridX - nodeB->gridX);
	int dstY = FMath::Abs(nodeA->gridY - nodeB->gridY);
	
	if (dstX > dstY)
		return 14 * dstY + 10 * (dstX - dstY);
	return 14 * dstX + 10 * (dstY - dstX);	
}

TArray<FGridNode*> UAStar::SearchDirection(TArray<FGridNode*> SearchPath)
{
	TArray<FGridNode*> TempPath = SearchPath;
	FGridNode* currentNode = SearchPath[0];
	
	while (currentNode != SearchPath[SearchPath.Num() - 1])
	{
		if (currentNode->parent == nullptr)
		 return TempPath;
		
		if (currentNode->Direction == currentNode->parent->Direction)
		{
			TempPath.Remove(currentNode->parent);
			currentNode = currentNode->parent;
		}
		else
		{			
			currentNode = currentNode->parent;
		}
	}

	for (int i = 0; i < TempPath.Num() - 1; i++)
	{
		float boxSize = grid->nodeRadius - (grid->nodeRadius / 10);
		
		UKismetSystemLibrary::DrawDebugBox(this, TempPath[i]->worldPosition,
		FVector::OneVector * boxSize, Color, GetOwner()->GetActorRotation(), 0.f,1);

		UKismetSystemLibrary::DrawDebugLine(this,Start,TempPath[TempPath.Num() - 1]->worldPosition,Color);	
		UKismetSystemLibrary::DrawDebugLine(this,TempPath[i]->worldPosition,TempPath[i + 1]->worldPosition,Color);	
	}

	return TempPath;
}

