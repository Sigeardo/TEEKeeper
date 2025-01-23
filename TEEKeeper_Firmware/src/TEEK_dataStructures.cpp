#include "TEEK_dataStructures.h"
#include <Arduino.h>

// This is only to prevent the IDE from complaining
#ifndef TEEK_GRAPHICS_H
#include <TEEK_graphics.h>
#include <TEEKeeper.h>
#endif

// ==== TEMPERATURE PROBE CLASS =====
TemperatureProbe::TemperatureProbe() : sensor(PIN_SPI_SCK, PIN_PROBE_CS, PIN_SPI_MISO) {unit = CELSIUS;};
TemperatureProbe::TemperatureProbe(TemperatureUnit u) : sensor(PIN_SPI_CLK, PIN_PROBE_CS, PIN_SPI_MISO) {unit = u;};



// Uncomment to disable the probe (USED FOR DEBUGGING ONLY!)
//#define PROBE_DEBUG

#ifndef PROBE_DEBUG
double TemperatureProbe::readTemp(){
  unsigned int errCount = 0;
  double temp = 0;
  extern char errorStreamChar[ERROR_BUFF_SIZE];


  // poll the temperature until you read something that isn't a NaN
  do{
    Serial.println("Temperature reading...");

    cli(); // turn off interrupts to avoid conflicts with the SPI bus
    temp = sensor.readCelsius();  // probe
    sei();  // turn on interrupts
    
    // if it is a NaN, increment the error counter
    if(isnan(temp)){
      errCount++;
      Serial.print(errCount); Serial.println(" error reading temperature. Retrying...");

      // if the error counter is too high, return a NaN
      if(errCount > MAX_N_ERROR_READINGS){
        sprintf(errorStreamChar, "Can't measure the temperature, check the wiring. Shutting off...");
        Serial.println(errorStreamChar);
        return NAN;
      }
      delay(5); // delay to avoid reading too fast
    }
  } while(isnan(temp));

  // temperature is read correctly
  // check boundaries
  if(temp > ERROR_TEMP){
    sprintf(errorStreamChar,"Temperature is too high. Shutting off to prevent damage...");
    Serial.println(errorStreamChar);
    return NAN;
  }
  else if(temp < MIN_TEMPERATURE){
    sprintf(errorStreamChar,"Temperature is too low. Possible damage to the probe. Shutting off...");
    Serial.println(errorStreamChar);
    return NAN;
  }

  // temperture is within boundaries
  Serial.println("Temp reading complete.");
  if(unit == FAHRENHEIT) return toFarhenheit(temp);
  else if(unit == KELVIN) return temp + 273.15;
  else return temp;
};
#else 
double TemperatureProbe::readTemp(){
  return 20.0+0.01*random(-100,100);
};

#endif



// ==== CORESYSTEM CLASS =====

CoreSystem::CoreSystem(TemperatureProbe& _probe){
  probe = _probe;
  status = IDLE;
  unit = CELSIUS;
  mode = NORMAL;
  targetTemperature = 0;
  currentTemperature = 0;
  dutyCycle = 0;
  kp = PWM_DEFAULT_KP;
  ki = PWM_DEFAULT_KI;
  kd = PWM_DEFAULT_KD;
  PWMPeriod = CYCLE_TIME;
  last_error = 0;
  integral = 0;
  nextPWMCycle = 0;
  dutyEnd = 0;
  allowFiringHeater = false;
  fireHeater = false;
  stabilityCounter = 0;
  isStable = false;
  keepLog = true;
  lastDoorOpenTime = 0;
  isTuning = false;
};

CoreSystem::CoreSystem(TemperatureProbe& _probe, double _kp, double _ki, double _kd){
  probe = _probe;
  status = IDLE;
  unit = CELSIUS;
  mode = NORMAL;
  targetTemperature = 0;
  currentTemperature = 0;
  dutyCycle = 0;
  kp = _kp;
  ki = _ki;
  kd = _kd;
  PWMPeriod = CYCLE_TIME;
  last_error = 0;
  integral = 0;
  nextPWMCycle = 0;
  dutyEnd = 0;
  allowFiringHeater = false;
  fireHeater = false;
  stabilityCounter = 0;
  isStable = false;
  keepLog = true;
  lastDoorOpenTime = 0;
  isTuning = false;
};


double CoreSystem::PID(const double error){
  integral += error;
  double derivative = error - last_error;
  dutyCycle = kp * error + ki * integral + kd * derivative;

  if(dutyCycle > 95){
    dutyCycle = 100;
  }
  else if(dutyCycle < 5){
    dutyCycle = 0;
  }
  return dutyCycle;
}


void CoreSystem::allowFiring(){
  
  if(currentTemperature > MAX_TEMPERATURE){
    sprintf(errorStreamChar, "Temperature is too high. Shutting off...");
    allowFiringHeater = false;
    updateStatus(ERROR);
  }
  else if(currentTemperature < MIN_TEMPERATURE){
    sprintf(errorStreamChar, "Temperature is too low. Shutting off...");
    allowFiringHeater = false;
    updateStatus(ERROR);
  }
  else if(status == DOOR_OPEN){
    allowFiringHeater = false;
  }
  else{
    allowFiringHeater = true;
  }


};

char* CoreSystem::getTextUnit() const{
  switch(unit){
    case CELSIUS:       return (char*)"Celsius";
    case FAHRENHEIT:    return (char*)"Fahrenheit";
    case KELVIN:        return (char*)"Kelvin";
    default:            return (char*)"Celsius";
  }
};


void CoreSystem::PIDAutotune(){

  // Prepare the system for the autotune process
  extern AutotuneParameters __autPar;
  // reset autotune parameters   
  __autPar.Ku = 0;
  __autPar.Tu = 0;
  __autPar.startTime = 0;
  __autPar.lastToggleTime = 0;
  __autPar.heaterState = false;
  __autPar.highTemp = 0;
  __autPar.lowTemp = 0;
  __autPar.oscillationCount = 0;

  extern ProgramManager __program;
  __program.clearProgram(); // clear the program manager

  // set the control mode to PID_AUTOTUNE
  mode = PID_AUTOTUNE;
}

void CoreSystem::Clear(){
  status = IDLE;
  targetTemperature = 0;
  currentTemperature = 0;
  dutyCycle = 0;
  lastTempReading = 0;
  last_error = 0;
  integral = 0;
  nextPWMCycle = 0;
  dutyEnd = 0;
  fireHeater = false;
  stabilityCounter = 0;
  isStable = false;
  lastDoorOpenTime = 0;
  isTuning = false;
}

void CoreSystem::setTarget(double target, bool newInstruction){
  if(newInstruction){
    targetTemperature = target;
    isStable = false;
    stabilityCounter = 0;
  }
  else{
    targetTemperature = target;
  }
}

void CoreSystem::ReadTemperature(){
  double temp = probe.readTemp();

  // check if the temperature is a NaN
  if(!isnan(temp)){
    currentTemperature = temp;
    lastTempReading = millis();
    return;
  }

  // if the temperature is a NaN, a critical error occurred
  CriticalError();
}

void CoreSystem::CriticalError(){

  // set the status to ERROR
  status = ERROR;

  // turn off the heater
  digitalWrite(PIN_HEATER, LOW);
  allowFiringHeater = false;

  // if we are running a program, log the critical error
  extern ProgramManager __program;
  if(__program.IsSelected() && keepLog){
    extern File *__file;  // log file
    updateLog(*__file, errorStreamChar, __program.elapsedTime()); 
    closeLog(*__file, __program);
  };

  // display error screen
  extern ScreenManager __GUI;
  extern CriticalErrorScreen __criticalErrorScreen;
  __GUI.setScreen(&__criticalErrorScreen);

  // get stuck here to force user reset
  while(true){
    delay(1000);
  }

}

// ===================================================================================

// ==== PROGRAM MANAGER CLASS =====

ProgramManager::ProgramManager(){
  strncpy(programName, "", MAX_FILENAME_LENGTH);
  numOfInstructions = 0;
  instructionIndex = 0;
  progStartTime = 0;
  instrStartTime = 0;
  soakTimeStart = 0;
  soakTimeEnd = 0;
  isSoaking = false;
  targetReached = false;
  isSelected = false;
};

// Add an instruction to the program
bool ProgramManager::addInstruction(Instruction instr) {
  if(numOfInstructions < MAX_INSTRUCTIONS_PER_PROGRAM) {
    instructions[numOfInstructions] = instr;
    numOfInstructions++;
    return true;
  } else {
    //errorStream = "ERROR: maximum number of instructions reached.\n";
    sprintf(errorStreamChar,"Maximum number of instructions reached.\n");
    return false;
  }
};
// --------------------------------------------------------------------------------------------

// Parse the CSV file and load the program into the program manager
bool ProgramManager::loadProgram(File& file) {
    // Check if the file is valid
    if (!file) {
        sprintf(errorStreamChar, "ERROR: File not found.\n");
        return false;
    }

    // Clear existing program instructions
    clearProgram();

    // Read and set the program's name
    char fileName[MAX_FILENAME_LENGTH];
    file.getName(fileName, MAX_FILENAME_LENGTH - 1);
    setName(fileName);

    // Skip the CSV header
    skipLine(file);

    // Buffers and variables for parsing
    char lineBuffer[128];            // Buffer for a single CSV line
    char name[MAX_INSTR_NAME_LENGHT]; // Instruction name buffer
    double target = 0, rampRate = 0;
    unsigned long holdTime = 0;
    bool waitForDoorOpen = false, waitForButtonPress = false;

    // Read and parse each line until the end of the file or max instructions reached
    while (file.available() && numOfInstructions < MAX_INSTRUCTIONS_PER_PROGRAM) {
        // Read a line from the CSV
        if (!readLine(file, lineBuffer, sizeof(lineBuffer))) {
            sprintf(errorStreamChar, "ERROR: Malformed CSV file.\n");
            file.close();
            return false;
        }

        // Parse the CSV line into individual fields
        if (!parseCSVLine(
                lineBuffer, name, sizeof(name),
                &target, &holdTime, &rampRate,
                &waitForDoorOpen, &waitForButtonPress)) {
            sprintf(errorStreamChar, "ERROR: Invalid data in line: %s\n", lineBuffer);
            file.close();
            return false;
        }

        // Add the instruction to the program
        if (!addInstruction(name, holdTime * MINUTE, target, rampRate, waitForDoorOpen, waitForButtonPress)) {
            sprintf(errorStreamChar, "ERROR: Could not add instruction.\n");
            file.close();
            return false;
        }
    }

    // Close the file and return success
    file.close();
    isSelected = true;
    return true;
}

// Skip a line
void ProgramManager::skipLine(File& file) {
    while (file.available()) {
        if (file.read() == '\n') break;
    }
}

// get a line from a file
bool ProgramManager::readLine(File& file, char* buffer, size_t bufferSize) {
    size_t i = 0;
    while (file.available() && i < bufferSize - 1) {
        char c = file.read();
        if (c == '\n') break; // End of line
        buffer[i++] = c;
    }
    buffer[i] = '\0'; // Null-terminate the line
    return (i > 0);   // Return true if any data was read
}

// Parse a line and extract the instruction fields
bool ProgramManager::parseCSVLine(
    const char* line,
    char* name, size_t nameSize,
    double* target, unsigned long* holdTime, double* rampRate,
    bool* waitForDoorOpen, bool* waitForButtonPress) {
    
    // Temporary buffer for numeric fields
    char tempBuffer[16];
    size_t tempIndex = 0;

    // Pointers for traversing the line
    const char* ptr = line;
    const char* start = ptr;

    // Parse the instruction name
    tempIndex = 0;
    while (*ptr != ',' && *ptr != '\0') {
        if (tempIndex < nameSize - 1) {
            name[tempIndex++] = *ptr++;
        } else {
            return false; // Name too long
        }
    }
    name[tempIndex] = '\0'; // Null-terminate the name
    if (*ptr++ != ',') return false; // Move past ',' or fail

    // Parse the target temperature
    start = ptr;
    ptr = extractField(ptr, tempBuffer, sizeof(tempBuffer));
    *target = atof(tempBuffer);

    // Parse the hold time
    start = ptr;
    ptr = extractField(ptr, tempBuffer, sizeof(tempBuffer));
    *holdTime = atol(tempBuffer);

    // Parse the ramp rate
    start = ptr;
    ptr = extractField(ptr, tempBuffer, sizeof(tempBuffer));
    *rampRate = atof(tempBuffer);

    // Parse the waitForDoorOpen flag
    *waitForDoorOpen = (*ptr == '1');
    ptr += 2; // Move past the flag and comma

    // Parse the waitForButtonPress flag
    *waitForButtonPress = (*ptr == '1');

    return true; // Parsing successful
}

// Extract a field from a CSV line into a buffer
const char* ProgramManager::extractField(const char* line, char* buffer, size_t bufferSize) {
    size_t i = 0;
    while (*line != ',' && *line != '\0' && i < bufferSize - 1) {
        buffer[i++] = *line++;
    }
    buffer[i] = '\0'; // Null-terminate the buffer
    if (*line == ',') line++; // Consume the comma
    return line;
}


// --------------------------------------------------------------------------------------------

// Add an instruction to the program
bool ProgramManager::addInstruction(char* name, unsigned long soakTime, double target, double tempVariationRate, bool waitForDoorOpen, bool waitForButtonPress) {
  if(numOfInstructions < MAX_INSTRUCTIONS_PER_PROGRAM) {
    sprintf(instructions[numOfInstructions].name, "%s", name); // copy instruction name
    // ensure null termination
    instructions[numOfInstructions].name[MAX_INSTR_NAME_LENGHT - 1] = '\0';
    instructions[numOfInstructions].soakTime = soakTime;
    instructions[numOfInstructions].target = target;
    instructions[numOfInstructions].tempVariationRate = tempVariationRate;
    instructions[numOfInstructions].waitForDoorOpen = waitForDoorOpen;
    instructions[numOfInstructions].waitForButtonPress = waitForButtonPress;

    numOfInstructions++;
    return true;
  } else {
    //errorStream = "ERROR: maximum number of instructions reached.\n";
    sprintf(errorStreamChar,"Maximum number of instructions reached.\n");
    return false;
  }
};

// --------------------------------------------------------------------------------------------

// Move to the next instruction
bool ProgramManager::nextInstruction(){
  if(instructionIndex < numOfInstructions) {
    instructionIndex++;
    // reset control fields
    instrStartTime = millis();
    soakTimeStart = 0;
    soakTimeEnd = 0;
    isSoaking = false;
    targetReached = false;
    return true;
  } else {
    return false;
  }
};

// --------------------------------------------------------------------------------------------

// Remove an instruction from the program
bool ProgramManager::removeInstruction(unsigned int index){
  if (index > numOfInstructions) {
    //errorStream = "ERROR: instruction index out of bounds.\n";
    sprintf(errorStreamChar,"Instruction index out of bounds.\n");
    return false;
  }

  for(unsigned int i = index; i < numOfInstructions; i++) {
    instructions[i] = instructions[i+1];
  }

  numOfInstructions--;
  return true;
};


// Check if the current instruction has been completed
bool ProgramManager::isInstructionDone(){
  // check if the soak time has run out
  if(isSoaking && millis() - soakTimeStart >= instructions[instructionIndex].soakTime) {
    if(soakTimeEnd == 0) soakTimeEnd = millis();  // set the end time of the soak time
    return true;
  } else {
    return false;
  }
};

// --------------------------------------------------------------------------------------------

// Get an instruction from the program
Instruction ProgramManager::GetInstruction(unsigned int index) {
  if (index > numOfInstructions) {
    //errorStream = "ERROR: instruction index out of bounds.\n";
    sprintf(errorStreamChar,"Instruction index out of bounds.\n");
    return Instruction();
  }

  return instructions[index];
};

// --------------------------------------------------------------------------------------------

// Get the current instruction
Instruction ProgramManager::CurrentInstruction() {
  return instructions[instructionIndex];
};

// --------------------------------------------------------------------------------------------

// Start the soaking timer
void ProgramManager::startSoakTimer(){
  soakTimeStart = millis();
  isSoaking = true;
  targetReached = false;
};

// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------

// Clear the program fields
void ProgramManager::clearProgram() {
  for (unsigned int i = 0; i < MAX_INSTRUCTIONS_PER_PROGRAM; i++) {
    instructions[i] = Instruction();
  }
  numOfInstructions = 0;
  instructionIndex = 0;
  instrStartTime = 0;
  soakTimeStart = 0;
  isSoaking = false;
  targetReached = false;
  sprintf(errorStreamChar, " ");
};

// --------------------------------------------------------------------------------------------

// Reset the timers and stability checks for the current instruction
// Execution is reset to the beginning of the instruction
void ProgramManager::resetCurrentInstruction(){
  instrStartTime = millis();
  soakTimeStart  = 0;
  soakTimeEnd  = 0;
  isSoaking = false;
  targetReached  = false;
}

// --------------------------------------------------------------------------------------------

// Resets the timers and stability checks for the current instruction,
// resuming the instruction from a given time. Temperature is assumed to be stable
// (i.e. resuming the execution after a PAUSE state)
void ProgramManager::resetCurrentInstruction(unsigned long time){
  instrStartTime = millis();
  soakTimeStart  = time;
  isSoaking = false;
  targetReached  = false;
}
