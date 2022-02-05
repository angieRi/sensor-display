/*
 * File:   reloj_main.c
 * Author: Angela Rios
 * PIC18F26J50
 * XC8  V 1.34
 * Created on 5 de octubre de 2021
 * 
 * ||
 */

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <delays.h>
#define _XTAL_FREQ 4000000
#include "lcd.h"
#include <rtcc.h>

#pragma config CPUDIV=OSC1
#pragma config CP0=OFF
#pragma config WDTEN=OFF  
#pragma config PLLDIV=1
#pragma config STVREN=ON
#pragma config XINST=OFF
#pragma config OSC=INTOSC
#pragma config T1DIG=ON
#pragma config LPT1OSC=OFF
#pragma config FCMEN=OFF
#pragma config IESO=OFF
#pragma config WDTPS=32768
#pragma config DSWDTOSC=INTOSCREF
#pragma config RTCOSC=T1OSCREF// RTCC Clock Select (RTCC uses T1OSC/T1CKI)
#pragma config DSBOREN=OFF
#pragma config DSWDTEN=OFF
#pragma config DSWDTPS=G2
#pragma config IOL1WAY=OFF
#pragma config MSSP7B_EN=MSK7
#pragma config WPCFG=OFF
#pragma config WPDIS=OFF

//proyecto
#define Up          PORTAbits.RA5 == 0
#define Left        PORTAbits.RA3 == 0
#define Enter       PORTAbits.RA2 == 0
#define Down        PORTAbits.RA1 == 0
#define Right       PORTAbits.RA0 == 0
#define Calef        LATAbits.LATA7 = 1
#define Frio         LATBbits.LATB6 = 1
#define Vidriera     LATBbits.LATB7 = 1
#define Alarma       LATCbits.LATC0 = 1
#define Detec_Mov    PORTCbits.RC7 = 1

unsigned char buffer1[20];
rtccTimeDate RtccTimeDate ,RtccAlrmTimeDate, Rtcc_read_TimeDate ;

char segundo_u;         //variables BCD donde se guradan o leen los
char segundo_d;         //datos del RTCC
char minuto_u;
char minuto_d;
char hora_u;
char hora_d;
char fecha_u;
char fecha_d;
char mes_u;
char mes_d;
char dia_semana;
char anio_u;
char anio_d;
int anio=21;                //año 2020 -> 00001110
int mes=10;                  //mes enero-> 00000001 (1) .... dici-> 00001100 (12)
int diasem=5;               //0 Dom , 1 Lun , 2 Mar......, 6 Sab
int dia=8;                  //dia 01 -> 00000001.....31 -> 00011111
int hora=9;                 //hora 00 -> 00000000.....24 -> 00011000
int minuto=35;               //variables de numeros enteros que permiten
int segundo=0;              //representar los valores en el LCD

rtccTime RtccTime; // Inicializa la estructura de tiempo
rtccTime RtccTimeVal;
rtccDate RtccDate;//Inicializa la estructura de Fecha

char lectura = 0;          //Lectura del conversor analogico
//
char lectura_temp = 0;  //lectura de temperatura
int config =0;      //estado de configuración,ejm 1 = configura dia de la semana
int aux_fecha =0;   //para saber si posicion es dia =1, mes=2 o año=3 
int aux_hora=0;     // para saber si posicion es hora=1 o minuto =2
int fila =0;        //es la fila donde se posiciona el cursor
int columna =0;     // Es la columna donde se posiciona el cursor
float grados= 24.0; // grado de temperatura por default
char *estacion;     // nombre de la estacion del anio
int temperatura= 24;    //temperatura programada por default
int hora_vdr_c = 22;    //hora de cierre de shoping por default es 22 horas
int minuto_vdr_c= 0;    //minutos de cierre de shoping por default es 00 minutos
int hora_vdr_a = 8;     //hora de apertura de shoping por default es 8 horas
int minuto_vdr_a= 30;   //minutos de cierre de shoping por default es 30 minutos

//funciones
void lee_ADC(void);
void Write_RTC(void);
void Read_RTC(void);
void Actualiza_datos(void);
void Iniciar_Display(void);
void Mostrar_DiaYfecha(int fila,int columna);
void Configurar_Pantalla(void);
void Conseguir_Dia(void);
void Configurar_Dia(void);
int Seleccionar_Dato(int datos,int fila,int columna,int inicio,int limite);
void Configurar_Fecha(int opcion,int aux_fecha);
void Configurar_Hora(int config,int aux_hora);
void conseguir_estacion(void);
void Programar_Temperatura(void);
void Encender_Aire_Calefaccion(void);
void Configurar_Hora_Grl(int opcion,int aux_hora,int fila_i,int columna_i,int tipo_hora);
void Encender_vidriera(void);
void Encender_Alarma(void);

/**********************************************************************************
 COMANDOS DEL LCD
***********************************************************************************
 Lcd_Cmd(0x01);             //Clear Display, Cursor al inicio
 Lcd_Cmd(0x02);             //Cursor al inicio
 Lcd_Cmd(0x00);             //Apaga Display
 Lcd_Cmd(0x08);             //Enciende Display
 Lcd_Cmd(0x0C);             //Enciende display sin cursor y sin blink
 Lcd_Cmd(0x0D);             //Enciende display sin cursor y con blink
 Lcd_Cmd(0x0E);             //Enciende display con cursor y sin blink
 Lcd_Cmd(0x0F);             //Enciende display con cursor y con blink
 Lcd_Cmd(0x06);             //Incrementa Direccion, Display fijo
 Lcd_Cmd(0x04);             //Decrementa Direccion, Display fijo
 Lcd_Cmd(0x07);             //Incrementa Direccion, Cursor fijo
 Lcd_Cmd(0x05);             //Decrementa Direccion, Cursor fijo
 Lcd_Cmd(0x10);             //Cursor a la Izquierda
 Lcd_Cmd(0x14);             //Cursor a la Derecha
 Lcd_Cmd(0x18);             //Display a la Izquierda
 Lcd_Cmd(0x1C);             //Display a la Derecha
 Lcd_Init();                //inicializa el lcd
 Lcd_Out(x,y,"string");     //posiciona cursor,fila, col, escribe sring
 Lcd_Out2(x,y,buffer1);     //posiciona cursor,fila, col, escribe dato
 Lcd_Chr_CP(c);             //escribe 1 caracter en el cursor
 *********************************************************************************/

void main()
{
OSCTUNEbits.INTSRC=1;           //setea el oscilador de 32768 para el RTC
OSCTUNEbits.PLLEN=0;            //desactiva PLL
OSCCONbits.IRCF0=1;             //selecciona el clock en 8MHz
OSCCONbits.IRCF1=1;
OSCCONbits.IRCF2=1;
OSCCONbits.SCS0=0;              //oscilator INTRC
OSCCONbits.SCS1=0;

RTCCFGbits.RTCEN=1;             //Seteos del RTCC
RTCCFGbits.RTCWREN=1;
T1CONbits.T1OSCEN=1;
    
TRISA = 0b01111111;             //A7...A2 entradas, A1 y A0 salidas
TRISB = 0;                      //Todas salidas
TRISC = 0b11111110;             //Todas entradas
ANCON0 = 0b11111111;            //Entradas digitales
ANCON1 = 0b11110111;            //Entradas AN11 analógica, el resto digitales
ADCON0 = 0b00101111;            //Ref:0-5,5 v Conversor apunta a A11
ADCON1 = 0b00111001;            //Justif izquierda, tiempo adquisicion 20TAD y FOSC/8

Write_RTC();                    //Escribe los valores por defecto en el RTCC
Lcd_Init();
Lcd_Cmd(LCD_CLEAR);
Lcd_Cmd(LCD_CURSOR_OFF);
config =0;

while(1){
    
    if(config == 0){
        Iniciar_Display();
    }
    Configurar_Pantalla(); 
    __delay_ms(100);              //Corresponde a un tiempo real de 100ms
    }
}
//--- inicio de funciones 

//Inicia el display con datos de fecha, Hora, temperatura,estacion del anio horario de encendido de vidriera, alarma y frio-calor
void Iniciar_Display(void)
{
    Actualiza_datos();
    Mostrar_DiaYfecha(1,0);  
    
    sprintf(buffer1,"%02u:%02u:%02u ",hora,minuto,segundo);
    Lcd_Out(2,0,buffer1);
    
    conseguir_estacion();
    
    sprintf(buffer1,"%s ",estacion);
    Lcd_Out(2,12,buffer1);
    sprintf(buffer1,"TempPro:%02u ",temperatura);
    Lcd_Out(3,0,buffer1);
    lectura_temp = lectura;
    grados = 0.196*lectura_temp;
    sprintf(buffer1,"Med:%2.1f ",grados);
    Lcd_Out(3,12,buffer1);
    
    Encender_Aire_Calefaccion();
    
    sprintf(buffer1,"Vidriera:%02u:%02u-%02u:%02u", hora_vdr_c,minuto_vdr_c,hora_vdr_a,minuto_vdr_a);
    Lcd_Out(4,0,buffer1);
    
    Encender_vidriera();
    Encender_Alarma();
}
/** Muestra en display dia y fecha , diasem es el dia que muestra **/
void Mostrar_DiaYfecha(int fila, int columna)
{
   switch(diasem) {
        case 0:
           sprintf(buffer1,"Domingo %02u/%02u/%02u",dia,mes,anio);
           break;
           
        case 1:
            sprintf(buffer1,"Lunes %02u/%02u/%02u",dia,mes,anio);
            break;

        case 2:
           sprintf(buffer1,"Martes %02u/%02u/%02u",dia,mes,anio);
           break;
           
        case 3:
           sprintf(buffer1,"Miercoles %02u/%02u/%02u",dia,mes,anio);
           break;
           
        case 4:
           sprintf(buffer1,"Jueves %02u/%02u/%02u",dia,mes,anio);
           break;
           
        case 5:
           sprintf(buffer1,"Viernes %02u/%02u/%02u",dia,mes,anio);
           break;
        
        case 6:
           sprintf(buffer1,"Sabado %02u/%02u/%02u",dia,mes,anio);
           break;       
   }
    Lcd_Out(fila,columna,buffer1);  
}

/**Configura la pantalla, el usuario busca la opcion con enter y elige con Up y Down 
 config es una opcion de la configuración de pantalla, 
 config =1 configura dia de la semana
 config =2 configura la fecha
 config =3 configura la hora
 config =4 configura la temperatura programada
 config =5 configura  la vidriera hora de cierre
  config =6 configura  la vidriera hora de apertura
 si config > 6 o igual a 0 muestra la pantalla inicial, entonces sale de la pantalla de conguracion*/
void Configurar_Pantalla(void)
{ 
    if(Enter){
        config = 1;
        aux_fecha=0;
        aux_hora=0;
        if(aux_fecha > 3){
            aux_fecha =1;
        }
        if(aux_hora > 2){
            aux_hora =1;
        }
        
        Lcd_Cmd(LCD_CLEAR);
        Read_RTC();
        Lcd_Out(1,0,"CONFIGURAR SHOPPING");
        sprintf(buffer1,"%02u/%02u/%02u",dia,mes,anio);
        Lcd_Out(2,10,buffer1);
        sprintf(buffer1,"%02u:%02u ",hora,minuto);
        Lcd_Out(3,0,buffer1);
        sprintf(buffer1,"TempPro:%02u ",temperatura);
        Lcd_Out(3,7,buffer1);
        sprintf(buffer1,"Vidriera:%02u:%02u-%02u:%02u", hora_vdr_c,minuto_vdr_c,hora_vdr_a,minuto_vdr_a);
        Lcd_Out(4,0,buffer1);
            
        if(config == 1){
            
            Configurar_Dia(); 
        }
        if(config == 2){
            Configurar_Fecha(config,aux_fecha);
        }
        if(config == 3){ 
           Configurar_Hora(config,aux_hora);
        }
         if(config == 4){
            Programar_Temperatura();
        }
        if(config == 5){
            fila = 4;
            columna=9;
            int tipo_hora=1;
            Configurar_Hora_Grl(config,aux_hora,fila,columna,tipo_hora);    
        }
        if(config == 6){
            fila=4;
            columna=15;
            int tipo_hora = 2;
            Configurar_Hora_Grl(config,aux_hora,fila,columna,tipo_hora);     
        }
        if(config > 6){
            Lcd_Cmd(LCD_CLEAR);
            config = 0;
        } 
     while(Enter);
    }
}

/** modifica el día de la semana, cambia el dia con left y right.*/
void Configurar_Dia(void)
{   
    diasem=0;
    Conseguir_Dia();
    int estado = 1;
    Lcd_Out(2,0,"");
    Lcd_Cmd(LCD_BLINK_CURSOR_ON);
    while(estado > 0){

        if(Down){
            if(diasem > 0){
                diasem --;
                Conseguir_Dia();
            }else{
                Conseguir_Dia();
            }
            Lcd_Out(2,0,"");
            Lcd_Cmd(LCD_BLINK_CURSOR_ON);
            while(Down);
        }
        if(Up){
            if(diasem < 7){
                diasem++;
                Conseguir_Dia();
            }
            Lcd_Out(2,0,"");
            Lcd_Cmd(LCD_BLINK_CURSOR_ON);
            while(Up); 
        }
         if(Right){
            config=2;
            estado=0;
            Lcd_Out(2,11,""); 
            Lcd_Cmd( LCD_MOVE_CURSOR_LEFT);
            while(Right); 
        }
        if(Enter && config == 1){
            config=3;
            estado=0;
            while(Enter); 
        }
    } 
    Write_RTC();
}

/**
 * se configura la fecha si config es igual a 2 se configura el dia;
 * si aux_fecha es igual a 2 se configura el mes,
 * si aux_fecha es igual a 3 se configura el año
 */
void Configurar_Fecha(int opcion,int aux_fecha){
    
    aux_fecha++;
    int config_fecha= opcion;
    
    while(config_fecha == 2){
       
        if(config_fecha ==2 && aux_fecha == 1){//configura el dia
            Lcd_Out(2,10,"");
            fila =2;
            columna =10;
            int estado =1;

            while(estado > 0){

                int inicio=0;
                int limite=32;
                dia= Seleccionar_Dato(dia,fila,columna,inicio,limite);//funcion que cambia el dia
               
                if(Right && aux_fecha == 1){
                    aux_fecha++;
                    estado=0;
                    while(Right); 
                }
                if(Enter ){
                    config_fecha=0;
                    aux_fecha++; 
                    config++;
                    estado=0;
                    while(Enter); 
                }
            }              
        }

        if( config_fecha ==2 && aux_fecha == 2){ //configura el mes

            fila =2;
            columna =columna +3;
            Lcd_Out(fila,columna,"");
            int estado =1;
            while(estado > 0){

                int inicio =0;
                int limite =13;
                mes= Seleccionar_Dato(mes,fila,columna,inicio,limite);//funcion que cambia el mes
                if(Left){
                    aux_fecha = 1;
                    estado =0;
                    while(Left); 
                }
                if(Right && aux_fecha == 2 ){
                    aux_fecha++;
                    estado=0;
                    while(Right);
                }
                if(Enter ){
                    config_fecha=0;
                    estado=0;
                    config++;
                    while(Enter); 
                }   
            }
        }

        if(config_fecha == 2 && aux_fecha == 3){//configura el anio
            fila =2;
            columna =columna +3;
            Lcd_Out(fila,columna,"");         
            int estado =1;
            while(estado > 0){

                int inicio=0;
                int limite=100;
                anio= Seleccionar_Dato(anio,fila,columna,inicio,limite);//funcion que cambia el anio
                if(Left){
                    fila =2;
                    columna =columna -6;
                    aux_fecha=2;
                    estado=0;
                    while(Left); 
                }
                if(Enter ){
                    config_fecha=0;
                    config++;
                    estado=0;
                    while(Enter); 
                }
            }
        }
    }
    Write_RTC();
}

/**
 * Si config es igua a 3 , cambia la hora y minutos del reloj;
 * si aux_hora es igual a 1 entonces configura la hora,
 * si aux_hora es igual a 2 entonces configura los minutos.
 */
void Configurar_Hora(int opcion,int aux_hora)
{
    int config_hora=opcion;
    aux_hora++; 
    while(config_hora==3){
        
        if(config_hora ==3 && aux_hora == 1){//configura la hora que muestra en pantalla
            Lcd_Out(3,0,"");
            fila =3;
            columna =0;
            int estado =1;
            while(estado > 0){

                int inicio= -1;
                int limite=24;
                hora= Seleccionar_Dato(hora,fila,columna,inicio,limite);//funcion que cambia la hora
                if(Right && aux_hora == 1){
                    aux_hora++; 
                    estado=0;
                   while(Right); 
                }
                if(Enter ){
                    aux_hora=3;
                    Lcd_Cmd(0x0C);
                    estado=0;
                    config = 5;
                    config_hora=0;
                    while(Enter); 
                }
            }              
        }
        if( config_hora == 3 && aux_hora == 2){//configura los minutos que muestra en pantalla

            fila =3;
            columna =columna +3;
            Lcd_Out(fila,columna,"");
            int estado =1;
            while(estado > 0){

                int inicio= -1;
                int limite=60;
                minuto= Seleccionar_Dato(minuto,fila,columna,inicio,limite);//funcion que cambia los minutos
                if(Left){
                    aux_hora = 1;
                    estado =0;
                    while(Left); 
                }
                if(Right){
                    aux_hora++;
                    config = 4;
                    estado =0;
                    Lcd_Out(3,15,""); 
                    config_hora = 0;
                    while(Right); 
                }
                if(Enter ){
                    aux_hora++;
                    config = 5;
                    Lcd_Cmd(0x0C);
                    estado=0;
                    config_hora=0;
                    while(Enter); 
               }
            }  
        } 
    }
    Write_RTC();
}

/**
 * @return dato
 * datos a modificar,el valor del dato aumenta con Up y decrece con Down, inicio y limite es desde y hasta donde se le permite aumentar el dato.
 * fila y columna es la ubicacion en pantalla donde se muestra el dato.
 */
int Seleccionar_Dato(int datos,int fila,int columna,int inicio,int limite)
{
    int dato =datos;
    if(Up){
        dato ++;
        if(dato > inicio && dato < limite){  
            
            sprintf(buffer1,"%02u",dato);
            Lcd_Out(fila,columna,buffer1); 
            Lcd_Out(fila,columna,"");
            Lcd_Cmd( LCD_BLINK_CURSOR_ON);
        }else{
            dato--;
        }
        while(Up);
    }
    if(Down){
        dato -- ;
        if(dato > inicio && dato < limite){
            
            sprintf(buffer1,"%02u",dato);
            Lcd_Out(fila,columna,buffer1); 
            Lcd_Out(fila,columna,"");
            Lcd_Cmd( LCD_BLINK_CURSOR_ON);
        }else{
            dato++;
        }
        while(Down);
    }
    return dato;
}

/** Muestra el dia de la semana */
void Conseguir_Dia(void)
{
    switch(diasem) 
    {
        case 0:
           sprintf(buffer1,"Domingo  ");
           break;
        case 1:
            sprintf(buffer1,"Lunes   ");
            break;
        case 2:
           sprintf(buffer1,"Martes   ");
           break;
        case 3:
           sprintf(buffer1,"Miercoles");
           break;
        case 4:
           sprintf(buffer1,"Jueves   ");
           break;
        case 5:
           sprintf(buffer1,"Viernes  ");
           break;
        case 6:
           sprintf(buffer1,"Sabado   ");
           break;
    }
    Lcd_Out(2,0,buffer1); 
}
/*muestra la estacion Verano(desde el 21 de septiembre al 21 de marzo) o Invierno */
void conseguir_estacion(void)
{
    if(hora >= 0 && minuto >=0){
        if(mes == 9 && dia > 20){
            estacion = "Verano";   
        }
        if(mes == 3 && dia < 22)
        {
            estacion = "Verano";
        }
        if(mes > 8 || mes < 3){
            estacion = "Verano";
        }else{
            estacion = "Invierno";   
        }
    }  
}
/*Funcion que permite configurar la temperatura programada*/
void Programar_Temperatura(void)
{
    fila =3;
    columna =15;
    Lcd_Out(fila,columna,"");
    int estado =1;
    while(estado == 1){
        
        temperatura = Seleccionar_Dato(temperatura,fila,columna,10,41);
        if(Right){
            estado =0;
            while(Right); 
        }
        if(Enter ){
            config ++;
            estado=0;
            while(Enter); 
        }
    }
}
/*funcion que permite encender el aire frio o calefaccion dependiendo 
 * de la temperatura programada y de condicion de la temperatura que marca sensor de temperatura,
 grados es la temperatura que se obtiene del sensor y temperatura es la temperatura programada, para encender el aire frio la temperatura
 debe ser mayor 2 grados de la temperatura programada, para encender la calefaccion la temperatura debe estar 3 grados menos de la temperatura programada*/
void Encender_Aire_Calefaccion(void)
{
    float aux_temp = (float)temperatura;
    if(grados > aux_temp +2){
        Frio == 1;   
    }
    if(grados <= aux_temp){
        Frio == 0;
    }
    if(grados < aux_temp - 3){
        Calef == 1;
        Frio == 0;    
    }
    if(grados >= aux_temp){
        Calef == 0;
    }
}


/*Configura la hora en gral de cualquier reloj de la pantalla*/
void Configurar_Hora_Grl(int opcion,int aux_hora,int fila_i,int columna_i,int tipo_hora)
{
    int config_hora=opcion;
    aux_hora++;
    Lcd_Cmd( LCD_BLINK_CURSOR_ON);
    while(config_hora==opcion){      
        if(aux_hora == 1){//configura la hora que muestra en pantalla
            Lcd_Out(fila_i,columna_i,"");
            fila =fila_i;
            columna =columna_i;
            int estado =1;

            while(estado > 0){
                
                int inicio= -1;
                int limite=24;
          
                if(tipo_hora==1){
                    hora_vdr_c= Seleccionar_Dato(hora_vdr_c,fila_i,columna_i,inicio,limite);//funcion que cambia la hora
                }
                if(tipo_hora==2){
                    hora_vdr_a= Seleccionar_Dato(hora_vdr_a,fila_i,columna_i,inicio,limite);//funcion que cambia la hora
                }
                
                if(Right && aux_hora == 1){
                    aux_hora++; 
                    estado=0;
                   while(Right); 
                }
                if(Enter ){
                    aux_hora=3;
                    Lcd_Cmd(0x0C);
                    estado=0;
                    config ++;
                    config_hora=0;
                    while(Enter); 
                }
            }
        }
        if( aux_hora == 2){//configura los minutos que muestra en pantalla

            fila = fila_i;
            columna =columna_i +3;
            Lcd_Out(fila,columna,"");
            int estado =1;
            while(estado > 0){

                int inicio= -1;
                int limite=60;
                
                if(tipo_hora==1){
                    minuto_vdr_c= Seleccionar_Dato(minuto_vdr_c,fila,columna,inicio,limite);//funcion que cambia la hora
                }
                if(tipo_hora==2){
                    minuto_vdr_a= Seleccionar_Dato(minuto_vdr_a,fila,columna,inicio,limite);//funcion que cambia la hora
                }
                if(Left){
                    aux_hora = 1;
                    estado =0;
                    while(Left); 
                }
                if(Right){
                    estado =0;
                    if(tipo_hora==1){
                        Lcd_Out(fila,columna+3,"");
                        aux_hora=0;
                    }
                    config++;
                    Lcd_Cmd(0x0C);
                    config_hora = 0;
                    while(Right); 
                }
                if(Enter ){
                    aux_hora++;
                    config = config +2;
                    Lcd_Cmd(0x0C);
                    estado=0;
                    config_hora=0;
                    while(Enter); 
               }
            } 
        }
    }
    Write_RTC();
}
/**Enciende la vidriera si la hora es igual a las hora de cierre  y menor a la hora de apertura ,
 * si está encendida la vidriera entonces enciende el detector de movimiento si no es asi apaga vidriera y detector de movimento*/
void Encender_vidriera(void)
{
    if(hora_vdr_c < hora || hora < hora_vdr_a ){
         Vidriera == 1;
         Detec_Mov == 1;
    }else{
        Vidriera == 0;
        Detec_Mov == 0;    
    }
    if(hora_vdr_c == hora){
        if(minuto_vdr_c <= minuto){
           Vidriera == 1;
           Detec_Mov == 1;  
        }else{
            Vidriera == 0;
            Detec_Mov == 0;
        }
    }
    if( hora == hora_vdr_a){
        if(minuto < minuto_vdr_a){
            Vidriera == 1;
            Detec_Mov == 1;
        }else{
            Vidriera == 0;
            Detec_Mov == 0;
        } 
    }  
}
/** Verifica que el detector de movimiento esta activado entonces enciende las alarmas*/
void Encender_Alarma(void)
{
    if(Detec_Mov){
        Alarma == 1;
    }else{
        Alarma == 0;
    }
} 
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
// Funcion Write_RTC
// Toma datos de los registros correspondientes(diasem, anio, dia, hora, etc)
// y los carga en el módulo RTCC del micro
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[

void Write_RTC(void)
{
   RtccWrOn();                     //write enable the rtcc registers
   //mRtccSetClockOe(1);            //saca hacia afuera el pulso(ver patita)RTCC output pin
   PIE3bits.RTCCIE=1;
   segundo_d=segundo/10;
   segundo_u=segundo-segundo_d*10;
   RtccTime.f.sec =segundo_d*16+segundo_u;  //guarda segundo en rtcc
   minuto_d=minuto/10;
   minuto_u=minuto-minuto_d*10;
   RtccTime.f.min =minuto_d*16+minuto_u;    //guarda minuto en rtcc
   hora_d=hora/10;
   hora_u=hora-hora_d*10;
   RtccTime.f.hour=hora_d*16+hora_u;        //guarda hora en rtcc
   anio_d=anio/10;
   anio_u=anio-anio_d*10;
   RtccDate.f.year=anio_d*16+anio_u;        //guarda año en rtcc
   mes_d=mes/10;
   mes_u=mes-mes_d*10;
   RtccDate.f.mon=mes_d*16+mes_u;         //guarda mes en rtcc
   fecha_d=dia/10;
   fecha_u=dia-fecha_d*10;
   RtccDate.f.mday=fecha_d*16+fecha_u;      //guarda dia (numero)en rtcc
   dia_semana=diasem;
   RtccDate.f.wday =diasem;                 //guarda dia (domingo ... sabado)

   RtccWriteTime(&RtccTime,1);      //write into registers
   RtccWriteDate(&RtccDate,1);      //write into registers
   mRtccOn();                       //habilita rtcc
}

//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
// Funcion Read_RTC
// Permite capturar los datos del RTC y cargarlos en los registros correspondientes
// (diasem, anio, dia, hora, etc)
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[

void Read_RTC(void)//los digitos del RTCC estan en formato BCD hay que particionar
                    //la lectura de la estructura RtccTime (cada elemento en 2 nibles)
  {
    RtccReadTime(&RtccTime);                //Lee estructura de la hora del RTCC
    segundo_u=(RtccTime.f.sec & 0x0F);      //lee la parte baja en BCD
    segundo_d=((RtccTime.f.sec & 0xF0)>>4); //lee la parte alta en BCD
    minuto_u=(RtccTime.f.min & 0x0F);
    minuto_d=((RtccTime.f.min & 0xF0)>>4);
    hora_u=(RtccTime.f.hour & 0x0F);
    hora_d=((RtccTime.f.hour & 0xF0)>>4);
    RtccReadDate(&RtccDate);                //Lee estructura del la Fecha del RTCC
    fecha_u=(RtccDate.f.mday & 0x0F);
    fecha_d=((RtccDate.f.mday & 0xF0)>>4);
    mes_u=(RtccDate.f.mon & 0x0F);
    mes_d=((RtccDate.f.mon & 0xF0)>>4);
    dia_semana=RtccDate.f.wday;
    anio_u=(RtccDate.f.year & 0x0F);
    anio_d=((RtccDate.f.year & 0xF0)>>4);
    segundo=segundo_d*10+segundo_u;
    minuto=minuto_d*10+minuto_u;
    hora=hora_d*10+hora_u;
    dia=fecha_d*10+fecha_u;
    diasem=dia_semana;
    mes=mes_d*10+mes_u;
    anio=anio_d*10+anio_u;
    }

//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/// Funcion lee_temperatura
/// Mide la tensión analógica y transfiere un valor a la variable lectura
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[

void lee_ADC(void)
{
    ADCON0bits.ADON = 1;
    ADCON0bits.GO = 1;
    while(ADCON0bits.GO);
    __delay_ms(1);
    lectura = ADRESH;
    ADCON0bits.ADON = 0;
}

//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/// Funcion Actualiza_datos
/// Cada 900ms lee RTCC y entrada analógica A11
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
void Actualiza_datos(void)
{
    Read_RTC();
    lee_ADC();
    int   i;
    i++;
    if(i>=9)
        {
        Encender_Aire_Calefaccion();
        i=0;
        }
}


