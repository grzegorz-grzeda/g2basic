/* SPDX-License-Identifier: MIT */
#include "g2basic_math.h"
#include "g2basic.h"
#include <math.h>

static double func_sin(double args[], int count) {
    if (count != 1)
        return NAN;
    return sin(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_cos(double args[], int count) {
    if (count != 1)
        return NAN;
    return cos(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_tan(double args[], int count) {
    if (count != 1)
        return NAN;
    return tan(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_sqrt(double args[], int count) {
    if (count != 1)
        return NAN;
    if (args[0] < 0)
        return NAN;
    return sqrt(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_abs(double args[], int count) {
    if (count != 1)
        return NAN;
    return fabs(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_pow(double args[], int count) {
    if (count != 2)
        return NAN;
    return pow(args[0], args[1]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_log(double args[], int count) {
    if (count != 1)
        return NAN;
    if (args[0] <= 0)
        return NAN;
    return log(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_log10(double args[], int count) {
    if (count != 1)
        return NAN;
    if (args[0] <= 0)
        return NAN;
    return log10(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_exp(double args[], int count) {
    if (count != 1)
        return NAN;
    return exp(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_floor(double args[], int count) {
    if (count != 1)
        return NAN;
    return floor(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_ceil(double args[], int count) {
    if (count != 1)
        return NAN;
    return ceil(args[0]);
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
