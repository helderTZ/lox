from lox_tests import run_lox_script

def comments(test):
  return run_lox_script("../clox/clox", "lox_scripts/comments/" + test + ".lox")

def test_line_at_eof():
  result = comments("line_at_eof")
  assert result.stdout == "ok\n"
  assert result.stderr == ""

def test_only_line_comment():
  result = comments("only_line_comment")
  assert result.stdout == ""
  assert result.stderr == ""

def test_unicode():
  result = comments("unicode")
  assert result.stdout == "ok\n"
  assert result.stderr == ""