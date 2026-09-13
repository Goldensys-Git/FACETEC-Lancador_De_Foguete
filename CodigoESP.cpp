// PINOS

const byte LED_1 = 13;
const byte LED_2 = 9;

const byte BUTTON_START = 7;
const byte BUTTON_EMERGENCY = 6;

// RELÉS

const byte RELAY_LAMP_1 = 10;
const byte RELAY_LAMP_2 = 11;

// SENSOR DE PRESSÃO / POTENCIÔMETRO

const byte PRESSURE_SENSOR = A0;


// RELÉS ATIVOS EM LOW

const byte RELAY_ON = LOW;
const byte RELAY_OFF = HIGH;

// LIMITE DE PRESSÃO
// 0    -> pressão mínima
// 1023 -> pressão máxima

const int PRESSURE_LIMIT = 800;

// ESTADOS

const byte STATE_SAFE = 0;
const byte STATE_COUNTDOWN = 1;
const byte STATE_READY = 2;
const byte STATE_ABORTED = 3;

byte currentState = STATE_SAFE;

// CONTAGEM

const byte COUNTDOWN_INITIAL = 10;

const unsigned long ONE_SECOND = 1000UL;
const unsigned long DEBOUNCE_TIME = 50UL;

byte countdown = COUNTDOWN_INITIAL;

unsigned long countdownTimer = 0;

// EMERGENCY

bool emergencyLatched = false;

// BOTÃO START

bool startRawState = HIGH;
bool startStableState = HIGH;

unsigned long startChangeTime = 0;

// BOTÃO EMERGENCY

bool emergencyRawState = HIGH;
bool emergencyStableState = HIGH;

unsigned long emergencyChangeTime = 0;

// CONTROLE DAS LÂMPADAS

void lampsOff()
{
  digitalWrite(RELAY_LAMP_1, RELAY_OFF);
  digitalWrite(RELAY_LAMP_2, RELAY_OFF);
}

void lamp1On()
{
  digitalWrite(RELAY_LAMP_2, RELAY_OFF);
  digitalWrite(RELAY_LAMP_1, RELAY_ON);
}

void lamp2On()
{
  digitalWrite(RELAY_LAMP_1, RELAY_OFF);
  digitalWrite(RELAY_LAMP_2, RELAY_ON);
}

// LEDs

void ledsOff()
{
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, LOW);
}

void led1On()
{
  digitalWrite(LED_1, HIGH);
  digitalWrite(LED_2, LOW);
}

void led2On()
{
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, HIGH);
}

// NOME DO ESTADO

const char* stateName(byte state)
{
  if (state == STATE_SAFE)
    return "SAFE";

  if (state == STATE_COUNTDOWN)
    return "COUNTDOWN";

  if (state == STATE_READY)
    return "READY";

  if (state == STATE_ABORTED)
    return "ABORTED";

  return "UNKNOWN";
}

// MUDANÇA DE ESTADO

void changeState(byte newState)
{
  currentState = newState;
 
  // SAFE

  if (currentState == STATE_SAFE)
  {
    countdown = COUNTDOWN_INITIAL;
    countdownTimer = 0;

    ledsOff();
    lampsOff();

    Serial.println();
    Serial.println(F("=============================="));
    Serial.println(F("SYSTEM STATE: SAFE"));
    Serial.println(F("LAMPS: OFF"));
    Serial.println(F("Waiting for START."));
    Serial.println(F("=============================="));
  }

  // COUNTDOWN

  else if (currentState == STATE_COUNTDOWN)
  {
    countdown = COUNTDOWN_INITIAL;
    countdownTimer = millis();

    lampsOff();
    led1On();

    Serial.println();
    Serial.println(F("=============================="));
    Serial.println(F("COUNTDOWN STARTED"));
    Serial.println(F("COUNT: 10"));
    Serial.println(F("LAMP 1: OFF"));
    Serial.println(F("LAMP 2: OFF"));
    Serial.println(F("=============================="));
  }

  // ----------------------------------------------------------
  // READY
  // ----------------------------------------------------------

  else if (currentState == STATE_READY)
  {
    ledsOff();

    lamp1On();

    Serial.println();
    Serial.println(F("=============================="));
    Serial.println(F("COUNT: 0"));
    Serial.println(F("COUNTDOWN COMPLETE"));
    Serial.println(F("SYSTEM STATE: READY"));
    Serial.println(F("LAMP 1: ON"));
    Serial.println(F("LAMP 2: OFF"));
    Serial.println(F("=============================="));

    Serial.println();
    Serial.println(F("Ao infinito e alem!"));
    Serial.println();
  }

  // ABORTED

  else if (currentState == STATE_ABORTED)
  {
    ledsOff();

    lamp2On();

    Serial.println();
    Serial.println(F("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
    Serial.println(F("       EMERGENCY ABORT"));
    Serial.println(F("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
    Serial.println(F("COUNTDOWN CANCELLED."));
    Serial.println(F("LAMP 1: OFF"));
    Serial.println(F("LAMP 2: ON"));
    Serial.println(F("SYSTEM LOCKED."));
    Serial.println(F("Use RESET."));
    Serial.println(F("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
  }
}

// BOTÃO START

bool startPressed()
{
  bool reading = digitalRead(BUTTON_START);

  if (reading != startRawState)
  {
    startRawState = reading;
    startChangeTime = millis();
  }

  if ((unsigned long)(millis() - startChangeTime) >= DEBOUNCE_TIME)
  {
    if (startStableState != startRawState)
    {
      startStableState = startRawState;

      if (startStableState == LOW)
      {
        return true;
      }
    }
  }

  return false;
}

// BOTÃO EMERGENCY

bool emergencyPressed()
{
  bool reading = digitalRead(BUTTON_EMERGENCY);

  if (reading != emergencyRawState)
  {
    emergencyRawState = reading;
    emergencyChangeTime = millis();
  }

  if ((unsigned long)(millis() - emergencyChangeTime) >= DEBOUNCE_TIME)
  {
    if (emergencyStableState != emergencyRawState)
    {
      emergencyStableState = emergencyRawState;

      if (emergencyStableState == LOW)
      {
        return true;
      }
    }
  }

  return false;
}

// EMERGENCY STOP

void emergencyStop()
{
  if (emergencyLatched)
  {
    return;
  }

  emergencyLatched = true;

  Serial.println();
  Serial.println(F("!!! EMERGENCY ACTIVATED !!!"));

  changeState(STATE_ABORTED);
}

// ABORT POR PRESSÃO

void pressureAbort()
{
  if (emergencyLatched)
  {
    return;
  }

  Serial.println();
  Serial.println(F("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
  Serial.println(F("     PRESSURE LIMIT EXCEEDED"));
  Serial.println(F("        SYSTEM ABORTED"));
  Serial.println(F("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));

  emergencyLatched = true;

  changeState(STATE_ABORTED);
}

// START

void startCountdown()
{
  if (emergencyLatched)
  {
    Serial.println(F("START DENIED: EMERGENCY LATCHED."));
    return;
  }

  if (currentState != STATE_SAFE)
  {
    Serial.print(F("START DENIED. STATE: "));
    Serial.println(stateName(currentState));

    return;
  }

  lampsOff();

  Serial.println(F("START ACCEPTED."));

  changeState(STATE_COUNTDOWN);
}

// RESET

void resetSystem()
{
  // Não permite RESET enquanto o botão de emergência estiver pressionado
  if (digitalRead(BUTTON_EMERGENCY) == LOW)
  {
    Serial.println(F("RESET DENIED."));
    Serial.println(F("Release EMERGENCY first."));
    return;
  }

  int pressure = analogRead(PRESSURE_SENSOR);

  // Não permite RESET se a pressão continuar acima do limite
  if (pressure >= PRESSURE_LIMIT)
  {
    Serial.println(F("RESET DENIED."));
    Serial.println(F("PRESSURE STILL ABOVE LIMIT."));
    return;
  }

  Serial.println();
  Serial.println(F("SYSTEM RESET."));

  emergencyLatched = false;

  startRawState = digitalRead(BUTTON_START);
  startStableState = startRawState;
  startChangeTime = millis();

  emergencyRawState = digitalRead(BUTTON_EMERGENCY);
  emergencyStableState = emergencyRawState;
  emergencyChangeTime = millis();

  lampsOff();

  changeState(STATE_SAFE);
}

// MONITORAMENTO DA PRESSÃO

void checkPressure()
{
 
  if (currentState != STATE_COUNTDOWN)
  {
    return;
  }

  int pressureValue = analogRead(PRESSURE_SENSOR);

  if (pressureValue >= PRESSURE_LIMIT)
  {
    pressureAbort();
  }
}


void updateCountdown()
{
  if (currentState != STATE_COUNTDOWN)
  {
    return;
  }

  unsigned long now = millis();

  if ((unsigned long)(now - countdownTimer) >= ONE_SECOND)
  {
    countdownTimer += ONE_SECOND;

    if (countdown > 0)
    {
      countdown--;
    }

    Serial.print(F("COUNT: "));
    Serial.println(countdown);

    // Alternar LEDs
    if (countdown > 0)
    {
      if ((countdown % 2) == 0)
      {
        led1On();
      }
      else
      {
        led2On();
      }
    }

    // Chegou a zero
    if (countdown == 0)
    {
      ledsOff();

      changeState(STATE_READY);
    }
  }
}

// STATUS


void showStatus()
{
  int pressure = analogRead(PRESSURE_SENSOR);

  Serial.println();
  Serial.println(F("========== STATUS =========="));

  Serial.print(F("STATE: "));
  Serial.println(stateName(currentState));

  Serial.print(F("COUNTDOWN: "));
  Serial.println(countdown);

  Serial.print(F("PRESSURE SENSOR: "));
  Serial.println(pressure);

  Serial.print(F("PRESSURE LIMIT: "));
  Serial.println(PRESSURE_LIMIT);

  Serial.print(F("EMERGENCY LATCHED: "));

  if (emergencyLatched)
    Serial.println(F("YES"));
  else
    Serial.println(F("NO"));

  Serial.print(F("LAMP 1: "));

  if (currentState == STATE_READY)
    Serial.println(F("ON"));
  else
    Serial.println(F("OFF"));

  Serial.print(F("LAMP 2: "));

  if (currentState == STATE_ABORTED)
    Serial.println(F("ON"));
  else
    Serial.println(F("OFF"));

  Serial.println(F("============================"));
}

// SERIAL

const byte SERIAL_BUFFER_SIZE = 32;

char serialBuffer[SERIAL_BUFFER_SIZE];
byte serialPosition = 0;

void processCommand(char* command)
{
  if (strcmp(command, "START") == 0)
  {
    startCountdown();
  }

  else if (strcmp(command, "RESET") == 0)
  {
    resetSystem();
  }

  else if (strcmp(command, "EMERGENCY") == 0)
  {
    emergencyStop();
  }

  else if (strcmp(command, "ABORT") == 0)
  {
    emergencyStop();
  }

  else if (strcmp(command, "STATUS") == 0)
  {
    showStatus();
  }

  else if (strcmp(command, "PING") == 0)
  {
    Serial.println(F("PONG"));
  }

  else if (strcmp(command, "HELP") == 0)
  {
    Serial.println();
    Serial.println(F("========== COMMANDS =========="));
    Serial.println(F("START"));
    Serial.println(F("RESET"));
    Serial.println(F("EMERGENCY"));
    Serial.println(F("ABORT"));
    Serial.println(F("STATUS"));
    Serial.println(F("PING"));
    Serial.println(F("HELP"));
    Serial.println(F("=============================="));
  }

  else if (command[0] != '\0')
  {
    Serial.print(F("UNKNOWN COMMAND: "));
    Serial.println(command);
  }
}

// LEITURA SERIAL

void readSerial()
{
  while (Serial.available() > 0)
  {
    char c = Serial.read();

    if (c == '\n' || c == '\r')
    {
      if (serialPosition > 0)
      {
        serialBuffer[serialPosition] = '\0';

        // Converte minúsculas para maiúsculas
        for (byte i = 0; i < serialPosition; i++)
        {
          if (serialBuffer[i] >= 'a' &&
              serialBuffer[i] <= 'z')
          {
            serialBuffer[i] =
              serialBuffer[i] - 'a' + 'A';
          }
        }

        processCommand(serialBuffer);

        serialPosition = 0;
      }
    }
    else
    {
      if (serialPosition < SERIAL_BUFFER_SIZE - 1)
      {
        serialBuffer[serialPosition] = c;
        serialPosition++;
      }
      else
      {
        serialPosition = 0;

        Serial.println(F("ERROR: COMMAND TOO LONG."));
      }
    }
  }
}

// SETUP

void setup()
{
  Serial.begin(9600);

  // LEDs

  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);

  // Botões

  pinMode(BUTTON_START, INPUT_PULLUP);
  pinMode(BUTTON_EMERGENCY, INPUT_PULLUP);

  // Relés

  pinMode(RELAY_LAMP_1, OUTPUT);
  pinMode(RELAY_LAMP_2, OUTPUT);

  // Relés ativos em LOW:
  // HIGH = OFF
  // LOW  = ON

  digitalWrite(RELAY_LAMP_1, RELAY_OFF);
  digitalWrite(RELAY_LAMP_2, RELAY_OFF);

  // Sensor

  pinMode(PRESSURE_SENSOR, INPUT);

  // LEDs

  ledsOff();

  // Botão START

  startRawState = digitalRead(BUTTON_START);
  startStableState = startRawState;
  startChangeTime = millis();

  // Botão EMERGENCY

  emergencyRawState = digitalRead(BUTTON_EMERGENCY);
  emergencyStableState = emergencyRawState;
  emergencyChangeTime = millis();

  // Estado inicial

  emergencyLatched = false;

  // Garantia de inicialização das lâmpadas
  lampsOff();

  // Serial

  Serial.println();
  Serial.println(F("================================"));
  Serial.println(F("     LAUNCH SAFETY CONTROLLER"));
  Serial.println(F("            ARDUINO UNO"));
  Serial.println(F("              V7.0"));
  Serial.println(F("================================"));

  changeState(STATE_SAFE);
}

// LOOP PRINCIPAL

void loop()
{
 
  // EMERGENCY

  if (emergencyPressed())
  {
    emergencyStop();
  }

  // Se houve emergência, não processa START
  if (currentState == STATE_ABORTED)
  {
    readSerial();
    return;
  }

  // START

  if (startPressed())
  {
    startCountdown();
  }

  // SERIAL

  readSerial();

  // PRESSÃO

  checkPressure();

  // Se a pressão causou abort, não continua o countdown
  if (currentState == STATE_ABORTED)
  {
    return;
  }


  // COUNTDOWN


  updateCountdown();


  // ESTADO DAS LÂMPADAS


  if (currentState == STATE_SAFE)
  {
    lampsOff();
  }
  else if (currentState == STATE_COUNTDOWN)
  {
    lampsOff();
  }
  else if (currentState == STATE_READY)
  {
    lamp1On();
  }
  else if (currentState == STATE_ABORTED)
  {
    lamp2On();
  }
}
