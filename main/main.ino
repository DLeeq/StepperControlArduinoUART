#define MOTOR 0
#define PRIVOD 1

class StepMotor
{
  private:
    const uint8_t pinDir;               //Направляющий пин
    const uint8_t pinStep;              //Шаговый пин
    const uint16_t SPR;                 //Количество шагов на полный оборот двигателя

    uint32_t stepTime;                  //Время одного шага в мкс

    bool controlMode;                   //Режим работы шагового двигателя

                                        //Параметр для работы в режиме мотора
    int8_t motorState;                    //Состояние мотора, старт 1, стоп 0, реверс -1

                                        //Параметры для работы в режиме привода
    uint32_t privodPos;                   //Фактическая позиция привода
    uint32_t privodTarget;                //Цель позиции привода

    bool pinStepState;                  //Состояние шагового пина
    
    bool pinDirInitial;                 //Исходное состояние пина направления

  public:
    StepMotor(uint8_t p_pinStep, uint8_t p_pinDir, bool p_cMode = MOTOR, uint16_t StepsPerRev = 200, uint32_t speed = 200) : 
      pinDir(p_pinDir), 
      pinStep(p_pinStep), 
      SPR(StepsPerRev),

      stepTime(1000000 / speed),

      controlMode(p_cMode),

      motorState(0),

      privodPos(0),     
      privodTarget(0), 

      pinStepState(0),

      pinDirInitial(0) 
    {}
    void tick()
    {
      static uint32_t timer = 0;

      if(controlMode == MOTOR)
      {
        if(micros() - timer > stepTime / 2 && motorState == 1)
        {
          timer = micros();

          digitalWrite(pinDir, pinDirInitial);

          pinStepState = !pinStepState;
          digitalWrite(pinStep, pinStepState);

          //Serial.println("MOTOR FORWARD");
        }
        else if(micros() - timer > stepTime / 2 && motorState == -1)
        {
          timer = micros();

          digitalWrite(pinDir, !pinDirInitial);

          pinStepState = !pinStepState;
          digitalWrite(pinStep, pinStepState);

          //Serial.println("MOTOR REVERSE");
        }
      }
      else if(controlMode == PRIVOD)
      {
        if(privodTarget > privodPos)
        {
          if(micros() - timer > stepTime / 2)
          {
            timer = micros();

            digitalWrite(pinDir, pinDirInitial);

            pinStepState = !pinStepState;
            digitalWrite(pinStep, pinStepState);

            if(pinStepState == 0)
            {
              privodPos++;
              //Serial.println("FORWARD_STEP privodPos:" + String(privodPos));
            }
          }
        }
        else if(privodTarget < privodPos)
        {
          if(micros() - timer > stepTime / 2)
          {
            timer = micros();

            digitalWrite(pinDir, !pinDirInitial);

            pinStepState = !pinStepState;
            digitalWrite(pinStep, pinStepState);

            if(pinStepState == 0)
            {
              privodPos--;
              //Serial.println("BACKWARD_STEP privodPos:" + String(privodPos));
            }
          }
        }
      }
    }
    //Функции управления мотором
    void Start()
    {
      motorState = 1;
      pinStepState = 0;
    }
    void Stop()
    {
      motorState = 0;
      pinStepState = 0;
    }
    void Reverse()
    {
      motorState = -1;
      pinStepState = 0;
    }

    //Функции управления приводом
    void Move(uint32_t angle)
    {
      privodTarget = angle * SPR / 360;
      pinStepState = 0;
    }
    void MoveSteps(uint32_t tar)
    {
      privodTarget = tar;
      pinStepState = 0;
    }
    void MovePercent(uint32_t percent)
    {
      privodTarget = percent * SPR / 100;
      pinStepState = 0;
    }

    //Общие функции
    void SetSpeed(uint32_t speed)
    {
      stepTime = 1000000 / speed;
    }
    void SetControlMode(bool mode)
    {
      controlMode = mode;

      motorState = 0;

      privodPos = 0;    
      privodTarget = 0; 

      pinStepState = 0;
    }

    //Программное переключение обмоток (опционально)
    void InvertControl(bool control)
    {
      pinDirInitial = control;
    }
};

StepMotor Motor1(7, 8);

void setup() 
{
  Serial.begin(9600);
  Serial.setTimeout(50);

  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
}

void loop() 
{
  static int32_t operation = -999;
  static uint32_t timer = 0;
  static bool flagMotor = 1;
  static bool flagPrivod = 1;

  if(millis() - timer > 500)
  {
    timer = millis();

    if(operation >= 1 && operation <= 3)
    {
      if(flagMotor)
      {
        Motor1.SetControlMode(MOTOR);
        flagMotor = 0;
        flagPrivod = 1;
      }
      switch(operation)
      {
        case 1: Motor1.Stop(); Serial.println("\n\n\n\n\nРежим: ДВИГАТЕЛЬ\nСостояние: ОСТАНОВЛЕН"); break;
        case 2: Motor1.Start(); Serial.println("\n\n\n\n\nРежим: ДВИГАТЕЛЬ\nСостояние: ПУСК"); break;
        case 3: Motor1.Reverse(); Serial.println("\n\n\n\n\nРежим: ДВИГАТЕЛЬ\nСостояние: РЕВЕРС"); break;
      }
      operation = -999;
    }

    else if(operation <= 0 && operation >= -360)
    {
      if(flagPrivod)
      {
        Motor1.SetControlMode(PRIVOD);
        flagMotor = 1;
        flagPrivod = 0;
      }
      Motor1.Move(-operation);
      Serial.print("\n\n\n\n\nРежим: ПРИВОД\nУгол: ");
      Serial.print(-operation);
      Serial.println(" гр.");
      operation = -999;
    }

    else if(operation >= 50 && operation <= 2000)
    {
      Motor1.SetSpeed(operation);
      Serial.print("\n\n\n\n\nУстановлена скорость привода: ");
      Serial.print(operation);
      Serial.println(" шагов/сек");
      operation = -999;
    }
    else if( operation != -999 )
    {
      Serial.println("\n\n\n\n\nОшибка! Введено неверное значение!");
      operation = -999;
    }
  }
  

  if(Serial.available())
    operation = Serial.parseInt();

  Motor1.tick();
}
