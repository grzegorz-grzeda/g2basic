/* SPDX-License-Identifier: MIT */
#include "g2basic.h"
#include "g2basic_math.h"
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_min(double args[], int count) {
    if (count < 1)
        return 0.0;
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
        return 0.0;
    double max_val = args[0];
    for (int i = 1; i < count; i++) {
        if (args[i] > max_val)
            max_val = args[i];
    }
    return max_val;
}
/*--------------------------------------------------------------------------------------------------------------------*/
void g2basic_init_math_functions(void) {
    g2basic_register_function("min", -1, func_min);
    g2basic_register_function("max", -1, func_max);
}
/*--------------------------------------------------------------------------------------------------------------------*/
