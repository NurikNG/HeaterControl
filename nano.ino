#define button1 2
#define button2 6
#define button3 4
#define button4 8

#define input1 3
#define input2 7
#define input3 5
#define input4 9

#define CLICK_DELAY 500

byte data[32];

byte numBytes;
#define MAGIC_COMMAND 0x55
#define MAGIC_COMMAND2 0x65

byte counter = 0;
bool was5Sec = 0;
bool checkCounter = true;

#define MAX_COUNTER 50

byte setButton;
byte needClickButton = 0;

byte needSendInput = 0;
byte sendInputValue = 0;

byte clicks;
byte needClickButtonCount;

void setup() {
  pinMode(button1, OUTPUT);
  digitalWrite(button1,LOW);
  pinMode(button2, OUTPUT);
  digitalWrite(button2,LOW);
  pinMode(button3, OUTPUT);
  digitalWrite(button3,LOW);
  pinMode(button4, OUTPUT);
  digitalWrite(button4,LOW);

  pinMode(input1, INPUT);
  pinMode(input2, INPUT);
  pinMode(input3, INPUT);
  pinMode(input4, INPUT);

  Serial.begin(9600);
}

void clickButton(char button){
  digitalWrite(button, HIGH);  
  delay(CLICK_DELAY);                      
  digitalWrite(button, LOW);  
  checkCounter = true;
  if (was5Sec){
    was5Sec = false;
    delay(CLICK_DELAY);
    clickButton(button);
  }
}
/*
void sendInput(byte input){
  if (was5Sec){
    was5Sec = false;
    return;
  }
  needSendInput = 1;
  sendInputValue = input;
}

void checkInputs(){
  if (digitalRead(input1) == LOW){
    sendInput(1);
  }
  if (digitalRead(input2) == LOW){
    sendInput(2);
  }
  if (digitalRead(input3) == LOW){
    sendInput(3);
  }
  if (digitalRead(input4) == LOW){
    sendInput(4);
  }
}*/

void loop() {
  //checkInputs();
  if (needClickButton){
    needClickButton = 0;
    if (setButton == 1)
      clickButton(button1);
    if (setButton == 2)
      clickButton(button2);
    if (setButton == 3)
      clickButton(button3);
    if (setButton == 4)
      clickButton(button4);
  }
  if (needClickButtonCount){
    needClickButton = 1;
    clicks--;
    if (clicks == 0){
      needClickButtonCount = 0;
    }
  }
  delay(100);
  numBytes = Serial.available();
  if (numBytes >= 2) {
    byte index;
    for (index = 0; index < numBytes; index++) {
        data[index] = Serial.read();
    }
   /* for (int i = 0 ; i < index; i++)
      Serial.println(data[i]);*/
    if (index == 2) {
      if (data[0] == MAGIC_COMMAND) {
        setButton = data[1];
        needClickButton = 1;
        checkCounter = false;
        counter = 0;
      }
    }
    if (index == 3) {
      if (data[0] == MAGIC_COMMAND2) {
        setButton = data[1];
        clicks = data[2];
        if (clicks == 0)
          needClickButtonCount = 0;
        else
          needClickButtonCount = 1;
        checkCounter = false;
        counter = 0;
      }
    }
  }

  if (checkCounter) {
    if (counter > MAX_COUNTER){
      counter = 0;
      was5Sec = true;
    } else {
      counter++;
    }
  }
/*
  if (needSendInput){

    needSendInput = 0;
    Serial.write(MAGIC_COMMAND);
    Serial.write(sendInputValue);
  }*/
}
