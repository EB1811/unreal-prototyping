#include "VordieScriptSubsystem.h"
#include "Containers/UnrealString.h"
#include "HAL/Platform.h"
#include "Logging/LogVerbosity.h"
#include "Misc/AssertionMacros.h"
#include "Misc/TVariant.h"
#include "UObject/UnrealType.h"

template <typename T>
inline auto EnsureReturn(const FString& Message) -> T {
  ensureAlwaysMsgf(false, TEXT("%s"), *Message);
  return {};
}

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
        return EnsureReturn<TArray<Token>>(FString::Printf(TEXT("Unknown character: %c"), C));
      }
    } else if (C == '&') {
      if (i + 1 < Code.Len() && Code[i + 1] == '&') {
        Tokens.Push(ToToken(VSOperatorTokenType::And, "&&"));
        i += 2;
      } else {
        return EnsureReturn<TArray<Token>>(FString::Printf(TEXT("Unknown character: %c"), C));
      }
    } else if (C == '=') {
      if (i + 1 < Code.Len() && Code[i + 1] == '=') {
        Tokens.Push(ToToken(VSOperatorTokenType::Equal, "=="));
        i += 2;
      } else {
        return EnsureReturn<TArray<Token>>(FString::Printf(TEXT("Unknown character: %c"), C));
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
        return EnsureReturn<TArray<Token>>(FString::Printf(TEXT("Unknown character: %c"), C));
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
        default: EnsureReturn<TArray<Token>>(FString::Printf(TEXT("Unknown character: %c"), C)); break;
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
  VSExpression Left =
      Visit(Overload{[&](VSOperatorTokenType Op) -> VSExpression {
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
                         default: {
                           return EnsureReturn<VSExpression>(FString::Printf(
                               TEXT("Unexpected operator: %d at index %d"), static_cast<int32>(Op), Index - 1));
                         }
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
                           return EnsureReturn<VSExpression>(FString::Printf(TEXT("Unexpected operand: %d at index %d"),
                                                                             static_cast<int32>(Op), Index - 1));
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
        return EnsureReturn<VSExpression>(FString::Printf(TEXT("Unexpected member access token: %d at index %d"),
                                                          static_cast<int32>(MemberToken.Type.GetIndex()), Index - 1));
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
inline auto ParseTokensToScript(const TArray<Token>& Tokens) -> VSScript {
  TArray<VSExpression> Expressions;
  int32 Index = 0;
  while (Index < Tokens.Num() && !TokenIs(Tokens[Index], VSOperandTokenType::Eof)) {
    if (TokenIs(Tokens[Index], VSOperandTokenType::Eol)) {
      Index++;
      continue;
    }
    Expressions.Push(ParseExpression(Tokens, Index, 0));
  }
  return VSScript{Expressions};
};

inline auto CompareVariants(const VSContainerable& A, const VSContainerable& B) -> bool {
  if (A.GetIndex() != B.GetIndex()) return false;

  Visit(Overload{[&](const auto& ValA) {
          using T = typename TDecay<decltype(ValA)>::Type;
          return ValA == B.Get<T>();
        }},
        A);
  return true;
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
inline auto ApplyUnaryOperation(VSOperatorTokenType Op, const T& Val) -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case VSOperatorTokenType::Not: Res.Set<bool>(!Val); break;
    case VSOperatorTokenType::Plus: Res.Set<T>(Val); break;
    case VSOperatorTokenType::Minus: Res.Set<T>(-Val); break;
    default: return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for unary operation"));
  }
  return Res;
}
template <>
inline auto ApplyUnaryOperation<FString>(VSOperatorTokenType Op, const FString& Val) -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case VSOperatorTokenType::Not: Res.Set<bool>(Val.IsEmpty()); break;
    default: return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for unary operation"));
  }
  return Res;
}
template <>
inline auto ApplyUnaryOperation<bool>(VSOperatorTokenType Op, const bool& Val) -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case VSOperatorTokenType::Not: Res.Set<bool>(!Val); break;
    default: return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for unary operation"));
  }
  return Res;
}
template <>
inline auto ApplyUnaryOperation<UObjectPtr>(VSOperatorTokenType Op, const UObjectPtr& Val) -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case VSOperatorTokenType::Not: Res.Set<bool>(!Val.IsValid()); break;
    default: return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for unary operation"));
  }
  return Res;
}
template <>
inline auto ApplyUnaryOperation<FStructProperty*>(VSOperatorTokenType Op, FStructProperty* const& Val)
    -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case VSOperatorTokenType::Not: Res.Set<bool>(Val == nullptr); break;
    default: return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for unary operation"));
  }
  return Res;
}
template <>
inline auto ApplyUnaryOperation<VSEvaluatedArray>(VSOperatorTokenType Op, const VSEvaluatedArray& Val)
    -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case VSOperatorTokenType::Not: Res.Set<bool>(Val.Num() == 0); break;
    default: return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for unary operation"));
  }
  return Res;
}
template <>
inline auto ApplyUnaryOperation<VSEvaluatedMap>(VSOperatorTokenType Op, const VSEvaluatedMap& Val) -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case VSOperatorTokenType::Not: Res.Set<bool>(Val.Num() == 0); break;
    default: return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for unary operation"));
  }
  return Res;
}

template <typename T>
inline auto ApplyBinaryOperation(VSOperatorTokenType Op, const T& LeftVal, const T& RightVal) -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case VSOperatorTokenType::And: Res.Set<bool>(LeftVal && RightVal); break;
    case VSOperatorTokenType::Or: Res.Set<bool>(LeftVal || RightVal); break;
    case VSOperatorTokenType::Equal: Res.Set<bool>(LeftVal == RightVal); break;
    case VSOperatorTokenType::NotEqual: Res.Set<bool>(LeftVal != RightVal); break;
    case VSOperatorTokenType::Less: Res.Set<bool>(LeftVal < RightVal); break;
    case VSOperatorTokenType::LessEqual: Res.Set<bool>(LeftVal <= RightVal); break;
    case VSOperatorTokenType::Greater: Res.Set<bool>(LeftVal > RightVal); break;
    case VSOperatorTokenType::GreaterEqual: Res.Set<bool>(LeftVal >= RightVal); break;
    case VSOperatorTokenType::Plus: Res.Set<T>(LeftVal + RightVal); break;
    case VSOperatorTokenType::Minus: Res.Set<T>(LeftVal - RightVal); break;
    case VSOperatorTokenType::Times: Res.Set<T>(LeftVal * RightVal); break;
    case VSOperatorTokenType::Divide: {
      if (RightVal == 0) return EnsureReturn<VSEvaluatedValue>(TEXT("Division by zero error"));
      Res.Set<T>(LeftVal / RightVal);
      break;
    }
    case VSOperatorTokenType::Dot: {
      if constexpr (TypeTests::TAreTypesEqual_V<T, int32>) {
        auto _Res = FString::FromInt(LeftVal) + "." + FString::FromInt(RightVal);
        Res.Set<float>(FCString::Atof(*_Res));
        break;
      } else {
        return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for binary operation"));
      }
    }
    default: return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for binary operation"));
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
    default: return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for binary operation"));
  }
  return Res;
}
template <>
inline auto ApplyBinaryOperation<float>(VSOperatorTokenType Op, const float& LeftVal, const float& RightVal)
    -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case VSOperatorTokenType::And: Res.Set<bool>(LeftVal && RightVal); break;
    case VSOperatorTokenType::Or: Res.Set<bool>(LeftVal || RightVal); break;
    case VSOperatorTokenType::Equal: Res.Set<bool>(FMath::IsNearlyEqual(LeftVal, RightVal)); break;
    case VSOperatorTokenType::NotEqual: Res.Set<bool>(!FMath::IsNearlyEqual(LeftVal, RightVal)); break;
    case VSOperatorTokenType::Less: Res.Set<bool>(LeftVal < RightVal); break;
    case VSOperatorTokenType::LessEqual:
      Res.Set<bool>(LeftVal < RightVal || FMath::IsNearlyEqual(LeftVal, RightVal));
      break;
    case VSOperatorTokenType::Greater: Res.Set<bool>(LeftVal > RightVal); break;
    case VSOperatorTokenType::GreaterEqual:
      Res.Set<bool>(LeftVal > RightVal || FMath::IsNearlyEqual(LeftVal, RightVal));
      break;
    case VSOperatorTokenType::Plus: Res.Set<float>(LeftVal + RightVal); break;
    case VSOperatorTokenType::Minus: Res.Set<float>(LeftVal - RightVal); break;
    case VSOperatorTokenType::Times: Res.Set<float>(LeftVal * RightVal); break;
    case VSOperatorTokenType::Divide: {
      if (RightVal == 0.0f) return EnsureReturn<VSEvaluatedValue>(TEXT("Division by zero error"));
      Res.Set<float>(LeftVal / RightVal);
      break;
    }
    default: return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for binary operation"));
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
    default: return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for binary operation"));
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
    default: return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for binary operation"));
  }
  return Res;
}
template <>
inline auto ApplyBinaryOperation<FStructProperty*>(VSOperatorTokenType Op,
                                                   FStructProperty* const& LeftVal,
                                                   FStructProperty* const& RightVal) -> VSEvaluatedValue {
  VSEvaluatedValue Res;
  switch (Op) {
    case VSOperatorTokenType::Equal: Res.Set<bool>(LeftVal == RightVal); break;
    case VSOperatorTokenType::NotEqual: Res.Set<bool>(LeftVal != RightVal); break;
    default: return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for binary operation"));
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
    default: return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for binary operation"));
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
    default: return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for binary operation"));
  }
  return Res;
}
template <typename T, typename U>
inline auto ApplyBinaryOperation(VSOperatorTokenType Op, const T& LeftVal, const U& RightVal) -> VSEvaluatedValue {
  return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for binary operation with different types"));
}
template <>
inline auto ApplyBinaryOperation<FString, int32>(VSOperatorTokenType Op, const FString& LeftVal, const int32& RightVal)
    -> VSEvaluatedValue {
  if (Op == VSOperatorTokenType::Dot) {
    auto _Res = LeftVal + "." + FString::FromInt(RightVal);
    return ToVSEvaluatedValue(FCString::Atof(*_Res));
  }
  return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for binary operation with different types"));
}
template <>
inline auto ApplyBinaryOperation<int32, FString>(VSOperatorTokenType Op, const int32& LeftVal, const FString& RightVal)
    -> VSEvaluatedValue {
  if (Op == VSOperatorTokenType::Dot) {
    auto _Res = FString::FromInt(LeftVal) + "." + RightVal;
    return ToVSEvaluatedValue(FCString::Atof(*_Res));
  }
  return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for binary operation with different types"));
}
template <>
inline auto ApplyBinaryOperation<int32, float>(VSOperatorTokenType Op, const int32& LeftVal, const float& RightVal)
    -> VSEvaluatedValue {
  float LeftValAsFloat = static_cast<float>(LeftVal);
  return ApplyBinaryOperation<float>(Op, LeftValAsFloat, RightVal);
}
template <>
inline auto ApplyBinaryOperation<float, int32>(VSOperatorTokenType Op, const float& LeftVal, const int32& RightVal)
    -> VSEvaluatedValue {
  float RightValAsFloat = static_cast<float>(RightVal);
  return ApplyBinaryOperation<float>(Op, LeftVal, RightValAsFloat);
}
template <>
inline auto ApplyBinaryOperation<VSEvaluatedArray, int32>(VSOperatorTokenType Op,
                                                          const VSEvaluatedArray& LeftVal,
                                                          const int32& RightVal) -> VSEvaluatedValue {
  switch (Op) {
    case VSOperatorTokenType::Equal: return ToVSEvaluatedValue(LeftVal.Num() == RightVal);
    case VSOperatorTokenType::NotEqual: return ToVSEvaluatedValue(LeftVal.Num() != RightVal);
    case VSOperatorTokenType::Less: return ToVSEvaluatedValue(LeftVal.Num() < RightVal);
    case VSOperatorTokenType::LessEqual: return ToVSEvaluatedValue(LeftVal.Num() <= RightVal);
    case VSOperatorTokenType::Greater: return ToVSEvaluatedValue(LeftVal.Num() > RightVal);
    case VSOperatorTokenType::GreaterEqual: return ToVSEvaluatedValue(LeftVal.Num() >= RightVal);
    default:
      return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for binary operation with different types"));
  }
}
template <>
inline auto ApplyBinaryOperation<int32, VSEvaluatedArray>(VSOperatorTokenType Op,
                                                          const int32& LeftVal,
                                                          const VSEvaluatedArray& RightVal) -> VSEvaluatedValue {
  switch (Op) {
    case VSOperatorTokenType::Equal: return ToVSEvaluatedValue(LeftVal == RightVal.Num());
    case VSOperatorTokenType::NotEqual: return ToVSEvaluatedValue(LeftVal != RightVal.Num());
    case VSOperatorTokenType::Less: return ToVSEvaluatedValue(LeftVal < RightVal.Num());
    case VSOperatorTokenType::LessEqual: return ToVSEvaluatedValue(LeftVal <= RightVal.Num());
    case VSOperatorTokenType::Greater: return ToVSEvaluatedValue(LeftVal > RightVal.Num());
    case VSOperatorTokenType::GreaterEqual: return ToVSEvaluatedValue(LeftVal >= RightVal.Num());
    default:
      return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for binary operation with different types"));
  }
}
template <>
inline auto ApplyBinaryOperation<VSEvaluatedMap, int32>(VSOperatorTokenType Op,
                                                        const VSEvaluatedMap& LeftVal,
                                                        const int32& RightVal) -> VSEvaluatedValue {
  switch (Op) {
    case VSOperatorTokenType::Equal: return ToVSEvaluatedValue(LeftVal.Num() == RightVal);
    case VSOperatorTokenType::NotEqual: return ToVSEvaluatedValue(LeftVal.Num() != RightVal);
    case VSOperatorTokenType::Less: return ToVSEvaluatedValue(LeftVal.Num() < RightVal);
    case VSOperatorTokenType::LessEqual: return ToVSEvaluatedValue(LeftVal.Num() <= RightVal);
    case VSOperatorTokenType::Greater: return ToVSEvaluatedValue(LeftVal.Num() > RightVal);
    case VSOperatorTokenType::GreaterEqual: return ToVSEvaluatedValue(LeftVal.Num() >= RightVal);
    default:
      return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for binary operation with different types"));
  }
}
template <>
inline auto ApplyBinaryOperation<int32, VSEvaluatedMap>(VSOperatorTokenType Op,
                                                        const int32& LeftVal,
                                                        const VSEvaluatedMap& RightVal) -> VSEvaluatedValue {
  switch (Op) {
    case VSOperatorTokenType::Equal: return ToVSEvaluatedValue(LeftVal == RightVal.Num());
    case VSOperatorTokenType::NotEqual: return ToVSEvaluatedValue(LeftVal != RightVal.Num());
    case VSOperatorTokenType::Less: return ToVSEvaluatedValue(LeftVal < RightVal.Num());
    case VSOperatorTokenType::LessEqual: return ToVSEvaluatedValue(LeftVal <= RightVal.Num());
    case VSOperatorTokenType::Greater: return ToVSEvaluatedValue(LeftVal > RightVal.Num());
    case VSOperatorTokenType::GreaterEqual: return ToVSEvaluatedValue(LeftVal >= RightVal.Num());
    default:
      return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operator for binary operation with different types"));
  }
}

// Evaluation functions.
auto UVordieScriptSubsystem::EvaluateOperand(const VSOperand& Op) -> VSEvaluatedValue {
  switch (Op.Type) {
    case VSOperandTokenType::Identifier: {
      const FString IdentifierName = Op.Value.Get<FString>();
      if (!GlobalEnviroment.Contains(FName(*IdentifierName)))
        return EnsureReturn<VSEvaluatedValue>(FString::Printf(TEXT("Undefined identifier: %s"), *IdentifierName));

      const auto& SymbolValue = GlobalEnviroment[FName(*IdentifierName)];
      return Visit(Overload{[&](const VSFunction& Func) -> VSEvaluatedValue {
                              // If being run as a variable, it means theres no arguments, so we call it with an empty argument list.
                              // Could be in a pipe context.
                              auto Args = TArray<VSEvaluatedValue>{};
                              if (GlobalEnviroment.Contains(TEXT("^"))) {
                                const auto& PipeValue = GlobalEnviroment[TEXT("^")];
                                Args.Push(Visit(Overload{[&](const VSFunction& Func) -> VSEvaluatedValue {
                                                           return EnsureReturn<VSEvaluatedValue>(
                                                               TEXT("Pipe variable '^' cannot be a function."));
                                                         },
                                                         [&](const auto& Val) -> VSEvaluatedValue {
                                                           return ToVSEvaluatedValue(Val);
                                                         }},
                                                PipeValue));
                              }
                              return Func(Args);
                            },
                            [&](const auto& Val) -> VSEvaluatedValue { return ToVSEvaluatedValue(Val); }},
                   SymbolValue);
    }
    case VSOperandTokenType::Number: return ToVSEvaluatedValue<int32>(Op.Value.Get<int32>());
    case VSOperandTokenType::String: return ToVSEvaluatedValue<FString>(Op.Value.Get<FString>());
    case VSOperandTokenType::Boolean: return ToVSEvaluatedValue<bool>(Op.Value.Get<bool>());
    case VSOperandTokenType::PipeVar: {
      if (!GlobalEnviroment.Contains(TEXT("^")))
        return EnsureReturn<VSEvaluatedValue>(TEXT("Pipe variable '^' is not defined in the environment."));

      const auto& PipeValue = GlobalEnviroment[TEXT("^")];
      return Visit(Overload{[&](const VSFunction& Func) -> VSEvaluatedValue {
                              return EnsureReturn<VSEvaluatedValue>(TEXT("Pipe variable '^' cannot be a function."));
                            },
                            [&](const auto& Val) -> VSEvaluatedValue { return ToVSEvaluatedValue(Val); }},
                   PipeValue);
    }
    default: return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported operand type during evaluation."));
  }
}

auto UVordieScriptSubsystem::EvalUnaryOperation(const VSOperation& Op) -> VSEvaluatedValue {
  check(Op.Operands.Num() == 1);

  VSEvaluatedValue OperandVal = EvaluateExpression(Op.Operands[0]);
  return Visit(Overload{[&](const auto& Val) -> VSEvaluatedValue {
                 using T = typename TDecay<decltype(Val)>::Type;
                 return ApplyUnaryOperation<T>(Op.Operator, Val);
               }},
               OperandVal);
}
auto UVordieScriptSubsystem::EvalBinaryOperation(const VSOperation& Op) -> VSEvaluatedValue {
  check(Op.Operands.Num() == 2);

  VSEvaluatedValue LeftVal = EvaluateExpression(Op.Operands[0]);
  if ((Op.Operator == VSOperatorTokenType::And && LeftVal.IsType<bool>() && !LeftVal.Get<bool>()) ||
      (Op.Operator == VSOperatorTokenType::Or && LeftVal.IsType<bool>() && LeftVal.Get<bool>()))
    return LeftVal;  // Short-circuit
  VSEvaluatedValue RightVal = EvaluateExpression(Op.Operands[1]);

  if (LeftVal.GetIndex() != RightVal.GetIndex()) {
    return Visit(Overload{[&](const auto& Left) -> VSEvaluatedValue {
                   using T = typename TDecay<decltype(Left)>::Type;
                   return Visit(Overload{[&](const auto& Right) -> VSEvaluatedValue {
                                  using U = typename TDecay<decltype(Right)>::Type;
                                  return ApplyBinaryOperation<T, U>(Op.Operator, Left, Right);
                                }},
                                RightVal);
                 }},
                 LeftVal);
  }

  return Visit(Overload{[&](const auto& Left) -> VSEvaluatedValue {
                 using T = typename TDecay<decltype(Left)>::Type;
                 const T& Right = RightVal.Get<T>();
                 return ApplyBinaryOperation<T>(Op.Operator, Left, Right);
               }},
               LeftVal);
}
auto UVordieScriptSubsystem::EvaluateArrayLiteral(const VSOperation& Op) -> VSEvaluatedValue {
  VSEvaluatedArray Res;
  for (const auto& Element : Op.Operands) {
    const VSEvaluatedValue ElementVal = EvaluateExpression(Element);

    VSContainerable CRes;
    if (ElementVal.IsType<FString>()) CRes.Set<FString>(ElementVal.Get<FString>());
    else if (ElementVal.IsType<int32>()) CRes.Set<int32>(ElementVal.Get<int32>());
    else if (ElementVal.IsType<float>()) CRes.Set<float>(ElementVal.Get<float>());
    else if (ElementVal.IsType<bool>()) CRes.Set<bool>(ElementVal.Get<bool>());
    else if (ElementVal.IsType<UObjectPtr>()) CRes.Set<UObjectPtr>(ElementVal.Get<UObjectPtr>());
    else return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported array element type during evaluation."));
    Res.Add(CRes);
  }
  return ToVSEvaluatedValue(Res);
}
auto UVordieScriptSubsystem::EvaluateArrayAccess(const VSOperation& Op) -> VSEvaluatedValue {
  check(Op.Operands.Num() == 2);

  const VSEvaluatedValue ArrayVal = EvaluateExpression(Op.Operands[0]);
  const VSEvaluatedValue IndexVal = EvaluateExpression(Op.Operands[1]);
  check(ArrayVal.IsType<VSEvaluatedArray>());
  check(IndexVal.IsType<int32>());

  const VSEvaluatedArray& Array = ArrayVal.Get<VSEvaluatedArray>();
  int32 Index = IndexVal.Get<int32>();
  if (Index < 0 || Index >= Array.Num())
    return EnsureReturn<VSEvaluatedValue>(FString::Printf(TEXT("Array index %d is out of bounds."), Index));

  auto ElementVal = Array[Index];
  return Visit(Overload{[&](const auto& Val) -> VSEvaluatedValue { return ToVSEvaluatedValue(Val); }}, ElementVal);
}
auto UVordieScriptSubsystem::EvaluateMapLiteral(const VSOperation& Op) -> VSEvaluatedValue {
  VSEvaluatedMap Res;
  for (int32 i = 0; i < Op.Operands.Num(); i += 2) {
    const VSEvaluatedValue KeyVal = EvaluateExpression(Op.Operands[i]);
    const VSEvaluatedValue ValueVal = EvaluateExpression(Op.Operands[i + 1]);
    check(KeyVal.IsType<FString>());

    VSContainerable CKey;
    CKey.Set<FString>(KeyVal.Get<FString>());

    VSContainerable CValue;
    if (ValueVal.IsType<FString>()) CValue.Set<FString>(ValueVal.Get<FString>());
    else if (ValueVal.IsType<int32>()) CValue.Set<int32>(ValueVal.Get<int32>());
    else if (ValueVal.IsType<float>()) CValue.Set<float>(ValueVal.Get<float>());
    else if (ValueVal.IsType<bool>()) CValue.Set<bool>(ValueVal.Get<bool>());
    else if (ValueVal.IsType<UObjectPtr>()) CValue.Set<UObjectPtr>(ValueVal.Get<UObjectPtr>());
    else return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported map value type during evaluation."));

    Res.Add(CKey.Get<FString>(), CValue);
  }
  return ToVSEvaluatedValue(Res);
}
auto UVordieScriptSubsystem::EvaluateMapAccess(const VSOperation& Op) -> VSEvaluatedValue {
  check(Op.Operands.Num() == 2);

  const VSEvaluatedValue MapVal = EvaluateExpression(Op.Operands[0]);
  const VSEvaluatedValue KeyVal = EvaluateExpression(Op.Operands[1]);
  check(MapVal.IsType<VSEvaluatedMap>());
  check(KeyVal.IsType<FString>());

  const VSEvaluatedMap& Map = MapVal.Get<VSEvaluatedMap>();
  const FString& Key = KeyVal.Get<FString>();
  if (!Map.Contains(Key))
    return EnsureReturn<VSEvaluatedValue>(FString::Printf(TEXT("Map key '%s' does not exist."), *Key));

  auto ElementVal = Map[Key];
  return Visit(Overload{[&](const auto& Val) -> VSEvaluatedValue { return ToVSEvaluatedValue(Val); }}, ElementVal);
}
auto UVordieScriptSubsystem::EvalTernaryOperation(const VSOperation& Op) -> VSEvaluatedValue {
  check(Op.Operands.Num() == 3);

  VSEvaluatedValue ConditionVal = EvaluateExpression(Op.Operands[0]);
  check(ConditionVal.IsType<bool>());

  if (ConditionVal.Get<bool>()) return EvaluateExpression(Op.Operands[1]);
  return EvaluateExpression(Op.Operands[2]);
}
auto UVordieScriptSubsystem::EvalPipeOperation(const VSOperation& Op) -> VSEvaluatedValue {
  check(Op.Operands.Num() == 2);

  VSEvaluatedValue PipeVal = EvaluateExpression(Op.Operands[0]);  // Pipe var
  GlobalEnviroment.Add(
      TEXT("^"), Visit(Overload{[&](const auto& Val) -> VSEnviromentContext {
                         using T = typename TDecay<decltype(Val)>::Type;
                         if constexpr (!TypeTests::TAreTypesEqual_V<T, FStructProperty*>) {
                           VSEnviromentContext PipeContext;
                           PipeContext.Set<T>(Val);
                           return PipeContext;
                         } else {
                           return EnsureReturn<VSEnviromentContext>(
                               TEXT("Unsupported type for pipe operation. Only string, int, float, bool, UObjectPtr, "
                                    "array, and map are supported."));
                         }
                       }},
                       PipeVal));

  VSEvaluatedValue RightVal = EvaluateExpression(Op.Operands[1]);
  GlobalEnviroment.Remove(TEXT("^"));

  return RightVal;
}
auto UVordieScriptSubsystem::EvalFuncArgsCall(const VSOperation& Op) -> VSEvaluatedValue {
  check(Op.Operands.Num() == 2);
  check(Op.Operands[0].IsType<VSOperand>() && Op.Operands[0].Get<VSOperand>().Type == VSOperandTokenType::Identifier);

  const FString FuncName = Op.Operands[0].Get<VSOperand>().Value.Get<FString>();
  if (!GlobalEnviroment.Contains(FName(*FuncName)))
    return EnsureReturn<VSEvaluatedValue>(FString::Printf(TEXT("Undefined function: %s"), *FuncName));

  VSFunction Func = GlobalEnviroment[FName(*FuncName)].Get<VSFunction>();

  check(Op.Operands[1].IsType<VSOperation>() &&
        Op.Operands[1].Get<VSOperation>().Operator == VSOperatorTokenType::Comma &&
        Op.Operands[1].Get<VSOperation>().Value == "args");

  const auto& ArgsOp = Op.Operands[1].Get<VSOperation>();
  TArray<VSEvaluatedValue> ArgValues;
  for (const auto& ArgExpr : ArgsOp.Operands) ArgValues.Push(EvaluateExpression(ArgExpr));

  return Func(ArgValues);
}
auto UVordieScriptSubsystem::EvalDotObjectAccess(const VSOperation& Op) -> VSEvaluatedValue {
  check(Op.Operands.Num() == 2);
  check(Op.Operands[1].IsType<VSOperand>() && Op.Operands[1].Get<VSOperand>().Type == VSOperandTokenType::Identifier);

  const FString PropertyName = Op.Operands[1].Get<VSOperand>().Value.Get<FString>();

  const auto& Left = EvaluateExpression(Op.Operands[0]);
  check(Left.IsType<UObjectPtr>() || Left.IsType<FStructProperty*>());
  if (const auto& ObjectPtr = Left.TryGet<UObjectPtr>()) {
    FProperty* Prop = (*ObjectPtr)->GetClass()->FindPropertyByName(FName(*PropertyName));
    check(Prop);
    // Since the FProperty could be of any type, handle the types explicitly.
    if (FStrProperty* StrProp = CastField<FStrProperty>(Prop)) {
      FString PropVal = StrProp->GetPropertyValue_InContainer(ObjectPtr->Get());
      return ToVSEvaluatedValue<FString>(PropVal);
    } else if (FIntProperty* IntProp = CastField<FIntProperty>(Prop)) {
      int32 PropVal = IntProp->GetPropertyValue_InContainer(ObjectPtr->Get());
      return ToVSEvaluatedValue<int32>(PropVal);
    } else if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop)) {
      float PropVal = FloatProp->GetPropertyValue_InContainer(ObjectPtr->Get());
      return ToVSEvaluatedValue<float>(PropVal);
    } else if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop)) {
      bool PropVal = BoolProp->GetPropertyValue_InContainer(ObjectPtr->Get());
      return ToVSEvaluatedValue<bool>(PropVal);
    } else if (FStructProperty* StructProp = CastField<FStructProperty>(Prop)) {
      return ToVSEvaluatedValue<FStructProperty*>(StructProp);
    } else if (FObjectProperty* ObjectProp = CastField<FObjectProperty>(Prop)) {
      UObject* PropVal = ObjectProp->GetObjectPropertyValue_InContainer(ObjectPtr->Get());
      return ToVSEvaluatedValue<UObjectPtr>(UObjectPtr(PropVal));
    } else {
      return EnsureReturn<VSEvaluatedValue>(
          FString::Printf(TEXT("Unsupported property type for dot access: %s"), *Prop->GetClass()->GetName()));
    }
  } else if (const auto& StructProp = Left.TryGet<FStructProperty*>()) {
    FProperty* Prop = (*StructProp)->Struct->FindPropertyByName(FName(*PropertyName));
    check(Prop);
    // Since the FProperty could be of any type, handle the types explicitly.
    if (FStrProperty* StrProp = CastField<FStrProperty>(Prop)) {
      FString PropVal = StrProp->GetPropertyValue_InContainer(StructProp);
      return ToVSEvaluatedValue<FString>(PropVal);
    } else if (FIntProperty* IntProp = CastField<FIntProperty>(Prop)) {
      int32 PropVal = IntProp->GetPropertyValue_InContainer(StructProp);
      return ToVSEvaluatedValue<int32>(PropVal);
    } else if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop)) {
      float PropVal = FloatProp->GetPropertyValue_InContainer(StructProp);
      return ToVSEvaluatedValue<float>(PropVal);
    } else if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop)) {
      bool PropVal = BoolProp->GetPropertyValue_InContainer(StructProp);
      return ToVSEvaluatedValue<bool>(PropVal);
    } else if (FStructProperty* NestedStructProp = CastField<FStructProperty>(Prop)) {
      return ToVSEvaluatedValue<FStructProperty*>(NestedStructProp);
    } else if (FObjectProperty* ObjectProp = CastField<FObjectProperty>(Prop)) {
      UObject* PropVal = ObjectProp->GetObjectPropertyValue_InContainer(StructProp);
      return ToVSEvaluatedValue<UObjectPtr>(UObjectPtr(PropVal));
    } else {
      return EnsureReturn<VSEvaluatedValue>(
          FString::Printf(TEXT("Unsupported property type for dot access: %s"), *Prop->GetClass()->GetName()));
    }
  } else {
    return EnsureReturn<VSEvaluatedValue>(
        TEXT("Left operand of dot operation must be a UObjectPtr or FStructProperty*"));
  }
}

auto UVordieScriptSubsystem::EvaluateOperation(const VSOperation& Op) -> VSEvaluatedValue {
  // Array literal.
  if (Op.Operator == VSOperatorTokenType::LeftSquare && Op.Value == "array_literal") return EvaluateArrayLiteral(Op);
  if (Op.Operator == VSOperatorTokenType::LeftSquare) return EvaluateArrayAccess(Op);
  // Map literal.
  if (Op.Operator == VSOperatorTokenType::LeftBrace && Op.Value == "map_literal") return EvaluateMapLiteral(Op);
  if (Op.Operator == VSOperatorTokenType::LeftBrace) return EvaluateMapAccess(Op);
  // Unary operators.
  switch (Op.Operator) {
    case VSOperatorTokenType::Not: return EvalUnaryOperation(Op);
    case VSOperatorTokenType::Plus:
    case VSOperatorTokenType::Minus: {
      if (Op.Operands.Num() > 1) break;

      return EvalUnaryOperation(Op);
    }
    default: break;
  }
  // Simple binary operators.
  switch (Op.Operator) {
    case VSOperatorTokenType::And:
    case VSOperatorTokenType::Or:
    case VSOperatorTokenType::Equal:
    case VSOperatorTokenType::NotEqual:
    case VSOperatorTokenType::Less:
    case VSOperatorTokenType::LessEqual:
    case VSOperatorTokenType::Greater:
    case VSOperatorTokenType::GreaterEqual:
    case VSOperatorTokenType::Plus:
    case VSOperatorTokenType::Minus:
    case VSOperatorTokenType::Times:
    case VSOperatorTokenType::Divide: return EvalBinaryOperation(Op);
    case VSOperatorTokenType::Dot:
      if (Op.Operands.Num() == 2 &&
          (Op.Operands[0].IsType<VSOperand>() && Op.Operands[0].Get<VSOperand>().Type == VSOperandTokenType::Number))
        return EvalBinaryOperation(Op);
    default: break;
  }
  // Ternary operator.
  if (Op.Operator == VSOperatorTokenType::Question) return EvalTernaryOperation(Op);
  // Pipe operator.
  if (Op.Operator == VSOperatorTokenType::Pipe) return EvalPipeOperation(Op);
  // Function call.
  if (Op.Operator == VSOperatorTokenType::LeftParen && Op.Value == "call") return EvalFuncArgsCall(Op);
  // Todo: FStructProperty, UObjectPtr, dot operation.
  if (Op.Operator == VSOperatorTokenType::Dot) return EvalDotObjectAccess(Op);

  return EnsureReturn<VSEvaluatedValue>(FString::Printf(TEXT("Unsupported operation: %s"), *Op.Value));
}

auto UVordieScriptSubsystem::EvaluateExpression(const VSExpression& Expr) -> VSEvaluatedValue {
  if (Expr.IsType<VSOperand>()) return EvaluateOperand(Expr.Get<VSOperand>());
  if (Expr.IsType<VSOperation>()) return EvaluateOperation(Expr.Get<VSOperation>());

  return EnsureReturn<VSEvaluatedValue>(TEXT("Unsupported expression type during evaluation."));
}

auto TestScriptTreeGen(const FString& ScriptCode) -> FString {
  TArray<Token> Tokens = Tokenize(ScriptCode);
  VSScript ParsedScript = ParseTokensToScript(Tokens);

  for (const auto& Expr : ParsedScript.Expressions) {
    FString ExprStr = ExpressionToString(Expr);
    UE_LOG(LogTemp, Log, TEXT("Parsed VSExpression: %s"), *ExprStr);
  }
  return ExpressionToString(ParsedScript.Expressions.Last());
}

UVordieScriptSubsystem::UVordieScriptSubsystem() {}

void UVordieScriptSubsystem::RegisterSymbol(FName SymbolName, VSEnviromentContext SymbolValue) {
  if (GlobalEnviroment.Contains(SymbolName)) {
    UE_LOG(LogTemp, Error, TEXT("Symbol %s is already registered."), *SymbolName.ToString());
    return;
  }

  GlobalEnviroment.Add(SymbolName, SymbolValue);
  UE_LOG(LogTemp, Log, TEXT("Registered symbol %s."), *SymbolName.ToString());
}

auto UVordieScriptSubsystem::EvaluateScript(const FString& ScriptCode) -> VSEvaluatedScript {
  TArray<Token> Tokens = Tokenize(ScriptCode);
  VSScript ParsedScript = ParseTokensToScript(Tokens);

  VSEvaluatedValue LastResult;
  for (const auto& Expr : ParsedScript.Expressions) LastResult = EvaluateExpression(Expr);

  VSEvaluatedScript Result = {true, "", LastResult.GetIndex() != 0 ? true : false, LastResult};
  return Result;
}