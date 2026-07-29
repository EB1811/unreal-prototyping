#include "VordieScriptSubsystem.h"
#include "HAL/Platform.h"
#include "Logging/LogVerbosity.h"
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
auto Tokenize(const FString& Code) -> TArray<Token> {
  int32 BracketDepth = 0;
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
        return {};
      }
    } else if (C == '&') {
      if (i + 1 < Code.Len() && Code[i + 1] == '&') {
        Tokens.Push(ToToken(OperatorTokenType::And, "&&"));
        i += 2;
      } else {
        UE_LOG(LogTemp, Error, TEXT("Unknown character: %c"), C);
        checkNoEntry();
        return {};
      }
    } else if (C == '=') {
      if (i + 1 < Code.Len() && Code[i + 1] == '=') {
        Tokens.Push(ToToken(OperatorTokenType::Equal, "=="));
        i += 2;
      } else {
        UE_LOG(LogTemp, Error, TEXT("Unknown character: %c"), C);
        checkNoEntry();
        return {};
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

inline auto ToExpr(OperandTokenType OperandTokenType, const OperandValue& Value) -> Expression {
  Expression Expr;
  Expr.Set<Operand>(Operand{OperandTokenType, Value});
  return Expr;
}
inline auto ToExpr(OperatorTokenType OperatorTokenType, const FString& Value, TArray<Expression> Operands)
    -> Expression {
  Expression Expr;
  Expr.Set<Operation>(Operation{OperatorTokenType, Value, Operands});
  return Expr;
}
inline auto ExpressionToString(const Expression& Expr) -> FString {
  return Visit(Overload{[](const Operand& Op) -> FString {
                          switch (Op.Type) {
                            case (OperandTokenType::Identifier): return Op.Value.Get<FString>();
                            case (OperandTokenType::Number): return FString::FromInt(Op.Value.Get<int32>());
                            case (OperandTokenType::String): return "'" + Op.Value.Get<FString>() + "'";
                            case (OperandTokenType::Boolean): return Op.Value.Get<bool>() ? "true" : "false";
                            default: return Op.Value.Get<FString>();
                          }
                        },
                        [](const Operation& Op) -> FString {
                          FString Res = "(" + Op.Value;
                          for (const auto& Operand : Op.Operands) Res += " " + ExpressionToString(Operand);
                          Res += ")";
                          return Res;
                        }},
               Expr);
};

auto ParseExpression(const TArray<Token>& Tokens, int32& Index, int32 MinBP) -> Expression {
  Token CurrentToken = Tokens[Index++];
  Expression Left = Visit(
      Overload{
          [&](OperatorTokenType Op) -> Expression {
            switch (Op) {
              case (OperatorTokenType::LeftParen):
              case (OperatorTokenType::LeftBrace): return ParseExpression(Tokens, Index, 0);
              // Array literal
              case (OperatorTokenType::LeftSquare): {
                TArray<Expression> Elements;
                while (Index < Tokens.Num()) {
                  Elements.Push(ParseExpression(Tokens, Index, 0));

                  // Explicitly check for closing square bracket to end array literal (closing bracket already consumed)
                  if (TokenIs(Tokens[Index - 1], OperatorTokenType::RightSquare)) break;
                  if (TokenIs(Tokens[Index], OperandTokenType::Eof) || TokenIs(Tokens[Index], OperandTokenType::Eol))
                    break;

                  Index++;
                }
                return ToExpr(Op, "array_literal", Elements);
              }
              // Unary
              case (OperatorTokenType::Not):
              case (OperatorTokenType::Plus):
              case (OperatorTokenType::Minus):
                return ToExpr(Op, CurrentToken.Value,
                              {ParseExpression(Tokens, Index, UNARY_OPERATOR_TOKEN_BINDINGS[Op])});
              default:
                UE_LOG(LogTemp, Error, TEXT("Unexpected operator: %d at index %d"), static_cast<int32>(Op), Index - 1);
                checkNoEntry();
                return ToExpr(Op, CurrentToken.Value, {});
            }
          },
          [&](OperandTokenType Op) -> Expression {
            OperandValue StrValue;
            StrValue.Set<FString>(CurrentToken.Value);
            switch (Op) {
              case (OperandTokenType::Identifier): return ToExpr(Op, StrValue);
              case (OperandTokenType::Number): {
                OperandValue NumValue;
                NumValue.Set<int32>(FCString::Atoi(*CurrentToken.Value));
                return ToExpr(Op, NumValue);
              }
              case (OperandTokenType::String): return ToExpr(Op, StrValue);
              case (OperandTokenType::Boolean): {
                OperandValue BoolValue;
                BoolValue.Set<bool>(CurrentToken.Value == "true");
                return ToExpr(Op, BoolValue);
              }
              case (OperandTokenType::PipeVar): return ToExpr(Op, StrValue);
              default:
                UE_LOG(LogTemp, Error, TEXT("Unexpected operand: %d at index %d"), static_cast<int32>(Op), Index - 1);
                checkNoEntry();
                return ToExpr(Op, StrValue);
            }
          }},
      CurrentToken.Type);

  while (Index < Tokens.Num()) {
    Token NextToken = Tokens[Index];

    if (TokenIs(NextToken, OperandTokenType::Eof) || TokenIs(NextToken, OperandTokenType::Eol)) break;

    // Function call when: next token is an operand, or an open parenthesis.
    if (NextToken.Type.IsType<OperandTokenType>() || (TokenIs(NextToken, OperatorTokenType::LeftParen))) {
      if (TokenIs(NextToken, OperatorTokenType::LeftParen)) {
        Index++;  // Consume the '('.
        if (TokenIs(Tokens[Index], OperatorTokenType::RightParen)) {
          Index++;  // Consume the ')'.
          continue;
        }
      }

      TArray<Expression> Args;
      while (Index < Tokens.Num()) {
        Args.Push(ParseExpression(Tokens, Index, 0));

        if (TokenIs(Tokens[Index], OperandTokenType::Eof) || TokenIs(Tokens[Index], OperandTokenType::Eol)) break;
        if (TokenIs(Tokens[Index], OperatorTokenType::Colon)) break;
        // Closing parenthesis are already consumed.
        if (TokenIs(Tokens[Index - 1], OperatorTokenType::RightParen) &&
            !TokenIs(Tokens[Index - 2], OperatorTokenType::LeftParen))
          break;

        Index++;
      }
      Left = ToExpr(OperatorTokenType::LeftParen, "call", {Left, {ToExpr(OperatorTokenType::Comma, "args", Args)}});

      continue;
    }

    if (!NextToken.Type.IsType<OperatorTokenType>()) break;

    OperatorTokenType NextOp = NextToken.Type.Get<OperatorTokenType>();
    // Comma handling, means it's a function argument separator.
    if (NextOp == OperatorTokenType::Comma) {
      // Index++;
      break;
    }
    // Closing parentheses/braces/brackets.
    if (NextOp == OperatorTokenType::RightParen || NextOp == OperatorTokenType::RightSquare ||
        NextOp == OperatorTokenType::RightBrace) {
      Index++;
      break;
    }
    // Ternary operator
    if (NextOp == OperatorTokenType::Question) {
      Index++;  // Consume the '?'
      Expression TrueExpr = ParseExpression(Tokens, Index, 0);
      Index++;  // Consume the ':'
      Expression FalseExpr = ParseExpression(Tokens, Index, 0);
      Left = ToExpr(NextOp, NextToken.Value, {Left, TrueExpr, FalseExpr});
      continue;
    }
    if (NextOp == OperatorTokenType::Colon) break;

    int32 NextBP = OPERATOR_TOKEN_BINDINGS.FindRef(NextOp, -1);
    if (NextBP < MinBP) break;

    Index++;
    Expression Right = ParseExpression(Tokens, Index, NextBP + 1);
    auto NewOp = ToExpr(NextOp, NextToken.Value, {Left, Right});
    Left = NewOp;
  };

  return Left;
};
inline auto ParseTokensToScript(const TArray<Token>& Tokens) -> Script {
  TArray<Expression> Expressions;
  int32 Index = 0;
  while (Index < Tokens.Num() && !TokenIs(Tokens[Index], OperandTokenType::Eof)) {
    if (TokenIs(Tokens[Index], OperandTokenType::Eol)) {
      Index++;
      continue;
    }
    Expressions.Push(ParseExpression(Tokens, Index, 0));
  }
  return Script{Expressions};
};

inline auto CompareVariants(const TVariant<FString, int32, float, bool, UObjectPtr, FStructProperty*>& A,
                            const TVariant<FString, int32, float, bool, UObjectPtr, FStructProperty*>& B) -> bool {
  if (A.GetIndex() != B.GetIndex()) return false;

  Visit(Overload{[&](const auto& ValA) {
          using T = std::decay_t<decltype(ValA)>;
          return ValA == B.Get<T>();
        }},
        A);
}

template <typename T>
inline auto ApplyUnaryOperation(OperatorTokenType Op, const T& Val) -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case OperatorTokenType::Not: Res.Set<T>(!Val); break;
    case OperatorTokenType::Plus: Res.Set<T>(Val); break;
    case OperatorTokenType::Minus: Res.Set<T>(-Val); break;
    default: {
      UE_LOG(LogTemp, Error, TEXT("Unsupported operator for unary operation"));
      checkNoEntry();
      return {};
    }
  }
  return Res;
}
template <>
inline auto ApplyUnaryOperation<FString>(OperatorTokenType Op, const FString& Val) -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case OperatorTokenType::Not: Res.Set<bool>(Val.IsEmpty()); break;
    default:
      UE_LOG(LogTemp, Error, TEXT("Unsupported operator for unary operation"));
      checkNoEntry();
      return {};
  }
  return Res;
}
template <>
inline auto ApplyUnaryOperation<bool>(OperatorTokenType Op, const bool& Val) -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case OperatorTokenType::Not: Res.Set<bool>(!Val); break;
    default:
      UE_LOG(LogTemp, Error, TEXT("Unsupported operator for unary operation"));
      checkNoEntry();
      return {};
  }
  return Res;
}
template <>
inline auto ApplyUnaryOperation<VSEvaluatedArray>(OperatorTokenType Op, const VSEvaluatedArray& Val)
    -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case OperatorTokenType::Not: Res.Set<bool>(Val.Num() == 0); break;
    default:
      UE_LOG(LogTemp, Error, TEXT("Unsupported operator for unary operation"));
      checkNoEntry();
      return {};
  }
  return Res;
}

template <typename T>
inline auto ApplyBinaryOperation(OperatorTokenType Op, const T& LeftVal, const T& RightVal) -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case OperatorTokenType::And: Res.Set<T>(LeftVal && RightVal); break;
    case OperatorTokenType::Or: Res.Set<T>(LeftVal || RightVal); break;
    case OperatorTokenType::Equal: Res.Set<T>(LeftVal == RightVal); break;
    case OperatorTokenType::NotEqual: Res.Set<T>(LeftVal != RightVal); break;
    case OperatorTokenType::Less: Res.Set<T>(LeftVal < RightVal); break;
    case OperatorTokenType::LessEqual: Res.Set<T>(LeftVal <= RightVal); break;
    case OperatorTokenType::Greater: Res.Set<T>(LeftVal > RightVal); break;
    case OperatorTokenType::GreaterEqual: Res.Set<T>(LeftVal >= RightVal); break;
    case OperatorTokenType::Plus: Res.Set<T>(LeftVal + RightVal); break;
    case OperatorTokenType::Minus: Res.Set<T>(LeftVal - RightVal); break;
    case OperatorTokenType::Times: Res.Set<T>(LeftVal * RightVal); break;
    case OperatorTokenType::Divide: {
      if (RightVal == 0) {
        UE_LOG(LogTemp, Error, TEXT("Division by zero"));
        checkNoEntry();
        return {};
      }
      Res.Set<T>(LeftVal / RightVal);
      break;
    }
    case OperatorTokenType::Dot: {
      if constexpr (TypeTests::TAreTypesEqual_V<T, int32>) {
        auto _Res = FString::FromInt(LeftVal) + FString::FromInt(RightVal);
        Res.Set<FString>(_Res);
        break;
      } else {
        UE_LOG(LogTemp, Error, TEXT("Unsupported operator for binary operation"));
        checkNoEntry();
        return {};
      }
    }
    default:
      UE_LOG(LogTemp, Error, TEXT("Unsupported operator for binary operation"));
      checkNoEntry();
      return {};
  }
  return Res;
}
template <>
inline auto ApplyBinaryOperation<FString>(OperatorTokenType Op, const FString& LeftVal, const FString& RightVal)
    -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case OperatorTokenType::Equal: Res.Set<bool>(LeftVal == RightVal); break;
    case OperatorTokenType::NotEqual: Res.Set<bool>(LeftVal != RightVal); break;
    case OperatorTokenType::Less: Res.Set<bool>(LeftVal < RightVal); break;
    case OperatorTokenType::LessEqual: Res.Set<bool>(LeftVal <= RightVal); break;
    case OperatorTokenType::Greater: Res.Set<bool>(LeftVal > RightVal); break;
    case OperatorTokenType::GreaterEqual: Res.Set<bool>(LeftVal >= RightVal); break;
    case OperatorTokenType::Plus: Res.Set<FString>(LeftVal + RightVal); break;
    default:
      UE_LOG(LogTemp, Error, TEXT("Unsupported operator for binary operation"));
      checkNoEntry();
      return {};
  }
  return Res;
}
template <>
inline auto ApplyBinaryOperation<bool>(OperatorTokenType Op, const bool& LeftVal, const bool& RightVal)
    -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case OperatorTokenType::And: Res.Set<bool>(LeftVal && RightVal); break;
    case OperatorTokenType::Or: Res.Set<bool>(LeftVal || RightVal); break;
    case OperatorTokenType::Equal: Res.Set<bool>(LeftVal == RightVal); break;
    case OperatorTokenType::NotEqual: Res.Set<bool>(LeftVal != RightVal); break;
    default:
      UE_LOG(LogTemp, Error, TEXT("Unsupported operator for binary operation"));
      checkNoEntry();
      return {};
  }
  return Res;
}
template <>
inline auto ApplyBinaryOperation<VSEvaluatedArray>(OperatorTokenType Op,
                                                   const VSEvaluatedArray& LeftVal,
                                                   const VSEvaluatedArray& RightVal) -> VSEvaluatedValue {
  VSEvaluatedValue Res;

  switch (Op) {
    case OperatorTokenType::Equal: {
      if (LeftVal.Num() != RightVal.Num()) {
        Res.Set<bool>(false);
        break;
      }

      for (int32 i = 0; i < LeftVal.Num(); ++i) {
        auto L = LeftVal[i];
        auto R = RightVal[i];
        Res.Set<bool>(CompareVariants(L, R));
      }
      break;
    }
    case OperatorTokenType::NotEqual: {
      if (LeftVal.Num() != RightVal.Num()) {
        Res.Set<bool>(true);
        break;
      }

      for (int32 i = 0; i < LeftVal.Num(); ++i) {
        auto L = LeftVal[i];
        auto R = RightVal[i];
        Res.Set<bool>(!CompareVariants(L, R));
      }
      break;
    }
    case OperatorTokenType::Plus: {
      VSEvaluatedArray Result = LeftVal;
      Result.Append(RightVal);
      Res.Set<VSEvaluatedArray>(Result);
      break;
    }
    case OperatorTokenType::Minus: {
      VSEvaluatedArray Result;
      for (const auto& LeftElem : LeftVal)
        if (!RightVal.ContainsByPredicate([&](const auto& RightElem) { return CompareVariants(LeftElem, RightElem); }))
          Result.Add(LeftElem);
      Res.Set<VSEvaluatedArray>(Result);
      break;
    }
    default:
      UE_LOG(LogTemp, Error, TEXT("Unsupported operator for binary operation"));
      checkNoEntry();
      return {};
  }
  return Res;
}
template <>
inline auto ApplyBinaryOperation<VSEvaluatedMap>(OperatorTokenType Op,
                                                 const VSEvaluatedMap& LeftVal,
                                                 const VSEvaluatedMap& RightVal) -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case OperatorTokenType::Equal: {
      for (const auto& [Key, LeftValue] : LeftVal) {
        if (!RightVal.Contains(Key)) {
          Res.Set<bool>(false);
          break;
        }
        const auto& RightValue = RightVal[Key];
        if (!CompareVariants(LeftValue, RightValue)) {
          Res.Set<bool>(false);
          break;
        }
      }
    }
    case OperatorTokenType::NotEqual: {
      for (const auto& [Key, LeftValue] : LeftVal) {
        if (!RightVal.Contains(Key)) {
          Res.Set<bool>(true);
          break;
        }
        const auto& RightValue = RightVal[Key];
        if (!CompareVariants(LeftValue, RightValue)) {
          Res.Set<bool>(true);
          break;
        }
      }
    }
    case OperatorTokenType::Plus: {
      VSEvaluatedMap Result = LeftVal;
      for (const auto& [Key, Value] : RightVal) Result.Add(Key, Value);
      Res.Set<VSEvaluatedMap>(Result);
      break;
    }
    case OperatorTokenType::Minus: {
      VSEvaluatedMap Result = LeftVal;
      for (const auto& [Key, _] : RightVal) Result.Remove(Key);
      Res.Set<VSEvaluatedMap>(Result);
      break;
    }
    default:
      UE_LOG(LogTemp, Error, TEXT("Unsupported operator for binary operation"));
      checkNoEntry();
      return {};
  }
  return Res;
}

auto TestScriptTreeGen(const FString& ScriptCode) -> FString {
  TArray<Token> Tokens = Tokenize(ScriptCode);
  Script ParsedScript = ParseTokensToScript(Tokens);

  for (const auto& Expr : ParsedScript.Expressions) {
    FString ExprStr = ExpressionToString(Expr);
    UE_LOG(LogTemp, Log, TEXT("Parsed Expression: %s"), *ExprStr);
  }
  return ExpressionToString(ParsedScript.Expressions.Last());
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
