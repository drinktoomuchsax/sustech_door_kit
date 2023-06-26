#include <Arduino.h>
#include <vector>
using std::vector;

vector<int> pswd;                                                       // initialize a password vector(array) to store the key user press
int keyboardMap[4][3] = {{7, 8, 9}, {4, 5, 6}, {1, 2, 3}, {-6, 0, -9}}; // keyboardMap[i][j], it depend on you
int keyboardOutput[3] = {1, 1, 1};                                      // defining gpio output pin
int keyboardInput[4] = {2, 2, 2, 2};                                    // defining gpio input pin
hw_timer_t *timer = NULL;                                               // timer for password input timeout
String STATE;
HardwareSerial SerialPort(2); // using UART2

/**
 * @brief detect which key been pressed
 * @note keyboard map
 *           0 1 2     j    i for raw, i<=4
 *       0 | 7 8 9 |        j for column, j<=3
 *       1 | 4 5 6 |        "*" is -6
 *       2 | 1 2 3 |        "#" is -9
 *       3 | * 0 # |
 *       i
 * @date 2023.6.20
 * @author weiyoudong，drinktoomuchsax
 * @version 0.0.2
 */
int whichKeyPress(int GPIO_output[3], int GPIO_input[4])
{
    for (int j = 0; j < 3; j++) // this is output loop
    {
        pinMode(GPIO_output[j], HIGH);
        for (int i = 0; i < 4; i++) // this is input loop
        {
            pinMode(GPIO_input[j], INPUT);
            if (GPIO_input[j] == HIGH)
            {
                return keyboardMap[i][j];
            }
            else
                return -1; // return -1 for find nothing
        }
        pinMode(GPIO_output[j], LOW);
    }
    return -1; // return -1 for find nothing
}

/**
 * @brief save the press key into a longlong int vector
  // the pswd here would be like -1-1-1-1-1222-1-1-1-1-1-1-1-100-100-100000-1-1-1-1-1-1-12-12222-1-1-1-1-1-133-1-13333
 * @date 2023.6.21, happy birthday drinktoomuchsax, wish myself a happy life :)
 * @author drinktoomuchsax,weiyoudong
 * @version 0.0.1
 */

vector<int> saveKeyToPswd(int GPIO_output[3], int GPIO_input[4], vector<int> PSWD) // capital locked PSWD for function
{
    PSWD.push_back(whichKeyPress(GPIO_output, GPIO_input));
    // the pswd here would be like -1-1-1-1-1222-1-1-1-1-1-1-1-100-100-100000-1-1-1-1-1-1-12-12222-1-1-1-1-1-133-1-13333
    return PSWD;
}

/**
 * @brief find a way to wipe off the jitter of key pressing
 * @date 2023.6.22
 * @author drinktoomuchsax
 * @version 0.0.1
 */
vector<int> wipeJitter(vector<int> rawPSWD)
{
    int lastSavedKey = -1;
    int nowSaveKey;
    vector<int> realPSWD;
    for (int i = 0; i < sizeof(rawPSWD); i++)
    {
        if (rawPSWD[i] == -1)
        {
        }
    }
}

/**
 * @brief passwords verifing, using "xxx" represent wahtever key, the real password is hiden in the key.
 *        so, the actual password woulb be defined as xxx20231xxxxxxxxx , "x" is whatever number except "#", "2023" is the real password and following 1 is user identify bit to record who open the door.
 *        The user could keep typing and typing but once them press "enter", the password would be verifyed.
 * @date 2023.6.20
 * @author drinktoomuchsax
 * @version 0.0.1
 */
bool verifyPSWD(vector<int> PSWD)
{
    int setpassword[4] = {6,
                          0,
                          1,
                          1}; // set your password here !!!
    int realPSWD[8];
    for (int i = 0; i < 8; i++)
    {
        realPSWD[i] = PSWD[i];
    }
    bool one, two, three, four;
    one = (setpassword[0] == realPSWD[3]);
    two = (setpassword[1] == realPSWD[4]);
    three = (setpassword[2] == realPSWD[5]);
    four = (setpassword[3] == realPSWD[6]);
    if (one && two && three && four) // all four bit is right, the
    {
        return true;
    }
    else
        return false;
}

void keyboardEvent()
{
}
void ARDUINO_ISR_ATTR resetSTATE()
{
    Serial.print("time out");
}

void setup()
{

    Serial.begin(115200);
    SerialPort.begin(57600, SERIAL_8N2, 16, 17);
    hw_timer_t *timer = timerBegin(0, 80, true); // timer 0, div 80ms, increasing count
    timerAttachInterrupt(timer, &resetSTATE, true);
    timerAlarmWrite(timer, 1000 * 1000 * 10, false); // set time in us
}

void loop()
{
    STATE = "standby";
    if (whichKeyPress(keyboardOutput, keyboardInput) != -1)
    {
        STATE = "checkPSWD";
        // keyboard password part, detail look on productdesign.excalidraw
        timerWrite(timer, 0);
        timerAlarmEnable(timer); // enable interrupt
        while (STATE == "checkPSWD")
        {
            saveKeyToPswd(keyboardOutput, keyboardInput, pswd);
            if (whichKeyPress(keyboardOutput, keyboardInput) == -9)
            {
                STATE = "verifyPSWD_wipeJitter";
            }
        }
        if (verifyPSWD(wipeJitter(pswd))) // send match to inMoudle
        {
            Serial.print("well, what you input is right.");
        }
        else // sned fail to nModule
        {
            Serial.print("no!!!!!!!!");
        };
    }
}