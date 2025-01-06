
#ifndef TEEK_DATASTRUCTURES_H
#define TEEK_DATASTRUCTURES_H

#include "TEEK_pins.h"
#include "TEEK_constants.h"
#ifndef ADAFRUIT_MAX31855_H
    // So the IDE doesn't complain
    #include <Adafruit_MAX31855.h>
    #include <SdFat.h>
    #include <EEPROM.h>
    extern char errorStreamChar[ERROR_BUFF_SIZE];
#endif




// ===== Enums ========================================================

enum SystemState {IDLE, BEGIN, EXECUTING, END, DOOR_OPEN, RECOVER, HOLD, ERROR, TUNING, USER_STOP};
enum TemperatureUnit {CELSIUS, FAHRENHEIT, KELVIN};
enum ControlMode {NORMAL, PID_AUTOTUNE};


// ===== Structs ===============================================


//* STRUCT Instruction 
// Container for the informations pertaining an instruction, corresponding to the input file definition
struct Instruction {
    char name[MAX_INSTR_NAME_LENGHT];   // name of the instruction for logging
    unsigned long soakTime = 0;         // [min] defines for how long the target temperature should be maintained
    double target = 0;                  // [C/F/K] target temperature
    double tempVariationRate = 0;       // [C/min] rate at which the temperature should increase/decrease
    bool waitForDoorOpen = false;       // Wait for the door to open before moving to the next instruction
    bool waitForButtonPress = false;    // Wait for the encoder button to be pressed before moving to the next instruction
};

//*STRUCT AutotuneParameters
// preallocates the parameters for the PID autotune process
struct AutotuneParameters {
    double Ku = 0;                // Ultimate gain
    double Tu = 0;                // Oscillation period
    unsigned long startTime = 0;  // Time when tuning started
    unsigned long lastToggleTime = 0; // Last time the heater toggled
    bool heaterState = false;     // Tracks heater on/off state during tuning
    double highTemp = 0;          // Maximum observed temperature
    double lowTemp = 0;           // Minimum observed temperature
    int oscillationCount = 0;     // Number of completed oscillations
};


// ===== CLASSES ========================================================

/**
 * @class TemperatureProbe
 * @brief Manages the temperature readings from a thermocouple sensor.
 * 
 * The TemperatureProbe class interfaces with the Adafruit_MAX31855 sensor to read temperatures
 * and convert them to the desired unit (Celsius, Fahrenheit, Kelvin). It provides methods to set
 * and get the temperature unit, and to read the current temperature.
 * 
 * @private
 * - TemperatureUnit unit: The unit of temperature measurement (Celsius, Fahrenheit, Kelvin).
 * - Adafruit_MAX31855 sensor: The sensor used for temperature readings.
 * 
 * @public
 * - TemperatureProbe(): Default constructor.
 * - TemperatureProbe(int _pin): Constructor with a specified pin.
 * - TemperatureProbe(TemperatureUnit _unit): Constructor with a specified temperature unit.
 * - TemperatureProbe(int _pin, TemperatureUnit _unit): Constructor with a specified pin and temperature unit.
 * - void setUnit(TemperatureUnit unit): Set the temperature unit.
 * - TemperatureUnit Unit(): Get the current temperature unit.
 * - Adafruit_MAX31855 Sensor(): Get the sensor object.
 * - double readTemp(): Read the current temperature and implement security checks.
 */

class TemperatureProbe {
    private: 
        TemperatureUnit unit;
        Adafruit_MAX31855 sensor;
        double toFarhenheit(double celsius){return celsius * 9.0/5.0 + 32;};

    public: 
        // Constructor
        TemperatureProbe();
        TemperatureProbe(TemperatureUnit u);

        // Setters and Getters
        void setUnit(TemperatureUnit unit){unit = unit;};

        TemperatureUnit   Unit()   const {return unit;};
        Adafruit_MAX31855 Sensor() const {return sensor;};
        
        // Methods
        double readTemp();
};

//* CLASS ProgramManager
/**
 * @class ProgramManager
 * @brief Manages the execution and control of a program consisting of multiple instructions.
 * 
 * This class is responsible for handling program information, variables, instruction management, 
 * button actions, execution control, and program loading. It provides methods to get and set 
 * various attributes related to the program and its instructions, as well as methods to manage 
 * the execution flow of the program.
 * 
 * @private
 * - char programName[MAX_FILENAME_LENGTH]: Name of the program.
 * - Instruction instructions[MAX_INSTRUCTIONS_PER_PROGRAM]: Array of instructions in the program.
 * - TemperatureUnit programUnit: Unit of temperature used in the program.
 * - unsigned int numOfInstructions: Number of instructions in the program.
 * - unsigned int instructionIndex: Index of the current instruction.
 * - unsigned long progStartTime: Start time of the program in milliseconds.
 * - unsigned long instrStartTime: Start time of the current instruction in milliseconds.
 * - unsigned long soakTimeStart: Start time of the soak phase in milliseconds.
 * - unsigned long soakTimeEnd: End time of the soak phase in milliseconds.
 * - bool isSoaking: Indicates if the program is in the soaking phase.
 * - bool targetReached: Indicates if the target has been reached in a stable way.
 * - bool isSelected: Indicates if a program has been loaded.
 * - bool buttonPressed: Indicates if the button has been pressed.
 * - void skipLine(File& file): Skips a line in the file.
 * - bool readLine(File& file, char* buffer, size_t bufferSize): Reads a line from the file.
 * - bool parseCSVLine(const char* line, char* name, size_t nameSize, double* target, unsigned long* soakTime, double* rampRate, bool* waitForDoorOpen, bool* waitForButtonPress): Parses a CSV line.
 * - const char* extractField(const char* line, char* buffer, size_t bufferSize): Extracts a field from a CSV line.
 * 
 * @public
 * - ProgramManager(): Constructor.
 * - const char* Name(): Returns the name of the program.
 * - const unsigned int NumOfInstructions(): Returns the number of instructions in the program.
 * - const unsigned int InstructionIndex(): Returns the index of the current instruction.
 * - const unsigned long ProgStartTime(): Returns the start time of the program.
 * - const unsigned long InstrStartTime(): Returns the start time of the current instruction.
 * - const unsigned long SoakTimeStart(): Returns the start time of the soak phase.
 * - const unsigned long SoakTimeEnd(): Returns the end time of the soak phase.
 * - const bool IsSoaking(): Returns true if the program is in the soaking phase.
 * - const bool IsTargetReached(): Returns true if the target has been reached in a stable way.
 * - const Instruction GetInstruction(unsigned int index): Returns the instruction at the specified index.
 * - const Instruction CurrentInstruction(): Returns the current instruction.
 * - const bool IsSelected() const: Returns true if a program has been selected and loaded.
 * - void setName(const char* name): Sets the name of the program.
 * - void setNumOfInstructions(unsigned int num): Sets the number of instructions in the program.
 * - void setInstructionIndex(unsigned int index): Sets the index of the current instruction.
 * - void setInstrStartTime(unsigned long time): Sets the start time of the current instruction.
 * - void setSoakTimeStart(unsigned long time): Sets the start time of the soak phase.
 * - void setIsSoaking(bool soaking): Sets the soaking state.
 * - void setTargetReached(bool stable): Sets the target reached state.
 * - void setProgStartTime(unsigned long time): Sets the start time of the program.
 * - bool addInstruction(Instruction instr): Adds an instruction to the program.
 * - bool addInstruction(char* name, unsigned long soakTime, double target, double tempVariationRate, bool waitForDoorOpen, bool waitForButtonPress): Adds an instruction to the program with specified parameters.
 * - bool nextInstruction(): Moves to the next instruction.
 * - bool removeInstruction(unsigned int index): Removes an instruction from the program.
 * - bool isInstructionDone(): Checks if the current instruction has been completed.
 * - bool hasButtonBeenPressed(): Returns true if the button has been pressed.
 * - void resetButtonPressed(): Resets the button pressed state.
 * - void ConfirmButtonPressed(): Confirms that the button has been pressed.
 * - unsigned long elapsedTime(): Returns the elapsed time since the program started.
 * - unsigned long remainingSoakTime(): Returns the remaining soak time.
 * - void rampCompleted(): Marks the ramp as completed.
 * - void startSoakTimer(): Starts the soak timer.
 * - void resetCurrentInstruction(): Resets the current instruction.
 * - void resetCurrentInstruction(unsigned long time): Resets the current instruction from a given time.
 * - void resetProgStartTime(): Resets the program start time.
 * - void clearProgram(): Clears the program fields.
 * - bool loadProgram(File& file): Loads a program from a file.
 */
class ProgramManager {
    private: 
        // == 1. Program Information ===================================================================
        char            programName[MAX_FILENAME_LENGTH];
        Instruction     instructions[MAX_INSTRUCTIONS_PER_PROGRAM];
        TemperatureUnit programUnit = CELSIUS;

        // == 2. Program Variables =====================================================================
        unsigned int    numOfInstructions   = 0;
        unsigned int    instructionIndex    = 0;
        unsigned long   progStartTime   = 0;    // [ms]
        unsigned long   instrStartTime  = 0;    // [ms]
        
        // == 3. Instruction Variables ================================================================
        unsigned long   soakTimeStart   = 0;    // [ms]
        unsigned long   soakTimeEnd     = 0;    // [ms]
        bool isSoaking      = false;            // True if the program is in the soaking phase
        bool targetReached  = false;           // True if the target has been reached in a stable way
        bool isSelected     = false;           // True if a program has been loaded

        // == 4. Button State ==========================================================================
        bool buttonPressed  = false;           // True if the button has been pressed

        // == 5. CSV Parsing ===========================================================================
        void skipLine(File& file);
        bool readLine(File& file, char* buffer, size_t bufferSize);
        bool parseCSVLine(const char* line, char* name, size_t nameSize, double* target, unsigned long* soakTime, double* rampRate, bool* waitForDoorOpen, bool* waitForButtonPress);
        const char* extractField(const char* line, char* buffer, size_t bufferSize);

    public:
        // == 6. Constructor ===========================================================================
        ProgramManager();

        // == 7. Getters ===============================================================================
        const char* Name() const { return programName; }
        unsigned int NumOfInstructions() const { return numOfInstructions; }
        unsigned int InstructionIndex() const { return instructionIndex; }
        unsigned long ProgStartTime() const { return progStartTime; }
        unsigned long InstrStartTime() const { return instrStartTime; }
        unsigned long SoakTimeStart() const { return soakTimeStart; }
        unsigned long SoakTimeEnd() const { return soakTimeEnd; }
        bool IsSoaking() const { return isSoaking; }
        bool IsTargetReached() const { return targetReached; }
        Instruction GetInstruction(unsigned int index);
        Instruction CurrentInstruction();
        bool IsSelected() const { return isSelected; } // True if a program has been selected and loaded

        // == 8. Setters ===============================================================================
        void setName(const char* name) {
            strncpy(programName, name, MAX_FILENAME_LENGTH - 1); 
            programName[MAX_FILENAME_LENGTH - 1] = '\0';
        }
        void setNumOfInstructions(unsigned int num) { numOfInstructions = num; }
        void setInstructionIndex(unsigned int index) { instructionIndex = index; }
        void setInstrStartTime(unsigned long time) { instrStartTime = time; }
        void setSoakTimeStart(unsigned long time) { soakTimeStart = time; }
        void setIsSoaking(bool soaking) { isSoaking = soaking; }
        void setTargetReached(bool stable) { targetReached = stable; }
        void setProgStartTime(unsigned long time) { progStartTime = time; }

        // == 9. Instruction Management ================================================================
        bool addInstruction(Instruction instr);
        bool addInstruction(char* name, unsigned long soakTime, double target, double tempVariationRate, bool waitForDoorOpen, bool waitForButtonPress);
        bool nextInstruction(); // Move to the next instruction
        bool removeInstruction(unsigned int index); // Remove an instruction from the program (unused)
        bool isInstructionDone(); // Check if the current instruction has been completed

        // == 10. Button Actions =======================================================================
        bool hasButtonBeenPressed() { return buttonPressed; }
        void resetButtonPressed() { buttonPressed = false; }
        void ConfirmButtonPressed() { buttonPressed = true; }

        // == 11. Execution Control =====================================================================
        unsigned long elapsedTime() { return millis() - progStartTime; }
        unsigned long remainingSoakTime() { return soakTimeStart + CurrentInstruction().soakTime - millis(); }
        void rampCompleted() { instructions[instructionIndex].tempVariationRate = 0; } // if the ramp has been completed, erase the ramp rate
        
        void startSoakTimer();              // Start the soak timer
        void resetCurrentInstruction();     // Reset the current instruction (unused)
        void resetCurrentInstruction(unsigned long time);   // Reset the current instruction, from a given time (unused)
        void resetProgStartTime() { progStartTime = millis(); } // Reset the program start time (unused)
        void clearProgram();    // Clear the program fields

        // == 12. Program Loading ======================================================================
        bool loadProgram(File& file); // Defined in the TEEKeeper.cpp file!
};

// TODO: all the functions marked by the "unused" comment are currently not used in the program, and can be removed if necessary.
// TODO: These are utility functions that were written for future use, but are not currently implemented in the program.
// TODO: i.e. in the HOLD state management definition, or to allow the user to create or modify a program file from the device


// ------------------------------------------------


/**
 * @class CoreSystem
 * @brief Manages the thermal control of the system using a PID controller.
 * 
 * The CoreSystem class is optimized for lightweight, fast, and basic control, ensuring time precision without relying on hardware timers.
 * It provides features such as heater control, stability monitoring, and optional logging for diagnostics.
 * 
 * @private
 * - TemperatureProbe probe: The temperature probe used for reading temperatures.
 * - SystemState status: The current state of the system.
 * - TemperatureUnit unit: The unit of temperature measurement (Celsius, Fahrenheit, Kelvin).
 * - ControlMode mode: The control mode (NORMAL, PID_AUTOTUNE).
 * - double targetTemperature: The target temperature to be achieved.
 * - double currentTemperature: The current temperature reading.
 * - unsigned long lastTempReading: The timestamp of the last temperature reading.
 * - double kp, ki, kd: PID controller parameters.
 * - double dutyCycle: The duty cycle for PWM control.
 * - unsigned long PWMPeriod: The period of the PWM cycle.
 * - double last_error: The last error value for PID calculation.
 * - double integral: The integral term for PID calculation.
 * - unsigned long nextPWMCycle: The timestamp for the next PWM cycle.
 * - unsigned long dutyEnd: The timestamp when the current duty cycle ends.
 * - bool allowFiringHeater: Security flag to allow or deny heater operation.
 * - bool fireHeater: Flag to indicate if the heater is currently firing.
 * - short int stabilityCounter: Counter for temperature stability checks.
 * - bool isStable: Flag to indicate if the temperature is stable.
 * - bool keepLog: Flag to indicate if logging is enabled.
 * - unsigned long lastDoorOpenTime: The timestamp of the last door opening event.
 * - bool isTuning: Flag to indicate if the system is in PID tuning mode.
 * 
 * @public
 * - CoreSystem(TemperatureProbe &probe): Constructor with temperature probe.
 * - CoreSystem(TemperatureProbe &probe, double kp, double ki, double kd): Constructor with temperature probe and PID parameters.
 * - void setTarget(double target, bool newInstruction = false): Set the target temperature.
 * - void setUnit(TemperatureUnit _unit): Set the temperature unit.
 * - void updatePID(double _kp, double _ki, double _kd): Update the PID parameters.
 * - void setKeepLog(bool log): Enable or disable logging.
 * - void setCurrentTemperature(double temp): Set the current temperature.
 * - SystemState const Status(): Get the current system status.
 * - TemperatureUnit const Unit(): Get the current temperature unit.
 * - ControlMode getControlMode(): Get the current control mode.
 * - double TargetTemperature(): Get the target temperature.
 * - double CurrentTemperature(): Get the current temperature.
 * - bool KeepLog(): Check if logging is enabled.
 * - double getKp(): Get the Kp parameter of the PID controller.
 * - double getKi(): Get the Ki parameter of the PID controller.
 * - double getKd(): Get the Kd parameter of the PID controller.
 * - char* getTextUnit(): Get the current temperature unit in text form.
 * - bool isFiringAllowed(): Check if the heater is allowed to turn on.
 * - bool isFiring(): Check if the heater is firing.
 * - bool IsStable(): Check if the temperature is stable.
 * - bool IsDoorOpen(): Check if the door is open.
 * - bool IsTuning(): Check if the system is in PID tuning mode.
 * - bool IsOn(): Check if the heater is on.
 * - unsigned long lastDoorOpening(): Get the timestamp of the last door opening event.
 * - void allowFiring(): Allow the heater to turn on.
 * - void denyFiring(): Deny the heater to turn on.
 * - void startFiring(): Start the heater.
 * - void startFiring(double target): Start the heater with a target temperature.
 * - void stopFiring(): Stop the heater.
 * - void ReadTemperature(): Read the current temperature from the probe.
 * - void recordDoorOpening(): Record the timestamp of the last door opening event.
 * - void updateStatus(SystemState newStatus): Update the system status.
 * - void Clear(): Reset the core system.
 * - void update(ProgramManager& __prog): Manage the PWM cycle (defined in TEEKeeper.cpp).
 * - void PIDAutotune(): Start the PID autotune process.
 */
class CoreSystem {
    private:
        // == 1. Temperature and Control State =======================================================
        TemperatureProbe probe;
        SystemState status = IDLE;
        TemperatureUnit unit = CELSIUS;
        ControlMode mode = NORMAL;

        // == 2. Temperature Control Variables =======================================================
        double targetTemperature = 0;
        double currentTemperature = 0;
        unsigned long lastTempReading = 0;

        // == 3. PWM PID Variables ===================================================================
        double kp;
        double ki;
        double kd;
        double dutyCycle = 0;
        unsigned long PWMPeriod = CYCLE_TIME;

        // == 4. PID Parameters ======================================================================
        double last_error = 0;
        double integral = 0;
        unsigned long nextPWMCycle = 0;
        unsigned long dutyEnd = 0;

        // == 5. Heater Control and Security =========================================================
        bool allowFiringHeater = false; // Security flag, overrides program execution
        bool fireHeater = false;        // If true, the PWM control is active

        // == 6. Temperature Stability ===============================================================
        short int stabilityCounter;
        bool isStable;

        // == 7. Logging Data ========================================================================
        bool keepLog;

        // == 8. Time Variables ======================================================================
        unsigned long lastDoorOpenTime = 0;

        // == 9. Autotune Variables ==================================================================
        bool isTuning = false;

        // == 10. Private Methods ====================================================================
        double PID(const double error);

    public:
        // == 1. Constructors ========================================================================
        CoreSystem(TemperatureProbe &probe);
        CoreSystem(TemperatureProbe &probe, double kp, double ki, double kd);

        // == 2. Setters =============================================================================
        void setTarget(double target, bool newInstruction = false);
        void setUnit(TemperatureUnit _unit) { unit = _unit; }
        void updatePID(double _kp, double _ki, double _kd) { kp = _kp; ki = _ki; kd = _kd; }
        void setKeepLog(bool log) { keepLog = log; }
        void setCurrentTemperature(double temp) { currentTemperature = temp; }

        // == 3. Getters =============================================================================
        SystemState     Status() const { return status; }   // Get the current system status
        TemperatureUnit Unit() const { return unit; }   // Get the current temperature unit
        ControlMode     getControlMode() const { return mode; }   // Get the current control mode

        double TargetTemperature() const { return targetTemperature; }
        double CurrentTemperature() const { return currentTemperature; }

        double getKp() const { return kp; }
        double getKi() const { return ki; }
        double getKd() const { return kd; }

        char* getTextUnit() const; // Get the current temperature unit in text form

        bool KeepLog() const { return keepLog; }        // If true, the system will keep an execution log
        bool isFiringAllowed() const { return allowFiringHeater; } // Check if the heater is allowed to turn on
        bool isFiring() const { return fireHeater; }    // Check if the heater is firing
        bool IsStable() const { return isStable; }      // Check if the temperature is stable
        bool IsDoorOpen() const { return digitalRead(PIN_DOOR_INTERRUPT); } // Check if the door is open
        bool IsTuning() const { return isTuning; }      // Check if the system is in PID tuning mode
        bool IsOn() const { return allowFiringHeater && fireHeater; } // Check if the heater is on

        unsigned long lastDoorOpening() const { return lastDoorOpenTime; }

        // == 4. Heater Control Methods ==============================================================
        void allowFiring();                                             // Allow the heater to turn on
        void denyFiring() { allowFiringHeater = false; }                // Deny the heater to turn on
        void startFiring() { fireHeater = true; }                       // Start the heater
        void startFiring(double target) { targetTemperature = target; fireHeater = true; }
        void stopFiring() { fireHeater = false; }                       // Stop the heater

        // == 5. Temperature Reading and Stability ===================================================
        void ReadTemperature() { currentTemperature = probe.readTemp(); } // Read the temperature
        void recordDoorOpening() { lastDoorOpenTime = millis(); }         // Record the last door opening time

        // == 6. System State Management =============================================================
        void updateStatus(SystemState newStatus) { status = newStatus; } // Update the system status
        void Clear();                                                   // Reset the core system

        // == 7. PID Control =========================================================================
        void update(ProgramManager& __prog);  // Manage the PWM cycle (defined in TEEKeeper.cpp)
        void PIDAutotune();                  // Start the PID autotune process
};



// ------------------------------------------------


// ------------------------------------------------


#endif