
#include "TEEKeeper.h"
#include <TimerOne.h>

#ifndef EEPROM_H
#include <EEPROM.h>
#endif 
/*
    The functions described in this file are divided in the following sections:
    0. Setup functions:             wrapper for the main setup function
    1. Core System functions:       heater control, PID autotuning and system state management
    2. Program execution functions: timers and handlers for program instructions
    3. File management functions:   loading programs from files and logging process information
    4. Interrupt functions:         door safety trigger and encoder timer
*/

// == Global variables
File *__file = nullptr; // Log file pointer

//* 0. Setup functions =====================================================================
bool TEEK_Setup(){

    __screen.begin();           // Initialize the TFT screen
    __screen.setRotation(1);    // Set the screen orientation

    // Initialize encod45rer
    Timer1.initialize(1000);                // Timer for smooth encoder input
    Timer1.attachInterrupt(timerIsr);       // Attach the encoder service instruction
    __encoder.setAccelerationEnabled(true); // Enable acceleration for the encoder

    // Initialize the temperature probe
    if(!__probe.Sensor().begin()){
        sprintf(errorStreamChar, "Could not initialize the temperature probe.\n");
        return false;
    }

    // Define the remaining pins
    pinMode(PIN_SD_CS, OUTPUT);     // SD PIN - Chip Select
    pinMode(PIN_HEATER, OUTPUT);    // Heater - ON/OFF
    digitalWrite(PIN_HEATER, LOW);  // normally off

    pinMode(PIN_DOOR_INTERRUPT, INPUT);  // Door interrupt pin
    attachInterrupt(digitalPinToInterrupt(PIN_DOOR_INTERRUPT), doorInterrupt, CHANGE);
    
    // == 2. Load previous configurations from the EEPROM, if there are any
    double kp = EEPROM.get(sizeof(double) * EEPROM_ADDR_KP, kp);
    if (kp != 0) {
        // Load PID values from EEPROM
        double ki = EEPROM.get(sizeof(double) * EEPROM_ADDR_KI, ki);
        double kd = EEPROM.get(sizeof(double) * EEPROM_ADDR_KD, kd);

        // Update the CORE with the loaded PID values
        __core = CoreSystem(__probe, kp, ki, kd);
    }  
    else {
        // Initialize the core system with the default values
        __core = CoreSystem(__probe);
    }

    return true;
};

// --------------------------------------------------------------------------------------------


//* 1. Core System functions =====================================================================

/**
 * @brief Updates the core system based on the current mode and temperature control logic.
 * 
 * This function handles the temperature control loop and PID autotuning for the core system.
 * It manages the heater firing, temperature readings, and stability checks.
 * 
 * @param __prog Reference to the ProgramManager object.
 * 
 * @details
 * - If the system is not allowed to fire the heater, the function returns immediately.
 * - In NORMAL mode, it performs the following steps:
 *   - Checks if the duty cycle has ended and turns off the heater if necessary.
 *   - Starts the next PWM cycle, reads the temperature, and calculates the error.
 *   - Updates the duty cycle using the PID controller and manages the heater state.
 *   - Checks for stability and updates the log if logging is enabled.
 * - In PID_AUTOTUNE mode, it performs the following steps:
 *   - Initializes autotune parameters if not already tuning.
 *   - Monitors the temperature and toggles the heater state based on the target temperature.
 *   - Records the maximum and minimum temperatures and calculates the oscillation period.
 *   - Updates the ultimate gain and finalizes tuning if sufficient oscillations are achieved.
 *   - Saves the PID parameters to EEPROM and returns to NORMAL mode.
 *   - Checks for timeout and updates the status to ERROR if autotuning times out.
 * 
 * @note This function assumes the presence of external variables and functions such as
 *       millis(), digitalWrite(), probe.readTemp(), denyFiring(), updateStatus(), updateLog(),
 *       PID(), EEPROM.put(), and constants like PIN_HEATER, PWMPeriod, MAX_TEMP_ERROR,
 *       MIN_STABLE_CYCLES, TARGET_TEMP_FOR_AUTOTUNE, MIN_N_OSCILLATIONS, AUTOTUNE_TIMEOUT,
 *       EEPROM_ADDR_KP, EEPROM_ADDR_KI, EEPROM_ADDR_KD.
 */
void CoreSystem::update(ProgramManager& __prog) {

    // Poll the temperature probe at a fixed interval
    if(millis() - lastTempReading> POLL_PROBE_INTERVAL){
        ReadTemperature();
    }

    //! the polling does not interfere with the temperature control loop
    // The temperature control polls the temperature probe indipendently,
    // following the PWM cycle time

    // TEMPERATURE CONTROL LOOP
    //if the system is not allowed to fire the heater, then we should not do anything
    // apart from monitoring the temperature
    if(allowFiringHeater == false || fireHeater == false){
        return;
    }

    // Manage the type of control
    if(mode == NORMAL){             //* ===========  NORMAL CONTROL ==============
        if(fireHeater == true){
            unsigned long time = millis();

            // if duty cycle has ended
            if(millis() > dutyEnd){
                // turn off the heater
                digitalWrite(PIN_HEATER, LOW);
            }

            // if the time has come to start the next cycle
            if(millis()>nextPWMCycle){
                // read the temperature
                ReadTemperature();

                // Compute the PID values
                double error = targetTemperature - currentTemperature;
                dutyCycle = PID(error);

                // calculate the end of the duty cycle
                dutyEnd = time + (dutyCycle * PWMPeriod / 100);

                // calculate the start of the next cycle
                nextPWMCycle = time + PWMPeriod;

                // turn on the heater
                digitalWrite(PIN_HEATER, HIGH);

                last_error = error;

                // check on stability   
                if(abs(error) < MAX_TEMP_ERROR && isStable == false){
                    stabilityCounter++;
                    if(stabilityCounter == MIN_STABLE_CYCLES){
                        isStable = true;
                    }
                }
                else{
                    stabilityCounter = 0;
                }

                // update log
                if(keepLog && __prog.IsSelected())
                    updateLog(*__file, (char*)__prog.CurrentInstruction().name, time, //...
                                currentTemperature, targetTemperature, dutyCycle);
            }
        }
    }
    //* ======================================== END of NORMAL CONTROL ========================================

    else if (mode == PID_AUTOTUNE) {   //* ===========  PID AUTOTUNE ==============
    
    
        extern AutotuneParameters __autPar;
        if (status != TUNING) {
            //* Start the autotune process
            
            status = TUNING;    // Update status
            isTuning = true;    // Start tuning mode

            // Initialize autotune parameters
            __autPar = AutotuneParameters(); // Reset struct to defaults
            __autPar.startTime      = millis(); // Timestamp
            __autPar.lastToggleTime = millis(); // Timestamp

            __autPar.heaterState = true; // Start with heater on
            digitalWrite(PIN_HEATER, HIGH);
        } else {
            unsigned long currentTime = millis();
            ReadTemperature();

            // Monitor temperature and toggle heater
            if (__autPar.heaterState && currentTemperature >= TARGET_TEMP_FOR_AUTOTUNE) {
                // Heater off
                __autPar.heaterState = false;
                digitalWrite(PIN_HEATER, LOW);
                __autPar.lastToggleTime = currentTime;

                // Record max temperature
                if (currentTemperature > __autPar.highTemp) {
                    __autPar.highTemp = currentTemperature;
                }
            } else if (!__autPar.heaterState && currentTemperature <= (TARGET_TEMP_FOR_AUTOTUNE - 0.5)) {
                // Heater on
                __autPar.heaterState = true;
                digitalWrite(PIN_HEATER, HIGH);
                __autPar.lastToggleTime = currentTime;

                // Record min temperature
                if (currentTemperature < __autPar.lowTemp) {
                    __autPar.lowTemp = currentTemperature;
                }

                // Calculate oscillation period
                double period = (currentTime - __autPar.lastToggleTime) * 2; // Full cycle
                if (__autPar.oscillationCount == 0) {
                    __autPar.Tu = period;
                } else {
                    __autPar.Tu = (__autPar.Tu * __autPar.oscillationCount + period) / 
                                        (__autPar.oscillationCount + 1); // Moving average
                }

                // Update oscillation count
                __autPar.oscillationCount++;

                // Update ultimate gain
                __autPar.Ku = (4.0 * PWMPeriod) / ((__autPar.highTemp - __autPar.lowTemp) * 0.5);

                // Finalize tuning if sufficient oscillations achieved
                if (__autPar.oscillationCount >= PID_N_OSCILLATIONS) {
                    kp = 0.6 * __autPar.Ku;
                    ki = (1.2 * __autPar.Ku) / __autPar.Tu;
                    kd = (3.0 * __autPar.Ku * __autPar.Tu) / 40.0;

                    // save the EEPROM values
                    EEPROM.put(EEPROM_ADDR_KP, kp);
                    EEPROM.put(EEPROM_ADDR_KI, ki);
                    EEPROM.put(EEPROM_ADDR_KD, kd);

                    // if there is an SD card, create  a new file and save the autotune parameters
                    if(__core.KeepLog()) {
                        File file = __sd.open("autotune.txt", FILE_WRITE);
                        if (file) {
                            file.print("Autotune parameters\n");
                            file.print("Kp: ");     file.println(kp);
                            file.print("Ki: ");     file.println(ki);
                            file.print("Kd: ");     file.println(kd);
                            file.print("Ku: ");     file.println(__autPar.Ku);
                            file.print("Tu: ");     file.println(__autPar.Tu);
                            file.close();
                        }
                    }

                    mode = NORMAL;    // Return to normal operation
                    isTuning = false; // End tuning mode
                    return;
                }
            }

            //* Safety Feature: max autotune runtime is limited to 2 hours
            if ((currentTime - __autPar.startTime) > AUTOTUNE_TIMEOUT) {
                denyFiring();
                updateStatus(ERROR); // Error status in case of timeout
                return;
            }
        }
    }
    //* ======================================== END of AUTOTUNE ========================================
};

// --------------------------------------------------------------------------------------------

/**
 * @brief Manages the state of the core system based on its current status.
 * 
 * This function handles the transitions and actions for various states of the core system.
 * 
 * @param sys Reference to the CoreSystem object.
 * @param prog Reference to the ProgramManager object.
 * @param sens Reference to the TemperatureProbe object.
 * @return true if the system state was managed successfully.
 * @return false if an error occurred and the system needs user intervention.
 * 
 * States:
 * - IDLE: No program is running, waiting for program selection.
 * - BEGIN: A program has been selected, initialize and start execution.
 * - EXECUTING: Manage the execution of the current program.
 * - END: Program has finished, close the log file and go to IDLE.
 * - DOOR_OPEN: The door is open, manage security features.
 * - RECOVER: Recover the state of the system after the door has been closed.
 * - HOLD: Hold the current temperature for a known or unknown period.
 * - ERROR: Manage errors and stop the system.
 */
bool manageSystemState(CoreSystem& sys, ProgramManager& prog, TemperatureProbe& sens) {
    
  // check for the status of the system
  switch(sys.Status()){

    // IDLE: no program is running, waiting for program selection
    case IDLE:    

        // if a program is selected, go to BEGIN
        if(prog.IsSelected()){
            sys.updateStatus(BEGIN);
        }

        // otherwise do nothing
            
        break;

    // BEGIN: program has been selected, initialize and start the execution
    case BEGIN:                     
        // load the first instruction from the file 
        sys.setTarget(prog.CurrentInstruction().target, true);

        // create & initialize log file
        prog.setProgStartTime(millis());

        // begin execution
        sys.updateStatus(EXECUTING); // update program status

        // Security check
        sys.allowFiring();           // enable heating 

        if(sys.KeepLog()) {          // if logging is enabled
            __file = createLog();    // create the log file
            beginLog(*__file, prog); // write the header
        }

        break;

    // EXECUTING: manage the execution of a program
    case EXECUTING:  
        return programExecution(sys, prog, sens);
        break;

    // END: program has finished, close the log file and go to IDLE
    case END:     // write the end of the log file and close it
        if(sys.KeepLog()) closeLog(*__file, prog);
        
        // free the memory
        prog.clearProgram();

        // go to IDLE
        digitalWrite(PIN_HEATER, LOW); // turn off the heater
        sys.Clear();                   // reset the core system
        delay(1000);                   // wait for the system to stabilize
        break;

    // DOOR OPEN: if the door is open, manage the security features
    case DOOR_OPEN:
        // if the door has just been opened, timestamp the event
        if (sys.lastDoorOpening() == 0){
            sys.recordDoorOpening(); // timestamp the door opening

            // report on the log file
            if(sys.KeepLog()) {
                updateLog(*__file, (char *)"EVENT: Door opened", prog.elapsedTime());
            }
        }
        else {
            // if the door has been open for more than 5 minutes, go to ERROR
            if(millis() - sys.lastDoorOpening() > (unsigned)MAX_TIME_DOOR_OPEN){
                sprintf(errorStreamChar, "Door has been oper over the time security limit. Shutting off...");  
                
                // Update the log file
                if(sys.KeepLog()) {
                    updateLog(*__file, errorStreamChar, prog.elapsedTime());
                    closeLog(*__file, prog);
                    __file = nullptr;
                }
                
                // move to error
                sys.updateStatus(ERROR);
                
            }
        }
        break;

    // RECOVER: recover the state of the system after the door has been closed
    case RECOVER: // report that the door has been closed on the log file
        if(sys.KeepLog()) {
            updateLog(*__file, (char *) "EVENT: Door closed", prog.elapsedTime());
        };

        // TODO: implement the recovery process
        // // wait for the system to stabilize
        //if(sys.IsStable()){
        //    sys.updateStatus(EXECUTING);
        //}
        sys.updateStatus(EXECUTING);

    // HOLD: hold the current temperature for a known or unknown period of time
    case HOLD:    // TODO: implement the HOLD state 
        break;

    // USER_STOP: the user has stopped the program for whatever reason
    case USER_STOP:
        if (sys.KeepLog()) {
            updateLog(*__file, (char *)"EVENT: User stopped the program", prog.elapsedTime());
        }
        sys.updateStatus(END);
        break;
    
    // ERROR: manage errors and stop the system
    case ERROR:   // manage errors
        digitalWrite(PIN_HEATER, LOW); // turn off the heater
        // write errorstream on log file
        if(sys.KeepLog()) {
            updateLog(*__file, errorStreamChar, prog.elapsedTime());    // write the error message
            closeLog(*__file, prog);    // close the log file
            __file = nullptr;        // free the memory
        }

        // TODO: implement data dump, eventually?

        // free the memory
        prog.clearProgram();
        sprintf(errorStreamChar, " "); // clear the error stream

        return false;
        break;
    
    // DEFAULT: do nothing
    default:  
        break;
  }
  return true;
};


// --------------------------------------------------------------------------------------------


//* 2. Program execution functions =====================================================================

/**
 * @brief Executes the current program instruction and manages the transition to the next one.
 * 
 * This function handles the execution of the current instruction in the program. It checks if the 
 * current instruction is completed and moves to the next one if necessary. It also manages special 
 * conditions such as waiting for a button press or door opening before proceeding. Additionally, 
 * it handles temperature ramping and soaking processes.
 * 
 * @param sys Reference to the CoreSystem object, which manages the core functionalities.
 * @param prog Reference to the ProgramManager object, which manages the sequence of instructions.
 * @param sens Reference to the TemperatureProbe object, which provides temperature readings.
 * 
 * @return true if the function executed successfully, false if it is waiting for a condition to be met.
 */
bool programExecution(CoreSystem& sys, ProgramManager& prog, TemperatureProbe& sens) {
    
    // if the instruction is done, move to the next one
    if(prog.isInstructionDone()){
        // eventually, for the door opening too
        if(prog.CurrentInstruction().waitForButtonPress){
  
            // update the message buffer
            if(messageStream[0] == '\0') sprintf(messageStream, "Press the button to continue.");
            
            // wait for the button press
            // the button press will be managed by the GUI            
            if(prog.hasButtonBeenPressed()){
                prog.resetButtonPressed();  // reset the flag
                return true;
            }

            return false;
        }
        else if (prog.CurrentInstruction().waitForDoorOpen){
            doorInterrupt(); // poll the door status
            // TODO maybe this needs a better implementation?
        }
        else {
            // move to the next instruction
            if(prog.nextInstruction()){
                sys.setTarget(prog.CurrentInstruction().target, true);
                return true;
            }   
            else{
                // if there are no more instructions, go to END
                sys.updateStatus(END);
                return true;
            }
        }   
    }
    else {
        
        // if the instruction has a ramp coefficient, compute the ramp target
        if(prog.CurrentInstruction().tempVariationRate != 0){

            // if we are far from the target temperature, compute the ramp
            if(abs(sys.CurrentTemperature() - prog.CurrentInstruction().target) > 2*MAX_TEMP_ERROR){
            
            double newtarget = sys.CurrentTemperature() + prog.CurrentInstruction().tempVariationRate * (millis() - prog.InstrStartTime())/60000;
            sys.setTarget(newtarget, true);
            }
            else {
                // if we are near the target temperature, we can stop ramping
                prog.rampCompleted();   
            }
        }

        // if the temperature has not been reached yet
        if (!prog.IsTargetReached()){

            // if the temperature is stable, start the soak timer
            if(sys.IsStable() && !prog.IsSoaking()){
                prog.startSoakTimer();
                return true;
            }

            // otherwise do nothing
        }
    }
    return true;
};


// --------------------------------------------------------------------------------------------

//* 3. File management functions =====================================================================

/**
 * @brief File management functions for the core system.
 * 
 * This section includes functions to handle file operations such as loading programs from CSV files,
 * creating and managing log files, and updating logs with process information or error messages.
 * 
 * Functions:
 * - createLog:     Creates a new log file. If the file already exists, creates a new one with an incremented index.
 * - beginLog:      Prints the header of the log file.
 * - updateLog:     Updates the log file with process information or error messages.
 * - closeLog:      Prints the end of the log file and frees the memory.
 */

// --------------------------------------------------------------------------------------------

// Create log
/**
 * @brief Creates a new log file in the "logs" folder. If a file already exists with the name "log_x.csv",
 *        the function increments the index (x) until it finds an available file name.
 * 
 * @return A pointer to the created File object. If the SD card is not present or a file cannot be created, returns NULL.
 */
File* createLog() {
    static File logFile; // Use static to keep memory usage under control

    // Check if the SD card is available
    if (!__sd.begin(PIN_SD_CS, SPI_HALF_SPEED)) {
        drawSoftError(__screen, (char*)"SD card not found."); // Inform user about the error
        delay(2000); 
        __core.setKeepLog(false);                             // Disable logging
        extern ScreenManager __GUI;
        __GUI.renderCurrent(__screen); // Refresh screen
        return NULL; // Return NULL if SD card initialization fails
    }

    // Navigate to the "logs" folder or create it if it doesn't exist
    if (!__sd.chdir("/logs")) { 
        __sd.mkdir("/logs"); // Create the "logs" folder if it doesn't exist
        if(!__sd.chdir("/logs")){ // Navigate to the folder
            // if the navigation fails, turn off logging 
            drawSoftError(__screen, (char*)"Failed to create log folder.");
            delay(2000);
            __core.setKeepLog(false);
            return NULL;
        };
    }

    // Generate a unique log file name in the format "log_x.csv"
    char logName[16]; // Enough space for "log_65535.csv\0"
    uint16_t logIndex = 0;
    do {
        snprintf(logName, sizeof(logName), "log_%d.csv", logIndex++); // Create file name
    } while (__sd.exists(logName) && logIndex < 65535); // Ensure the file does not already exist, limit index to 65535

    if (logIndex >= 65535) { 
        // overwrite the last log
        logIndex = 0;
        snprintf(logName, sizeof(logName), "log_%d.csv", logIndex);
    }

    // Create the log file in write mode
    logFile = __sd.open(logName, FILE_WRITE); // Open the file for writing
    if (!logFile) { // Check if file creation was successful
        drawSoftError(__screen, (char*)"Failed to create log file.");
        delay(2000);
        __core.setKeepLog(false); // disable logging
        return NULL;
    }

    // Return a pointer to the created file
    return &logFile;
}


// --------------------------------------------------------------------------------------------

// Print the header of the log file
bool beginLog(File &log, ProgramManager& __prog) {    
    if(!log) {
        sprintf(errorStreamChar, "ERROR: log file not found.\n");
        __core.setKeepLog(false);
        return false;
    }
    // write the program name
    log.print("Program: ");     log.println(__prog.Name());

    // write the header
    log.println("Time,Name,Temperature,Target,DutyCycle");
    log.println("[ms],[],[C],[C],[%]");

    return true;
};

// --------------------------------------------------------------------------------------------

// Update the log file with process info
// Update log with process data (name, time, temperature, target, duty cycle)
bool updateLog(File &log, const char* name, unsigned long time, double temp, double target, double duty) {
    // Check if the file is valid and open


    if (!log.isOpen()) {
        sprintf(errorStreamChar, "Log file not found or not open.\n");
        __core.setKeepLog(false);
        return false;
    }

    // Format the timestamp
    char buff[9];
    timeStampConverter(time, buff, 3); // Converts the time to a formatted string

    // Write the log entry as a CSV line
    log.print(buff);   // Timestamp
    log.print(",");    // Field delimiter
    log.print(name);   // Current instruction name
    log.print(",");    
    log.print(temp, 2); // Temperature with 2 decimal places
    log.print(",");    
    log.print(target,0); // Target temperature with no decimal places
    log.print(",");    
    log.print(duty, 2); // Duty cycle with 2 decimal places
    log.println();     // End the line

    // Ensure the data is written to the SD card
    if (!log.sync()) {
        sprintf(errorStreamChar, "Failed to sync log data.\n");
        return false;
    }

    return true;
}


// --------------------------------------------------------------------------------------------

// Update log with a message (i.e. error message)
bool updateLog(File& log, const char* message, unsigned long time) {
    // Check if the file is open
    if (!log || !log.isOpen()) {
        sprintf(errorStreamChar, "Log file not found or not open.\n");
        __core.setKeepLog(false);
        return false;
    }

    // Format the timestamp
    char buff[9];
    timeStampConverter(time, buff, 3); // Converts the time to a formatted string

    // Write the log entry
    log.print(buff);  // Write the timestamp
    log.print(",");   // Separate fields with a comma
    log.println(message); // Write the message

    // Ensure the data is written to the SD card
    if (!log.sync()) {
        sprintf(errorStreamChar, "Failed to sync log data.\n");
        return false;
    }

    return true;
}
// --------------------------------------------------------------------------------------------

// Print the end of the log file and free the memory
bool closeLog(File &log, ProgramManager& __prog) {
    if(!log) {
        sprintf(errorStreamChar, "log file not found.\n");
        return false;
    }

    // write the end of the log file
    log.println("End of the program.");
    char buff[9];
    timeStampConverter(__prog.elapsedTime(), buff, 3);
    log.print("Elapsed time: "); log.println(buff);
    log.println("Have a nice day! :)");

    log.close(); // close the file
    __file = nullptr; // free the memory
    return true;
};

//* ========================================= Auxiliary functions ==============================

// Converts a time in millis to the HH:MM:SS format
void timeStampConverter(unsigned long millisTime, char* buff, int terms) {
    unsigned seconds = millisTime / 1000;
    unsigned minutes = seconds / 60;
    unsigned hours = minutes / 60;

    seconds %= 60;  // Get remaining seconds
    minutes %= 60;  // Get remaining minutes
    hours %= 24;    // Optional: If you want hours to wrap around after 24 hours

    buff[0] = hours / 10 + '0';
    buff[1] = hours % 10 + '0';
    if (terms == 1) {
        buff[2] = '\0'; // Null terminator
        return;
    }
    buff[2] = ':';
    buff[3] = minutes / 10 + '0';
    buff[4] = minutes % 10 + '0';
    buff[5] = ':';
    if (terms == 2) {
        buff[6] = '\0'; // Null terminator
        return;
    }
    buff[6] = seconds / 10 + '0';
    buff[7] = seconds % 10 + '0';
    buff[8] = '\0'; // Null terminator
}



// --------------------------------------------------------------------------------------------





//* 4. Interrupt functions =====================================================================

void doorInterrupt(){

  cli(); // disable interrupts
  
  // HIGH == door open
  if(digitalRead(PIN_DOOR_INTERRUPT) == HIGH){
    __core.denyFiring(); // prevent the heater from turning on
    digitalWrite(PIN_HEATER, LOW);      // turn off the heater

    // if the system is executing a program, pause the execution
    if(__program.IsSelected()){
      __core.updateStatus(DOOR_OPEN);        // manage door opening during execution
    }
  }  
  else { // if the door is closed, resume the system
    __core.allowFiring();  // allow the heater to turn on

    // if the system was executing a program, resume the execution
    if(__program.IsSelected()){

      // Move back to EXECUTING
     __core.updateStatus(EXECUTING);

      // if the door action was expected, move to the next instruction
      if(__program.CurrentInstruction().waitForDoorOpen){
        __program.nextInstruction();
        
      }
      else {
        // if the door action was not expected, recover the system temperature
        __core.updateStatus(RECOVER); // TODO: implement the recover state
      }

    }
    else {
      __core.updateStatus(IDLE);
    }
  }

  sei(); // enable interrupts
};

void timerIsr(){
  __encoder.service();
};



// --------------------------------------------------------------------------------------------

