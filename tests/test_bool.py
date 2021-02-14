from lox_tests import run_lox_script

def bool(test):
  return run_lox_script("../clox/clox", "lox_scripts/bool/" + test + ".lox")

def test_equality():
  result = bool("equality")
  assert result.stdout == """true
false
false
true
false
false
false
false
false
false
true
true
false
true
true
true
true
true
"""
  assert result.stderr == ""

def test_not():
  result = bool("not")
  assert result.stdout == "false\ntrue\ntrue\n"
  assert result.stderr == ""