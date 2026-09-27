/* SPDX-License-Identifier: MIT */
#include <math.h>
#include "g2basic.h"
#include "g2basic_math.h"

/* Small approximations for embedded builds without libm. */
static double basic_abs(double x) {
    return x < 0 ? -x : x;
}

static double basic_floor(double x) {
    if (!isfinite(x) || basic_abs(x) >= 4503599627370496.0)
        return x;
    long long whole = (long long)x;
    return (double)whole > x ? (double)whole - 1 : (double)whole;
}

static double basic_ceil(double x) {
    return -basic_floor(-x);
}

/* Restrict argument reduction to a range where this crude method is useful. */
static double reduce_angle(double x) {
    const double tau = 6.2831853071795864769;
    if (!isfinite(x) || basic_abs(x) > 1000000)
        return NAN;
    return x - basic_floor(x / tau + 0.5) * tau;
}

static double basic_sin(double x) {
    const double pi = 3.14159265358979323846;
    x = reduce_angle(x);
    if (x > pi / 2)
        x = pi - x;
    if (x < -pi / 2)
        x = -pi - x;
    double x2 = x * x;
    return x * (1 + x2 * (-1.0 / 6 +
                          x2 * (1.0 / 120 + x2 * (-1.0 / 5040 + x2 / 362880))));
}

static double basic_cos(double x) {
    return basic_sin(reduce_angle(x) + 1.57079632679489661923);
}

static double basic_tan(double x) {
    double c = basic_cos(x);
    if (basic_abs(c) < 0.00001)
        return NAN;
    return basic_sin(x) / c;
}

static double basic_sqrt(double x) {
    if (x < 0)
        return NAN;
    if (x == 0 || !isfinite(x))
        return x;
    double scale = 1;
    while (x >= 4) {
        x *= 0.25;
        scale *= 2;
    }
    while (x < 1) {
        x *= 4;
        scale *= 0.5;
    }
    double root = 1;
    for (int i = 0; i < 7; i++)
        root = 0.5 * (root + x / root);
    return root * scale;
}

static double basic_log(double x) {
    if (x <= 0)
        return NAN;
    if (!isfinite(x))
        return x;
    int exponent = 0;
    while (x >= 2) {
        x *= 0.5;
        exponent++;
    }
    while (x < 1) {
        x *= 2;
        exponent--;
    }
    double z = (x - 1) / (x + 1);
    double term = z, sum = z;
    for (int n = 3; n <= 19; n += 2) {
        term *= z * z;
        sum += term / n;
    }
    return 2 * sum + exponent * 0.69314718055994530942;
}

static double basic_log10(double x) {
    return basic_log(x) / 2.30258509299404568402;
}

static double basic_exp(double x) {
    if (isnan(x))
        return x;
    if (x > 709.782712893384)
        return INFINITY;
    if (x < -745.133219101941)
        return 0;
    int exponent = (int)basic_floor(x / 0.69314718055994530942);
    double remainder = x - exponent * 0.69314718055994530942;
    double term = 1, result = 1;
    for (int n = 1; n <= 12; n++) {
        term *= remainder / n;
        result += term;
    }
    while (exponent > 0) {
        result *= 2;
        exponent--;
    }
    while (exponent < 0) {
        result *= 0.5;
        exponent++;
    }
    return result;
}

static double basic_pow(double base, double exponent) {
    if (exponent == 0 || base == 1)
        return 1;
    if (isnan(base) || isnan(exponent))
        return NAN;
    if (base == 0)
        return exponent > 0 ? 0 : INFINITY;
    if (!isfinite(exponent)) {
        double magnitude = basic_abs(base);
        if (magnitude == 1)
            return 1;
        return (magnitude > 1) == (exponent > 0) ? INFINITY : 0;
    }
    if (basic_floor(exponent) == exponent &&
        basic_abs(exponent) < 9007199254740992.0) {
        unsigned long long n = (unsigned long long)basic_abs(exponent);
        if (exponent < 0)
            base = 1 / base;
        double result = 1;
        while (n) {
            if (n & 1)
                result *= base;
            n >>= 1;
            if (n)
                base *= base;
        }
        return result;
    }
    if (base < 0)
        return NAN;
    return basic_exp(exponent * basic_log(base));
}

static double func_sin(double args[], int count) {
    if (count != 1)
        return NAN;
    return basic_sin(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_cos(double args[], int count) {
    if (count != 1)
        return NAN;
    return basic_cos(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_tan(double args[], int count) {
    if (count != 1)
        return NAN;
    return basic_tan(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_sqrt(double args[], int count) {
    if (count != 1)
        return NAN;
    if (args[0] < 0)
        return NAN;
    return basic_sqrt(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_abs(double args[], int count) {
    if (count != 1)
        return NAN;
    return basic_abs(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_pow(double args[], int count) {
    if (count != 2)
        return NAN;
    return basic_pow(args[0], args[1]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_log(double args[], int count) {
    if (count != 1)
        return NAN;
    if (args[0] <= 0)
        return NAN;
    return basic_log(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_log10(double args[], int count) {
    if (count != 1)
        return NAN;
    if (args[0] <= 0)
        return NAN;
    return basic_log10(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_exp(double args[], int count) {
    if (count != 1)
        return NAN;
    return basic_exp(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_floor(double args[], int count) {
    if (count != 1)
        return NAN;
    return basic_floor(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_ceil(double args[], int count) {
    if (count != 1)
        return NAN;
    return basic_ceil(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_min(double args[], int count) {
    if (count < 1)
        return NAN;
    double min_val = args[0];
    for (int i = 1; i < count; i++) {
        if (args[i] < min_val)
            min_val = args[i];
    }
    return min_val;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_max(double args[], int count) {
    if (count < 1)
        return NAN;
    double max_val = args[0];
    for (int i = 1; i < count; i++) {
        if (args[i] > max_val)
            max_val = args[i];
    }
    return max_val;
}
/*--------------------------------------------------------------------------------------------------------------------*/
void g2basic_init_math_functions(void) {
    g2basic_register_function("sin", 1, func_sin);
    g2basic_register_function("cos", 1, func_cos);
    g2basic_register_function("tan", 1, func_tan);
    g2basic_register_function("sqrt", 1, func_sqrt);
    g2basic_register_function("abs", 1, func_abs);
    g2basic_register_function("pow", 2, func_pow);
    g2basic_register_function("log", 1, func_log);
    g2basic_register_function("log10", 1, func_log10);
    g2basic_register_function("exp", 1, func_exp);
    g2basic_register_function("floor", 1, func_floor);
    g2basic_register_function("ceil", 1, func_ceil);
    g2basic_register_function("min", -1, func_min);
    g2basic_register_function("max", -1, func_max);
}
/*--------------------------------------------------------------------------------------------------------------------*/
