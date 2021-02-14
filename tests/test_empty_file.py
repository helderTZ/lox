from lox_tests import run_lox_script

def empty_file():
  return run_lox_script("../clox/clox", "lox_scripts/empty_file.lox")

def test_empty_file():
  result = empty_file()
  assert result.stdout == ""
  assert result.stderr == ""
