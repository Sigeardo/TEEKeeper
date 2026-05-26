#ifndef TEEK_CONSTANTS_H
#define TEEK_CONSTANTS_H

// ===== System Constants ===============================================

#define ERROR_BUFF_SIZE 80 // size of the error message buffer

// Time is expressed in milliseconds, unless otherwise stated
#define SECOND 1000
#define MINUTE SECOND * 60

// Time limits are expressed in milliseconds
#define MAX_TIME_DOOR_OPEN 5 * MINUTE * SECOND
#define MAX_HOLD_TIME 30 * MINUTE * SECOND
#define MAX_TIME_BETWEEN_TEMP_CHECKS 1 * SECOND

// Temperature limits are defined in celsius and converted later
#define MAX_TEMP_ERROR 2            // Max error for stability
#define MAX_TEMPERATURE 1100        // Max input temperature
#define MIN_TEMPERATURE 0           // Min input temperature
#define ERROR_TEMP 1200             // Upper temperature to trigger error
#define MIN_STABLE_CYCLES 10        // Number of PWM cycles to consider the temperature stable
#define POLL_PROBE_INTERVAL 5000    // Polling interval for temp reading when not firing


// PWM cycle time
#define CYCLE_TIME 5 * SECOND

// Maximum number of allowed consecutives temperature reading errors
// before considering the temperature probe faulty and stop the system
#define MAX_N_ERROR_READINGS 10

// Placeholder values for untuned systems
// TODO find a SIMPLE way to eyeball the default values from
// TODO the system characteristics (outside this code)
// TODO like a spreadsheet idk 
// TODO or a simple python script
// TODO no matlab simulink or other fancy stuff
#define PWM_DEFAULT_KP 0.4
#define PWM_DEFAULT_KI 0.5
#define PWM_DEFAULT_KD 0.2

// Encoder steps per click
#define ENCODER_STEPS 4


// Memory addresses for EEPROM
// nomen est omen
#define EEPROM_ADDR_KP 0
#define EEPROM_ADDR_KI EEPROM_ADDR_KP + sizeof(double)
#define EEPROM_ADDR_KD EEPROM_ADDR_KP + 2*sizeof(double)
#define EEPROM_DEFAULT_UNIT EEPROM_KP + 3*sizeof(double)

// Autotune parameters 
#define TARGET_TEMP_FOR_AUTOTUNE 800  // Target setpoint
#define MIN_TEMP_ERROR 1              // Min deviation to count as stable
#define AUTOTUNE_TIMEOUT (unsigned long) 120*SECOND*60   // 2h timeout for tuning
#define PID_N_OSCILLATIONS 10         // Number of oscillations required for tuning


// ===== GRAPHICS ========
#define MIN_TIME_BETWEEN_SCREEN_UPDATES 3000 //  [ms]
#define MAX_FILES 30            // max number of files fetched from the sd card

// ====== FILES =========== 
#define MAX_INSTRUCTIONS_PER_PROGRAM 20 // Maximum number of instructions in a program
#define MAX_FILENAME_LENGTH 50          // max length of a filename
#define MAX_FILES_ON_SCREEN 5           // max number of files displayed on the screen 
#define MAX_INSTR_NAME_LENGHT 30        // max length of an instruction name

// ===== SERIAL =====
// comment out to disable serial communications
#define SERIAL_COMMS


#endif

