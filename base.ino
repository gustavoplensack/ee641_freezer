/*  Projeto 02 - EA076 - Turma D - 1s2018
 *  Alunos: 
 *  Gustavo Granela Plensack - RA:155662
 *  Guilherme Rosa - RA 157955 
 */

//bibliotecas
#include "TimerOne.h" // biblioteca que simplifica a configuração do timer
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

//defines
#define base 1000 //base de 1ms
#define SCLK 8
#define DIN 9
#define DC 10
#define CSCE 11 
#define RST 12
#define ENBL 5 
#define INP1A 6
#define INP2A 7
#define func_vent "VENT"
#define func_exaust "EXAUST"
#define func_para "PARA"
#define func_vel "VEL"
#define func_ret "RETVEL"
#define pinoInterruptExterna 3

//funções
void iniciaDisplay(); // faz toda a sequência de definições de pinos como entradas e saidas
void increaseTime(); // ISR que é responsável pela sincronização dos eventos no Loop
void interruptOn(); //ativa a interrução do timer 1
void interruptOff(); // desativa a interrupçaõ do timer 1
void iniciaPinosMotor();
void atualizaDisplay(bool isExaustor,bool movimento,int potencia,int velocidade); // atualização dos valores do display temporizada 
void defineSentido(bool horario);
void definePotencia(int potencia);
void freiaSistema();
void serialRXFlush();
void contaPulso();


//variaveis e objetos globais
int timeCounter=0; //variavel contadora do tempo, segundo a formula tempo = timeCounter * base
Adafruit_PCD8544 display = Adafruit_PCD8544(SCLK, DIN, DC, CSCE, RST);
bool isMoving = false;
int power = 0;
int RPM = 0;
bool isHorario = false;
char entradaDado = '/0';
String stringEntrada = "";
String auxStringEntrada = "";
bool isNewInput = false;
int timeOutCounter = 0;
bool isSerialError = false;
int lastRPM = 0;
int estado = 0;
int changed = 0;
bool isVelError = false;
int auxPower = 0;

//programa
void setup() {
 iniciaDisplay();
 iniciaPinosMotor();
 interruptOn();
 Serial.begin(9600);//inicia a comunicação com o bluetooth
 attachInterrupt(digitalPinToInterrupt(pinoInterruptExterna),contaPulso,FALLING);
 }

void loop() {
  
  if (!isNewInput && Serial.available()>0){
    isSerialError = false;
    timeOutCounter = 0;
    while(1){
       if(Serial.available()>0 && timeOutCounter>1){
        timeOutCounter = 0;
        entradaDado = Serial.read();   
        if (entradaDado == '*'){
          serialRXFlush();
          isNewInput = true;
          break;
         }
         auxStringEntrada += entradaDado;
        }
      
       if (timeOutCounter>=500){
         isSerialError = true;
  //       Serial.println("ERRO: TIMEOUT");
         isNewInput = false;
         break; 
         }
    }
     entradaDado = '/0';
     stringEntrada = auxStringEntrada;
     auxStringEntrada = "";
   }
  

  if (timeCounter>=1000){
    lastRPM = RPM;
    atualizaDisplay(isHorario,isMoving,power,60*lastRPM/2);
    RPM = 0;

    if (changed == 0){
      defineSentido(isHorario);
      if(power>0){
        definePotencia(power);
        }else {
          analogWrite(ENBL,0);
          }
    }
    
    if(changed == 1){
      if(lastRPM != 0){
      }else{
        changed = 0;
        defineSentido(isHorario);
        if(power>0){
          definePotencia(power);
          }else {
            analogWrite(ENBL,0);
            }
      }
    }
       timeCounter = 0;
      }



  if (isNewInput){

    if(stringEntrada == func_vent){
      if(estado == 2 && lastRPM != 0){
        freiaSistema();
        changed = 1;        
        }
      isHorario = true;
      isMoving = true;      
      estado = 1;
      Serial.println("OK VENT");

      } else if(stringEntrada == func_exaust){
        if(estado == 1 && lastRPM != 0){
          freiaSistema();
          changed = 1;
          }
        isHorario = false;
        isMoving = true;
        estado = 2;
        Serial.println("OK EXAUST");

      }else if(stringEntrada == func_para){
        power = 0;
        isMoving = false;
        freiaSistema();
        estado = 0;
        Serial.println("OK PARA");

      }else if (stringEntrada == func_ret){
        Serial.println("VEL: "+String(lastRPM*60/2)+" RPM");
      }

      else if(stringEntrada.substring(0,3) == func_vel){
       
        if(stringEntrada.length() <= 3){
        Serial.println("ERRO: PARAMETRO AUSENTE");
        isVelError = true;
        } else if(stringEntrada.length()>7){
          Serial.println("ERRO: PARAMETRO INCORRETO");
          }else{

            if(stringEntrada.substring(4,7) == "000"){
              auxPower = 0;
              power = 0;
              isMoving = true;
              
              Serial.println("OK VEL "+String(power)+"%");
              } else if (stringEntrada.substring(4,7).toInt() > 0){
                auxPower = (stringEntrada.substring(4,7).toInt());
                if (auxPower>=0 && auxPower <= 100){
                  power = auxPower;
                  isMoving = true;
                  Serial.println("OK VEL "+String(power)+"%");
                }else {
                  isVelError = true;
                  Serial.println("ERRO: PARAMETRO INCORRETO");
                  }
            }else{
                  Serial.println("ERRO: PARAMETRO INCORRETO");            
            }
     }
   }

    if (stringEntrada.substring(0,3) != func_vel && stringEntrada != func_para && stringEntrada != func_exaust && stringEntrada != func_ret && stringEntrada != func_vent){
      Serial.println("ERRO:COMANDO INEXISTENTE");
      }
         
    
     stringEntrada = "";
     isVelError = false;
     isNewInput = false;
 }

 }

void contaPulso(){
  RPM = RPM+1;
  }

void serialRXFlush(){
    while(Serial.available()>0){
            Serial.read();
            }
  
  }

void freiaSistema(){
  digitalWrite(ENBL,HIGH);
  digitalWrite(INP1A,LOW);
  digitalWrite(INP2A,LOW);
 }


void definePotencia(int potencia){
  analogWrite(ENBL,map(potencia,1,100,55,255));  
  }


void iniciaPinosMotor(){
    pinMode(ENBL, OUTPUT);
    pinMode(INP1A, OUTPUT);
    pinMode(INP2A, OUTPUT);
}

void defineSentido(bool horario){
    if(horario){
      digitalWrite(INP1A,HIGH);
      digitalWrite(INP2A,LOW);
    }
    if(!horario){
      digitalWrite(INP1A,LOW);
      digitalWrite(INP2A,HIGH);
    }
}

void atualizaDisplay(bool isVent,bool movimento,int potencia,int velocidade){
  display.clearDisplay();
  if (!movimento){
    display.println("Proj2 - EA076"); // linha 1
    display.println("ESTADO: PARADO"); // linha 2
    display.println("Potencia: 0"); // linha 3
    display.println("VEL: 0 RPM"); // linha 4  
   }

  if (movimento && isVent){
    display.println("Proj2 - EA076"); // linha 1
    display.println("ESTADO: VENT"); // linha 2
    display.println("Potencia: " + String(potencia) + "%"); // linha 3
    display.println("VEL: " + String(velocidade) + " RPM"); // linha 4     
   }

  if (movimento && (!isVent)){
    display.println("Proj2 - EA076"); // linha 1
    display.println("ESTADO: EXAUST"); // linha 2
    display.println("Potencia: " + String(potencia) + "%"); // linha 3
    display.println("VEL: " + String(velocidade) + " RPM"); // linha 4
   }
  display.display(); //apresenta os valores no display
 }

void iniciaDisplay(){
  display.begin();
  display.setContrast(70 ); //Ajusta o contraste do display
  display.clearDisplay();   //Apaga o buffer e o display
  display.setTextSize(1);  //Seta o tamanho para caber na linha como o desejado
  display.setTextColor(BLACK); //Seta letra preta
  display.setCursor(0,0);  //Seta a posição do cursor
  display.println("Proj2 - EA076"); // linha 1
  display.println("ESTADO: "); // linha 2
  display.println("Potencia: "); // linha 3
  display.println("VEL: "); // linha 4
  display.display();// apresenta os dados na tela
 }
 
void increaseTime(){
  timeCounter++;
  timeOutCounter++;
}

void interruptOn(){//liga as interrupcoes do timerOne
  Timer1.initialize(base);
  Timer1.attachInterrupt(increaseTime);
  }
  
void interruptOff(){//desliga as interrupcoes do timerOne
    Timer1.detachInterrupt(); 
  }
