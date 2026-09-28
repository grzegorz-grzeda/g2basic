"""Run with python3 tests/check_nesting.py [interpreter] after a standalone build.

Checks that each construct nests exactly G2BASIC_MAX_NESTING levels (32 in a
standalone build), that IF-THEN levels and parentheses share the limit, and
that one level deeper fails cleanly without breaking the interpreter.
"""
import re
import subprocess
import sys

INTERPRETER = sys.argv[1] if len(sys.argv) > 1 else "build/examples/interactive/g2basic-interactive"
LIMIT = 32
ERROR = "Error: expression too deeply nested"


def run(*lines):
    result = subprocess.run([INTERPRETER], input="\n".join(lines) + "\n",
                            capture_output=True, text=True, timeout=10, check=True)
    return result.stdout


def nested(kind, depth):
    if kind == "parentheses":
        return "PRINT " + "(" * depth + "7" + ")" * depth
    if kind == "unary":
        return "PRINT " + "- " * depth + "7"
    if kind == "calls":
        return "PRINT " + "min(" * depth + "7" + ")" * depth
    # IF levels and parentheses share the limit. 10 IF levels plus parentheses
    # keep the line within the example's 255-character input buffer.
    inner = depth - 10
    return "10 " + "IF 1 = 1 THEN " * 10 + "PRINT " + "(" * inner + "7" + ")" * inner


for kind in ("parentheses", "unary", "calls", "if"):
    run_program = ("RUN",) if kind == "if" else ()
    at_limit = run(nested(kind, LIMIT), *run_program)
    # Values print after the "> " prompt; an even number of unary signs gives 7.
    assert ERROR not in at_limit and re.search(r"(?m)(^|> )-?7$", at_limit), (kind, at_limit)
    over_limit = run(nested(kind, LIMIT + 1), *run_program)
    assert "too deeply nested" in over_limit, (kind, over_limit)
    # The interpreter keeps working after the error.
    assert "42" in run(nested(kind, LIMIT + 1), *run_program, "PRINT 6 * 7"), kind
print(f"PASS: nesting limit {LIMIT} for parentheses, unary signs, calls, and IF-THEN")
