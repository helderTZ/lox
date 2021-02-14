from lox_tests import run_lox_script

def assignment(test):
  return run_lox_script("../clox/clox", "lox_scripts/assignment/" + test + ".lox")

def test_associativity():
  result = assignment("associativity")
  assert result.stdout == "c\nc\nc\n"
  assert result.stderr == ""

def test_global():
  result = assignment("global")
  assert result.stdout == "before\nafter\narg\narg\n"
  assert result.stderr == ""

def test_local():
  result = assignment("local")
  assert result.stdout == "before\nafter\narg\narg\n"
  assert result.stderr == ""

def test_grouping():
  result = assignment("grouping")
  assert result.stdout == ""
  assert result.stderr == "[line 2] Error at '=': Invalid assignment target.\n"

def test_infix_operator():
  result = assignment("infix_operator")
  assert result.stdout == ""
  assert result.stderr == "[line 3] Error at '=': Invalid assignment target.\n"

def test_prefix_operator():
  result = assignment("prefix_operator")
  assert result.stdout == ""
  assert result.stderr == "[line 2] Error at '=': Invalid assignment target.\n"

def test_syntax():
  result = assignment("syntax")
  assert result.stdout == "var\nvar\n"
  assert result.stderr == ""

def test_undefined():
  result = assignment("undefined")
  assert result.stdout == ""
  assert result.stderr == "Undefined variable 'unknown'.\n[line 1] in script\n"