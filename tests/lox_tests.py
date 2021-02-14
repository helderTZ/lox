# tests taken from https://github.com/munificent/craftinginterpreters

import argparse
import subprocess
import sys
import os
import pytest

def run_lox_script(lox_exec, script):
  return subprocess.run([lox_exec, script], capture_output=True, text=True)

def run_test(lox_exec, script, expected_out, expected_err):
  result = run_lox_script(lox_exec, script)
  assert result.stdout == expected_out
  assert result.stderr == expected_err

def main(lox_exec):
  run_test(lox_exec, "empty_file.lox", "", "")
  run_test(lox_exec, "precedence.lox", "14\n8\n4\n0\ntrue\ntrue\ntrue\n0\n0\n0\n0\n4", "")
  run_test(lox_exec, "unexpected_character.lox", "", "[line 3] Error: Unexpected character.")

if __name__ == "__main__":
  parser = argparse.ArgumentParser()
  parser.add_argument('--lox', action='store', required=True, help='Path to Lox interpreter.')

  args = parser.parse_args()
  lox_interpreter_path = os.path.abspath(args.lox)
  main(lox_interpreter_path)
