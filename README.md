# EDU_BOT

A small ESP32-based Bluetooth quiz app that displays questions on an SSD1306 OLED and accepts answers via Bluetooth text/voice input.

## Files
- [main.c](main.c)
- [LICENSE](LICENSE)
- [.gitignore](.gitignore)

## Quick start
1. Pair with Bluetooth device named "Edu_Bot".
2. Send "Start Quiz".
3. When asked "How many Qs? (1-50)", reply with a number (e.g., `5` or `five`).
4. Answer each question by sending `A`, `B`, `C`, or `D`.
5. Send `exit` to quit.

## Dependencies
- Adafruit GFX
- Adafruit SSD1306
- BluetoothSerial (ESP32 core)

## Hardware components
- ESP32 DevKitC (ESP32 board with Bluetooth enabled)
- SSD1306 128x64 OLED display (I2C, address 0x3C)
- Jumper wires (male-to-female / male-to-male as required)
- Power supply (5V USB power bank or USB adapter for ESP32)
- Extension / breadboard (for easier wiring and mounting)
- Optional: sound sensor (connected to analog pin 34 in code as SOUND_SENSOR_PIN)

## Workflow

1. Power & initialize
   - User plugs in ESP32. `setup()` initializes Serial, Bluetooth (named "Edu_Bot"), and the SSD1306 OLED.
   - OLED shows welcome text prompting "Say 'Start Quiz'".

2. Start quiz (waiting for user)
   - `loop()` continuously calls `detectBluetoothVoice()` which reads incoming Bluetooth text.
   - When a Bluetooth message containing "start" is received and no quiz is active, the device prompts: "How many Qs? (1-50)" and sets `quizStarted = true`.

3. Choose quiz size
   - User replies with a number (digit or word). `normalizeNumber()` parses the text and returns an integer 1–50.
   - If valid, `initializeQuiz()` is called:
     - Shuffles the `allQuestions` pool (Fisher–Yates style).
     - Copies the first N questions into `quizQuestions`.
     - Resets `currentQuestionIndex` and `score`.
   - `quizInProgress` is set to true and `showQuestion()` displays the first question and options on the OLED.

4. Answering questions
   - For each incoming Bluetooth message while `quizInProgress == true`, `handleAnswer()` runs:
     - Parses A/B/C/D (case-insensitive and substring match).
     - Maps the letter to the corresponding option string.
     - Compares with the correct `answer` and increments `score` if correct.
     - Increments `currentQuestionIndex`. If more questions remain, `showQuestion()` displays next question; otherwise `showScore()` is called.

5. Completion & exit
   - `showScore()` displays final score and a completion message, then prompts the user to "Say 'exit' to quit" or "start quiz" again.
   - User can send "exit" at any time; `exitQuiz()` clears the state, shows "Thank You" and logs the exit.

## Commands / Examples
- "Start Quiz" — begin quiz setup.
- "Five" or "5" — choose 5 questions.
- "A" / "B" / "C" / "D" — answer current question.
- "Exit" — abort or finish and return to welcome screen.

## Internal mapping (where to look in code)
- Command parsing: `detectBluetoothVoice()` and `normalizeNumber()`
- Quiz lifecycle: `initializeQuiz()`, `showQuestion()`, `handleAnswer()`, `showScore()`, `exitQuiz()`
- Display handling: Adafruit_SSD1306 calls within `showQuestion()` and others

## Troubleshooting
- "Bluetooth not enabled!" build error: enable Bluetooth in your ESP32 environment or use a core that provides BluetoothSerial.
- OLED blank: check I2C wiring, correct 0x3C address, and power. Serial will log "OLED not found!" on failure.
- Incorrect question count: ensure the number is between 1 and 50. The normalizer supports common words ("one"..."twenty") and digits.

## License
MIT — see [LICENSE](LICENSE)