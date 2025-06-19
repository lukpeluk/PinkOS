#ifndef TESTS_H
#define TESTS_H


// Para usar la macro se tiene que seguir el estándar de tener las variables globales
// total_tests, tests_passed y tests_failed, donde se van a ir guardando los resultados de los tests
#define ASSERT(condition, test_name) do { \
    total_tests++; \
    if (condition) { \
        tests_passed++; \
        // log_to_serial("S: PASS: " test_name); \
    } else { \
        tests_failed++; \
        log_to_serial("E: FAIL: " test_name); \
    } \
} while(0)

#define ASSERT_EQUALS(expected, actual, test_name) do { \
    total_tests++; \
    if ((expected) == (actual)) { \
        tests_passed++; \
        // log_to_serial("S: PASS: " test_name); \
    } else { \
        tests_failed++; \
        log_to_serial("E: FAIL: " test_name); \
        log_to_serial("E:   Expected: "); \
        // log_decimal("", (uint64_t)(expected)); \
        log_to_serial("E:   Actual: "); \
        // log_decimal("", (uint64_t)(actual)); \
    } \
} while(0)


// --- Tests ---

void schedulingTest(char *args);


#endif // TESTS_H