//sensor imports
#include <QTRSensors.h>

// For serial communication with the emic
#include <SoftwareSerial.h>

//To use timed actions -> protothreading
#include <arduino-timer.h>

// To save data to program space
#include <avr/pgmspace.h>

//Create timer to schedule the project parts
auto timer = timer_create_default();

//definitions
#define rxPin 6  // Connect SOUT pin of the Emic 2 module to the RX pin
#define txPin 7  // Connect SIN pin of the Emic 2 module to the TX pin
#define keepAlivePin 8
#define StatusLED 9


//CUSTOM_SETTINGS_DECLARATION_START
#define voiceStyle CUSTOM_SETTING_0 // Male or female voice
#define initialVolume CUSTOM_SETTING_1  // Update Volume Funktion einbinden, Aus dem Customizationtool kommt 1-5
#define closedDuration CUSTOM_SETTING_2 // How long to close the eyes to open the menu
#define keepAliveOpt CUSTOM_SETTING_3
#define statusLEDActive CUSTOM_SETTING_4 // Learning LED which turns on when a direction is detected // Funktion to be implemented
#define dirDuration CUSTOM_SETTING_5 // Duration for which a direction has to be measured to be recognized
#define dirPause CUSTOM_SETTING_6 // Cooldown after dir1 has been recognized
#define timeOutDuration CUSTOM_SETTING_7 // Timelimit after which direction 1 gets rejected
#define useAutoCalibration CUSTOM_SETTING_8
//CUSTOM_SETTINGS_DECLARATION_END


//variables: settings
bool muted = false;
uint8_t volume = initialVolume;
uint8_t activeProfile = 1;
bool inMenu = true;

// Margins for eye tracking detection
uint8_t factorL = 87;
uint8_t factorR = 87;
uint8_t factorUp = 90;
uint8_t factorDown = 83;

//variables: program logic
//Sensors

QTRSensors qtr;
uint16_t sensorValue[4];
uint16_t neutralSensorValue[4];

//Emic
SoftwareSerial emicSerial =  SoftwareSerial(rxPin, txPin);
bool emicReady;

//Input treatment
int8_t dir1 = -1; //0 1 2 3 for left Up right down; 4 for closed; -1 for null
int8_t dir2 = -1; //0 1 2 3 for left Up right down; 4 for closed; -1 for null
uint8_t status = 0; //0: Waiting for recognition; 1: timeout after dir1; 2: waiting for recognition 2
unsigned long signalTracker; //Variable to track for how long a certain signal has been detected
unsigned long timeoutTracker; //Variable to track the time when a timeout is reached



char textdataBuffer[100]; // To buffer PROGMEM readings - Has to be large enough for the largest string in must hold

//Loading Text Data into Progmem
// Menu
const char textdata_0[] PROGMEM = "Leave Menu";
const char textdata_1[] PROGMEM = "Volume Up";
const char textdata_2[] PROGMEM = "Toggle Mute"; // Muted - Unmuted möglich?
const char textdata_3[] PROGMEM = "Volume Down";
const char textdata_4[] PROGMEM = "No";
const char textdata_5[] PROGMEM = "Okay";
const char textdata_6[] PROGMEM = "Yes";
const char textdata_7[] PROGMEM = "Help";
const char textdata_8[] PROGMEM = "Profile 1";
const char textdata_9[] PROGMEM = "Profile 2";
const char textdata_10[] PROGMEM = "Profile 3";
const char textdata_11[] PROGMEM = "Profile 4";
const char textdata_12[] PROGMEM = "Profile 5";
const char textdata_13[] PROGMEM = "Profile 6";
const char textdata_14[] PROGMEM = "Profile 7";
const char textdata_15[] PROGMEM = "Profile 8";

// PLATZHALTER Profile 1 - Home
//CUSTOM_EXPRESSION_DECLARATION_START
const char textdata_16[] PROGMEM = "CUSTOM_EXPRESSION_0";	
const char textdata_17[] PROGMEM = "CUSTOM_EXPRESSION_1";	
const char textdata_18[] PROGMEM = "CUSTOM_EXPRESSION_2";	
const char textdata_19[] PROGMEM = "CUSTOM_EXPRESSION_3";	
const char textdata_20[] PROGMEM = "CUSTOM_EXPRESSION_4";	
const char textdata_21[] PROGMEM = "CUSTOM_EXPRESSION_5";	
const char textdata_22[] PROGMEM = "CUSTOM_EXPRESSION_6";	
const char textdata_23[] PROGMEM = "CUSTOM_EXPRESSION_7";	
const char textdata_24[] PROGMEM = "CUSTOM_EXPRESSION_8";	
const char textdata_25[] PROGMEM = "CUSTOM_EXPRESSION_9";	
const char textdata_26[] PROGMEM = "CUSTOM_EXPRESSION_10";	
const char textdata_27[] PROGMEM = "CUSTOM_EXPRESSION_11";	
const char textdata_28[] PROGMEM = "CUSTOM_EXPRESSION_12";	
const char textdata_29[] PROGMEM = "CUSTOM_EXPRESSION_13";	
const char textdata_30[] PROGMEM = "CUSTOM_EXPRESSION_14";	
const char textdata_31[] PROGMEM = "CUSTOM_EXPRESSION_15";	

// Profile 2
const char textdata_32[] PROGMEM = "CUSTOM_EXPRESSION_16";	
const char textdata_33[] PROGMEM = "CUSTOM_EXPRESSION_17";	
const char textdata_34[] PROGMEM = "CUSTOM_EXPRESSION_18";	
const char textdata_35[] PROGMEM = "CUSTOM_EXPRESSION_19";	
const char textdata_36[] PROGMEM = "CUSTOM_EXPRESSION_20";	
const char textdata_37[] PROGMEM = "CUSTOM_EXPRESSION_21";	
const char textdata_38[] PROGMEM = "CUSTOM_EXPRESSION_22";	
const char textdata_39[] PROGMEM = "CUSTOM_EXPRESSION_23";	
const char textdata_40[] PROGMEM = "CUSTOM_EXPRESSION_24";	
const char textdata_41[] PROGMEM = "CUSTOM_EXPRESSION_25";	
const char textdata_42[] PROGMEM = "CUSTOM_EXPRESSION_26";	
const char textdata_43[] PROGMEM = "CUSTOM_EXPRESSION_27";	
const char textdata_44[] PROGMEM = "CUSTOM_EXPRESSION_28";	
const char textdata_45[] PROGMEM = "CUSTOM_EXPRESSION_29";	
const char textdata_46[] PROGMEM = "CUSTOM_EXPRESSION_30";	
const char textdata_47[] PROGMEM = "CUSTOM_EXPRESSION_31";	

// Profile 3	
const char textdata_48[] PROGMEM = "CUSTOM_EXPRESSION_32";	
const char textdata_49[] PROGMEM = "CUSTOM_EXPRESSION_33";	
const char textdata_50[] PROGMEM = "CUSTOM_EXPRESSION_34";	
const char textdata_51[] PROGMEM = "CUSTOM_EXPRESSION_35";	
const char textdata_52[] PROGMEM = "CUSTOM_EXPRESSION_36";	
const char textdata_53[] PROGMEM = "CUSTOM_EXPRESSION_37";	
const char textdata_54[] PROGMEM = "CUSTOM_EXPRESSION_38";	
const char textdata_55[] PROGMEM = "CUSTOM_EXPRESSION_39";	
const char textdata_56[] PROGMEM = "CUSTOM_EXPRESSION_40";	
const char textdata_57[] PROGMEM = "CUSTOM_EXPRESSION_41";	
const char textdata_58[] PROGMEM = "CUSTOM_EXPRESSION_42";	
const char textdata_59[] PROGMEM = "CUSTOM_EXPRESSION_43";	
const char textdata_60[] PROGMEM = "CUSTOM_EXPRESSION_44";	
const char textdata_61[] PROGMEM = "CUSTOM_EXPRESSION_45";	
const char textdata_62[] PROGMEM = "CUSTOM_EXPRESSION_46";	
const char textdata_63[] PROGMEM = "CUSTOM_EXPRESSION_47";	

// Profile 4	
const char textdata_64[] PROGMEM = "CUSTOM_EXPRESSION_48";	
const char textdata_65[] PROGMEM = "CUSTOM_EXPRESSION_49";	
const char textdata_66[] PROGMEM = "CUSTOM_EXPRESSION_50";	
const char textdata_67[] PROGMEM = "CUSTOM_EXPRESSION_51";	
const char textdata_68[] PROGMEM = "CUSTOM_EXPRESSION_52";	
const char textdata_69[] PROGMEM = "CUSTOM_EXPRESSION_53";	
const char textdata_70[] PROGMEM = "CUSTOM_EXPRESSION_54";	
const char textdata_71[] PROGMEM = "CUSTOM_EXPRESSION_55";	
const char textdata_72[] PROGMEM = "CUSTOM_EXPRESSION_56";	
const char textdata_73[] PROGMEM = "CUSTOM_EXPRESSION_57";	
const char textdata_74[] PROGMEM = "CUSTOM_EXPRESSION_58";	
const char textdata_75[] PROGMEM = "CUSTOM_EXPRESSION_59";	
const char textdata_76[] PROGMEM = "CUSTOM_EXPRESSION_60";	
const char textdata_77[] PROGMEM = "CUSTOM_EXPRESSION_61";	
const char textdata_78[] PROGMEM = "CUSTOM_EXPRESSION_62";	
const char textdata_79[] PROGMEM = "CUSTOM_EXPRESSION_63";	

// Profile 5
const char textdata_80[] PROGMEM = "CUSTOM_EXPRESSION_64";	
const char textdata_81[] PROGMEM = "CUSTOM_EXPRESSION_65";	
const char textdata_82[] PROGMEM = "CUSTOM_EXPRESSION_66";	
const char textdata_83[] PROGMEM = "CUSTOM_EXPRESSION_67";	
const char textdata_84[] PROGMEM = "CUSTOM_EXPRESSION_68";	
const char textdata_85[] PROGMEM = "CUSTOM_EXPRESSION_69";	
const char textdata_86[] PROGMEM = "CUSTOM_EXPRESSION_70";	
const char textdata_87[] PROGMEM = "CUSTOM_EXPRESSION_71";	
const char textdata_88[] PROGMEM = "CUSTOM_EXPRESSION_72";	
const char textdata_89[] PROGMEM = "CUSTOM_EXPRESSION_73";	
const char textdata_90[] PROGMEM = "CUSTOM_EXPRESSION_74";	
const char textdata_91[] PROGMEM = "CUSTOM_EXPRESSION_75";	
const char textdata_92[] PROGMEM = "CUSTOM_EXPRESSION_76";	
const char textdata_93[] PROGMEM = "CUSTOM_EXPRESSION_77";	
const char textdata_94[] PROGMEM = "CUSTOM_EXPRESSION_78";	
const char textdata_95[] PROGMEM = "CUSTOM_EXPRESSION_79";	

// Profile 6
const char textdata_96[] PROGMEM = "CUSTOM_EXPRESSION_80";	
const char textdata_97[] PROGMEM = "CUSTOM_EXPRESSION_81";	
const char textdata_98[] PROGMEM = "CUSTOM_EXPRESSION_82";	
const char textdata_99[] PROGMEM = "CUSTOM_EXPRESSION_83";	
const char textdata_100[] PROGMEM = "CUSTOM_EXPRESSION_84";	
const char textdata_101[] PROGMEM = "CUSTOM_EXPRESSION_85";	
const char textdata_102[] PROGMEM = "CUSTOM_EXPRESSION_86";	
const char textdata_103[] PROGMEM = "CUSTOM_EXPRESSION_87";	
const char textdata_104[] PROGMEM = "CUSTOM_EXPRESSION_88";	
const char textdata_105[] PROGMEM = "CUSTOM_EXPRESSION_89";	
const char textdata_106[] PROGMEM = "CUSTOM_EXPRESSION_90";	
const char textdata_107[] PROGMEM = "CUSTOM_EXPRESSION_91";	
const char textdata_108[] PROGMEM = "CUSTOM_EXPRESSION_92";	
const char textdata_109[] PROGMEM = "CUSTOM_EXPRESSION_93";	
const char textdata_110[] PROGMEM = "CUSTOM_EXPRESSION_94";	
const char textdata_111[] PROGMEM = "CUSTOM_EXPRESSION_95";	

// profile 7	
const char textdata_112[] PROGMEM = "CUSTOM_EXPRESSION_96";	
const char textdata_113[] PROGMEM = "CUSTOM_EXPRESSION_97";	
const char textdata_114[] PROGMEM = "CUSTOM_EXPRESSION_98";	
const char textdata_115[] PROGMEM = "CUSTOM_EXPRESSION_99";	
const char textdata_116[] PROGMEM = "CUSTOM_EXPRESSION_100";	
const char textdata_117[] PROGMEM = "CUSTOM_EXPRESSION_101";	
const char textdata_118[] PROGMEM = "CUSTOM_EXPRESSION_102";	
const char textdata_119[] PROGMEM = "CUSTOM_EXPRESSION_103";	
const char textdata_120[] PROGMEM = "CUSTOM_EXPRESSION_104";	
const char textdata_121[] PROGMEM = "CUSTOM_EXPRESSION_105";	
const char textdata_122[] PROGMEM = "CUSTOM_EXPRESSION_106";	
const char textdata_123[] PROGMEM = "CUSTOM_EXPRESSION_107";	
const char textdata_124[] PROGMEM = "CUSTOM_EXPRESSION_108";	
const char textdata_125[] PROGMEM = "CUSTOM_EXPRESSION_109";	
const char textdata_126[] PROGMEM = "CUSTOM_EXPRESSION_110";	
const char textdata_127[] PROGMEM = "CUSTOM_EXPRESSION_111";	

// profile 8
const char textdata_128[] PROGMEM = "CUSTOM_EXPRESSION_112";	
const char textdata_129[] PROGMEM = "CUSTOM_EXPRESSION_113";	
const char textdata_130[] PROGMEM = "CUSTOM_EXPRESSION_114";	
const char textdata_131[] PROGMEM = "CUSTOM_EXPRESSION_115";	
const char textdata_132[] PROGMEM = "CUSTOM_EXPRESSION_116";	
const char textdata_133[] PROGMEM = "CUSTOM_EXPRESSION_117";	
const char textdata_134[] PROGMEM = "CUSTOM_EXPRESSION_118";	
const char textdata_135[] PROGMEM = "CUSTOM_EXPRESSION_119";	
const char textdata_136[] PROGMEM = "CUSTOM_EXPRESSION_120";	
const char textdata_137[] PROGMEM = "CUSTOM_EXPRESSION_121";	
const char textdata_138[] PROGMEM = "CUSTOM_EXPRESSION_122";	
const char textdata_139[] PROGMEM = "CUSTOM_EXPRESSION_123";	
const char textdata_140[] PROGMEM = "CUSTOM_EXPRESSION_124";	
const char textdata_141[] PROGMEM = "CUSTOM_EXPRESSION_125";	
const char textdata_142[] PROGMEM = "CUSTOM_EXPRESSION_126";	
const char textdata_143[] PROGMEM = "CUSTOM_EXPRESSION_127";
//CUSTOM_EXPRESSION_DECLARATION_END



const char *const data_table[] PROGMEM = {textdata_0, textdata_1, textdata_2, textdata_3, textdata_4, textdata_5, textdata_6, textdata_7, textdata_8, textdata_9, textdata_10,
                                          textdata_11, textdata_12, textdata_13, textdata_14, textdata_15, textdata_16, textdata_17, textdata_18, textdata_19, textdata_20,
                                          textdata_21, textdata_22, textdata_23, textdata_24, textdata_25, textdata_26, textdata_27, textdata_28, textdata_29, textdata_30,
                                          textdata_31, textdata_32, textdata_33, textdata_34, textdata_35, textdata_36, textdata_37, textdata_38, textdata_39, textdata_40,
                                          textdata_41, textdata_42, textdata_43, textdata_44, textdata_45, textdata_46, textdata_47, textdata_48, textdata_49, textdata_50,
                                          textdata_51, textdata_52, textdata_53, textdata_54, textdata_55, textdata_56, textdata_57, textdata_58, textdata_59, textdata_60,
                                          textdata_61, textdata_62, textdata_63, textdata_64, textdata_65, textdata_66, textdata_67, textdata_68, textdata_69, textdata_70,
                                          textdata_71, textdata_72, textdata_73, textdata_74, textdata_75, textdata_76, textdata_77, textdata_78, textdata_79, textdata_80,
                                          textdata_81, textdata_82, textdata_83, textdata_84, textdata_85, textdata_86, textdata_87, textdata_88, textdata_89, textdata_90,
                                          textdata_91, textdata_92, textdata_93, textdata_94, textdata_95, textdata_96, textdata_97, textdata_98, textdata_99, textdata_100,
                                          textdata_101, textdata_102, textdata_103, textdata_104, textdata_105, textdata_106, textdata_107, textdata_108, textdata_109, textdata_110,
                                          textdata_111, textdata_112, textdata_113, textdata_114, textdata_115, textdata_116, textdata_117, textdata_118, textdata_119, textdata_120,
                                          textdata_121, textdata_122, textdata_123, textdata_124, textdata_125, textdata_126, textdata_127, textdata_128, textdata_129, textdata_130,
                                          textdata_131, textdata_132, textdata_133, textdata_134, textdata_135, textdata_136, textdata_137, textdata_138, textdata_139, textdata_140,
                                          textdata_141, textdata_142, textdata_143
                                         }; //has to include all 144 strings in the end


void setup()
{
  pinMode(keepAlivePin, OUTPUT);//LED for debug purposes at current stage of the project
  pinMode(StatusLED, OUTPUT);//LED for debug purposes at current stage of the project

  // set the data rate for the hardware serial port
  Serial.begin(9600);

  // configure the sensors
  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]) {
    2, 3, 4, 5
  }, 4);
  
  qtr.setTimeout(5000);
  
  // set the data rate for the SoftwareSerial port
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  emicSerial.begin(9600);
  emicSerial.print('\n');             // Send a CR in case the system is already up
  while (emicSerial.read() != ':');   // When the Emic 2 has initialized and is ready, it will send a single ':' character, so wait here until we receive it
  emicSerial.flush();                 // Flush the receive buffer

  // set volume of emic
  emicSerial.print('V');
  emicSerial.print(String(volume));
  emicSerial.print('\n');

  // set voicestyle of emic
  emicSerial.print('N');
  switch (voiceStyle) {
    case 1: emicSerial.print('0'); break;
    case 2: emicSerial.print('8'); break;
  }
  emicSerial.print('\n');

  emicReady = true;

  if (useAutoCalibration) {
    autocalibrateSensors();
  } else {
    calibrateSensors();
    setupTimers();
  }
}

void setupTimers() {
  // Setting up the timers
  timer.every(100, readSensors);
  timer.every(100, updateEmicReady);
  timer.every(3000, printDebugInfo);
  if (keepAlive) {
    timer.every(5000, keepAlive);
  }
}

void calibrateSensors() {
  qtr.read(neutralSensorValue);
  neutralSensorValue[0] = uint32_t(neutralSensorValue[0]) * factorL / 100;
  neutralSensorValue[1] = uint32_t(neutralSensorValue[1]) * factorUp / 100;
  neutralSensorValue[2] = uint32_t(neutralSensorValue[2]) * factorL / 100;
  neutralSensorValue[3] = uint32_t(neutralSensorValue[3]) * factorDown / 100;
  Serial.println(neutralSensorValue[0]);
  Serial.println(neutralSensorValue[1]);
  Serial.println(neutralSensorValue[2]);
  Serial.println(neutralSensorValue[3]);
}

void autocalibrateSensors() {
  qtr.read(neutralSensorValue);
  
  letEmicSpeak("Left");
  while (emicSerial.read() != ':');
  emicReady = true;
  delay(2000);
  autocalibrateSensor(2);
  
  letEmicSpeak("Up");
  while (emicSerial.read() != ':');
  emicReady = true;
  delay(2000);
  autocalibrateSensor(3);
  
  letEmicSpeak("Right");
  while (emicSerial.read() != ':');
  emicReady = true;
  delay(2000);
  autocalibrateSensor(0);
  
  letEmicSpeak("Down");
  while (emicSerial.read() != ':');
  emicReady = true;
  delay(2000);
  autocalibrateSensor(1);
  
  letEmicSpeak("Calibration completed");
  while (emicSerial.read() != ':');
  emicReady = true;
  setupTimers();
}

void autocalibrateSensor(uint8_t sensornumber) {
  qtr.read(sensorValue);
  neutralSensorValue[sensornumber] = (4 * sensorValue[sensornumber] + neutralSensorValue[sensornumber]) / 5;
}

void setFactor(uint8_t sensornumber) {
  
}
bool updateEmicReady() {
  if (emicSerial.read() == ':') {
    emicReady = true;
  }
}

void letEmicSpeak(char message[]) {
  if (emicReady) { //Sollten wir bei jeder Sprachausgabe prüfen, um dem emic erst eine neue Ausgabe zu senden, wenn er bereit ist
    emicReady = false;
    emicSerial.print('S');
    emicSerial.print(message);  // Send the desired string to convert to speech
    emicSerial.print('\n');
  }
}

void toggleMute() {
  muted = !muted;
}

void bufferTextdata(uint8_t entrynumber) {
  strcpy_P(textdataBuffer, (char *)pgm_read_word(&(data_table[entrynumber])));
}

// Gets the detected directions as an input and performs the suitable actions depending on active profile, muted, etc.
void performCommand(int8_t dir1, int8_t dir2) {

  int8_t combinedDirs = dir1 * 10 + dir2; //combine two dirs into one variable

  if (muted) {
    if (combinedDirs == 2) {
      toggleMute();
      letEmicSpeak("Unmuted");
    }
  } else {

    if (inMenu) { //menu commands
      switch (combinedDirs) {
        
        case 0: // leave menu
          inMenu = false;
          letEmicSpeak("Closing Menu");
          break;

        case 1: // Volume Up
          raiseVolume();
          break;

        case 2: // Toggle Mute
          toggleMute();
          letEmicSpeak("Muted");
          break;

        case 3: // Volume Down
          lowerVolume();
          break;

        case 10:
          bufferTextdata(4);
          letEmicSpeak(textdataBuffer);
          break;

        case 11:
          bufferTextdata(5);
          letEmicSpeak(textdataBuffer);
          break;

        case 12:
          bufferTextdata(6);
          letEmicSpeak(textdataBuffer);
          break;

        case 13:
          bufferTextdata(7);
          letEmicSpeak(textdataBuffer);
          break;

        case 20:
          changeProfile(4);
          break;

        case 21:
          changeProfile(1);
          break;

        case 22:
          changeProfile(2);
          break;

        case 23:
          changeProfile(3);
          break;

        case 30: // down
          changeProfile(8);
          break;

        case 31:
          changeProfile(5);
          break;

        case 32:
          changeProfile(6);
          break;

        case 33:
          changeProfile(7);
          break;
      }
      
    } else { //profile commands
      switch (combinedDirs) {
        
        case 0:
          bufferTextdata(16 * activeProfile);
          letEmicSpeak(textdataBuffer);
          break;

        case 1:
          bufferTextdata(16 * activeProfile + 1);
          letEmicSpeak(textdataBuffer);
          break;

        case 2:
          bufferTextdata(16 * activeProfile + 2);
          letEmicSpeak(textdataBuffer);
          break;

        case 3:
          bufferTextdata(16 * activeProfile + 3);
          letEmicSpeak(textdataBuffer);
          break;

        case 10:
          bufferTextdata(16 * activeProfile + 4);
          letEmicSpeak(textdataBuffer);
          break;

        case 11:
          bufferTextdata(16 * activeProfile + 5);
          letEmicSpeak(textdataBuffer);
          break;

        case 12:
          bufferTextdata(16 * activeProfile + 6);
          letEmicSpeak(textdataBuffer);
          break;

        case 13:
          bufferTextdata(16 * activeProfile + 7);
          letEmicSpeak(textdataBuffer);
          break;

        case 20:
          bufferTextdata(16 * activeProfile + 8);
          letEmicSpeak(textdataBuffer);
          break;

        case 21:
          bufferTextdata(16 * activeProfile + 9);
          letEmicSpeak(textdataBuffer);
          break;

        case 22:
          bufferTextdata(16 * activeProfile + 10);
          letEmicSpeak(textdataBuffer);
          break;

        case 23:
          bufferTextdata(16 * activeProfile + 11);
          letEmicSpeak(textdataBuffer);
          break;

        case 30:
          bufferTextdata(16 * activeProfile + 12);
          letEmicSpeak(textdataBuffer);
          break;

        case 31:
          bufferTextdata(16 * activeProfile + 13);
          letEmicSpeak(textdataBuffer);
          break;

        case 32:
          bufferTextdata(16 * activeProfile + 14);
          letEmicSpeak(textdataBuffer);
          break;

        case 33:
          bufferTextdata(16 * activeProfile + 15);
          letEmicSpeak(textdataBuffer);
          break;

        case 40:
          inMenu = true;
          letEmicSpeak("Menu");
          break;
      }
    }
  }
}

// processes a recognized (or no recognized) eye detection direction and fires the according events depending on status, singalTracker and timeoutTracker
void processDirection(int8_t recognizedDir) {
  if (status == 0) {
    
    if (recognizedDir == -1) {
      dir1 = -1;
      
    } else if (recognizedDir != dir1) {
      dir1 = recognizedDir;
      signalTracker = millis();
      
    } else if (dir1 == 4) {
      if (millis() -  signalTracker > closedDuration) {
        dir1 = -1;
        performCommand(4, 0);
      }
    }
    
    else if (millis() -  signalTracker > dirDuration) {
      status = 1;
      timeoutTracker = millis();
      if (!muted) {
        letStatusLEDBlink();
      }
    }
    
  } else if (status == 1) {
    
    if (millis() - timeoutTracker > dirPause) {
      status = 2;
      timeoutTracker = millis();
      processDirection(recognizedDir);
    }
    
  } else if (status == 2) {

    if (millis() - timeoutTracker > timeOutDuration) {
      status = 0;
      dir1 = -1;
      dir2 = -1;
      processDirection(recognizedDir);
      
    } else if (recognizedDir == -1) {
      dir2 = -1;
      
    } else if (recognizedDir != dir2) {
      dir2 = recognizedDir;
      signalTracker = millis();
      
    } else if (millis() -  signalTracker > dirDuration) {
      performCommand(dir1, dir2);
      status = 0;
      dir1 = -1;
      dir2 = -1;
      timeoutTracker = millis();
    }
  }
}

// uses the hardware to read a set of sensor values and assumes in which direction the user was looking
bool readSensors() {
  qtr.read(sensorValue);

  if (sensorValue[0] < neutralSensorValue[0] //if eyes closed
      && sensorValue[2] < neutralSensorValue[2]
      && sensorValue[1] < neutralSensorValue[1]
      && sensorValue[3] < neutralSensorValue[3]) {
    Serial.println("ReadSensors: Closed eyes");
    processDirection(4);
  } else if (sensorValue[3] < neutralSensorValue[3]) { //if looking up
    Serial.println("ReadSensors: U ");
    processDirection(1);
  } else if (sensorValue[1] < neutralSensorValue[1]) { //if looking down
    Serial.println("ReadSensors: D ");
    processDirection(3);
  } else if (sensorValue[0] < neutralSensorValue[0]) { //if looking right
    Serial.println("ReadSensors: R ");
    processDirection(2);
  } else if (sensorValue[2] < neutralSensorValue[2]) { //if looking left
    Serial.println("ReadSensors: L ");
    processDirection(0);
  } else {
    Serial.println("ReadSensors: X ");
    processDirection(-1);
  }

  return true;
}

void changeProfile(uint8_t profilenumber) {
  activeProfile = profilenumber;
  inMenu = false;
  bufferTextdata(7 + profilenumber);
  letEmicSpeak(textdataBuffer);
}

void raiseVolume() {
  if (volume < 5) {
    updateVolume(volume + 1);
    bufferTextdata(1);
    timer.in(150, [] { letEmicSpeak(textdataBuffer); });
  } else {
    letEmicSpeak("Maximal volume reached");
  }
}

void lowerVolume() {
  if (volume > 1) {
    updateVolume(volume - 1);
    bufferTextdata(3);
    timer.in(150, [] { letEmicSpeak(textdataBuffer); });
  } else {
    letEmicSpeak("Minimal volume reached");
  }
}

void updateVolume(int8_t newVolume) {
  if (emicReady) { //Should be tested at every emic output to be sure its ready to receive commands
    emicReady = false;
    volume = newVolume;
    emicSerial.print('V');
    switch (volume) {
      case 1: emicSerial.print("-40"); break;
      case 2: emicSerial.print("-20"); break;
      case 3: emicSerial.print("0"); break;
      case 4: emicSerial.print("10"); break;
      case 5: emicSerial.print("18"); break;
    }
    emicSerial.print('\n');;
  }
}

void letStatusLEDBlink() {
  if (statusLEDActive) {
    digitalWrite(StatusLED, HIGH);
    timer.in(200, [] { digitalWrite(StatusLED, LOW); });
  }
}

bool keepAlive() { //uses the keep alive wiring to consume some power so that powerbanks do not turn off
  digitalWrite(keepAlivePin, HIGH);
  timer.in(300, [] { digitalWrite(keepAlivePin, LOW); });
  return true;
}

bool printDebugInfo() {
  Serial.print(sensorValue[0]);
  Serial.print('\t');
  Serial.print(sensorValue[1]);
  Serial.print('\t');
  Serial.print(sensorValue[2]);
  Serial.print('\t');
  Serial.print(sensorValue[3]);
  Serial.print('\t');
  Serial.println();

  return true;
}

void loop()
{
  timer.tick();
}