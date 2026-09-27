"""Run with python3 tests/check_math.py; requires a host C compiler."""
import ctypes
import math
from pathlib import Path
import subprocess
import tempfile

SOURCE = Path(__file__).resolve().parents[1] / "src"
# Capture the public registration callbacks so NaN/Inf can be tested directly.
HARNESS = """
#include "g2basic_math.h"
#include <string.h>
typedef double (*function)(double[], int);
static function callbacks[13];
static const char *names[13];
static int used;
int g2basic_register_function(const char *name, int count, function callback) {
    (void)count;
    names[used] = name;
    callbacks[used++] = callback;
    return 0;
}
double evaluate(const char *name, double a, double b) {
    double args[] = {a, b};
    for (int i = 0; i < used; ++i)
        if (!strcmp(names[i], name))
            return callbacks[i](args, !strcmp(name, "pow") ||
                !strcmp(name, "min") || !strcmp(name, "max") ? 2 : 1);
    return 123456789;
}
"""
with tempfile.TemporaryDirectory() as directory:
    root = Path(directory)
    harness = root / "harness.c"
    harness.write_text(HARNESS)
    for mode in (0, 1):
        library = root / f"math{mode}.so"
        source = "g2basic_math.c" if mode else "g2basic_math_crude.c"
        subprocess.run([
            "cc", "-shared", "-fPIC", "-Wall", "-Wextra", "-Werror",
            "-I", str(SOURCE),
            str(harness), str(SOURCE / source), "-o", str(library),
            *(["-lm"] if mode else []),
        ], check=True)
        if not mode:
            symbols = subprocess.check_output(["nm", "-u", str(library)], text=True)
            for name in ("sin", "cos", "tan", "sqrt", "pow", "log", "log10",
                         "exp", "floor", "ceil", "fabs"):
                assert not any(line.split()[-1].split("@")[0] == name
                               for line in symbols.splitlines()), symbols
        lib = ctypes.CDLL(str(library))
        lib.g2basic_init_math_functions()
        lib.evaluate.argtypes = [ctypes.c_char_p, ctypes.c_double, ctypes.c_double]
        lib.evaluate.restype = ctypes.c_double
        def evaluate(name, a, b=0):
            return lib.evaluate(name.encode(), a, b)
        for name in ("sin", "cos", "tan", "sqrt", "abs", "log", "log10",
                     "exp", "floor", "ceil"):
            reference = abs if name == "abs" else getattr(math, name)
            values = [0.001, 0.125, 0.5, 1, 2, 10, 100]
            if name not in ("sqrt", "log", "log10"):
                values += [-100, -2.5, -0.5, 0]
            for value in values:
                result = evaluate(name, value)
                expected = reference(value)
                assert math.isclose(result, expected, rel_tol=0.001,
                                    abs_tol=0.00001), (mode, name, value, result, expected)
        for a, b in [(2, 10), (2, -10), (9, 0.5), (-2, 3), (-2, -3), (0, 2)]:
            assert math.isclose(evaluate("pow", a, b), a**b, rel_tol=1e-8)
        assert evaluate("min", -2, 4) == -2
        assert evaluate("max", -2, 4) == 4
        for name, a in [("sqrt", -1), ("log", -1), ("log10", -1),
                        ("sin", math.inf), ("cos", math.nan)]:
            assert math.isnan(evaluate(name, a))
        assert math.isnan(evaluate("pow", -2, 0.5))
        assert evaluate("exp", 1000) == math.inf
        assert evaluate("exp", -1000) == 0
        for value in (5e-324, 1e-300, 1e300):
            assert math.isclose(evaluate("sqrt", value), math.sqrt(value), rel_tol=1e-10)
            assert math.isclose(evaluate("log", value), math.log(value), rel_tol=1e-10)
        if not mode:
            assert math.isnan(evaluate("sin", 1e20))
            assert math.isnan(evaluate("tan", math.pi / 2))
        print(f"PASS: math mode {mode}, values, domains, extremes, linkage")
