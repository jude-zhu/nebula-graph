/* Copyright (c) 2020 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#include "visitor/FoldConstantExprVisitor.h"
#include "visitor/test/VisitorTestBase.h"

#include <gtest/gtest.h>

#include "common/base/ObjectPool.h"

namespace nebula {
namespace graph {

class FoldConstantExprVisitorTest : public ValidatorTestBase {
public:
    void TearDown() override {
        pool.clear();
    }

protected:
    ObjectPool pool;
};

TEST_F(FoldConstantExprVisitorTest, TestArithmeticExpr) {
    // (5 - 1) + 2 => 4 + 2
    auto expr = pool.add(addExpr(minusExpr(constantExpr(5), constantExpr(1)), constantExpr(2)));
    FoldConstantExprVisitor visitor;
    expr->accept(&visitor);
    auto expected = pool.add(addExpr(constantExpr(4), constantExpr(2)));
    ASSERT_EQ(*expr, *expected) << expr->toString() << " vs. " << expected->toString();
    ASSERT(visitor.canBeFolded());

    // 4+2 => 6
    auto root = pool.add(visitor.fold(expr));
    auto rootExpected = pool.add(constantExpr(6));
    ASSERT_EQ(*root, *rootExpected) << root->toString() << " vs. " << rootExpected->toString();
}

TEST_F(FoldConstantExprVisitorTest, TestRelationExpr) {
    // false == !(3 > (1+1)) => false == false
    auto expr = pool.add(
        eqExpr(constantExpr(false),
               notExpr(gtExpr(constantExpr(3), addExpr(constantExpr(1), constantExpr(1))))));
    auto expected = pool.add(eqExpr(constantExpr(false), constantExpr(false)));
    FoldConstantExprVisitor visitor;
    expr->accept(&visitor);
    ASSERT_EQ(*expr, *expected) << expr->toString() << " vs. " << expected->toString();
    ASSERT(visitor.canBeFolded());

    // false==false => true
    auto root = pool.add(visitor.fold(expr));
    auto rootExpected = pool.add(constantExpr(true));
    ASSERT_EQ(*root, *rootExpected) << root->toString() << " vs. " << rootExpected->toString();
}

TEST_F(FoldConstantExprVisitorTest, TestLogicalExpr) {
    // false AND (false || (3 > (1 + 1))) => false AND true
    auto expr = pool.add(
        andExpr(constantExpr(false),
                orExpr(constantExpr(false),
                       gtExpr(constantExpr(3), addExpr(constantExpr(1), constantExpr(1))))));
    auto expected = pool.add(andExpr(constantExpr(false), constantExpr(true)));
    FoldConstantExprVisitor visitor;
    expr->accept(&visitor);
    ASSERT_EQ(*expr, *expected) << expr->toString() << " vs. " << expected->toString();
    ASSERT(visitor.canBeFolded());

    // false AND true => false
    auto root = pool.add(visitor.fold(expr));
    auto rootExpected = pool.add(constantExpr(false));
    ASSERT_EQ(*root, *rootExpected) << root->toString() << " vs. " << rootExpected->toString();
}

TEST_F(FoldConstantExprVisitorTest, TestSubscriptExpr) {
    // 1 + [1, pow(2, 2+1), 2][2-1] => 1 + 8
    auto expr = pool.add(addExpr(
        constantExpr(1),
        subExpr(
            listExpr({constantExpr(1),
                      fnExpr("pow", {constantExpr(2), addExpr(constantExpr(2), constantExpr(1))}),
                      constantExpr(2)}),
            minusExpr(constantExpr(2), constantExpr(1)))));
    auto expected = pool.add(addExpr(constantExpr(1), constantExpr(8)));
    FoldConstantExprVisitor visitor;
    expr->accept(&visitor);
    ASSERT_EQ(*expr, *expected) << expr->toString() << " vs. " << expected->toString();
    ASSERT(visitor.canBeFolded());

    // 1+8 => 9
    auto root = pool.add(visitor.fold(expr));
    auto rootExpected = pool.add(constantExpr(9));
    ASSERT_EQ(*root, *rootExpected) << root->toString() << " vs. " << rootExpected->toString();
}

TEST_F(FoldConstantExprVisitorTest, TestListExpr) {
    // [3+4, pow(2, 2+1), 2] => [7, 8, 2]
    auto expr = pool.add(
        listExpr({addExpr(constantExpr(3), constantExpr(4)),
                  fnExpr("pow", {constantExpr(2), addExpr(constantExpr(2), constantExpr(1))}),
                  constantExpr(2)}));
    auto expected = pool.add(listExpr({constantExpr(7), constantExpr(8), constantExpr(2)}));
    FoldConstantExprVisitor visitor;
    expr->accept(&visitor);
    ASSERT_EQ(*expr, *expected) << expr->toString() << " vs. " << expected->toString();
    ASSERT(visitor.canBeFolded());
}

TEST_F(FoldConstantExprVisitorTest, TestSetExpr) {
    // {sqrt(19-3), pow(2, 3+1), 2} => {4, 16, 2}
    auto expr = pool.add(
        setExpr({fnExpr("sqrt", {minusExpr(constantExpr(19), constantExpr(3))}),
                 fnExpr("pow", {constantExpr(2), addExpr(constantExpr(3), constantExpr(1))}),
                 constantExpr(2)}));
    auto expected = pool.add(setExpr({constantExpr(4), constantExpr(16), constantExpr(2)}));
    FoldConstantExprVisitor visitor;
    expr->accept(&visitor);
    ASSERT_EQ(*expr, *expected) << expr->toString() << " vs. " << expected->toString();
    ASSERT(visitor.canBeFolded());
}

TEST_F(FoldConstantExprVisitorTest, TestMapExpr) {
    // {"jack":1, "tom":pow(2, 2+1), "jerry":5-1} => {"jack":1, "tom":8, "jerry":4}
    auto expr = pool.add(mapExpr(
        {{"jack", constantExpr(1)},
         {"tom", fnExpr("pow", {constantExpr(2), addExpr(constantExpr(2), constantExpr(1))})},
         {"jerry", minusExpr(constantExpr(5), constantExpr(1))}}));
    auto expected = pool.add(
        mapExpr({{"jack", constantExpr(1)}, {"tom", constantExpr(8)}, {"jerry", constantExpr(4)}}));
    FoldConstantExprVisitor visitor;
    expr->accept(&visitor);
    ASSERT_EQ(*expr, *expected) << expr->toString() << " vs. " << expected->toString();
    ASSERT(visitor.canBeFolded());
}

TEST_F(FoldConstantExprVisitorTest, TestCaseExpr) {
    // CASE pow(2, (2+1)) WHEN (2+3) THEN (5-1) ELSE (7+8)
    auto expr = pool.add(caseExpr(fnExpr("pow", {constantExpr(2),
                                  addExpr(constantExpr(2), constantExpr(1))}),
                                  addExpr(constantExpr(7), constantExpr(8)),
                                  addExpr(constantExpr(2), constantExpr(3)),
                                  minusExpr(constantExpr(5), constantExpr(1))));
    auto expected = pool.add(caseExpr(constantExpr(8), constantExpr(15),
                                      constantExpr(5), constantExpr(4)));
    FoldConstantExprVisitor visitor;
    expr->accept(&visitor);
    ASSERT_EQ(*expr, *expected) << expr->toString() << " vs. " << expected->toString();
    ASSERT(visitor.canBeFolded());

    // CASE 8 WHEN 5 THEN 4 ELSE 15 => 15
    auto root = pool.add(visitor.fold(expr));
    auto rootExpected = pool.add(constantExpr(15));
    ASSERT_EQ(*root, *rootExpected) << root->toString() << " vs. " << rootExpected->toString();
}

TEST_F(FoldConstantExprVisitorTest, TestFoldFunction) {
    // pure function
    // abs(-1) + 1 => 1 + 1
    {
        auto expr = pool.add(addExpr(fnExpr("abs", {constantExpr(-1)}), constantExpr(1)));
        auto expected = pool.add(addExpr(constantExpr(1), constantExpr(1)));
        FoldConstantExprVisitor visitor;
        expr->accept(&visitor);
        ASSERT_TRUE(visitor.canBeFolded());
        ASSERT_EQ(*expr, *expected) << expr->toString() << " vs. " << expected->toString();
    }
    // not pure function
    // rand32(4) + 1 => rand32(4) + 1
    {
        auto expr = pool.add(addExpr(fnExpr("rand32", {constantExpr(4)}), constantExpr(1)));
        auto expected = pool.add(addExpr(fnExpr("rand32", {constantExpr(4)}), constantExpr(1)));
        FoldConstantExprVisitor visitor;
        expr->accept(&visitor);
        ASSERT_FALSE(visitor.canBeFolded());
        ASSERT_EQ(*expr, *expected) << expr->toString() << " vs. " << expected->toString();
    }
}

TEST_F(FoldConstantExprVisitorTest, TestFoldFailed) {
    // function call
    {
        // pow($v, (1+2)) => pow($v, 3)
        auto expr =
            pool.add(fnExpr("pow", {varExpr("v"), addExpr(constantExpr(1), constantExpr(2))}));
        auto expected = pool.add(fnExpr("pow", {varExpr("v"), constantExpr(3)}));
        FoldConstantExprVisitor visitor;
        expr->accept(&visitor);
        ASSERT_EQ(*expr, *expected) << expr->toString() << " vs. " << expected->toString();
        ASSERT_FALSE(visitor.canBeFolded());
    }
    // list
    {
        // [$v, pow(1, 2), 1+2][2-1] => [$v, 1, 3][0]
        auto expr = pool.add(subExpr(listExpr({varExpr("v"),
                                               fnExpr("pow", {constantExpr(1), constantExpr(2)}),
                                               addExpr(constantExpr(1), constantExpr(2))}),
                                     minusExpr(constantExpr(1), constantExpr(1))));
        auto expected = pool.add(
            subExpr(listExpr({varExpr("v"), constantExpr(1), constantExpr(3)}), constantExpr(0)));
        FoldConstantExprVisitor visitor;
        expr->accept(&visitor);
        ASSERT_EQ(*expr, *expected) << expr->toString() << " vs. " << expected->toString();
        ASSERT_FALSE(visitor.canBeFolded());
    }
}

TEST_F(FoldConstantExprVisitorTest, TestPathBuild) {
    auto expr = pool.makeAndAdd<PathBuildExpression>();
    expr->add(std::unique_ptr<FunctionCallExpression>(fnExpr("upper", {constantExpr("tom")})));
    FoldConstantExprVisitor fold;
    expr->accept(&fold);

    auto expected = pool.makeAndAdd<PathBuildExpression>();
    expected->add(std::unique_ptr<ConstantExpression>(constantExpr("TOM")));
    ASSERT_EQ(*expr, *expected);
}
}   // namespace graph
}   // namespace nebula
