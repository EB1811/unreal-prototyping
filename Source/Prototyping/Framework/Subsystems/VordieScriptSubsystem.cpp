#include "VordieScriptSubsystem.h"
#include "HAL/Platform.h"
#include "Logging/LogVerbosity.h"
#include "Misc/TVariant.h"

inline auto ToToken(VSOperatorTokenType Type, const FString& Value) -> Token {
  TVariant<VSOperatorTokenType, VSOperandTokenType> T;
  T.Set<VSOperatorTokenType>(Type);
  return {T, Value};
}
inline auto ToToken(VSOperandTokenType Type, const FString& Value) -> Token {
  TVariant<VSOperatorTokenType, VSOperandTokenType> T;
  T.Set<VSOperandTokenType>(Type);
  return {T, Value};
}
inline auto TokenIs(const Token& Token, VSOperatorTokenType OpType) -> bool {
  if (auto Operator = Token.Type.TryGet<VSOperatorTokenType>()) return *Operator == OpType;
  return false;
}
inline auto TokenIs(const Token& Token, VSOperandTokenType OpType) -> bool {
  if (auto VSOperand = Token.Type.TryGet<VSOperandTokenType>()) return *VSOperand == OpType;
  return false;
}

inline auto CanEndStatement(const Token& Token) -> bool {
  return TokenIs(Token, VSOperatorTokenType::RightBrace) || TokenIs(Token, VSOperatorTokenType::RightBrace) ||
         TokenIs(Token, VSOperatorTokenType::RightSquare);
}
inline auto CanStartStatement(const Token& Token) -> bool {
  return TokenIs(Token, VSOperatorTokenType::LeftParen) || TokenIs(Token, VSOperatorTokenType::LeftBrace) ||
         TokenIs(Token, VSOperatorTokenType::LeftSquare);
}
inline auto ShouldEndStatement(const FString& Code, int32 Index, TArray<Token>& Tokens, int32 BracketDepth) -> bool {
  if (Tokens.IsEmpty()) return false;
  if (TokenIs(Tokens.Last(), VSOperandTokenType::Eol)) return false;

  if (BracketDepth > 0) return false;
  if (Tokens.Last().Type.IsType<VSOperatorTokenType>() && !CanEndStatement(Tokens.Last())) return false;

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
        Tokens.Push(ToToken(VSOperandTokenType::Eol, "eol"));
      i++;
      continue;
    }

    if (isalpha(C) || C == '_') {
      int32 Start = i;
      while (i < Code.Len() && (isalnum(Code[i]) || Code[i] == '_')) i++;
      FString Identifier = Code.Mid(Start, i - Start);
      if (Identifier == "true" || Identifier == "false") Tokens.Push(ToToken(VSOperandTokenType::Boolean, Identifier));
      else Tokens.Push(ToToken(VSOperandTokenType::Identifier, Identifier));

      // Func() => Func
      if (i + 1 < Code.Len() && TokenIs(Tokens.Last(), VSOperandTokenType::Identifier) && Code[i] == '(' &&
          Code[i + 1] == ')') {
        i += 2;  // Skip the '()' after the identifier.
      }
    } else if (isdigit(C)) {
      int32 Start = i;
      while (i < Code.Len() && isdigit(Code[i])) i++;
      Tokens.Push(ToToken(VSOperandTokenType::Number, Code.Mid(Start, i - Start)));
    } else if (C == '\'') {
      int32 Start = ++i;
      while (i < Code.Len() && Code[i] != '\'') i++;
      Tokens.Push(ToToken(VSOperandTokenType::String, Code.Mid(Start, i - Start)));
      i++;
    } else if (C == '|') {
      if (i + 1 < Code.Len() && Code[i + 1] == '|') {
        Tokens.Push(ToToken(VSOperatorTokenType::Or, "||"));
        i += 2;
      } else {
        UE_LOG(LogTemp, Error, TEXT("Unknown character: %c"), C);
        checkNoEntry();
        return {};
      }
    } else if (C == '&') {
      if (i + 1 < Code.Len() && Code[i + 1] == '&') {
        Tokens.Push(ToToken(VSOperatorTokenType::And, "&&"));
        i += 2;
      } else {
        UE_LOG(LogTemp, Error, TEXT("Unknown character: %c"), C);
        checkNoEntry();
        return {};
      }
    } else if (C == '=') {
      if (i + 1 < Code.Len() && Code[i + 1] == '=') {
        Tokens.Push(ToToken(VSOperatorTokenType::Equal, "=="));
        i += 2;
      } else {
        UE_LOG(LogTemp, Error, TEXT("Unknown character: %c"), C);
        checkNoEntry();
        return {};
      }
    } else if (C == '!') {
      if (i + 1 < Code.Len() && Code[i + 1] == '=') {
        Tokens.Push(ToToken(VSOperatorTokenType::NotEqual, "!="));
        i += 2;
      } else {
        Tokens.Push(ToToken(VSOperatorTokenType::Not, "!"));
        i++;
      }
    } else if (C == '>') {
      if (i + 1 < Code.Len() && Code[i + 1] == '=') {
        Tokens.Push(ToToken(VSOperatorTokenType::GreaterEqual, ">="));
        i += 2;
      } else {
        Tokens.Push(ToToken(VSOperatorTokenType::Greater, ">"));
        i++;
      }
    } else if (C == '<') {
      if (i + 1 < Code.Len() && Code[i + 1] == '=') {
        Tokens.Push(ToToken(VSOperatorTokenType::LessEqual, "<="));
        i += 2;
      } else {
        Tokens.Push(ToToken(VSOperatorTokenType::Less, "<"));
        i++;
      }
    } else if (C == '~') {
      if (i + 1 < Code.Len() && Code[i + 1] == '>') {
        Tokens.Push(ToToken(VSOperatorTokenType::Pipe, "~>"));
        i += 2;
      } else {
        UE_LOG(LogTemp, Error, TEXT("Unknown character: %c"), C);
        checkNoEntry();
      }
    } else {
      switch (C) {
        case '(': Tokens.Push(ToToken(VSOperatorTokenType::LeftParen, "(")); break;
        case ')': Tokens.Push(ToToken(VSOperatorTokenType::RightParen, ")")); break;
        case '{': Tokens.Push(ToToken(VSOperatorTokenType::LeftBrace, "{")); break;
        case '}': Tokens.Push(ToToken(VSOperatorTokenType::RightBrace, "}")); break;
        case '[': Tokens.Push(ToToken(VSOperatorTokenType::LeftSquare, "[")); break;
        case ']': Tokens.Push(ToToken(VSOperatorTokenType::RightSquare, "]")); break;
        case ',': Tokens.Push(ToToken(VSOperatorTokenType::Comma, ",")); break;
        case '.': Tokens.Push(ToToken(VSOperatorTokenType::Dot, ".")); break;
        case '?': Tokens.Push(ToToken(VSOperatorTokenType::Question, "?")); break;
        case ':': Tokens.Push(ToToken(VSOperatorTokenType::Colon, ":")); break;
        case '^': Tokens.Push(ToToken(VSOperandTokenType::PipeVar, "^")); break;
        case '*': Tokens.Push(ToToken(VSOperatorTokenType::Times, "*")); break;
        case '/': Tokens.Push(ToToken(VSOperatorTokenType::Divide, "/")); break;
        case '+': Tokens.Push(ToToken(VSOperatorTokenType::Plus, "+")); break;
        case '-': Tokens.Push(ToToken(VSOperatorTokenType::Minus, "-")); break;
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
  Tokens.Push(ToToken(VSOperandTokenType::Eof, ""));
  return Tokens;
};

static const TMap<VSOperatorTokenType, int32> OPERATOR_TOKEN_BINDINGS = {
    {VSOperatorTokenType::Or, 5},         {VSOperatorTokenType::And, 5},        {VSOperatorTokenType::Pipe, 1},
    {VSOperatorTokenType::Equal, 10},     {VSOperatorTokenType::NotEqual, 10},  {VSOperatorTokenType::Less, 10},
    {VSOperatorTokenType::LessEqual, 10}, {VSOperatorTokenType::Greater, 10},   {VSOperatorTokenType::GreaterEqual, 10},
    {VSOperatorTokenType::Plus, 20},      {VSOperatorTokenType::Minus, 20},     {VSOperatorTokenType::Times, 30},
    {VSOperatorTokenType::Divide, 30},    {VSOperatorTokenType::LeftParen, 50}, {VSOperatorTokenType::LeftSquare, 50},
    {VSOperatorTokenType::LeftBrace, 50}, {VSOperatorTokenType::Question, 7},   {VSOperatorTokenType::Dot, 55},
};
static const TMap<VSOperatorTokenType, int32> UNARY_OPERATOR_TOKEN_BINDINGS = {
    {VSOperatorTokenType::Not, 50},
    {VSOperatorTokenType::Plus, 50},
    {VSOperatorTokenType::Minus, 50},
};

// Utility to allow overloading lambdas for use in TVariant::Visit
template <typename... Ts>
struct Overload : Ts... {
  using Ts::operator()...;
};
template <typename... Ts>
Overload(Ts...) -> Overload<Ts...>;

inline auto ToExpr(VSOperandTokenType VSOperandTokenType, const VSOperandValue& Value) -> VSExpression {
  VSExpression Expr;
  Expr.Set<VSOperand>(VSOperand{VSOperandTokenType, Value});
  return Expr;
}
inline auto ToExpr(VSOperatorTokenType VSOperatorTokenType, const FString& Value, TArray<VSExpression> Operands)
    -> VSExpression {
  VSExpression Expr;
  Expr.Set<VSOperation>(VSOperation{VSOperatorTokenType, Value, Operands});
  return Expr;
}
inline auto ExpressionToString(const VSExpression& Expr) -> FString {
  return Visit(Overload{[](const VSOperand& Op) -> FString {
                          switch (Op.Type) {
                            case (VSOperandTokenType::Identifier): return Op.Value.Get<FString>();
                            case (VSOperandTokenType::Number): return FString::FromInt(Op.Value.Get<int32>());
                            case (VSOperandTokenType::String): return "'" + Op.Value.Get<FString>() + "'";
                            case (VSOperandTokenType::Boolean): return Op.Value.Get<bool>() ? "true" : "false";
                            default: return Op.Value.Get<FString>();
                          }
                        },
                        [](const VSOperation& Op) -> FString {
                          FString Res = "(" + Op.Value;
                          for (const auto& VSOperand : Op.Operands) Res += " " + ExpressionToString(VSOperand);
                          Res += ")";
                          return Res;
                        }},
               Expr);
};

auto ParseExpression(const TArray<Token>& Tokens, int32& Index, int32 MinBP) -> VSExpression {
  Token CurrentToken = Tokens[Index++];
  VSExpression Left = Visit(
      Overload{
          [&](VSOperatorTokenType Op) -> VSExpression {
            switch (Op) {
              case (VSOperatorTokenType::LeftParen):
              case (VSOperatorTokenType::LeftBrace): return ParseExpression(Tokens, Index, 0);
              // Array literal
              case (VSOperatorTokenType::LeftSquare): {
                TArray<VSExpression> Elements;
                while (Index < Tokens.Num()) {
                  Elements.Push(ParseExpression(Tokens, Index, 0));

                  // Explicitly check for closing square bracket to end array literal (closing bracket already consumed)
                  if (TokenIs(Tokens[Index - 1], VSOperatorTokenType::RightSquare)) break;
                  if (TokenIs(Tokens[Index], VSOperandTokenType::Eof) ||
                      TokenIs(Tokens[Index], VSOperandTokenType::Eol))
                    break;

                  Index++;
                }
                return ToExpr(Op, "array_literal", Elements);
              }
              // Unary
              case (VSOperatorTokenType::Not):
              case (VSOperatorTokenType::Plus):
              case (VSOperatorTokenType::Minus):
                return ToExpr(Op, CurrentToken.Value,
                              {ParseExpression(Tokens, Index, UNARY_OPERATOR_TOKEN_BINDINGS[Op])});
              default:
                UE_LOG(LogTemp, Error, TEXT("Unexpected operator: %d at index %d"), static_cast<int32>(Op), Index - 1);
                checkNoEntry();
                return ToExpr(Op, CurrentToken.Value, {});
            }
          },
          [&](VSOperandTokenType Op) -> VSExpression {
            VSOperandValue StrValue;
            StrValue.Set<FString>(CurrentToken.Value);
            switch (Op) {
              case (VSOperandTokenType::Identifier): return ToExpr(Op, StrValue);
              case (VSOperandTokenType::Number): {
                VSOperandValue NumValue;
                NumValue.Set<int32>(FCString::Atoi(*CurrentToken.Value));
                return ToExpr(Op, NumValue);
              }
              case (VSOperandTokenType::String): return ToExpr(Op, StrValue);
              case (VSOperandTokenType::Boolean): {
                VSOperandValue BoolValue;
                BoolValue.Set<bool>(CurrentToken.Value == "true");
                return ToExpr(Op, BoolValue);
              }
              case (VSOperandTokenType::PipeVar): return ToExpr(Op, StrValue);
              default:
                UE_LOG(LogTemp, Error, TEXT("Unexpected operand: %d at index %d"), static_cast<int32>(Op), Index - 1);
                checkNoEntry();
                return ToExpr(Op, StrValue);
            }
          }},
      CurrentToken.Type);

  while (Index < Tokens.Num()) {
    Token NextToken = Tokens[Index];

    if (TokenIs(NextToken, VSOperandTokenType::Eof) || TokenIs(NextToken, VSOperandTokenType::Eol)) break;

    // Function call when: next token is an operand, or an open parenthesis.
    if (NextToken.Type.IsType<VSOperandTokenType>() || (TokenIs(NextToken, VSOperatorTokenType::LeftParen))) {
      if (TokenIs(NextToken, VSOperatorTokenType::LeftParen)) {
        Index++;  // Consume the '('.
        if (TokenIs(Tokens[Index], VSOperatorTokenType::RightParen)) {
          Index++;  // Consume the ')'.
          continue;
        }
      }

      TArray<VSExpression> Args;
      while (Index < Tokens.Num()) {
        Args.Push(ParseExpression(Tokens, Index, 0));

        if (TokenIs(Tokens[Index], VSOperandTokenType::Eof) || TokenIs(Tokens[Index], VSOperandTokenType::Eol)) break;
        if (TokenIs(Tokens[Index], VSOperatorTokenType::Colon)) break;
        // Closing parenthesis are already consumed.
        if (TokenIs(Tokens[Index - 1], VSOperatorTokenType::RightParen) &&
            !TokenIs(Tokens[Index - 2], VSOperatorTokenType::LeftParen))
          break;

        Index++;
      }
      Left = ToExpr(VSOperatorTokenType::LeftParen, "call", {Left, {ToExpr(VSOperatorTokenType::Comma, "args", Args)}});

      continue;
    }

    if (!NextToken.Type.IsType<VSOperatorTokenType>()) break;

    VSOperatorTokenType NextOp = NextToken.Type.Get<VSOperatorTokenType>();
    // Comma handling, means it's a function argument separator.
    if (NextOp == VSOperatorTokenType::Comma) {
      // Index++;
      break;
    }
    // Closing parentheses/braces/brackets.
    if (NextOp == VSOperatorTokenType::RightParen || NextOp == VSOperatorTokenType::RightSquare ||
        NextOp == VSOperatorTokenType::RightBrace) {
      Index++;
      break;
    }
    // Ternary operator
    if (NextOp == VSOperatorTokenType::Question) {
      Index++;  // Consume the '?'
      VSExpression TrueExpr = ParseExpression(Tokens, Index, 0);
      Index++;  // Consume the ':'
      VSExpression FalseExpr = ParseExpression(Tokens, Index, 0);
      Left = ToExpr(NextOp, NextToken.Value, {Left, TrueExpr, FalseExpr});
      continue;
    }
    if (NextOp == VSOperatorTokenType::Colon) break;
    // Member access operator. Only bind the immediate identifier.
    if (NextOp == VSOperatorTokenType::Dot) {
      Index++;  // Consume the '.'
      Token MemberToken = Tokens[Index++];

      VSOperandValue MemberValue;
      if (TokenIs(MemberToken, VSOperandTokenType::Identifier)) MemberValue.Set<FString>(MemberToken.Value);
      else if (TokenIs(MemberToken, VSOperandTokenType::Number))
        MemberValue.Set<int32>(FCString::Atoi(*MemberToken.Value));
      else {
        UE_LOG(LogTemp, Error, TEXT("Unexpected member token: %d at index %d"),
               static_cast<int32>(MemberToken.Type.Get<VSOperandTokenType>()), Index - 1);
        checkNoEntry();
        MemberValue.Set<FString>(MemberToken.Value);
      }

      VSExpression Right = ToExpr(MemberToken.Type.Get<VSOperandTokenType>(), MemberValue);
      Left = ToExpr(NextOp, NextToken.Value, {Left, Right});
      continue;
    }

    int32 NextBP = OPERATOR_TOKEN_BINDINGS.FindRef(NextOp, -1);
    if (NextBP < MinBP) break;

    Index++;
    VSExpression Right = ParseExpression(Tokens, Index, NextBP + 1);
    auto NewOp = ToExpr(NextOp, NextToken.Value, {Left, Right});
    Left = NewOp;
  };

  return Left;
};
inline auto ParseTokensToScript(const TArray<Token>& Tokens) -> Script {
  TArray<VSExpression> Expressions;
  int32 Index = 0;
  while (Index < Tokens.Num() && !TokenIs(Tokens[Index], VSOperandTokenType::Eof)) {
    if (TokenIs(Tokens[Index], VSOperandTokenType::Eol)) {
      Index++;
      continue;
    }
    Expressions.Push(ParseExpression(Tokens, Index, 0));
  }
  return Script{Expressions};
};

// Helper functions.
inline auto CompareVariants(const VSContainerable& A, const VSContainerable& B) -> bool {
  if (A.GetIndex() != B.GetIndex()) return false;

  Visit(Overload{[&](const auto& ValA) {
          using T = typename TDecay<decltype(ValA)>::Type;
          return ValA == B.Get<T>();
        }},
        A);
}
// template <typename T>
// concept VSEvaluatedValueTypes =
//     TypeTests::TAreTypesEqual_V<T, FString> || TypeTests::TAreTypesEqual_V<T, int32> ||
//     TypeTests::TAreTypesEqual_V<T, float> || TypeTests::TAreTypesEqual_V<T, bool> ||
//     TypeTests::TAreTypesEqual_V<T, UObjectPtr> || TypeTests::TAreTypesEqual_V<T, FStructProperty*> ||
//     TypeTests::TAreTypesEqual_V<T, VSEvaluatedArray> || TypeTests::TAreTypesEqual_V<T, VSEvaluatedMap>;
template <typename T>
inline auto ToVSEvaluatedValue(T Val) -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  Res.Set<T>(Val);
  return Res;
}

template <typename T>
inline auto UnaryOperations(TMap<VSOperatorTokenType, TFunction<VSEvaluatedValue(const T&)>> UnaryOps,
                            VSOperatorTokenType Op,
                            const T& Val) -> VSEvaluatedValue {
  if (UnaryOps.Contains(Op)) {
    return UnaryOps[Op](Val);
  } else {
    UE_LOG(LogTemp, Error, TEXT("Unsupported operator for unary operation"));
    checkNoEntry();
    return {};
  }
}
template <typename T>
inline auto ApplyUnaryOperation(VSOperatorTokenType Op, const T& Val) -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case VSOperatorTokenType::Not: Res.Set<T>(!Val); break;
    case VSOperatorTokenType::Plus: Res.Set<T>(Val); break;
    case VSOperatorTokenType::Minus: Res.Set<T>(-Val); break;
    default: {
      UE_LOG(LogTemp, Error, TEXT("Unsupported operator for unary operation"));
      checkNoEntry();
      return {};
    }
  }
  return Res;
}
template <>
inline auto ApplyUnaryOperation<FString>(VSOperatorTokenType Op, const FString& Val) -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case VSOperatorTokenType::Not: Res.Set<bool>(Val.IsEmpty()); break;
    default:
      UE_LOG(LogTemp, Error, TEXT("Unsupported operator for unary operation"));
      checkNoEntry();
      return {};
  }
  return Res;
}
template <>
inline auto ApplyUnaryOperation<bool>(VSOperatorTokenType Op, const bool& Val) -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case VSOperatorTokenType::Not: Res.Set<bool>(!Val); break;
    default:
      UE_LOG(LogTemp, Error, TEXT("Unsupported operator for unary operation"));
      checkNoEntry();
      return {};
  }
  return Res;
}
template <>
inline auto ApplyUnaryOperation<UObjectPtr>(VSOperatorTokenType Op, const UObjectPtr& Val) -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case VSOperatorTokenType::Not: Res.Set<bool>(!Val.IsValid()); break;
    default:
      UE_LOG(LogTemp, Error, TEXT("Unsupported operator for unary operation"));
      checkNoEntry();
      return {};
  }
  return Res;
}
template <>
inline auto ApplyUnaryOperation<VSEvaluatedArray>(VSOperatorTokenType Op, const VSEvaluatedArray& Val)
    -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case VSOperatorTokenType::Not: Res.Set<bool>(Val.Num() == 0); break;
    default:
      UE_LOG(LogTemp, Error, TEXT("Unsupported operator for unary operation"));
      checkNoEntry();
      return {};
  }
  return Res;
}
template <>
inline auto ApplyUnaryOperation<VSEvaluatedMap>(VSOperatorTokenType Op, const VSEvaluatedMap& Val) -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case VSOperatorTokenType::Not: Res.Set<bool>(Val.Num() == 0); break;
    default:
      UE_LOG(LogTemp, Error, TEXT("Unsupported operator for unary operation"));
      checkNoEntry();
      return {};
  }
  return Res;
}

template <typename T>
inline auto ApplyBinaryOperation(VSOperatorTokenType Op, const T& LeftVal, const T& RightVal) -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case VSOperatorTokenType::And: Res.Set<T>(LeftVal && RightVal); break;
    case VSOperatorTokenType::Or: Res.Set<T>(LeftVal || RightVal); break;
    case VSOperatorTokenType::Equal: Res.Set<T>(LeftVal == RightVal); break;
    case VSOperatorTokenType::NotEqual: Res.Set<T>(LeftVal != RightVal); break;
    case VSOperatorTokenType::Less: Res.Set<T>(LeftVal < RightVal); break;
    case VSOperatorTokenType::LessEqual: Res.Set<T>(LeftVal <= RightVal); break;
    case VSOperatorTokenType::Greater: Res.Set<T>(LeftVal > RightVal); break;
    case VSOperatorTokenType::GreaterEqual: Res.Set<T>(LeftVal >= RightVal); break;
    case VSOperatorTokenType::Plus: Res.Set<T>(LeftVal + RightVal); break;
    case VSOperatorTokenType::Minus: Res.Set<T>(LeftVal - RightVal); break;
    case VSOperatorTokenType::Times: Res.Set<T>(LeftVal * RightVal); break;
    case VSOperatorTokenType::Divide: {
      if (RightVal == 0) {
        UE_LOG(LogTemp, Error, TEXT("Division by zero"));
        checkNoEntry();
        return {};
      }
      Res.Set<T>(LeftVal / RightVal);
      break;
    }
    case VSOperatorTokenType::Dot: {
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
inline auto ApplyBinaryOperation<FString>(VSOperatorTokenType Op, const FString& LeftVal, const FString& RightVal)
    -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case VSOperatorTokenType::Equal: Res.Set<bool>(LeftVal == RightVal); break;
    case VSOperatorTokenType::NotEqual: Res.Set<bool>(LeftVal != RightVal); break;
    case VSOperatorTokenType::Less: Res.Set<bool>(LeftVal < RightVal); break;
    case VSOperatorTokenType::LessEqual: Res.Set<bool>(LeftVal <= RightVal); break;
    case VSOperatorTokenType::Greater: Res.Set<bool>(LeftVal > RightVal); break;
    case VSOperatorTokenType::GreaterEqual: Res.Set<bool>(LeftVal >= RightVal); break;
    case VSOperatorTokenType::Plus: Res.Set<FString>(LeftVal + RightVal); break;
    default:
      UE_LOG(LogTemp, Error, TEXT("Unsupported operator for binary operation"));
      checkNoEntry();
      return {};
  }
  return Res;
}
template <>
inline auto ApplyBinaryOperation<bool>(VSOperatorTokenType Op, const bool& LeftVal, const bool& RightVal)
    -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case VSOperatorTokenType::And: Res.Set<bool>(LeftVal && RightVal); break;
    case VSOperatorTokenType::Or: Res.Set<bool>(LeftVal || RightVal); break;
    case VSOperatorTokenType::Equal: Res.Set<bool>(LeftVal == RightVal); break;
    case VSOperatorTokenType::NotEqual: Res.Set<bool>(LeftVal != RightVal); break;
    default:
      UE_LOG(LogTemp, Error, TEXT("Unsupported operator for binary operation"));
      checkNoEntry();
      return {};
  }
  return Res;
}
template <>
inline auto ApplyBinaryOperation<UObjectPtr>(VSOperatorTokenType Op,
                                             const UObjectPtr& LeftVal,
                                             const UObjectPtr& RightVal) -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case VSOperatorTokenType::And: Res.Set<bool>(LeftVal.IsValid() && RightVal.IsValid()); break;
    case VSOperatorTokenType::Or: Res.Set<bool>(LeftVal.IsValid() || RightVal.IsValid()); break;
    case VSOperatorTokenType::Equal: Res.Set<bool>(LeftVal == RightVal); break;
    case VSOperatorTokenType::NotEqual: Res.Set<bool>(LeftVal != RightVal); break;
    default:
      UE_LOG(LogTemp, Error, TEXT("Unsupported operator for binary operation"));
      checkNoEntry();
      return {};
  }
  return Res;
}
template <>
inline auto ApplyBinaryOperation<VSEvaluatedArray>(VSOperatorTokenType Op,
                                                   const VSEvaluatedArray& LeftVal,
                                                   const VSEvaluatedArray& RightVal) -> VSEvaluatedValue {
  VSEvaluatedValue Res;

  switch (Op) {
    case VSOperatorTokenType::Equal: {
      if (LeftVal.Num() != RightVal.Num()) {
        Res.Set<bool>(false);
        break;
      }

      for (int32 i = 0; i < LeftVal.Num(); ++i) Res.Set<bool>(CompareVariants(LeftVal[i], RightVal[i]));
      break;
    }
    case VSOperatorTokenType::NotEqual: {
      if (LeftVal.Num() != RightVal.Num()) {
        Res.Set<bool>(true);
        break;
      }

      for (int32 i = 0; i < LeftVal.Num(); ++i) Res.Set<bool>(!CompareVariants(LeftVal[i], RightVal[i]));
      break;
    }
    case VSOperatorTokenType::Plus: {
      VSEvaluatedArray Result = LeftVal;
      Result.Append(RightVal);
      Res.Set<VSEvaluatedArray>(Result);
      break;
    }
    case VSOperatorTokenType::Minus: {
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
inline auto ApplyBinaryOperation<VSEvaluatedMap>(VSOperatorTokenType Op,
                                                 const VSEvaluatedMap& LeftVal,
                                                 const VSEvaluatedMap& RightVal) -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case VSOperatorTokenType::Equal: {
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
    case VSOperatorTokenType::NotEqual: {
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
    case VSOperatorTokenType::Plus: {
      VSEvaluatedMap Result = LeftVal;
      for (const auto& [Key, Value] : RightVal) Result.Add(Key, Value);
      Res.Set<VSEvaluatedMap>(Result);
      break;
    }
    case VSOperatorTokenType::Minus: {
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

// Evaluation functions.
auto UVordieScriptSubsystem::EvaluateOperand(const VSOperand& Op) -> VSEvaluatedValue {
  switch (Op.Type) {
    case VSOperandTokenType::Identifier: {
      FString IdentifierName = Op.Value.Get<FString>();
      if (!GlobalEnviroment.Contains(FName(*IdentifierName))) {
        UE_LOG(LogTemp, Error, TEXT("Undefined identifier: %s"), *IdentifierName);
        checkNoEntry();
        return {};
      }
    }
    case VSOperandTokenType::Number: return ToVSEvaluatedValue<int32>(Op.Value.Get<int32>());
    case VSOperandTokenType::String: return ToVSEvaluatedValue<FString>(Op.Value.Get<FString>());
    case VSOperandTokenType::Boolean: return ToVSEvaluatedValue<bool>(Op.Value.Get<bool>());
    default:
      UE_LOG(LogTemp, Error, TEXT("Unsupported operand type for evaluation"));
      checkNoEntry();
      return {};
  }
}

auto TestScriptTreeGen(const FString& ScriptCode) -> FString {
  TArray<Token> Tokens = Tokenize(ScriptCode);
  Script ParsedScript = ParseTokensToScript(Tokens);

  for (const auto& Expr : ParsedScript.Expressions) {
    FString ExprStr = ExpressionToString(Expr);
    UE_LOG(LogTemp, Log, TEXT("Parsed VSExpression: %s"), *ExprStr);
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