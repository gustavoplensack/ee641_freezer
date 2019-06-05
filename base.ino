/*  Projeto 02 - EA076 - Turma D - 1s2018
 *  Alunos: 
 *  Gustavo Granela Plensack - RA:155662
 *  Walter Azevedo - RA
 *  Caique Campos - RA 
 */

//bibliotecas
#include "TimerOne.h" // biblioteca que simplifica a configuração do timer

//defines
#define base 1000 //base de 1ms
#define func_pwm "PWM" 
#define func_temp "TEMP"
#define pinoPWM 5 //pino do PWM

//funções
void increaseTime(); // ISR que é responsável pela sincronização dos eventos no Loop
void interruptOn(); //ativa a interrução do timer 1
void interruptOff(); // desativa a interrupçaõ do timer
void definePotencia(int potencia);
void serialRXFlush();
void iniciaPWM();

//variaveis e objetos globais
int timeCounter=0; //variavel contadora do tempo, segundo a formula tempo = timeCounter * base
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
 interruptOn();
 Serial.begin(9600);//inicia comunicação com a UART
 iniciaPWM();
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
         Serial.println("ERRO: TIMEOUT");
         isNewInput = false;
         break; 
         }
    }
     entradaDado = '/0';
     stringEntrada = auxStringEntrada;
     auxStringEntrada = "";
   }
  

  if (isNewInput){

    if(stringEntrada.substring(0,4) == "TEMP"){
        
        Serial.println(stringEntrada.substring(5,8).toInt());
      } 

  
    if(stringEntrada.substring(0,3) == "PWM"){
        if (stringEntrada.substring(4,8).toInt()>255){
          Serial.println("Valor acima do esperado. Foi limitado para 255");
          analogWrite(pinoPWM,255);
        } else if(stringEntrada.substring(4,8).toInt()<0){
           Serial.println("Valor invalido! Insira valores maiores do que zero");
        }else{
          analogWrite(pinoPWM,stringEntrada.substring(4,8).toInt());
          Serial.println("Saida definda pelo PWM em: "+stringEntrada.substring(4,7)+"/255");
        } 
    }
    
    if (stringEntrada.substring(0,4) != func_temp && stringEntrada.substring(0,3) != func_pwm){
      Serial.println("ERRO:COMANDO INEXISTENTE");
      }
         
    
     stringEntrada = "";
     isNewInput = false;
 }

 }


void iniciaPWM(){
  pinMode(pinoPWM,OUTPUT);
}

void serialRXFlush(){
    while(Serial.available()>0){
            Serial.read();
            }
  
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
