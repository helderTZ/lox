package com.craftinginterpreters.lox;

import java.util.ArrayList;

class AstPrinter implements Expr.Visitor<String> {
  String print(Expr expr) {
    return expr.accept(this);
  }

  @Override
  public String visitTernaryExpr(Expr.Ternary expr) {
    return parenthesize("?", expr.test, expr.if_true,
                        expr.if_false);
  }

  @Override
  public String visitBinaryExpr(Expr.Binary expr) {
    return parenthesize(expr.operator.lexeme,
                        expr.left, expr.right);
  }

  @Override
  public String visitGroupingExpr(Expr.Grouping expr) {
    return parenthesize("group", expr.expression);
  }

  @Override
  public String visitLiteralExpr(Expr.Literal expr) {
    if (expr.value == null) return "nil";
    return expr.value.toString();
  }

  @Override
  public String visitUnaryExpr(Expr.Unary expr) {
    return parenthesize(expr.operator.lexeme, expr.right);
  }

  @Override
  public String visitVariableExpr(Expr.Variable expr) {
    return "var" + expr.name;
  }

  @Override
  public String visitAssignExpr(Expr.Assign expr) {
    return "(" + "= " + expr.name.lexeme + " " + expr.value + ")";
  }

  @Override
  public String visitLogicalExpr(Expr.Logical expr) {
    return parenthesize(expr.operator.lexeme,
                        expr.left, expr.right);
  }

  @Override
  public String visitSetExpr(Expr.Set expr) {
    return "(" + expr.object + "." + expr.name + " = " + expr.value + ")";
  }

  @Override
  public String visitGetExpr(Expr.Get expr) {
    return "(" + expr.object + "." + expr.name + ")";
  }

  @Override
  public String visitThisExpr(Expr.This expr) {
    return "(this)";
  }

  @Override
  public String visitSuperExpr(Expr.Super expr) {
    return "(super." + expr.method + ")";
  }

  @Override
  public String visitCallExpr(Expr.Call expr) {
    String str = "(" + expr.callee + "( ";
    ArrayList<String> args = new ArrayList<String>();
    for(Expr e : expr.arguments) {
      args.add(new AstPrinter().print(e));
    }
    str = str + String.join(", ", args) + "))";
    return str;
  }

  private String parenthesize(String name, Expr... exprs) {
    StringBuilder builder = new StringBuilder();

    builder.append("(").append(name);
    for (Expr expr : exprs) {
      builder.append(" ");
      builder.append(expr.accept(this));
    }
    builder.append(")");

    return builder.toString();
  }

  // public static void main(String[] args) {
  //   Expr expression = new Expr.Binary(
  //       new Expr.Unary(
  //           new Token(TokenType.MINUS, "-", null, 1),
  //           new Expr.Literal(123)),
  //       new Token(TokenType.STAR, "*", null, 1),
  //       new Expr.Grouping(
  //           new Expr.Literal(45.67)));

  //   System.out.println(new AstPrinter().print(expression));
  // }

}
