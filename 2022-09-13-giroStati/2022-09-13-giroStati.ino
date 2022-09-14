#include <Servo.h>
#include <Wire.h>
#include <MPU6050.h>

//pin
#define S0_PIN 8
#define S1_PIN 7
#define S2_PIN 4
#define S3_PIN 5
#define OUT_PIN 6
#define pwmMotore 3       //PWM control for motor outputs 1 and 2 
#define dirMotore 2       //direction control for motor outputs 1 and 2 

//COSTANTI
#define GRADI_CURVA 80

//OGGETTI
Servo myservo;            //oggetto servomotore
MPU6050 mpu;              //oggetto giroscopio

//VARIABILI
int velocita=150;         //velocità media in tutto il percorso
int colori[3],vecchi[3];  //array RGB di lettura del sensore 
                          //^^^aggiornati alla chiamata di updatergb^^^

int bluWhite=0;           //quantità di blu quando sei sul bianco
int redStandard=0;        //quantità di rosso quando sei sul bianco
                          //^^^aggiornati al setup^^^
                          
short dirTrovata=0;       //stabilisce se ha trovato una direzione
short verso=0;            //orario oppure antiorario

//NECESSARI ALLA FUNC giroscopio()
// Timers
unsigned long timer = 0;
float timeStep = 0.01;

//Yaw values
float yaw = 0;
float yawPrecedente = 0;
//variabile di stato
int stato=0;

//prototipi
void raddrizza();
void updateRGB();
void vai(int );
void destra();
void sinistra();
short direzione();
void giroscopio();
void orario();
void antiorario();

void setup(){
    Serial.begin(115200);

    while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G)){
      Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
      delay(500);
    }
    
    mpu.calibrateGyro();
    mpu.setThreshold(3);
    
    pinMode(pwmMotore, OUTPUT);  //Set control pins to be outputs
    pinMode(dirMotore, OUTPUT);
    pinMode(S0_PIN, OUTPUT);
    pinMode(S1_PIN, OUTPUT);
    pinMode(S2_PIN, OUTPUT);
    pinMode(S3_PIN, OUTPUT);  
    pinMode(OUT_PIN, INPUT);
    myservo.attach(9);
    raddrizza();
    // Power down
    //digitalWrite(S0_PIN, LOW);
    //digitalWrite(S1_PIN, LOW);
    //Frequency 50%
    digitalWrite(S0_PIN, HIGH);
    digitalWrite(S1_PIN, LOW);
    vecchi[0]=colori[0];

    //salviamo il valore blu del bianco
    updateRGB();
    bluWhite = colori[0];
    //salvo anche il rosso standard (quando è sul bianco)
    redStandard = colori[2];

    Serial.print("ROSSO STANDARD ");
    Serial.println(redStandard);

    Serial.print("VALORE BLU SU BIANCO ");
    Serial.println(bluWhite);
  
}

void loop(){
    updateRGB();
    switch(stato){
      case 0:
        stopMotore();
        break;
      case 1:                       //stato ricerca del verso
        vai(velocita);
        while(dirTrovata==0){
            updateRGB();
            verso = direzione();
            if(verso!=0){
              dirTrovata = 1;
              stato=2;
            }
        }
        break;
      case 2:                       //stato in cui si curva 
        giro();
        break;
      case 3:
        break;
    }
    vecchi[0]=colori[0]+20;
}

//funzioni per il movimento dei motori
void stopMotore(){
  analogWrite(pwmMotore, 0);  
  Serial.print("STOP");
}
void vai(int velocita){
  analogWrite(pwmMotore, velocita);  
}
void avanti(){
  digitalWrite(dirMotore, LOW);  
  //Serial.println("avanti"); 
}
void indietro(){
  digitalWrite(dirMotore, HIGH);   
  Serial.println("indietro");
}
void raddrizza(){
  myservo.write(88);
  Serial.println("raddrizza");
}
void destra(){
  myservo.write(130);
  Serial.println("destra");
}
void sinistra(){
  myservo.write(50);
  Serial.println("sinistra");
}

//funzione che aggiorna i dati del sensore di colore 
void updateRGB() {
  /*//Frequency 20%
  digitalWrite(S0_PIN, HIGH);
  digitalWrite(S1_PIN, LOW);
  delay(10);
  */
  colori[0]=0;
  colori[1]=0;
  colori[2]=0;

  for (int i = 0; i < 5; i++) {
    
    //stabilire frequenza di lettura del sensore di colore
    digitalWrite(S2_PIN, LOW);
    digitalWrite(S3_PIN, LOW);
    
    colori[0] += pulseIn(OUT_PIN, LOW);
    delay(1);
    /*
    digitalWrite(S2_PIN, HIGH);
    clear += pulseIn(OUT_PIN, LOW);
    delay(1);
    */
    digitalWrite(S3_PIN, HIGH); 
    colori[1] += pulseIn(OUT_PIN, LOW);
    delay(1);
    
    digitalWrite(S2_PIN, LOW);
    colori[2] += pulseIn(OUT_PIN, LOW);
    delay(1);
  }
   
  colori[0] /= 5;  
  colori[1] /= 5;
  colori[2] /= 5;
  //clear /= 5; 
  /*
  // Power down
  digitalWrite(S0_PIN, LOW);
  digitalWrite(S1_PIN, LOW);
  */
}

//funzione che rileva il corlore -1 per il rosso 0 per il bianco 1 per il blu
short direzione(){
  if(colori[2]-redStandard<=-50 || colori[2]-redStandard>=50){
      return -1;
    }else{
      if((colori[0]-bluWhite>-GRADI_CURVA && colori[0]-bluWhite<GRADI_CURVA))
        return 0;
      else
        return 1;
   }
}

void giroscopio(){
  timer = millis();
  
  // Read normalized values
  Vector norm = mpu.readNormalizeGyro();

  // Calculate Yaw
  yaw = yaw + norm.ZAxis * timeStep;

  Serial.print(" Yaw = ");
  Serial.println(yaw);

  // Wait to full timeStep period
  delay((timeStep*1000) - (millis() - timer));
}

void giro(){
    if(direzione()==verso){
        if(verso==1)
          sinistra();
        else 
          destra();
        while(true){
          giroscopio();
          if(valAssoluto(yaw)>GRADI_CURVA+yawPrecedente){
            yawPrecedente+=GRADI_CURVA;
            Serial.println(yawPrecedente);
            raddrizza();
            updateRGB();
            break;
          }
          if(valAssoluto(yaw)>=GRADI_CURVA*(12))
            stato=0;
        }
    }
}

float valAssoluto(float x){
  if(x>0)
    return x;
  return x*(-1);
}
