// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "VordieScriptSubsystem.generated.h"

// * Pratt parser implementation for the VordieScript language.

// GlobalEnviroment is not a UPORPERTY, so registering a UObject* as a symbol does not create a reference to it.
// Check the pointer before using it, as it may have been garbage collected, and if it has, remove it from the GlobalEnviroment.
using UObjectPtr = TWeakObjectPtr<UObject>;

using VSContainerable = TVariant<FString, int32, float, bool, UObjectPtr, FStructProperty*>;
using VSEvaluatedArray = TArray<VSContainerable>;
using VSEvaluatedMap = TMap<FString, VSContainerable>;
using VSEvaluatedValue =
    TVariant<FString, int32, float, bool, UObjectPtr, FStructProperty*, VSEvaluatedArray, VSEvaluatedMap>;

using VSFunction = TFunction<VSEvaluatedValue(const TArray<VSEvaluatedValue>&)>;
using VSEnviromentContext =
    TVariant<FString, int32, float, bool, UObjectPtr, VSEvaluatedArray, VSEvaluatedMap, VSFunction>;

struct VSEvaluatedScript {
  bool bSuccess;
  FString ErrorMessage;

  bool bHasReturn;
  VSEvaluatedValue ReturnValue;
};

auto TestScriptTreeGen(const FString& ScriptCode) -> FString;

UCLASS()
class PROTOTYPING_API UVordieScriptSubsystem : public UWorldSubsystem {
  GENERATED_BODY()

public:
  UVordieScriptSubsystem();

  TMap<FName, VSEnviromentContext> GlobalEnviroment;

  void RegisterSymbol(FName SymbolName, VSEnviromentContext SymbolValue);
};