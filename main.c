#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "BluetoothSerial.h"

#define LOG(fmt, ...)  Serial.printf("[%lu ms] " fmt "\n", millis(), ##__VA_ARGS__)

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth not enabled! Please enable it in menuconfig.
#endif

BluetoothSerial SerialBT;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SOUND_SENSOR_PIN 34

struct Question {
  String question;
  String options[4];
  String answer;
};

// ---------------------------
// QUESTIONS
// ---------------------------
Question allQuestions[] = {
  {"What is 5 + 3?", {"7", "8", "9", "10"}, "8"},
  {"What is 10 - 4?", {"5", "6", "7", "8"}, "6"},
  {"What is 3 x 3?", {"6", "9", "12", "15"}, "9"},
  {"What is 8 / 2?", {"2", "3", "4", "5"}, "4"},
  {"How many sides does a triangle have?", {"2", "3", "4", "5"}, "3"},
  {"How many sides does a square have?", {"3", "4", "5", "6"}, "4"},
  {"What number comes after 19?", {"18", "19", "20", "21"}, "20"},
  {"How many minutes are in one hour?", {"30", "50", "60", "100"}, "60"},
  {"What shape is a ball?", {"Square", "Circle", "Sphere", "Triangle"}, "Sphere"},
  {"How many items are in a 'dozen'?", {"6", "10", "12", "20"}, "12"},
  {"What is 10 + 10 + 10?", {"20", "30", "40", "100"}, "30"},
  {"If you have 5 apples and give 2 away, how many are left?", {"1", "2", "3", "4"}, "3"},
  {"What do plants need to grow?", {"Sunlight", "Milk", "Rocks", "Candy"}, "Sunlight"},
  {"What gas do we breathe to live?", {"Oxygen", "Carbon Dioxide", "Helium", "Smoke"}, "Oxygen"},
  {"What is H2O?", {"Fire", "Air", "Water", "Soil"}, "Water"},
  {"What planet do we live on?", {"Mars", "Venus", "Earth", "Jupiter"}, "Earth"},
  {"What do bees make?", {"Honey", "Silk", "Wool", "Juice"}, "Honey"},
  {"What is a baby dog called?", {"Kitten", "Puppy", "Cub", "Fawn"}, "Puppy"},
  {"What is a baby cat called?", {"Puppy", "Kitten", "Chick", "Lamb"}, "Kitten"},
  {"Where do fish live?", {"In the sky", "In water", "In trees", "In the desert"}, "In water"},
  {"What pulls things down to the ground?", {"Magnets", "Wind", "Gravity", "Light"}, "Gravity"},
  {"What season comes after winter?", {"Summer", "Spring", "Autumn", "Rainy"}, "Spring"},
  {"What is the center of our solar system?", {"The Moon", "The Earth", "The Sun", "Mars"}, "The Sun"},
  {"What happens to water when it freezes?", {"It turns to steam", "It turns to ice", "It disappears", "It turns to fire"}, "It turns to ice"},
  {"What animal is big and gray with a long trunk?", {"Lion", "Elephant", "Zebra", "Giraffe"}, "Elephant"},
  {"Which of these is a mammal?", {"Shark", "Whale", "Frog", "Snake"}, "Whale"},
  {"What part of a plant soaks up water from the soil?", {"Leaves", "Flower", "Stem", "Roots"}, "Roots"},
  {"How many colors are in a rainbow?", {"3", "5", "7", "10"}, "7"},
  {"What is the capital of France?", {"London", "Rome", "Berlin", "Paris"}, "Paris"},
  {"How many days are in a week?", {"5", "6", "7", "8"}, "7"},
  {"What month comes after December?", {"November", "February", "January", "March"}, "January"},
  {"How many continents are there?", {"4", "5", "6", "7"}, "7"},
  {"What is the first letter of the alphabet?", {"A", "B", "C", "Z"}, "A"},
  {"What color is a school bus (usually)?", {"Red", "Blue", "Green", "Yellow"}, "Yellow"},
  {"What holiday is on December 25th?", {"Halloween", "Easter", "Christmas", "Thanksgiving"}, "Christmas"},
  {"What do you use to write on a blackboard?", {"Pen", "Crayon", "Chalk", "Marker"}, "Chalk"},
  {"What is the opposite of 'hot'?", {"Warm", "Cold", "Spicy", "Fast"}, "Cold"},
  {"What is the opposite of 'big'?", {"Small", "Tall", "Wide", "Heavy"}, "Small"},
  {"What sound does a cow make?", {"Oink", "Meow", "Bark", "Moo"}, "Moo"},
  {"What do you use to see things far away?", {"Microscope", "Telescope", "Magnifying glass", "Goggles"}, "Telescope"},
  {"Which of these is a fruit?", {"Carrot", "Broccoli", "Apple", "Potato"}, "Apple"},
  {"Which of these is a vegetable?", {"Banana", "Orange", "Grape", "Carrot"}, "Carrot"},
  {"What shape is a stop sign?", {"Circle", "Triangle", "Square", "Octagon"}, "Octagon"},
  {"What do you use to measure temperature?", {"Ruler", "Clock", "Thermometer", "Scale"}, "Thermometer"},
  {"What is the color of the sky on a clear day?", {"Green", "Red", "Blue", "Black"}, "Blue"},
  {"How many hours are in a day?", {"12", "24", "30", "60"}, "24"},
  {"What animal is known as the 'King of the Jungle'?", {"Tiger", "Lion", "Bear", "Wolf"}, "Lion"},
  {"Which of these can fly?", {"Dog", "Pig", "Bird", "Fish"}, "Bird"},
  {"How many letters are in the English alphabet?", {"20", "24", "26", "30"}, "26"}
};

int totalQuestionsAvailable = sizeof(allQuestions) / sizeof(allQuestions[0]);

Question quizQuestions[50];
int quizSize = 0;
int currentQuestionIndex = 0;
int score = 0;
bool quizStarted = false;
bool quizInProgress = false;

// ---------------------------
// NORMALIZE NUMBER
// ---------------------------
int normalizeNumber(String input) {
  input.trim();
  input.toLowerCase();

  input.replace("question", "");
  input.replace("questions", "");
  input.replace("i want", "");
  input.replace("give me", "");
  input.replace("please", "");
  input.trim();

  if (input.indexOf("one") >= 0 || input.indexOf("1") >= 0) return 1;
  if (input.indexOf("two") >= 0 || input.indexOf("2") >= 0) return 2;
  if (input.indexOf("three") >= 0 || input.indexOf("3") >= 0) return 3;
  if (input.indexOf("four") >= 0 || input.indexOf("4") >= 0) return 4;
  if (input.indexOf("five") >= 0 || input.indexOf("5") >= 0) return 5;
  if (input.indexOf("six") >= 0 || input.indexOf("6") >= 0) return 6;
  if (input.indexOf("seven") >= 0 || input.indexOf("7") >= 0) return 7;
  if (input.indexOf("eight") >= 0 || input.indexOf("8") >= 0) return 8;
  if (input.indexOf("nine") >= 0 || input.indexOf("9") >= 0) return 9;
  if (input.indexOf("ten") >= 0 || input.indexOf("10") >= 0) return 10;
  if (input.indexOf("eleven") >= 0 || input.indexOf("11") >= 0) return 11;
  if (input.indexOf("twelve") >= 0 || input.indexOf("12") >= 0) return 12;
  if (input.indexOf("thirteen") >= 0 || input.indexOf("13") >= 0) return 13;
  if (input.indexOf("fourteen") >= 0 || input.indexOf("14") >= 0) return 14;
  if (input.indexOf("fifteen") >= 0 || input.indexOf("15") >= 0) return 15;
  if (input.indexOf("sixteen") >= 0 || input.indexOf("16") >= 0) return 16;
  if (input.indexOf("seventeen") >= 0 || input.indexOf("17") >= 0) return 17;
  if (input.indexOf("eighteen") >= 0 || input.indexOf("18") >= 0) return 18;
  if (input.indexOf("nineteen") >= 0 || input.indexOf("19") >= 0) return 19;
  if (input.indexOf("twenty") >= 0 || input.indexOf("20") >= 0) return 20;

  return input.toInt();
}

// ---------------------------
// SETUP
// ---------------------------
void setup() {
  Serial.begin(115200);
  LOG("Booting Edu_Bot...");

  SerialBT.begin("Edu_Bot");
  LOG("Bluetooth initialized as 'Edu_Bot'");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    LOG("OLED not found!");
    for (;;) delay(1000);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println("Edu_Bot");
  display.setCursor(10, 40);
  display.println("Say 'Start Quiz'");
  display.display();
}

// ---------------------------
// MAIN LOOP
// ---------------------------
void loop() {
  detectBluetoothVoice();
  delay(10);
}

// ---------------------------
// VOICE HANDLER
// ---------------------------
void detectBluetoothVoice() {
  if (SerialBT.available()) {
    String voiceCmd = SerialBT.readString();
    voiceCmd.trim();
    voiceCmd.toLowerCase();
    LOG("Received: '%s'", voiceCmd.c_str());

    if (voiceCmd.indexOf("exit") >= 0) {
      exitQuiz();
      return;
    }

    if (!quizStarted) {
      if (voiceCmd.indexOf("start") >= 0) {
        display.clearDisplay();
        display.setCursor(0, 20);
        display.println("How many Qs? (1-50)");
        display.display();
        quizStarted = true;
        return;
      }
    } else if (quizStarted && !quizInProgress) {
      int num = normalizeNumber(voiceCmd);
      if (num >= 1 && num <= 50) {
        quizSize = num;
        initializeQuiz();
        quizInProgress = true;
        showQuestion();
      } else {
        display.clearDisplay();
        display.setCursor(0, 20);
        display.println("Invalid size!");
        display.display();
        delay(1000);
        display.clearDisplay();
        display.setCursor(0, 20);
        display.println("How many Qs? (1-50)");
        display.display();
      }
    } else if (quizInProgress) {
      handleAnswer(voiceCmd);
    }
  }
}

// ---------------------------
// INITIALIZE QUIZ
// ---------------------------
void initializeQuiz() {
  for (int i = totalQuestionsAvailable - 1; i > 0; i--) {
    int j = random(i + 1);
    Question temp = allQuestions[i];
    allQuestions[i] = allQuestions[j];
    allQuestions[j] = temp;
  }
  for (int i = 0; i < quizSize; i++) quizQuestions[i] = allQuestions[i];

  currentQuestionIndex = 0;
  score = 0;
}

// ---------------------------
// SHOW QUESTION
// ---------------------------
void showQuestion() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(quizQuestions[currentQuestionIndex].question);

  for (int i = 0; i < 4; i++) {
    display.setCursor(0, 16 + (i * 10));
    display.printf("%c) %s", 'A' + i, quizQuestions[currentQuestionIndex].options[i].c_str());
  }
  display.display();
}

// ---------------------------
// HANDLE ANSWER
// ---------------------------
void handleAnswer(String ans) {
  ans.trim();
  ans.toLowerCase();

  String userAnswer = "";
  if (ans.indexOf("a") >= 0) userAnswer = quizQuestions[currentQuestionIndex].options[0];
  else if (ans.indexOf("b") >= 0) userAnswer = quizQuestions[currentQuestionIndex].options[1];
  else if (ans.indexOf("c") >= 0) userAnswer = quizQuestions[currentQuestionIndex].options[2];
  else if (ans.indexOf("d") >= 0) userAnswer = quizQuestions[currentQuestionIndex].options[3];

  if (userAnswer != "") {
    if (userAnswer == quizQuestions[currentQuestionIndex].answer) score++;
    currentQuestionIndex++;
    if (currentQuestionIndex >= quizSize) showScore();
    else showQuestion();
  }
}

// ---------------------------
// SHOW SCORE
// ---------------------------
void showScore() {
  display.clearDisplay();
  display.setCursor(10, 20);
  display.printf("Score: %d/%d", score, quizSize);
  display.setCursor(10, 40);
  display.println("Quiz Complete!");
  display.display();

  delay(2000);

  display.clearDisplay();
  display.setCursor(10, 20);
  display.println("Say 'exit' to quit 😊");
  display.setCursor(10, 35);
  display.println("or 'start quiz' again!");
  display.display();

  quizStarted = false;
  quizInProgress = false;
}

// ---------------------------
// EXIT QUIZ (Modified)
// ---------------------------
void exitQuiz() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.println("Thank You");
  display.display();

  quizStarted = false;
  quizInProgress = false;
  LOG("User ended session. Displayed 'Thank You!'");
}
