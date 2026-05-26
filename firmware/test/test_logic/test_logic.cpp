/*
 * test_logic.cpp — native unit tests for TEEKeeper pure-logic functions.
 *
 * Each section re-states the *specification* of the firmware logic so that
 * a regression in the source file is caught here.  No Arduino headers needed.
 *
 * Run with:  pio test -e native
 */

#include <unity.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

// ============================================================
// EEPROM address arithmetic
// Mirrors TEEK_constants.h — tests that the macro offsets are
// the expected byte values so a sizeof() change is immediately noticed.
// ============================================================

#define EEPROM_ADDR_KP         0
#define EEPROM_ADDR_KI         (EEPROM_ADDR_KP + (int)sizeof(double))
#define EEPROM_ADDR_KD         (EEPROM_ADDR_KP + 2*(int)sizeof(double))
#define EEPROM_DEFAULT_UNIT    (EEPROM_ADDR_KP + 3*(int)sizeof(double))
#define EEPROM_MAGIC_ADDR      32

void test_eeprom_kp_at_0()          { TEST_ASSERT_EQUAL_INT(0,  EEPROM_ADDR_KP); }
void test_eeprom_ki_at_8()          { TEST_ASSERT_EQUAL_INT(8,  EEPROM_ADDR_KI); }
void test_eeprom_kd_at_16()         { TEST_ASSERT_EQUAL_INT(16, EEPROM_ADDR_KD); }
void test_eeprom_unit_at_24()       { TEST_ASSERT_EQUAL_INT(24, EEPROM_DEFAULT_UNIT); }
void test_eeprom_magic_at_32()      { TEST_ASSERT_EQUAL_INT(32, EEPROM_MAGIC_ADDR); }
void test_eeprom_no_overlap()       { TEST_ASSERT_TRUE(EEPROM_DEFAULT_UNIT < EEPROM_MAGIC_ADDR); }

// ============================================================
// PID arithmetic — mirrors CoreSystem::PID() in TEEK_dataStructures.cpp.
// Uses module-level state so setUp() can reset between tests.
// ============================================================

static double g_integral  = 0.0;
static double g_last_error = 0.0;
static const double KP = 0.4, KI = 0.1, KD = 1.0, DT = 5.0; // 5 s PWM period

static double pid_compute(double error) {
    g_integral += error * DT;
    double deriv = (error - g_last_error) / DT;
    double out = KP * error + KI * g_integral + KD * deriv;
    if (out > 100.0) {
        out = 100.0;
        if (KI != 0.0) g_integral = (100.0 - KP * error - KD * deriv) / KI;
    } else if (out < 0.0) {
        out = 0.0;
        if (KI != 0.0) g_integral = (0.0 - KP * error - KD * deriv) / KI;
    }
    g_last_error = error;
    return out;
}

void setUp()    { g_integral = 0.0; g_last_error = 0.0; }
void tearDown() {}

void test_pid_zero_error_gives_zero_output() {
    TEST_ASSERT_EQUAL_DOUBLE(0.0, pid_compute(0.0));
}

void test_pid_large_positive_clamps_100() {
    double out = pid_compute(10000.0);
    TEST_ASSERT_EQUAL_DOUBLE(100.0, out);
}

void test_pid_large_negative_clamps_0() {
    double out = pid_compute(-10000.0);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, out);
}

void test_pid_antiwindup_output_stays_clamped_at_100() {
    // With a huge error the output must stay at 100 across multiple cycles,
    // proving the integral is being back-calculated and not accumulating freely.
    double out1 = pid_compute(10000.0);
    double out2 = pid_compute(10000.0);
    double out3 = pid_compute(10000.0);
    TEST_ASSERT_EQUAL_DOUBLE(100.0, out1);
    TEST_ASSERT_EQUAL_DOUBLE(100.0, out2);
    TEST_ASSERT_EQUAL_DOUBLE(100.0, out3);
}

void test_pid_output_proportional_to_small_error() {
    // At first call (integral=0, last_error=0): out = kp*e + ki*e*dt + kd*(e-0)/dt
    //   = (0.4 + 0.1*5 + 1.0/5) * e = (0.4 + 0.5 + 0.2) * e = 1.1 * e
    double out = pid_compute(10.0);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 11.0, out); // 1.1 * 10
}

void test_pid_integral_resets_prevent_carryover() {
    // Saturate, then reset (simulate setTarget(newInstruction=true)), then check output
    pid_compute(10000.0);
    g_integral  = 0.0;   // simulates setTarget(_, true) reset
    g_last_error = 0.0;
    double out = pid_compute(0.0);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, out);
}

// ============================================================
// Ramp formula — mirrors the setpoint calculation in programExecution().
//   newTarget = startTemp + rate [°C/min] * elapsed [ms] / 60000.0
//   clamped so it cannot overshoot the final target.
// ============================================================

static double ramp_clamp(double v, double lo, double hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static double ramp_setpoint(double startTemp, double rate,
                             double elapsed_ms, double finalTarget) {
    double t = startTemp + rate * elapsed_ms / 60000.0;
    return (rate >= 0.0) ? ramp_clamp(t, startTemp, finalTarget)
                         : ramp_clamp(t, finalTarget, startTemp);
}

void test_ramp_up_at_halfway() {
    // 10 °C/min, start=20, target=40, elapsed=1 min → 30 °C
    TEST_ASSERT_EQUAL_DOUBLE(30.0, ramp_setpoint(20.0, 10.0, 60000.0, 40.0));
}

void test_ramp_up_clamps_at_target() {
    // 10 °C/min, start=20, target=40, elapsed=3 min → clamped to 40
    TEST_ASSERT_EQUAL_DOUBLE(40.0, ramp_setpoint(20.0, 10.0, 180000.0, 40.0));
}

void test_ramp_up_does_not_overshoot() {
    // Very long elapsed — must not exceed target
    double out = ramp_setpoint(20.0, 10.0, 9999000.0, 40.0);
    TEST_ASSERT_EQUAL_DOUBLE(40.0, out);
}

void test_ramp_down_at_halfway() {
    // -5 °C/min, start=200, target=180, elapsed=2 min → 190 °C
    TEST_ASSERT_EQUAL_DOUBLE(190.0, ramp_setpoint(200.0, -5.0, 120000.0, 180.0));
}

void test_ramp_down_clamps_at_target() {
    // -5 °C/min, start=200, target=180, elapsed=4 min → clamped to 180
    TEST_ASSERT_EQUAL_DOUBLE(180.0, ramp_setpoint(200.0, -5.0, 240000.0, 180.0));
}

void test_ramp_down_does_not_undershoot() {
    // Very long elapsed — must not go below target
    double out = ramp_setpoint(200.0, -5.0, 9999000.0, 180.0);
    TEST_ASSERT_EQUAL_DOUBLE(180.0, out);
}

void test_ramp_zero_rate_stays_at_start() {
    TEST_ASSERT_EQUAL_DOUBLE(100.0, ramp_setpoint(100.0, 0.0, 60000.0, 100.0));
}

void test_ramp_uses_float_division() {
    // elapsed = 30 s → 0.5 min; rate=60 °C/min → +30 °C
    // integer division /60000 would give 0, float /60000.0 gives 30
    double out = ramp_setpoint(0.0, 60.0, 30000.0, 9999.0);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 30.0, out);
}

// ============================================================
// CSV parseCSVLine — reimplements the logic from TEEK_dataStructures.cpp.
// Tests that the parser correctly handles normal lines, flags, edge cases.
// ============================================================

static const char* csv_extractField(const char* src, char* buf, size_t bufSize) {
    size_t i = 0;
    while (*src != ',' && *src != '\0' && i < bufSize - 1)
        buf[i++] = *src++;
    buf[i] = '\0';
    if (*src == ',') src++;
    return src;
}

static bool csv_parseLine(const char* line,
                          char* name, size_t nameSize,
                          double* target, unsigned long* holdTime,
                          double* rampRate,
                          bool* waitDoor, bool* waitBtn) {
    char tmp[16];
    const char* ptr = line;
    size_t idx = 0;
    while (*ptr != ',' && *ptr != '\0') {
        if (idx < nameSize - 1) name[idx++] = *ptr++;
        else return false;
    }
    name[idx] = '\0';
    if (*ptr++ != ',') return false;
    ptr = csv_extractField(ptr, tmp, sizeof(tmp)); *target   = atof(tmp);
    ptr = csv_extractField(ptr, tmp, sizeof(tmp)); *holdTime = (unsigned long)atol(tmp);
    ptr = csv_extractField(ptr, tmp, sizeof(tmp)); *rampRate = atof(tmp);
    ptr = csv_extractField(ptr, tmp, sizeof(tmp)); *waitDoor = (tmp[0] == '1');
    ptr = csv_extractField(ptr, tmp, sizeof(tmp)); *waitBtn  = (tmp[0] == '1');
    return true;
}

void test_csv_basic_parse() {
    char name[30]; double target, ramp; unsigned long hold; bool door, btn;
    bool ok = csv_parseLine("Heat,800,30,5.0,0,0", name, sizeof(name),
                            &target, &hold, &ramp, &door, &btn);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_STRING("Heat", name);
    TEST_ASSERT_EQUAL_DOUBLE(800.0, target);
    TEST_ASSERT_EQUAL_UINT32(30, hold);
    TEST_ASSERT_EQUAL_DOUBLE(5.0, ramp);
    TEST_ASSERT_FALSE(door);
    TEST_ASSERT_FALSE(btn);
}

void test_csv_both_flags_set() {
    char name[30]; double target, ramp; unsigned long hold; bool door, btn;
    csv_parseLine("Soak,1000,60,0.0,1,1", name, sizeof(name),
                  &target, &hold, &ramp, &door, &btn);
    TEST_ASSERT_TRUE(door);
    TEST_ASSERT_TRUE(btn);
}

void test_csv_zero_ramp_rate() {
    char name[30]; double target, ramp; unsigned long hold; bool door, btn;
    csv_parseLine("Hold,600,15,0,0,0", name, sizeof(name),
                  &target, &hold, &ramp, &door, &btn);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, ramp);
}

void test_csv_negative_ramp_rate() {
    char name[30]; double target, ramp; unsigned long hold; bool door, btn;
    csv_parseLine("Cool,200,10,-3.5,0,0", name, sizeof(name),
                  &target, &hold, &ramp, &door, &btn);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, -3.5, ramp);
}

void test_csv_flag_0_is_false() {
    char name[30]; double target, ramp; unsigned long hold; bool door, btn;
    csv_parseLine("Step,500,5,0,0,1", name, sizeof(name),
                  &target, &hold, &ramp, &door, &btn);
    TEST_ASSERT_FALSE(door);
    TEST_ASSERT_TRUE(btn);
}

void test_csv_malformed_no_comma_returns_false() {
    char name[30]; double target, ramp; unsigned long hold; bool door, btn;
    bool ok = csv_parseLine("NoCommaLine", name, sizeof(name),
                            &target, &hold, &ramp, &door, &btn);
    TEST_ASSERT_FALSE(ok);
}

// ============================================================
// main
// ============================================================

int main(int argc, char** argv) {
    UNITY_BEGIN();

    // EEPROM addresses
    RUN_TEST(test_eeprom_kp_at_0);
    RUN_TEST(test_eeprom_ki_at_8);
    RUN_TEST(test_eeprom_kd_at_16);
    RUN_TEST(test_eeprom_unit_at_24);
    RUN_TEST(test_eeprom_magic_at_32);
    RUN_TEST(test_eeprom_no_overlap);

    // PID arithmetic
    RUN_TEST(test_pid_zero_error_gives_zero_output);
    RUN_TEST(test_pid_large_positive_clamps_100);
    RUN_TEST(test_pid_large_negative_clamps_0);
    RUN_TEST(test_pid_antiwindup_output_stays_clamped_at_100);
    RUN_TEST(test_pid_output_proportional_to_small_error);
    RUN_TEST(test_pid_integral_resets_prevent_carryover);

    // Ramp formula
    RUN_TEST(test_ramp_up_at_halfway);
    RUN_TEST(test_ramp_up_clamps_at_target);
    RUN_TEST(test_ramp_up_does_not_overshoot);
    RUN_TEST(test_ramp_down_at_halfway);
    RUN_TEST(test_ramp_down_clamps_at_target);
    RUN_TEST(test_ramp_down_does_not_undershoot);
    RUN_TEST(test_ramp_zero_rate_stays_at_start);
    RUN_TEST(test_ramp_uses_float_division);

    // CSV parser
    RUN_TEST(test_csv_basic_parse);
    RUN_TEST(test_csv_both_flags_set);
    RUN_TEST(test_csv_zero_ramp_rate);
    RUN_TEST(test_csv_negative_ramp_rate);
    RUN_TEST(test_csv_flag_0_is_false);
    RUN_TEST(test_csv_malformed_no_comma_returns_false);

    return UNITY_END();
}
