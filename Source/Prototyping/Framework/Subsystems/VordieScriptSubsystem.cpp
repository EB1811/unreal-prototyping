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
inline auto ToToken(OperatorTokenType Type, const FString& Value) -> Token {
  TVariant<OperatorTokenType, OperandTokenType> T;
  T.Set<OperatorTokenType>(Type);
  return {T, Value};
}
inline auto ToToken(OperandTokenType Type, const FString& Value) -> Token {
  TVariant<OperatorTokenType, OperandTokenType> T;
  T.Set<OperandTokenType>(Type);
  return {T, Value};
}
inline auto TokenIs(const Token& Token, OperatorTokenType OpType) -> bool {
  if (auto Operator = Token.Type.TryGet<OperatorTokenType>()) return *Operator == OpType;
  return false;
}
inline auto TokenIs(const Token& Token, OperandTokenType OpType) -> bool {
  if (auto Operand = Token.Type.TryGet<OperandTokenType>()) return *Operand == OpType;
  return false;
}

inline auto CanEndStatement(const Token& Token) -> bool {
  return TokenIs(Token, OperatorTokenType::RightBrace) || TokenIs(Token, OperatorTokenType::RightBrace) ||
         TokenIs(Token, OperatorTokenType::RightSquare);
}
inline auto CanStartStatement(const Token& Token) -> bool {
  return TokenIs(Token, OperatorTokenType::LeftParen) || TokenIs(Token, OperatorTokenType::LeftBrace) ||
         TokenIs(Token, OperatorTokenType::LeftSquare);
}
inline auto ShouldEndStatement(const FString& Code, int32 Index, TArray<Token>& Tokens, int32 BracketDepth) -> bool {
  if (Tokens.IsEmpty()) return false;
  if (TokenIs(Tokens.Last(), OperandTokenType::Eol)) return false;

  if (BracketDepth > 0) return false;
  if (Tokens.Last().Type.IsType<OperatorTokenType>() && !CanEndStatement(Tokens.Last())) return false;

  for (int32 i = Index; i < Code.Len(); ++i) {
    auto C = Code[i];
    if (isspace(C) || C == '\n') continue;
    if (C == ')' || C == '}' || C == ']' || C == ',' || C == '.' || C == '?' || C == ':' || C == '^' || C == '*' ||
        C == '/' || C == '+' || C == '-' || C == '|' || C == '&' || C == '=' || C == '!' || C == '<' || C == '>' ||
        C == '~') {
      return false;
    }
    break;
  }

  return true;
}
inline auto Tokenize(const FString& Code) -> TArray<Token> {
  int BracketDepth = 0;
  TArray<Token> Tokens;
  int32 i = 0;
  while (i < Code.Len()) {
    char C = Code[i];
    if (isspace(C)) {
      if (C == '\n' && ShouldEndStatement(Code, i, Tokens, BracketDepth))
        Tokens.Push(ToToken(OperandTokenType::Eol, "eol"));
      i++;
      continue;
    }

    if (isalpha(C) || C == '_') {
      int32 Start = i;
      while (i < Code.Len() && (isalnum(Code[i]) || Code[i] == '_')) i++;
      FString Identifier = Code.Mid(Start, i - Start);
      if (Identifier == "true" || Identifier == "false") Tokens.Push(ToToken(OperandTokenType::Boolean, Identifier));
      else Tokens.Push(ToToken(OperandTokenType::Identifier, Identifier));

      // Func() => Func
      if (i + 1 < Code.Len() && TokenIs(Tokens.Last(), OperandTokenType::Identifier) && Code[i] == '(' &&
          Code[i + 1] == ')') {
        i += 1;
      }
    } else if (isdigit(C)) {
      int32 Start = i;
      while (i < Code.Len() && isdigit(Code[i])) i++;
      Tokens.Push(ToToken(OperandTokenType::Number, Code.Mid(Start, i - Start)));
    } else if (C == '\'') {
      int32 Start = ++i;
      while (i < Code.Len() && Code[i] != '\'') i++;
      Tokens.Push(ToToken(OperandTokenType::String, Code.Mid(Start, i - Start)));
      i++;
    } else if (C == '|') {
      if (i + 1 < Code.Len() && Code[i + 1] == '|') {
        Tokens.Push(ToToken(OperatorTokenType::Or, "||"));
        i += 2;
      } else {
        UE_LOG(LogTemp, Error, TEXT("Unknown character: %c"), C);
        checkNoEntry();
      }
    } else if (C == '&') {
      if (i + 1 < Code.Len() && Code[i + 1] == '&') {
        Tokens.Push(ToToken(OperatorTokenType::And, "&&"));
        i += 2;
      } else {
        UE_LOG(LogTemp, Error, TEXT("Unknown character: %c"), C);
        checkNoEntry();
      }
    } else if (C == '=') {
      if (i + 1 < Code.Len() && Code[i + 1] == '=') {
        Tokens.Push(ToToken(OperatorTokenType::Equal, "=="));
        i += 2;
      } else {
        UE_LOG(LogTemp, Error, TEXT("Unknown character: %c"), C);
        checkNoEntry();
      }
    } else if (C == '!') {
      if (i + 1 < Code.Len() && Code[i + 1] == '=') {
        Tokens.Push(ToToken(OperatorTokenType::NotEqual, "!="));
        i += 2;
      } else {
        Tokens.Push(ToToken(OperatorTokenType::Not, "!"));
        i++;
      }
    } else if (C == '>') {
      if (i + 1 < Code.Len() && Code[i + 1] == '=') {
        Tokens.Push(ToToken(OperatorTokenType::GreaterEqual, ">="));
        i += 2;
      } else {
        Tokens.Push(ToToken(OperatorTokenType::Greater, ">"));
        i++;
      }
    } else if (C == '<') {
      if (i + 1 < Code.Len() && Code[i + 1] == '=') {
        Tokens.Push(ToToken(OperatorTokenType::LessEqual, "<="));
        i += 2;
      } else {
        Tokens.Push(ToToken(OperatorTokenType::Less, "<"));
        i++;
      }
    } else if (C == '~') {
      if (i + 1 < Code.Len() && Code[i + 1] == '>') {
        Tokens.Push(ToToken(OperatorTokenType::Pipe, "~>"));
        i += 2;
      } else {
        UE_LOG(LogTemp, Error, TEXT("Unknown character: %c"), C);
        checkNoEntry();
      }
    } else {
      switch (C) {
        case '(': Tokens.Push(ToToken(OperatorTokenType::LeftParen, "(")); break;
        case ')': Tokens.Push(ToToken(OperatorTokenType::RightParen, ")")); break;
        case '{': Tokens.Push(ToToken(OperatorTokenType::LeftBrace, "{")); break;
        case '}': Tokens.Push(ToToken(OperatorTokenType::RightBrace, "}")); break;
        case '[': Tokens.Push(ToToken(OperatorTokenType::LeftSquare, "[")); break;
        case ']': Tokens.Push(ToToken(OperatorTokenType::RightSquare, "]")); break;
        case ',': Tokens.Push(ToToken(OperatorTokenType::Comma, ",")); break;
        case '.': Tokens.Push(ToToken(OperatorTokenType::Dot, ".")); break;
        case '?': Tokens.Push(ToToken(OperatorTokenType::Question, "?")); break;
        case ':': Tokens.Push(ToToken(OperatorTokenType::Colon, ":")); break;
        case '^': Tokens.Push(ToToken(OperandTokenType::PipeVar, "^")); break;
        case '*': Tokens.Push(ToToken(OperatorTokenType::Times, "*")); break;
        case '/': Tokens.Push(ToToken(OperatorTokenType::Divide, "/")); break;
        case '+': Tokens.Push(ToToken(OperatorTokenType::Plus, "+")); break;
        case '-': Tokens.Push(ToToken(OperatorTokenType::Minus, "-")); break;
        default:
          UE_LOG(LogTemp, Error, TEXT("Unknown character: %c"), C);
          checkNoEntry();
          break;
      }
      i++;
    }

    switch (C) {
      case '(':
      case '{':
      case '[': BracketDepth++; break;
      case ')':
      case '}':
      case ']': BracketDepth--; break;
      default: break;
    }
  }
  Tokens.Push(ToToken(OperandTokenType::Eof, ""));
  return Tokens;
};

static const TMap<OperatorTokenType, int32> OPERATOR_TOKEN_BINDINGS = {
    {OperatorTokenType::Or, 5},         {OperatorTokenType::And, 5},        {OperatorTokenType::Pipe, 1},
    {OperatorTokenType::Equal, 10},     {OperatorTokenType::NotEqual, 10},  {OperatorTokenType::Less, 10},
    {OperatorTokenType::LessEqual, 10}, {OperatorTokenType::Greater, 10},   {OperatorTokenType::GreaterEqual, 10},
    {OperatorTokenType::Plus, 20},      {OperatorTokenType::Minus, 20},     {OperatorTokenType::Times, 30},
    {OperatorTokenType::Divide, 30},    {OperatorTokenType::LeftParen, 50}, {OperatorTokenType::LeftSquare, 50},
    {OperatorTokenType::LeftBrace, 50}, {OperatorTokenType::Question, 7},   {OperatorTokenType::Dot, 55},
};
static const TMap<OperatorTokenType, int32> UNARY_OPERATOR_TOKEN_BINDINGS = {
    {OperatorTokenType::Not, 50},
    {OperatorTokenType::Plus, 50},
    {OperatorTokenType::Minus, 50},
};

using OperandValue = TVariant<FString, int32, bool>;
struct Operand {
  OperandTokenType Type;
  OperandValue Value;
};
using Expression = TVariant<Operand, struct Operation>;
struct Operation {
  OperatorTokenType Operator;
  FString Value;
  TArray<Expression> Operands;
};
struct Script {
  TArray<Expression> Expressions;
};

// Utility to allow overloading lambdas for use in TVariant::Visit
template <typename... Ts>
struct Overload : Ts... {
  using Ts::operator()...;
};
template <typename... Ts>
Overload(Ts...) -> Overload<Ts...>;

UVordieScriptSubsystem::UVordieScriptSubsystem() {}

void UVordieScriptSubsystem::RegisterSymbol(FName SymbolName, VSEnviromentContext SymbolValue) {
  if (GlobalEnviroment.Contains(SymbolName)) {
    UE_LOG(LogTemp, Warning, TEXT("Symbol %s is already registered."), *SymbolName.ToString());
    return;
  }

  GlobalEnviroment.Add(SymbolName, SymbolValue);
  UE_LOG(LogTemp, Log, TEXT("Registered symbol %s."), *SymbolName.ToString());
}