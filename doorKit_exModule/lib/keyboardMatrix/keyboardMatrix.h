#include <vector>
#include <Arduino.h>
int whichKeyPress(int GPIO_output[3], int GPIO_input[4]);
string verifyPSWD(vector<int> PSWD);
int wipeJitter(vector<int> rawPSWD);
