#include <Arduino.h>
#include <vector>
using namespace std;

vector<int> setpassword = {6, 0, 6, 0};                                 // set your password here !!!vector<int> realpswd;                                                   // realpswd
int keyboardMap[4][3] = {{7, 8, 9}, {4, 5, 6}, {1, 2, 3}, {-6, 0, -9}}; // keyboardMap[i][j], it depend on you
// int keyboardOutput[3] = {21, 22, 23};           // defining gpio output pin
// int keyboardInput[4] = {33, 32, 35, 4};         // defining gpio input pin
/**
 * @brief detect which key been pressed
 * @note keyboard map
 *           0 1 2     j    i for raw, i<=4
 *       0 | 7 8 9 |        j for column, j<=3
 *       1 | 4 5 6 |        "C" is -6, which is clear
 *       2 | 1 2 3 |        "#" is -9, which is enter
 *       3 | C 0 # |
 *       i
 * @date 2023.6.20
 * @author weiyoudong，drinktoomuchsax
 * @version 0.0.8
 */
int whichKeyPress(int GPIO_output[3], int GPIO_input[4])
{
    for (int j = 0; j < 3; j++) // this is output loop
    {
        pinMode(GPIO_output[j], OUTPUT);
        digitalWrite(GPIO_output[j], HIGH);
        for (int i = 0; i < 4; i++) // this is input loop
        {
            pinMode(GPIO_input[i], INPUT);
            if (digitalRead(GPIO_input[i]) == 1)
            {
                // Serial.print(keyboardMap[i][j]);
                digitalWrite(GPIO_output[j], LOW);
                return keyboardMap[i][j];
            }
        }
        digitalWrite(GPIO_output[j], LOW);
    }
    // Serial.printf("no key been pressed\n");
    return -1; // return -1 for find nothing
}

/**
 * @brief find a way to wipe off the jitter of key pressing, like 6666666666666666666666666666-16-166-166-16-1-1-1-1-1-1-1-13-13-133-1333333333{more than 1k 3 in 1ms}333333333333-1333-13-13-1-1-1-1-1{more than 1k -1 in 1ms}-1-1-1-1-13-1-1-1-133-13333-13333333333333-1-1-1-1-1-1-9-9-9
 * this function devide the raw password into interval, counting the most one as real password. If you want to do some develop, don't try to understand this one, rewrite your onw to save time.
 * @date 2023.6.23
 * @author weiyoudong, drinktoomuchsax
 * @version 0.2.0
 */
int last;
bool gap = true;
int wipeJitter(vector<int> rawPSWD)
{
    vector<int> realPSWD;
    int checkLength = rawPSWD.size(); // this can be changed
    int number[5] = {-1, -1, -1, -1, -1};
    int num = 0;
    int countNum[5] = {0, 0, 0, 0, 0};
    for (int j = 0; j < checkLength; j++)
    {
        int equal = 0;
        for (int x = 0; x < 5; x++)
        {
            if (number[x] == rawPSWD[j])
            {
                countNum[x] = countNum[x] + 1;
                equal = 1;
                break;
            }
        }
        if (equal == 0)
        {
            number[num] = rawPSWD[j];
            countNum[num] = countNum[num] + 1;
            num = num + 1;
        }
    }
    for (int z = 0; z < 5; z++)
    {
        bool needToBeMost = (countNum[z] > 0.9 * checkLength) && (number[z] >= 0) && (gap == true);
        bool dontRepeatSameOne = (countNum[z] > 0.9 * checkLength) && (number[z] >= 0) && (number[z] != last);
        if (needToBeMost || dontRepeatSameOne)
        {
            last = number[z];
            gap = false;
            return number[z]; // coefficient1 can be changed
        }
        bool areYouGap = (countNum[z] > checkLength * 0.5) && (number[z] == -1) && (gap == false);
        if (areYouGap)
            gap = true;
    }
    return -1;
}

/**
 * @brief passwords verifing, using "xxx" represent wahtever key, the real password is hiden in the key.
 *        so, the actual password woulb be defined as xxx20231xxxxxxxxx , "x" is whatever number except "#", "2023" is the real password and following 1 is user identify bit to record who open the door.
 *        The user could keep typing and typing but once them press "enter", the password would be verifyed.
 * @date 2023.6.20
 * @author drinktoomuchsax
 * @version 0.0.3
 */
bool verifyPSWD(vector<int> PSWD)
{
    if (PSWD.size() >= 6)
    {
        bool one, two, three, four;
        one = (setpassword[0] == PSWD[3]);
        two = (setpassword[1] == PSWD[4]);
        three = (setpassword[2] == PSWD[5]);
        four = (setpassword[3] == PSWD[6]);
        if (one && two && three && four) // all four bit is right, the
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}
