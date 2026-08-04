// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "VordieScriptSubsystem.generated.h"

// * Pratt parser implementation for the VordieScript language.

enum class VSOperatorTokenType {
  LeftParen,
  RightParen,
  LeftBrace,
  RightBrace,
  LeftSquare,
  RightSquare,
  Comma,

  Dot,
  Question,
  Colon,
  Pipe,

  Equal,
  NotEqual,
  Less,
  LessEqual,
  Greater,
  GreaterEqual,

  Times,
  Divide,
  Plus,
  Minus,

  Not,
  And,
  Or,
};
enum class VSOperandTokenType {
  Eof,
  Eol,

  Identifier,
  Number,
  String,
  Boolean,
  PipeVar,
};
struct Token {
  TVariant<VSOperatorTokenType, VSOperandTokenType> Type;
  FString Value;
};
using VSOperandValue = TVariant<FString, int32, bool>;
struct VSOperand {
  VSOperandTokenType Type;
  VSOperandValue Value;
};
using VSExpression = TVariant<VSOperand, struct VSOperation>;
struct VSOperation {
  VSOperatorTokenType Operator;
  FString Value;
  TArray<VSExpression> Operands;
};
struct VSScript {
  TArray<VSExpression> Expressions;
};

// GlobalEnviroment is not a UPORPERTY, so registering a UObject* as a symbol does not create a reference to it.
// Check the pointer before using it, as it may have been garbage collected, and if it has, remove it from the GlobalEnviroment.
using UObjectPtr = TWeakObjectPtr<UObject>;

// Pairs a FStructProperty's metadata with a pointer to the actual struct instance it describes,
// since the property alone does not know where its value lives in memory.
struct VSStructInstance {
  FStructProperty* Property = nullptr;
  void* ContainerPtr = nullptr;

  bool operator==(const VSStructInstance& Other) const {
    return Property == Other.Property && ContainerPtr == Other.ContainerPtr;
  }
  bool operator!=(const VSStructInstance& Other) const { return !(*this == Other); }
};
// Caller is needed to use ProcessEvent.
struct VSUFunctionInstance {
  UFunction* Function = nullptr;
  UObjectPtr Caller = nullptr;
};

using VSContainerable = TVariant<FString, int32, float, bool, UObjectPtr, VSStructInstance>;
using VSEvaluatedArray = TArray<VSContainerable>;
using VSEvaluatedMap = TMap<FString, VSContainerable>;
using VSEvaluatedValue = TVariant<FString,
                                  int32,
                                  float,
                                  bool,
                                  UObjectPtr,
                                  VSStructInstance,
                                  VSEvaluatedArray,
                                  VSEvaluatedMap,
                                  VSUFunctionInstance>;

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

  auto EvaluateScript(const FString& ScriptCode) -> VSEvaluatedScript;

private:
  auto EvaluateOperand(const VSOperand& Op) -> VSEvaluatedValue;

  auto EvalUnaryOperation(const VSOperation& Op) -> VSEvaluatedValue;
  auto EvalBinaryOperation(const VSOperation& Op) -> VSEvaluatedValue;
  auto EvaluateArrayLiteral(const VSOperation& Op) -> VSEvaluatedValue;
  auto EvaluateArrayAccess(const VSOperation& Op) -> VSEvaluatedValue;
  auto EvaluateMapLiteral(const VSOperation& Op) -> VSEvaluatedValue;
  auto EvaluateMapAccess(const VSOperation& Op) -> VSEvaluatedValue;
  auto EvalTernaryOperation(const VSOperation& Op) -> VSEvaluatedValue;
  auto EvalPipeOperation(const VSOperation& Op) -> VSEvaluatedValue;
  auto EvalFuncArgsCall(const VSOperation& Op) -> VSEvaluatedValue;
  auto EvalDotObjectAccess(const VSOperation& Op) -> VSEvaluatedValue;
  auto EvaluateOperation(const VSOperation& Op) -> VSEvaluatedValue;

  auto EvaluateExpression(const VSExpression& Expr) -> VSEvaluatedValue;
};