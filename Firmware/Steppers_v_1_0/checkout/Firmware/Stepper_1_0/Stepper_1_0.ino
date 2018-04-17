#include <stepperCore.h>

//###################################################################
#define IF 40000 
#define IT 25
#define MinRPM 1
#define MaxRPM 300
#define KCLimit 1200

//###################################################################
uint8_t MotorLed[6]	= {14, 15, 16, 17, 18, 19};
//Variables de control del motor
uint8_t MotorPin[6]	= {27, 26, 25, 50, 49, 48};
uint8_t MotorDir[6]	= {24, 23, 22, 53, 52, 51};
uint8_t MotorEn[6]	= {30, 29, 28, 47, 46, 45};
//Switchs (finales de carrera)
uint8_t MotorS0[6]	= {12, 10, 8, A4, A2, A0};
uint8_t MotorS1[6]	= {13, 11, 9, A5, A3, A1};
//Botones (indicar manualmente la direccion)
uint8_t MotorPinBtn0[6] = {33, 35, 37, 39, 41, 43};
uint8_t MotorPinBtn1[6] = {32, 34, 36, 38, 40, 42};
//Potenc. (indicar manualmente la velocidad)
uint8_t MotorPinVel[6] = {A6, A7, A8, A9, A10, A11};
//###################################################################
typedef struct
{
	//Variables de velocidad
	uint32_t	TicsPerCom	= 100;				//Tics de interrupcion hasta conmutar (para tener cierta velocidad en RPM)
	uint8_t		StepsPerRev = 200;				//Pasos por revolucion
	//Variables de tipo de movimiento
	uint8_t		MovementMod = 0;				//Modo de movimiento: 0: M, 1: MR, 2: ML, 3:Cal
	//Posicionamiento
	uint32_t	StepRange 	= 0;				//Rango: # de pasos de todo el trayecto --> Se obtiene por calibracion
	uint32_t	StepPos 	= 0;				//Posicion actual en pasos 		--> PP <= RG
	uint32_t 	StepTarget 	= 0;				//Posicion de destino en pasos 	--> DP <= RG
	//Contadores auxiliades
	uint32_t	TicCount 	= 0; 				//Contador de tics
	uint8_t		State 		= 0;
	//Señales de control
	boolean 	S 	= false;			//Pulse --> Senal de control
	boolean 	E	= true;				//Enable: 		1 off, 0 on
	boolean		D	= false;			//Direccion:	0 acc, 1 cc	
} MotorData;
MotorData Motor[6];

uint32_t KeyCheck = 0;
uint8_t ICount 	= 0;
uint8_t LCount 	= 0;
//###################################################################
void setup()
{
	for(int LCount = 0; LCount < 6; LCount++)
	{
		pinMode(MotorPin[LCount], OUTPUT);
		pinMode(MotorLed[LCount], OUTPUT);
		pinMode(MotorDir[LCount], OUTPUT);
		pinMode(MotorEn[LCount],  OUTPUT);
		pinMode(MotorS0[LCount],  INPUT);
		pinMode(MotorS1[LCount],  INPUT);
	    pinMode(MotorPinBtn0[LCount], INPUT_PULLUP);
	    pinMode(MotorPinBtn1[LCount], INPUT_PULLUP);
	}
	for(int LCount = 0; LCount < 6; LCount++)
	{
		digitalWriteDirect(MotorPin[LCount], Motor[LCount].S);
		digitalWriteDirect(MotorDir[LCount], Motor[LCount].D);
		digitalWriteDirect(MotorEn[LCount], 	Motor[LCount].E);
		digitalWriteDirect(MotorLed[LCount], !Motor[LCount].E);
	}
	Timer3.attachInterrupt(IT_Handler);
	Timer3.start(IT);
}

//###################################################################
void loop()
{
	if(KeyCheck > (KCLimit-1))
	{
		KeyCheck = 0;
		for(int LCount = 0; LCount < 6; LCount++)
		{
			if(!digitalReadDirect(MotorPinBtn0[LCount]))
			{
				Motor[LCount].TicsPerCom = Rpm2ComTics(map(analogRead(MotorPinVel[LCount]),0,1023,MinRPM,MaxRPM),Motor[LCount].StepsPerRev); 
				Motor[LCount].E = false;
				Motor[LCount].D = false;
			}
			else if(!digitalReadDirect(MotorPinBtn1[LCount]))
			{
				Motor[LCount].TicsPerCom = Rpm2ComTics(map(analogRead(MotorPinVel[LCount]),0,1023,MinRPM,MaxRPM),Motor[LCount].StepsPerRev); 
				Motor[LCount].E = false;
				Motor[LCount].D = true;
			}
			else
			{
				Motor[LCount].E = true;
			}
		}
	}
}

//###################################################################
void IT_Handler()
{
	KeyCheck++;
	for(int ICount = 0; ICount < 6; ICount++)
	{
		switch(Motor[ICount].State)
		{
			case 0:		//Stand-by
			{
				if(!Motor[ICount].E)
				{
					Motor[ICount].State = 1;
					Motor[ICount].TicCount = 0;
					digitalWriteDirect(MotorDir[ICount], Motor[ICount].D);
				}
				digitalWriteDirect(MotorEn[ICount], 	Motor[ICount].E);
				digitalWriteDirect(MotorLed[ICount], !Motor[ICount].E);
			}
			break;
			case 1:		//Comm #1: LOW -> HIGH
			{ 
				Motor[ICount].S 		= true;
				Motor[ICount].TicCount++;
				if(Motor[ICount].TicCount >= (Motor[ICount].TicsPerCom-1))
				{
					Motor[ICount].TicCount = 0;
					Motor[ICount].State 	= 2;	
				}
			}
			break;
			case 2:		//Comm #2: HIGH -> LOW ==> Step ready
			{
				Motor[ICount].S 		= false;
				Motor[ICount].TicCount++;
				if(Motor[ICount].TicCount >= (Motor[ICount].TicsPerCom-1))
				{
					Motor[ICount].TicCount = 0;
					if(!Motor[ICount].E)
						Motor[ICount].State = 1;
					else
						Motor[ICount].State = 0;
				}
			}
			break;
		}
	}
	for(int ICount = 0; ICount < 6; ICount++)
	{
		digitalWriteDirect(MotorPin[ICount], Motor[ICount].S);
	}
}

//###################################################################
uint32_t Rpm2ComTics(uint16_t RPM, uint8_t V) {
	return (60*IF)/(2*RPM*V);
}
uint8_t digitalReadDirect(uint8_t pin) {
	return !!(g_APinDescription[pin].pPort -> PIO_PDSR & g_APinDescription[pin].ulPin);
}
void 	digitalWriteDirect(uint8_t pin, boolean val) {
	if(val) g_APinDescription[pin].pPort -> PIO_SODR = g_APinDescription[pin].ulPin;
	else 	g_APinDescription[pin].pPort -> PIO_CODR = g_APinDescription[pin].ulPin;
}