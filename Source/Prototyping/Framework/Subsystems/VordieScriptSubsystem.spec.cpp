#include "Prototyping/AI/TestEnemyManager.h"
#include "Misc/AutomationTest.h"
#include "UObject/UObjectGlobals.h"
#include "VordieScriptSubsystem.h"

BEGIN_DEFINE_SPEC(FVordieScriptSubsystemSpec,
                  "Prototyping.VordieScript",
                  EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
// Asserts that parsing Script and stringifying the final expression's tree equals Expected,
// e.g. TestTree(TEXT("5 + 5"), TEXT("(+ 5 5)")).
void TestTree(const FString& Script, const FString& Expected);

// Asserts that evaluating Script on a fresh subsystem produces the expected int32/bool/FString result.
void TestEvalInt(const FString& Script, int32 Expected);
void TestEvalFloat(const FString& Script, float Expected);
void TestEvalBool(const FString& Script, bool bExpected);
void TestEvalString(const FString& Script, const FString& Expected);

END_DEFINE_SPEC(FVordieScriptSubsystemSpec)

void FVordieScriptSubsystemSpec::TestTree(const FString& Script, const FString& Expected) {
  const FString Actual = TestScriptTreeGen(Script);
  TestEqual(FString::Printf(TEXT("Script '%s' should produce expected expression tree."), *Script), Actual, Expected);
}

void FVordieScriptSubsystemSpec::TestEvalInt(const FString& Script, int32 Expected) {
  UVordieScriptSubsystem* Subsystem = NewObject<UVordieScriptSubsystem>();
  const VSEvaluatedScript Result = Subsystem->EvaluateScript(Script);
  TestTrue(FString::Printf(TEXT("Script '%s' should evaluate successfully."), *Script), Result.bSuccess);
  if (TestTrue(FString::Printf(TEXT("Script '%s' should produce an int32 result."), *Script),
               Result.ReturnValue.IsType<int32>()))
    TestEqual(FString::Printf(TEXT("Script '%s' should evaluate to %d."), *Script, Expected),
              Result.ReturnValue.Get<int32>(), Expected);
}

void FVordieScriptSubsystemSpec::TestEvalFloat(const FString& Script, float Expected) {
  UVordieScriptSubsystem* Subsystem = NewObject<UVordieScriptSubsystem>();
  const VSEvaluatedScript Result = Subsystem->EvaluateScript(Script);
  TestTrue(FString::Printf(TEXT("Script '%s' should evaluate successfully."), *Script), Result.bSuccess);
  if (TestTrue(FString::Printf(TEXT("Script '%s' should produce a float result."), *Script),
               Result.ReturnValue.IsType<float>()))
    TestEqual(FString::Printf(TEXT("Script '%s' should evaluate to %f."), *Script, Expected),
              Result.ReturnValue.Get<float>(), Expected);
}

void FVordieScriptSubsystemSpec::TestEvalBool(const FString& Script, bool bExpected) {
  UVordieScriptSubsystem* Subsystem = NewObject<UVordieScriptSubsystem>();
  const VSEvaluatedScript Result = Subsystem->EvaluateScript(Script);
  TestTrue(FString::Printf(TEXT("Script '%s' should evaluate successfully."), *Script), Result.bSuccess);
  if (TestTrue(FString::Printf(TEXT("Script '%s' should produce a bool result."), *Script),
               Result.ReturnValue.IsType<bool>()))
    TestEqual(
        FString::Printf(TEXT("Script '%s' should evaluate to %s."), *Script, bExpected ? TEXT("true") : TEXT("false")),
        Result.ReturnValue.Get<bool>(), bExpected);
}

void FVordieScriptSubsystemSpec::TestEvalString(const FString& Script, const FString& Expected) {
  UVordieScriptSubsystem* Subsystem = NewObject<UVordieScriptSubsystem>();
  const VSEvaluatedScript Result = Subsystem->EvaluateScript(Script);
  TestTrue(FString::Printf(TEXT("Script '%s' should evaluate successfully."), *Script), Result.bSuccess);
  if (TestTrue(FString::Printf(TEXT("Script '%s' should produce an FString result."), *Script),
               Result.ReturnValue.IsType<FString>()))
    TestEqual(FString::Printf(TEXT("Script '%s' should evaluate to '%s'."), *Script, *Expected),
              Result.ReturnValue.Get<FString>(), Expected);
}

void FVordieScriptSubsystemSpec::Define() {
  Describe("Parsing", [this]() {
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
      It("applies repeated unary operators right-to-left",
         [this]() { TestTree(TEXT("!!true"), TEXT("(! (! true))")); });
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
      It("parses a dot operator with a function call",
         [this]() { TestTree(TEXT("obj.func()"), TEXT("(. obj func)")); });
      It("parses a dot operator with two integers", [this]() { TestTree(TEXT("1.2"), TEXT("(. 1 2)")); });
    });
  });

  Describe("Evaluation", [this]() {
    Describe("Literals", [this]() {
      It("evaluates integer literals", [this]() { TestEvalInt(TEXT("5"), 5); });
      It("evaluates string literals", [this]() { TestEvalString(TEXT("'hello'"), TEXT("hello")); });
      It("evaluates the true boolean literal", [this]() { TestEvalBool(TEXT("true"), true); });
      It("evaluates the false boolean literal", [this]() { TestEvalBool(TEXT("false"), false); });
    });

    Describe("Arithmetic operators", [this]() {
      It("adds two numbers", [this]() { TestEvalInt(TEXT("5 + 5"), 10); });
      It("subtracts two numbers", [this]() { TestEvalInt(TEXT("10 - 3"), 7); });
      It("multiplies two numbers", [this]() { TestEvalInt(TEXT("4 * 2"), 8); });
      It("divides two numbers", [this]() { TestEvalInt(TEXT("8 / 2"), 4); });
      It("respects operator precedence", [this]() { TestEvalInt(TEXT("5 + 5 * 2"), 15); });
      It("respects parentheses to override precedence", [this]() { TestEvalInt(TEXT("(5 + 5) * 2"), 20); });
    });

    Describe("Unary operators", [this]() {
      It("applies unary minus", [this]() { TestEvalInt(TEXT("-5"), -5); });
      It("applies unary plus", [this]() { TestEvalInt(TEXT("+5"), 5); });
      It("applies logical not to true", [this]() { TestEvalBool(TEXT("!true"), false); });
      It("applies logical not to false", [this]() { TestEvalBool(TEXT("!false"), true); });
    });

    Describe("Comparison operators", [this]() {
      It("evaluates equality", [this]() { TestEvalBool(TEXT("5 == 5"), true); });
      It("evaluates inequality", [this]() { TestEvalBool(TEXT("5 != 3"), true); });
      It("evaluates less than", [this]() { TestEvalBool(TEXT("5 < 10"), true); });
      It("evaluates greater than", [this]() { TestEvalBool(TEXT("10 > 5"), true); });
      It("evaluates less than or equal", [this]() { TestEvalBool(TEXT("5 <= 5"), true); });
      It("evaluates greater than or equal as false", [this]() { TestEvalBool(TEXT("5 >= 10"), false); });
    });

    Describe("Logical operators", [this]() {
      It("evaluates logical and", [this]() { TestEvalBool(TEXT("true && false"), false); });
      It("evaluates logical or", [this]() { TestEvalBool(TEXT("true || false"), true); });
      It("short-circuits logical and on a false left operand",
         [this]() { TestEvalBool(TEXT("false && true"), false); });
    });

    Describe("Ternary operator", [this]() {
      It("evaluates the true branch", [this]() { TestEvalInt(TEXT("true ? 1 : 2"), 1); });
      It("evaluates the false branch", [this]() { TestEvalInt(TEXT("false ? 1 : 2"), 2); });
    });

    Describe("String operations", [this]() {
      It("concatenates two strings", [this]() { TestEvalString(TEXT("'foo' + 'bar'"), TEXT("foobar")); });
      It("compares two equal strings", [this]() { TestEvalBool(TEXT("'abc' == 'abc'"), true); });
    });

    Describe("Type coercion", [this]() {
      It("compares an integer and a float for equality", [this]() { TestEvalBool(TEXT("5 == 5.0"), true); });
      It("compares an integer and a float for inequality", [this]() { TestEvalBool(TEXT("5 != 5.0"), false); });
      It("compares an integer and a float for less than", [this]() { TestEvalBool(TEXT("5 < 5.1"), true); });
      It("turns an array into its length when compared to an integer",
         [this]() { TestEvalBool(TEXT("[1, 2, 3] == 3"), true); });
    });

    Describe("Array literals and access", [this]() {
      It("accesses the first element", [this]() { TestEvalInt(TEXT("[1, 2, 3][0]"), 1); });
      It("accesses the last element", [this]() { TestEvalInt(TEXT("[1, 2, 3][2]"), 3); });
    });

    Describe("Pipe operator",
             [this]() { It("makes the piped value available as '^'", [this]() { TestEvalInt(TEXT("5 ~> ^"), 5); }); });

    Describe("Dot operator on numbers", [this]() {
      It("concatenates two integers into a string", [this]() { TestEvalFloat(TEXT("1.2"), 1.2f); });
    });

    Describe("Registered symbols", [this]() {
      It("resolves a registered identifier", [this]() {
        UVordieScriptSubsystem* Subsystem = NewObject<UVordieScriptSubsystem>();
        VSEnviromentContext Value;
        Value.Set<int32>(42);
        Subsystem->RegisterSymbol(FName(TEXT("x")), Value);

        const VSEvaluatedScript Result = Subsystem->EvaluateScript(TEXT("x"));
        TestTrue(TEXT("Script should evaluate successfully."), Result.bSuccess);
        if (TestTrue(TEXT("Result should be an int32."), Result.ReturnValue.IsType<int32>()))
          TestEqual(TEXT("Registered identifier 'x' should evaluate to 42."), Result.ReturnValue.Get<int32>(), 42);
      });
      It("uses an identifier in an expression", [this]() {
        UVordieScriptSubsystem* Subsystem = NewObject<UVordieScriptSubsystem>();
        VSEnviromentContext Value;
        Value.Set<int32>(10);
        Subsystem->RegisterSymbol(FName(TEXT("y")), Value);

        const VSEvaluatedScript Result = Subsystem->EvaluateScript(TEXT("y + 5"));
        TestTrue(TEXT("Script should evaluate successfully."), Result.bSuccess);
        if (TestTrue(TEXT("Result should be an int32."), Result.ReturnValue.IsType<int32>()))
          TestEqual(TEXT("Expression 'y + 5' should evaluate to 15."), Result.ReturnValue.Get<int32>(), 15);
      });

      It("calls a registered function with arguments", [this]() {
        UVordieScriptSubsystem* Subsystem = NewObject<UVordieScriptSubsystem>();
        VSFunction AddOne = [](const TArray<VSEvaluatedValue>& Args) -> VSEvaluatedValue {
          VSEvaluatedValue Res;
          Res.Set<int32>(Args[0].Get<int32>() + 1);
          return Res;
        };
        VSEnviromentContext FuncValue;
        FuncValue.Set<VSFunction>(AddOne);
        Subsystem->RegisterSymbol(FName(TEXT("AddOne")), FuncValue);

        const VSEvaluatedScript Result = Subsystem->EvaluateScript(TEXT("AddOne(5)"));
        TestTrue(TEXT("Script should evaluate successfully."), Result.bSuccess);
        if (TestTrue(TEXT("Result should be an int32."), Result.ReturnValue.IsType<int32>()))
          TestEqual(TEXT("AddOne(5) should evaluate to 6."), Result.ReturnValue.Get<int32>(), 6);
      });
    });

    Describe("Reflection access", [this]() {
      It("accesses a reflected property of a UObject via the dot operator", [this]() {
        UVordieScriptSubsystem* Subsystem = NewObject<UVordieScriptSubsystem>();
        ATestEnemyManager* EnemyManager = NewObject<ATestEnemyManager>();
        EnemyManager->Health = 150;

        VSEnviromentContext Value;
        Value.Set<UObjectPtr>(UObjectPtr(EnemyManager));
        Subsystem->RegisterSymbol(FName(TEXT("enemy")), Value);

        const VSEvaluatedScript Result = Subsystem->EvaluateScript(TEXT("enemy.Health > 100"));
        TestTrue(TEXT("Script should evaluate successfully."), Result.bSuccess);
        if (TestTrue(TEXT("Result should be a bool."), Result.ReturnValue.IsType<bool>()))
          TestEqual(TEXT("'enemy.Health > 100' should evaluate to true."), Result.ReturnValue.Get<bool>(), true);
      });

      It("accesses a reflected property of a struct via the dot operator", [this]() {
        UVordieScriptSubsystem* Subsystem = NewObject<UVordieScriptSubsystem>();
        ATestEnemyManager* EnemyManager = NewObject<ATestEnemyManager>();
        EnemyManager->BehaviourParams.bAggressive = true;

        VSEnviromentContext Value;
        Value.Set<UObjectPtr>(UObjectPtr(EnemyManager));
        Subsystem->RegisterSymbol(FName(TEXT("enemy")), Value);

        const VSEvaluatedScript Result = Subsystem->EvaluateScript(TEXT("enemy.BehaviourParams.bAggressive"));
        TestTrue(TEXT("Script should evaluate successfully."), Result.bSuccess);
        if (TestTrue(TEXT("Result should be a bool."), Result.ReturnValue.IsType<bool>()))
          TestEqual(TEXT("'enemy.BehaviourParams.bAggressive' should evaluate to true."),
                    Result.ReturnValue.Get<bool>(), true);
      });
    });
  });
}
