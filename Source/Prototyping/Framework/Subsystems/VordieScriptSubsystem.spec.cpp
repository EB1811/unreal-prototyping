#include "Misc/AutomationTest.h"
#include "VordieScriptSubsystem.h"

BEGIN_DEFINE_SPEC(FVordieScriptSubsystemSpec,
                  "Prototyping.VordieScript",
                  EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

// Asserts that parsing Script and stringifying the final expression's tree equals Expected,
// e.g. TestTree(TEXT("5 + 5"), TEXT("(+ 5 5)")).
void TestTree(const FString& Script, const FString& Expected);

END_DEFINE_SPEC(FVordieScriptSubsystemSpec)

void FVordieScriptSubsystemSpec::TestTree(const FString& Script, const FString& Expected) {
  const FString Actual = TestScriptTreeGen(Script);
  TestEqual(FString::Printf(TEXT("Script '%s' should produce expected expression tree."), *Script), Actual, Expected);
}

void FVordieScriptSubsystemSpec::Define() {
  Describe("Literals", [this]() {
    It("parses integer literals", [this]() { TestTree(TEXT("5"), TEXT("5")); });
    It("parses string literals", [this]() { TestTree(TEXT("'hello'"), TEXT("'hello'")); });
    It("parses string literals containing spaces",
       [this]() { TestTree(TEXT("'hello world'"), TEXT("'hello world'")); });
    It("parses the true boolean literal", [this]() { TestTree(TEXT("true"), TEXT("true")); });
    It("parses the false boolean literal", [this]() { TestTree(TEXT("false"), TEXT("false")); });
    It("parses identifiers", [this]() { TestTree(TEXT("foo"), TEXT("foo")); });
  });

  Describe("Arithmetic operators", [this]() {
    It("adds two numbers", [this]() { TestTree(TEXT("5 + 5"), TEXT("(+ 5 5)")); });
    It("subtracts two numbers", [this]() { TestTree(TEXT("10 - 3"), TEXT("(- 10 3)")); });
    It("multiplies two numbers", [this]() { TestTree(TEXT("4 * 2"), TEXT("(* 4 2)")); });
    It("divides two numbers", [this]() { TestTree(TEXT("8 / 2"), TEXT("(/ 8 2)")); });
    It("gives multiplication higher precedence than addition",
       [this]() { TestTree(TEXT("5 + 5 * 2"), TEXT("(+ 5 (* 5 2))")); });
    It("gives division higher precedence than subtraction",
       [this]() { TestTree(TEXT("10 - 8 / 2"), TEXT("(- 10 (/ 8 2))")); });
    It("respects parentheses to override precedence",
       [this]() { TestTree(TEXT("(5 + 5) * 2"), TEXT("(* (+ 5 5) 2)")); });
    It("is left-associative for subtraction", [this]() { TestTree(TEXT("10 - 3 - 2"), TEXT("(- (- 10 3) 2)")); });
    It("is left-associative for division", [this]() { TestTree(TEXT("20 / 4 / 2"), TEXT("(/ (/ 20 4) 2)")); });
  });

  Describe("Unary operators", [this]() {
    It("applies unary minus", [this]() { TestTree(TEXT("-5"), TEXT("(- 5)")); });
    It("applies unary plus", [this]() { TestTree(TEXT("+5"), TEXT("(+ 5)")); });
    It("applies logical not", [this]() { TestTree(TEXT("!true"), TEXT("(! true)")); });
    It("binds tighter than binary addition", [this]() { TestTree(TEXT("-5 + 3"), TEXT("(+ (- 5) 3)")); });
    It("applies repeated unary operators right-to-left", [this]() { TestTree(TEXT("!!true"), TEXT("(! (! true))")); });
    It("parses a negated grouped comparison", [this]() { TestTree(TEXT("!(5 == 5)"), TEXT("(! (== 5 5))")); });
  });

  Describe("Comparison operators", [this]() {
    It("compares equality", [this]() { TestTree(TEXT("5 == 5"), TEXT("(== 5 5)")); });
    It("compares inequality", [this]() { TestTree(TEXT("5 != 3"), TEXT("(!= 5 3)")); });
    It("compares less than", [this]() { TestTree(TEXT("5 < 10"), TEXT("(< 5 10)")); });
    It("compares less than or equal", [this]() { TestTree(TEXT("5 <= 10"), TEXT("(<= 5 10)")); });
    It("compares greater than", [this]() { TestTree(TEXT("10 > 5"), TEXT("(> 10 5)")); });
    It("compares greater than or equal", [this]() { TestTree(TEXT("10 >= 5"), TEXT("(>= 10 5)")); });
  });

  Describe("Logical operators", [this]() {
    It("evaluates logical and", [this]() { TestTree(TEXT("true && false"), TEXT("(&& true false)")); });
    It("evaluates logical or", [this]() { TestTree(TEXT("true || false"), TEXT("(|| true false)")); });
    It("gives comparisons higher precedence than logical and",
       [this]() { TestTree(TEXT("5 < 10 && 3 > 1"), TEXT("(&& (< 5 10) (> 3 1))")); });
  });

  Describe("Ternary operator", [this]() {
    It("parses a conditional expression", [this]() { TestTree(TEXT("true ? 1 : 2"), TEXT("(? true 1 2)")); });
    It("parses a grouped condition", [this]() { TestTree(TEXT("(5 > 3) ? 1 : 2"), TEXT("(? (> 5 3) 1 2)")); });
    It("nests ternaries right-associatively in the false branch",
       [this]() { TestTree(TEXT("true ? 1 : false ? 2 : 3"), TEXT("(? true 1 (? false 2 3))")); });
  });

  Describe("Array literals", [this]() {
    It("parses an array of numbers", [this]() { TestTree(TEXT("[1, 2, 3]"), TEXT("(array_literal 1 2 3)")); });
  });

  Describe("Function calls", [this]() {
    It("parses a call with arguments", [this]() { TestTree(TEXT("Func(1, 2)"), TEXT("(call Func (args 1 2))")); });
  });

  Describe("Member access and pipe operators", [this]() {
    It("parses dot member access", [this]() { TestTree(TEXT("obj.prop"), TEXT("(. obj prop)")); });
    It("parses the pipe operator", [this]() { TestTree(TEXT("value ~> Transform"), TEXT("(~> value Transform)")); });
  });

  Describe("Multi-statement scripts", [this]() {
    It("returns only the final expression's tree", [this]() { TestTree(TEXT("5 + 5\n10 + 2"), TEXT("(+ 10 2)")); });
  });

  Describe("Dot operator", [this]() {
    It("parses a dot operator with an identifier", [this]() { TestTree(TEXT("obj.prop"), TEXT("(. obj prop)")); });
    It("parses a dot operator with a function call", [this]() { TestTree(TEXT("obj.func()"), TEXT("(. obj func)")); });
    It("parses a dot operator with two integers", [this]() { TestTree(TEXT("1.2"), TEXT("(. 1 2)")); });
  });
}
