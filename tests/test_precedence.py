from lox_tests import run_lox_script

def precedence():
  return run_lox_script("../clox/clox", "lox_scripts/precedence.lox")

def test_precedence():
  result = precedence()
  assert result.stdout == """14
8
4
0
true
true
true
true
0
0
0
0
4
"""
  assert result.stderr == ""
