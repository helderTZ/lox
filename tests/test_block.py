from lox_tests import run_lox_script

def block(test):
  return run_lox_script("../clox/clox", "lox_scripts/block/" + test + ".lox")

def test_empty():
  result = block("empty")
  assert result.stdout == "ok\n"
  assert result.stderr == ""

def test_scope():
  result = block("scope")
  assert result.stdout == "inner\nouter\n"
  assert result.stderr == ""