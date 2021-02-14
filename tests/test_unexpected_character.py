from lox_tests import run_lox_script

def unexpected_character():
  return run_lox_script("../clox/clox", "lox_scripts/unexpected_character.lox")

def test_unexpected_character():
  result = unexpected_character()
  assert result.stdout == ""
  assert result.stderr == "[line 3] Error: Unexpected character.\n"
