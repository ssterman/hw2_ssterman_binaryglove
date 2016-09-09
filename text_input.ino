#if defined(ARDUINO) 
SYSTEM_MODE(SEMI_AUTOMATIC); 
#endif


//the alphabet in an array 
char alphabet[26] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

//start and end indices of the alphabet array
//if you choose to add more characters, these must change
int alphaStart = 0;
int alphaEnd = 25;

//maintains the current range of the binary search
int curStart = alphaStart;
int curEnd = alphaEnd;

//which pins on the controller are used
static int leftPin = D0;
static int rightPin = D4;

//control flags:
//indicates end of binary search
bool foundEnd = false;
//write special character (space) 
//rather than searched character
bool space = false;

//remember the last state of the pin
//to determine if state has changed
int lastLeft = -1;
int lastRight = -1;

//the user-generated text
String output = "";

void setup() {
  Serial.begin(9600);
  pinMode(leftPin, INPUT);
  pinMode(rightPin, INPUT);
}

void loop() {
  //listen for voltage on a pin
  int left = digitalRead(leftPin);
  int right = digitalRead(rightPin);

  //find the centerpoint of the current range
  int half =  halve();

  //only do character recognition if state has changed
  //to avoid artifacts from slow user response
  if (changedState(left, right)) {
    if (left == HIGH && right == HIGH) {
       if (curStart == alphaStart && curEnd == alphaEnd) {
          //if both are pressed and a space is the last character, delete two characters. 
          if (output.charAt(output.length() - 1) == ' ') {
            output = output.remove(output.length() -1);
            output = output.remove(output.length() -1);
            writeCurOutput();
          } //if both are pressed and 26 letters available, write a space
          else {
            curStart = curEnd;
            space = true;
          }
       } // both are pressed and 1 < x < 26 letters available, escape to initial range
       else {
         curStart = alphaStart;
         curEnd = alphaEnd;
       }
       
       updateKeyboardView();
     } else if (left == HIGH) {
        //move binary search to lower half of range
        if (half == -1) {
          curEnd = curStart;
        } else {
          curEnd = curStart + half - 1;
        }
        updateKeyboardView();
     } else if (right == HIGH) {
        //move binary search to upper half of range
        if (half == -1) {
          curStart = curEnd;
        } else {
          curStart = curStart + half;
        }
        updateKeyboardView();
     }
   }

   //update state
   lastLeft = left;
   lastRight = right;
   
   //limit responsiveness
   delay(100);
}


/*
 * Determine if state of switches has changed since last read
 * If not, then the user has not added new input
 */
bool changedState(int left, int right) {
  if (lastLeft == left && lastRight == right) {
    return false;
  }
  return true;
}

/*
 * Find the centerpoint of the range
 * Extra character goes to the right
 */
int halve() {
  //if we have the last character, return -1
  if (foundEnd) {
    foundEnd = false;
    return -1;
  } 
  //if we have two characters to choose from, 
  // the next round will be the end
  if (curEnd == curStart + 1) {
    foundEnd = true;
  }

  return (curEnd - curStart + 1) / 2;
}

void updateKeyboardView() {
  //we have chosen a character:
  if (curStart == curEnd) {
      //reset flag
      foundEnd = false;

      //identify chosen character
      char newChar = alphabet[curStart];
      if (space) {
        newChar = ' ';
        space = false;
      }

      //update output and reset range
      output = output + newChar;
      curStart = alphaStart;
      curEnd = alphaEnd;

      writeCurOutput();
  } 
  writeCurView();
}


/*
 * Display user generated text
 */
void writeCurOutput() {
    Serial.print("****");
    Serial.print(output);
    Serial.print("****");
    Serial.write("\n");  
}

/*
 * Display current binary search state
 */
void writeCurView() {
  int half = halve();
  int len = curEnd - curStart + 1;
  char curView[len];
  for (int i = 0; i < half; i++) {
    Serial.print(alphabet[curStart + i]);
  }
  Serial.print("*|*");
  for (int i = half; i < len; i++) {
    Serial.print(alphabet[curStart + i]);
  }
  Serial.write("\n");
}
