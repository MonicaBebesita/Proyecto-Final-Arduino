// include the library code:
#include "DHT.h"
#include  <LiquidCrystal.h>
#include "AsyncTaskLib.h"
#include <LiquidMenu.h>
//#include "AverageValue.h"
#include <Keypad.h>
#include "StateMachineLib.h"
#include <RunningAverage.h>
//#include <Average.h>
#include <OneButton.h>

#define enter 1
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
//--contraseña--//
byte intentos = 0;
const char password [8] = "1234567"; // #1234567
bool contrasenia = false;
char password_us [8];
int timer1 = 10;
int timer2 = 4; 
//----------------------
//Pins para el led
const int ledRojo = 9;
const int ledVerde = 8;
const int ledAzul = 10;
//-------------------------
const char forward = 'A';
const char backward = 'B';
#define buttonPin 13

OneButton btn = OneButton(
  buttonPin,  // Input pin for the button
  false,        // Button is active LOW
  false         // Enable internal pull-up resistor
);


bool executeMenu;


byte j;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {22, 24, 26, 27}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {30, 32, 34,36}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
#define DHTPIN 28         // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

#define DEBUG(a)  Serial.print(millis()); Serial.print(": "); Serial.println(a);

const char text1[] PROGMEM = "Mon_Ambiental";
const char text2[] PROGMEM = "Reset Limites";
const char text3[] PROGMEM = "TH TEMP HIGH";
const char text4[] PROGMEM = "TH TEMP LOW";
const char text5[] PROGMEM = "TH LIGHT HIGH";
const char text6[] PROGMEM = "TH LIGHT LOW";
/*const char text7[] PROGMEM = "TH HUM HIGH";
const char text8[] PROGMEM = "TH HUM LOW";*/
const char text9[] PROGMEM = "TH GAS HIGH";
//const char text10[] PROGMEM = "TH GAS LOW";

void readLight(void); 
void readGas(void);
void readTem(void);
void readHum(void);
void playSonidoError(void);
void playSonidoCorrecto(void);
void showGreenLed(void);
void showRedLed(void);
void showBlueLed(void);
//void goBackWard(void);
void updateMonAmbiental(void);
void updateMonHall(void);
void manejoTimer1(void);
void manejoTimer2(void);
void manejoTimer3(void);
void manejoSeguridad(void);
void monLimiteHall(void);
void monLimitesAmbiental(void);
void myMenu(void);
void alarma(void);
//------TAREAS---------
AsyncTask TaskLuz(1500, true, readLight);
AsyncTask TaskGas(1000, true, readGas);
AsyncTask TaskTemperatura(3000, true, readTem);
AsyncTask TaskHumedad(1800, true,  readHum);
AsyncTask TaskBuzzerError(1800,true,playSonidoError);
AsyncTask TaskBuzzerCorrecto(1800,true,playSonidoCorrecto);
AsyncTask TaskShowGreen(1800,true,showGreenLed);
AsyncTask TaskShowBlue(1800,true,showBlueLed);
AsyncTask TaskShowRed(1800,true,showRedLed);
//AsyncTask TaskBackward(1,true,goBackWard);
AsyncTask TaskActualizarMonAmbiental(100,true,updateMonAmbiental);
AsyncTask TaskMenu(0,true,myMenu);
AsyncTask TaskActualizarMonHall(100,true,updateMonHall);
AsyncTask TaskTimer1(7000,true,manejoTimer1);
AsyncTask TaskTimer2(3000,true,manejoTimer2);
AsyncTask TaskTimer3(4000,true,manejoTimer3);
AsyncTask TaskSecurity(10000,true,manejoSeguridad);
AsyncTask TaskAlarmaMonAmbiental(3000,true,monLimitesAmbiental);
AsyncTask TaskAlarmaMonHall(3000,true,monLimiteHall);
AsyncTask TaskAlarma(100,true,alarma);
AsyncTask TaskBoton(0,[](){btn.tick();});
//---UMBRALES PARA MONITOREO AMBIENTAL 

//variables para los umbrales 
int temp_high; 
int temp_low;
int luz_high;
int luz_low;
int hum_high;
int hum_low;
int gas_high;
int gas_low; 

RunningAverage averageTem(10);
//Average<float> averageTem1;
RunningAverage averageLuz(10);
//Average<float> averageLuz1;
RunningAverage averageHum(10);
RunningAverage averageGas(10);
//Average<float> averageGas1;

#define DHTTYPE DHT11   // DHT 11

DHT dht (DHTPIN , DHTTYPE);
const int photocellPin = A0;
const int gasPin = A1;
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
const int buzzerPin = 14;
int fre;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

LiquidLine opt1(1, 0, text1);
LiquidLine opt2(1, 1, text2);
LiquidScreen scr1(opt1, opt2);

LiquidLine opt3(1, 0, text3);
LiquidLine opt4(1, 1, text4);
LiquidScreen scr2(opt3, opt4);

LiquidLine opt5(1, 0, text5);
LiquidLine opt6(1, 1, text6);
LiquidScreen scr3(opt5, opt6);

/*LiquidLine opt7(1, 0, text7);
LiquidLine opt8(1, 1, text8);
LiquidScreen scr4(opt7, opt8);*/

LiquidLine opt9(1, 0, text9);
//LiquidLine opt10(1, 1, text10);
//LiquidScreen scr5(opt9, opt10);
LiquidScreen scr5(opt9);


LiquidMenu menu(lcd,scr1);
int outputValue = 0;
int gas = 0;

int newValue = 0;
byte pos = 0;
bool isNegative = false;
//Estados
enum State
{
  E_leerContrasenia=0, //A
  E_systemaBloqueado=1, //B
  E_configuracion=2, //C
  E_alarma=3, //F
  E_monitoreoAmbiental=4, //D
  E_monitoreoEventos = 5 //E
  
};

enum Input
{
  
  Unknown = 0,
  correct_password = 1,
  out_of_tries= 2, 
  wait_ended = 3,
  time_out = 4,
  time_out2 = 5,
  limit_exceeded = 6,
  button = 7
};

StateMachine stateMachine(6, 13);
Input input;

void setupStateMachine()
{
  stateMachine.AddTransition(E_leerContrasenia, E_configuracion, []() { return input == correct_password; });
  stateMachine.AddTransition(E_leerContrasenia, E_systemaBloqueado, []() { return input == out_of_tries; });
  stateMachine.AddTransition(E_configuracion, E_monitoreoAmbiental, []() { return input == button; });
  stateMachine.AddTransition(E_monitoreoAmbiental, E_configuracion, []() { return input == button; });
  stateMachine.AddTransition(E_systemaBloqueado,E_leerContrasenia, []() { return input == wait_ended; });
  stateMachine.AddTransition(E_monitoreoAmbiental, E_monitoreoEventos, []() { return input == time_out; });
  stateMachine.AddTransition(E_monitoreoEventos, E_monitoreoAmbiental, []() { return input == time_out2; });
  stateMachine.AddTransition(E_monitoreoEventos, E_configuracion, []() { return input == button; });
  stateMachine.AddTransition(E_monitoreoEventos, E_alarma, []() { return input == limit_exceeded; });
  stateMachine.AddTransition(E_monitoreoAmbiental, E_alarma, []() { return input == limit_exceeded; });
  stateMachine.AddTransition(E_alarma, E_monitoreoAmbiental, []() { return input == time_out; });
  stateMachine.AddTransition(E_alarma, E_leerContrasenia, []() { return input == button; });

  stateMachine.SetOnEntering(E_leerContrasenia, outputA);
  stateMachine.SetOnEntering(E_systemaBloqueado, outputB);
  stateMachine.SetOnEntering(E_configuracion, outputC);
  stateMachine.SetOnEntering(E_alarma, outputF);
  stateMachine.SetOnEntering(E_monitoreoAmbiental, outputD);
  stateMachine.SetOnEntering(E_monitoreoEventos, outputE);
  
     
  stateMachine.SetOnLeaving(E_leerContrasenia,[]() {Serial.println("Leaving A"); 
                                                    TaskSecurity.Stop(); 
                                                    apagarLeds(); });
  stateMachine.SetOnLeaving(E_systemaBloqueado,[]() {Serial.println("Leaving B"); });
  stateMachine.SetOnLeaving(E_configuracion, []() {Serial.println("Leaving C"); 
                                                    lcd.clear();
                                                    executeMenu= false;});
  stateMachine.SetOnLeaving(E_alarma,[]() {Serial.println("Leaving F"); 
                                          lcd.clear();     
                                           apagarLeds(); 
                                           TaskTimer3.Stop(); 
                                           TaskAlarma.Stop(); }); 
  stateMachine.SetOnLeaving(E_monitoreoAmbiental,[]() {Serial.println("Leaving D");
                                                      lcd.clear(); 
                                                      TaskActualizarMonAmbiental.Stop();
                                                      TaskTimer1.Stop();
                                                      TaskAlarmaMonAmbiental.Stop();});
  stateMachine.SetOnLeaving(E_monitoreoEventos,[]() {Serial.println("Leaving E");
                                                      TaskActualizarMonHall.Stop(); 
                                                      TaskTimer2.Stop();
                                                      TaskAlarmaMonHall.Stop();});   
                                  
  

  
}

LiquidLine welcome_line1(0, 0, "Temp alto");

void setup() {
  lcd.begin(16,2);
  pinMode(buzzerPin,OUTPUT);
  Serial.begin(9600);
  Serial.println(F("Comienzo!"));
  pinMode(ledRojo,LOW);
  pinMode(ledVerde,LOW);
  pinMode(ledAzul,LOW);
  
  dht.begin();
  TaskBuzzerError.Start();
  TaskBuzzerCorrecto.Start(); 
  TaskShowGreen.Start();
  TaskShowRed.Start();
  TaskShowBlue.Start();
  //TaskBackward.Start();
  setupStateMachine();
  
  iniciarTareasMonAmbiental();
  stateMachine.SetState(E_leerContrasenia, false, true);

    opt1.set_asProgmem(1);
    opt2.set_asProgmem(1);
    opt3.set_asProgmem(1);
    opt4.set_asProgmem(1);
    opt5.set_asProgmem(1);
    opt6.set_asProgmem(1);
    //opt7.set_asProgmem(1);
    //opt8.set_asProgmem(1);
    opt9.set_asProgmem(1);
    //opt10.set_asProgmem(1);

    opt1.set_focusPosition(Position::LEFT);
    opt2.set_focusPosition(Position::LEFT);
    opt3.set_focusPosition(Position::LEFT);
    opt4.set_focusPosition(Position::LEFT);
    opt5.set_focusPosition(Position::LEFT);
    opt6.set_focusPosition(Position::LEFT);
    //opt7.set_focusPosition(Position::LEFT);
    //opt8.set_focusPosition(Position::LEFT);
    opt9.set_focusPosition(Position::LEFT);
    //opt10.set_focusPosition(Position::LEFT);

    opt1.attach_function(enter, gotoMonAmbiental);
    opt2.attach_function(enter, reiniciarUmbrales);
    opt3.attach_function(enter, setTempHigh);
    opt4.attach_function(enter, setTempLow);
    opt5.attach_function(enter, setLightHigh);
    opt6.attach_function(enter, setLightLow);
    //opt7.attach_function(enter, setHumHigh);
    //opt8.attach_function(enter, setHumLow);
    opt9.attach_function(enter, setGasHigh);
    //opt10.attach_function(enter, setGasLow);

    menu.add_screen(scr1);
    menu.add_screen(scr2);
    menu.add_screen(scr3);
    //menu.add_screen(scr4);
    menu.add_screen(scr5);
    
    menu.set_focusedLine(1);
     btn.attachClick(manejoClick);
    
}

void loop() {
  // Wait a few seconds between measurements.
 
   TaskBoton.Update();
    if (executeMenu)
    {
      myMenu();
    }
    
    if(TaskActualizarMonAmbiental.IsActive()){
      TaskActualizarMonAmbiental.Update();
    }
    if(TaskActualizarMonHall.IsActive()){
      TaskActualizarMonHall.Update();
    }
    if(TaskAlarma.IsActive()){
      //Serial.print("i'm here");
      TaskAlarma.Update();
    }
     btn.tick();
    //input = static_cast<Input>(readInput());
    stateMachine.Update();
    
}

void leerContrasenia(){
  intentos = 0;
  timer1 = 10;
  apagarLeds();
  Serial.println(F("Ingrese la contraseña:"));
  limpiarlcd();
  lcd.setCursor(0, 0);
  //lcd.print("Contrasenia:");
  while (intentos<3 && input != out_of_tries){
      lcd.setCursor(0, 0);
      lcd.print("Contrasenia:");
      lcd.setCursor(15, 0);
      lcd.print(intentos);
      j=0;
      lcd.setCursor(j, 1);
      while (j<7 && input != out_of_tries){
        char key = keypad.getKey();
        if(key){
          restartTask(TaskSecurity);
          password_us[j]= key;
          Serial.print(key);
          lcd.print("*");
          j++;
        }
        TaskSecurity.Update();
      }
      if(strncmp(password,password_us,7)==0){
        Serial.println("Clave correcta");
        contrasenia = true; 
        intentos = 4;
        TaskShowGreen.Update();
        TaskBuzzerCorrecto.Update();
        lcd.setCursor(0,1);
        lcd.print("Clave correcta");
        limpiarlcd();
        vaciarArreglo();
        input = Input::correct_password;
        
      } 
      else{
          Serial.println("Clave incorrecta");
          intentos++;
          TaskShowRed.Update();
          TaskBuzzerError.Update();
          lcd.setCursor(0,1);
          lcd.setCursor(15, 0);
          lcd.print(intentos);
          lcd.setCursor(0,1);
          lcd.print("Clave incorrecta");
          
          limpiarlcd();
      }
    }
    if(!contrasenia){
        input = Input::out_of_tries;
    }
  
}

void sistemaBloqueado(){
  Serial.println("Sistema Bloqueado");
  
  intentos = 0;
  contrasenia = false;  
  TaskBuzzerError.Update(); 
  while(timer1>=0){
    TaskShowRed.Update();
    lcd.setCursor(0,0);
    lcd.print("Sis bloqueado");
    lcd.setCursor(0,1);
    lcd.print( "Reinicio en ");
    lcd.setCursor(12,1);
    lcd.print( timer1);
    delay(1000);
    timer1--;
    lcd.clear();
  }
  lcd.clear();
  input = Input::wait_ended;
}

void readLight(){
  lcd.setCursor(0, 0);
  long prevTime = micros();
  outputValue = analogRead(photocellPin);
  averageLuz.addValue(outputValue);
  //Serial.println(outputValue);
  long currentTime = micros();
  lcd.print("Luz:");
  lcd.setCursor(5, 0);
  lcd.print(outputValue);
 
}

void readGas (){
    lcd.setCursor(0, 0);
    long prevTime = micros();
    gas = analogRead(gasPin);
    averageGas.addValue(gas);
    long currentTime = micros();
    lcd.print("Gas:");
    lcd.setCursor(5, 0);
    lcd.print(gas);
}

void readTem(){
    long prevTime = micros();
    float t = dht.readTemperature();
    averageTem.addValue(t);  
    long currentTime = micros();
    lcd.setCursor(0, 1);
    lcd.print("Tem:");
    lcd.setCursor(5, 1);
    lcd.print((int) t);
}

void readHum(){
    long prevTime = micros();
    float h = dht.readHumidity();
    averageHum.addValue(h);
     long currentTime = micros();
    lcd.setCursor(9, 1);
    lcd.print("Hum:");
    lcd.setCursor(14, 1);
    lcd.print(h);

}

void playSonidoError(){
  for(int i=200; i<=300; i++){
    tone(buzzerPin,i);
    delay(1);
  }
  delay(1);
  for(int i=800; i>=700; i--){
    tone(buzzerPin,i);
    delay(10);
  }
  delay(10);
  noTone(buzzerPin);
  apagarLeds();
}
void playSonidoCorrecto(){
  for(int i=800; i<=900; i++){
    tone(buzzerPin,i);
    delay(1);
  }
  delay(1);
  for(int i=1100; i>=1500; i--){
    tone(buzzerPin,i);
    delay(1);
  }
 
  delay(10);
  noTone(buzzerPin);
  
}

  void showGreenLed(){
    pinMode(ledRojo,LOW);
    pinMode(ledVerde,HIGH);
    pinMode(ledAzul,LOW);
    delay(200);
  }

void showRedLed(){
   pinMode(ledRojo,HIGH);
    pinMode(ledVerde,LOW);
    pinMode(ledAzul,LOW);
  delay(200);

}

void showBlueLed(){
   pinMode(ledRojo,LOW);
    pinMode(ledVerde,LOW);
    pinMode(ledAzul,HIGH);
  delay(200);
}
void configuracion(){

  Serial.println("Configuracion");
  
  lcd.setCursor(0,0);
  lcd.print("Mon_Ambiental");
  lcd.setCursor(0,1);
  lcd.print("Limites");
  
}

void updateMonAmbiental(){
  TaskLuz.Update();
  TaskTemperatura.Update();
  TaskHumedad.Update();
   TaskBoton.Update();
  //TaskBackward.Update();
  TaskTimer1.Update();
  TaskAlarmaMonAmbiental.Update();
}

/*void goBackWard(){
  char key = keypad.getKey();
  if(key){
      if(strncmp(backward,key,1)==0){
        input = Input::Backward;
      }
  }
}*/




void outputA()
{
  Serial.println("A   B   C   D   E   F");
  Serial.println("X                    ");
  Serial.println();
  TaskSecurity.Start();
  leerContrasenia();
  //TaskShowGreen.Start();
  //TaskShowRed.Start();
}

void outputB()
{
  Serial.println("A   B   C   D   E   F");
  Serial.println("    X                ");
  Serial.println();
  //TaskShowRed.Start();
  sistemaBloqueado(); 
}

void outputC()
{
  
  executeMenu= true;
  Serial.println("A   B   C   D   E   F");
  Serial.println("        X            ");
  Serial.println();
  TaskBoton.Start();
  input = Input::Unknown;
  menu.update();
  //myMenu();
}

void outputD()
{

  Serial.println("A   B   C   D   E   F");
  Serial.println("            X        ");
  Serial.println();
  input = Input::Unknown;
  TaskActualizarMonAmbiental.Start();
  TaskTimer1.Start();
  TaskAlarmaMonAmbiental.Start();
}

void outputE()
{
  
  Serial.println("A   B   C   D   E   F");
  Serial.println("                X    ");
  Serial.println();
  input = Input::Unknown;
  TaskActualizarMonHall.Start();
  TaskTimer2.Start();
  TaskAlarmaMonHall.Start();
}

void outputF()
{
  Serial.println("A   B   C   D   E   F");
  Serial.println("                    X");
  Serial.println();
  //TaskShowBlue.Start();
  TaskTimer3.Start();
  //alarma();
  TaskAlarma.Start();
}

void limpiarlcd(){
  delay(500);
  lcd.clear();
}



void iniciarTareasMonAmbiental(){
  TaskLuz.Start();
  TaskGas.Start();
  TaskTemperatura.Start();
  TaskHumedad.Start();
}

void myMenu()
{
  //menu.update();
    TaskBoton.Update();
  char key = keypad.getKey();
  //Serial.println(key);
 switch (key){
    case 'B': 
      menu.next_screen();
      break;
    case 'A':
      menu.previous_screen();
      break;
    case '#':
      menu.call_function(enter);
      break;
    case '*':
      menu.switch_focus(); // Cambia el foco a la segunda línea
      menu.update(); // Actualiza el menú para reflejar el cambio de foco
      break;
  };
 
}
void printLineValue(const byte spaces, const char *msg, const byte spacesValue, const int value)
{
    lcd.clear();
    lcd.setCursor(spaces, 0);
    lcd.cursor();
    lcd.print(msg);
    lcd.setCursor(spacesValue, 1);
    lcd.cursor();
    lcd.print(value);
}

void clearLine(int line)
{
    lcd.setCursor(0, line);
    lcd.cursor();
    for (int i = 0; i < 16; i++)
    {
        lcd.print(" ");
    }
    lcd.setCursor(0, line); // Vuelve a colocar el cursor al inicio de la línea
}


//---Funciones para establecer limites

void setTempHigh()
{
    const byte posStart = 6;
    const byte spaces = 3;
    printLineValue(spaces, "Temp High", posStart, temp_high);
    setLimite(temp_high,1);
    //setSensorLimit(temp_high, posStart, temp_low, 50);
}

void setTempLow()
{
    const byte posStart = 6;
    const byte spaces = 3;
    printLineValue(spaces, "Temp Low", posStart, temp_low);
    setLimite(temp_low,1);
    //setSensorLimit(temp_low, posStart, -50, temp_high);
}

void setLightHigh()
{
    const byte posStart = 6;
    const byte spaces = 3;
    printLineValue(spaces, "Light High", posStart, luz_high);
    setLimite(luz_high,2);
    //setSensorLimit(luz_high, posStart, luz_low, 1000);
}

void setLightLow()
{
    const byte posStart = 6;
    const byte spaces = 3;
    printLineValue(spaces, "Light Low", posStart, luz_low);
    setLimite(luz_low,2);
    //setSensorLimit(luz_low, posStart, 0, luz_high);
}

void setHumHigh()
{
    const byte posStart = 6;
    const byte spaces = 3;
    printLineValue(spaces, "Hum High", posStart, hum_high);
    setLimite(hum_high,1);

    //setSensorLimit(hum_high, posStart, hum_low, 100);
}

void setHumLow()
{
    const byte posStart = 6;
    const byte spaces = 3;
    printLineValue(spaces, "Hum Low", posStart, hum_low);
    setLimite(hum_low,1);

    //setSensorLimit(hum_low, posStart, 0, hum_high);
}

void setGasHigh()
{
    const byte posStart = 6;
    const byte spaces = 3;
    printLineValue(spaces, "Gas High", posStart, gas_high);
    setLimite(gas_high,2);

    //setSensorLimit(gas_high, posStart, gas_low, 2000);
}



void reiniciarUmbrales(){
//umbrales por defecto
  temp_high= 25; 
  temp_low = 20;
  luz_high = 190;
  luz_low = 50;
  hum_high = 100;
  hum_low =12;
  gas_high = 1000;
  gas_low =12;
  Serial.println("Los limites han sido reiniciados");

}

void gotoMonAmbiental()
{
    input = Input::button;
}

void updateMonHall(){
  TaskGas.Update();
  //TaskBackward.Update();
  TaskBoton.Update();
  TaskTimer2.Update();
  TaskAlarmaMonHall.Update();
}

void manejoTimer1(){

  input = Input :: time_out;
}

void manejoTimer2(){

  input = Input :: time_out2;
}

void manejoSeguridad (){
  lcd.setCursor(0,1);
  lcd.print("Inactivity Error");
  Serial.print("Inactivity Error");
  input = Input :: out_of_tries;
  
}

void restartTask(AsyncTask& task) {
  task.Stop(); // Deshabilitar la tarea
  delay(10);      // Pequeño retraso para asegurar que se deshabilita correctamente
  task.Start();  // Habilitar la tarea nuevamente
  //Serial.println("Task restarted");
}

void monLimitesAmbiental(){

   if(averageTem.getAverage()> temp_high && averageLuz.getAverage() > luz_high ){
    Serial.println("Temperatura:");
    showLimitePromedio(temp_high,averageTem.getAverage());
    Serial.println("Luz:");
    showLimitePromedio(luz_high,averageLuz.getAverage());
    input = Input :: limit_exceeded;
  }
}

void monLimiteHall(){

   if(averageGas.getAverage() > gas_high){
    Serial.println("Gas:");
    showLimitePromedio(gas_high,averageGas.getAverage());
    input = Input:: limit_exceeded;
  }
}

void showLimitePromedio(int lim_high, int promedio){
   Serial.print("avg:");
  Serial.print(promedio);
  Serial.print(">");
   Serial.print("limit high:");
  Serial.println(lim_high);

}

void alarma(){
  lcd.setCursor(0,0);
  //Serial.print("limit exceeded");
  lcd.print("limit exceeded");
  TaskBuzzerCorrecto.Update();
  TaskShowBlue.Update();
  TaskBoton.Update();
  //TaskBackward.Update();
  TaskTimer3.Update();  
}


void apagarLeds(){
     pinMode(ledRojo,LOW);
    pinMode(ledVerde,LOW);
    pinMode(ledAzul,LOW);
   
}
void manejoTimer3(){
  input = Input :: time_out;
}

void vaciarArreglo(){
  for(int i=0; i<8;i++){
    password_us[i]= " ";
  }
}

void manejoClick(){
  Serial.println("i'm here");
  input = Input :: button;
}

void setLimite(int &sensorValue, int cifras){
  int k = 0; 
  newValue = 0;
  bool modificado = false;
  bool enviado = false;
  char key = keypad.getKey();
  while(k<= cifras){
    key = keypad.getKey();
    if(key){
      Serial.println(key);
      if(isdigit(key)){
          lcd.setCursor(6+k,1);
          lcd.print(key);
          if(k==0){
            newValue = obtenerNumber(key);
          }
          else{
            newValue = newValue*10 + obtenerNumber(key);
          }
          if(!modificado){
            vaciarLinea(7+k,1);
          }
          modificado = true;
          
        k++;
      }
      else{
        if(key =='#'){
          if(modificado){
            sensorValue = newValue;    
          }
          enviado = true;
          break;  
        }
      }
    }
  }
  if(!enviado){
    while(!key || key !='#'){
      key = keypad.getKey();
    }
    if(modificado){
      sensorValue = newValue; 
      newValue = 0;
    }
  }
}

int obtenerNumber(char key){

  switch(key)
  {
   case '0':
      return 0;
    case '1':
      return 1;
    case '2':
      return 2;
    case '3':
      return 3;
    case '4':
      return 4;
    case '5':
      return 5; 
    case '6':
      return 6;
    case '7':
      return 7;
    case '8':
      return 8;  
    case '9':
      return 9;
  };
}

void vaciarLinea(int columna, int fila){
  for(int j=columna; j<15;j++){
    lcd.setCursor(j,fila);
    lcd.print(" ");
  }
}