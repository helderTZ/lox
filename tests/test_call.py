from lox_tests import run_lox_script

def call(test):
  return run_lox_script("../clox/clox", "lox_scripts/call/" + test + ".lox")

def test_bool():
  result = call("bool")
  assert result.stdout == ""
  assert result.stderr == "Can only call functions and classes.\n[line 1] in script\n"

def test_nil():
  result = call("nil")
  assert result.stdout == ""
  assert result.stderr == "Can only call functions and classes.\n[line 1] in script\n"

def test_num():
  result = call("num")
  assert result.stdout == ""
  assert result.stderr == "Can only call functions and classes.\n[line 1] in script\n"

# unimplemented
# def test_object():
#   result = call("object")
#   assert result.stdout == ""
#   assert result.stderr == "Can only call functions and classes.\n[line 1] in script\n"

def test_string():
  result = call("string")
  assert result.stdout == ""
  assert result.stderr == "Can only call functions and classes.\n[line 1] in script\n"