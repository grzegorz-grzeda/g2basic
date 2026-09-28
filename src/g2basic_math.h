/* SPDX-License-Identifier: MIT */
/**
 * @file g2basic_math.h
 * @brief Internal registration hook for the built-in functions.
 */
#ifndef G2BASIC_MATH_H
#define G2BASIC_MATH_H

/**
 * @defgroup math_backends Built-in function backends
 * @ingroup internals
 * @brief Build-time choice of built-in functions, with or without libm.
 *
 * CMake compiles exactly one backend source, selected by
 * `G2BASIC_ENABLE_MATH_FUNCTIONS` and `G2BASIC_ENABLE_MATH`:
 *
 * | Backend | `..._FUNCTIONS` | `..._MATH` | Registers | libm |
 * | --- | --- | --- | --- | --- |
 * | `g2basic_math.c` | ON | ON | all 13 | yes |
 * | `g2basic_math_crude.c` | ON | OFF | all 13, approximated | no |
 * | `g2basic_math_none.c` | OFF | any | `min`, `max` | no |
 *
 * The 13 functions are `sin`, `cos`, `tan`, `sqrt`, `abs`, `pow`, `log`,
 * `log10`, `exp`, `floor`, `ceil`, `min`, and `max`. `pow` takes two
 * arguments. `min` and `max` take up to eight and return 0 with none. The
 * others take one.
 * @{
 */

/**
 * @brief Register the built-in functions of the selected backend.
 *
 * Called by g2basic_init() after it clears all registered functions. Host
 * programs do not call it.
 */
void g2basic_init_math_functions(void);

/** @} */

#endif /* G2BASIC_MATH_H */
