#include "VordieScriptSubsystem.h"
#include "Misc/TVariant.h"

enum class OperatorTokenType {
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
enum class OperandTokenType {
  Eof,
  Eol,

  Identifier,
  Number,
  String,
  Boolean,
  PipeVar,
};
struct Token {
  TVariant<OperatorTokenType, OperandTokenType> Type;
  FString Value;
};
auto inline TokenIs(const Token& Token, OperatorTokenType OpType) -> bool {
  if (auto Operator = Token.Type.TryGet<OperatorTokenType>()) return *Operator == OpType;
  return false;
}
auto inline TokenIs(const Token& Token, OperandTokenType OpType) -> bool {
  if (auto Operand = Token.Type.TryGet<OperandTokenType>()) return *Operand == OpType;
  return false;
}
auto inline CanEndStatement(const Token& Token) -> bool {
  return TokenIs(Token, OperatorTokenType::RightBrace) || TokenIs(Token, OperatorTokenType::RightBrace) ||
         TokenIs(Token, OperatorTokenType::RightSquare);
}
auto inline CanStartStatement(const Token& Token) -> bool {
  return TokenIs(Token, OperatorTokenType::LeftParen) || TokenIs(Token, OperatorTokenType::LeftBrace) ||
         TokenIs(Token, OperatorTokenType::LeftSquare);
}

UVordieScriptSubsystem::UVordieScriptSubsystem() {}

void UVordieScriptSubsystem::RegisterSymbol(FName SymbolName, VSEnviromentContext SymbolValue) {
  if (GlobalEnviroment.Contains(SymbolName)) {
    UE_LOG(LogTemp, Warning, TEXT("Symbol %s is already registered."), *SymbolName.ToString());
    return;
  }

  GlobalEnviroment.Add(SymbolName, SymbolValue);
  UE_LOG(LogTemp, Log, TEXT("Registered symbol %s."), *SymbolName.ToString());
}