/*--------------------------------------------------------*/
// GestPWM.c
/*--------------------------------------------------------*/
//	Description :	Gestion des PWM 
//			        pour TP1 2016-2017
//
//	Auteur 		: 	C. HUBER
//
//	Version		:	V1.1
//	Compilateur	:	XC32 V1.42 + Harmony 1.08
//
/*--------------------------------------------------------*/

#include "app.h"
#include "GestPWM.h"
#include "Mc32DriverLcd.h"
#include "Mc32DriverAdc.h"


void GPWM_Initialize(S_pwmSettings *pData)
{
   // Init lCD 
    lcd_init();
    lcd_bl_on();
    lcd_gotoxy(1,1);
    printf_lcd("TP1 PWM 2023");
    lcd_gotoxy(1,2);
    printf_lcd("Diogo Ferreira");
    lcd_gotoxy(1,3);
    printf_lcd("Julien Decrausaz");
    //Init ADC
    BSP_InitADC10();
    // Init état du pont en H
    BSP_EnableHbrige();
    // Lancement des timers et OC
    // Start des OCs
    DRV_OC0_Start();
    DRV_OC1_Start();
    // Start des timers
    DRV_TMR0_Start();
    DRV_TMR1_Start();
    DRV_TMR2_Start();
    DRV_TMR3_Start();
}

// Obtention vitesse et angle (mise a jour des 4 champs de la structure)
void GPWM_GetSettings(S_pwmSettings *pData)	
{
    // Lecture des 2 canaux du convertisseur AD
    S_ADCResults ADCRes;
    
    //Tableaux de 10 cases pour stockage des valeurs des 2 canaux de l'ADC
    static uint16_t TabValADC1[10] = {0}; //AD1
    static uint16_t TabValADC2[10] = {0}; //AD2

    //Compteurs pour boucles
    static uint8_t i = 0; //variable statique pour conserver la valeur après interruption
    uint8_t j = 0;
    
    //Variables pour moyennes (initialisées à 0))
    uint16_t Moyenne1 = 0;
    uint16_t Moyenne2 = 0;
    
    // Variable pour stockage de la valeur brute de l'AD
    uint16_t ValBruteADC = 0;
    
    // Lecture valeur canal 0 de l'AD
    ADCRes = BSP_ReadAllADC();
    
    //===========================CALCUL DES MOYENNES==========================//
    //MOYENNES GLISSANTES
    
    TabValADC1[i] = ADCRes.Chan0; //Lecture canal 1 et mise de la valeur dans le tableau 1
    TabValADC2[i] = ADCRes.Chan1; //Lecture canal 2 et mise de la valeur dans le tableau 2
    
    //Changement de case du tableau (vers la droite)
    if(i < 10)
    {
        i++;
    }
    else //Si on est tout à droite, on reviens au début
    {
        i = 0;
    }
     
    //Récupération des valeurs du tableau
    for(j = 0; j < 10; j++)
    {
        Moyenne1 += TabValADC1[j];
        Moyenne2 += TabValADC2[j];
    }
    
    Moyenne1 = Moyenne1 / 10;
    Moyenne2 = Moyenne2 / 10;
    
    //========================================================================//
    //Conversion Vitesse
    ValBruteADC = (Moyenne1*198) / 1023;
    
    pData -> SpeedSetting = ValBruteADC - 99;
    if(pData -> SpeedSetting < 0)
    {
        pData -> absSpeed = pData -> SpeedSetting * -1;
    }
    else
    {
        pData -> absSpeed = pData -> SpeedSetting;
    }
    
    //Conversion Angle
    pData -> AngleSetting = ((Moyenne2*180) / 1023) - 90;
}


// Affichage des information en exploitant la structure
void GPWM_DispSettings(S_pwmSettings *pData)
{
    // Affichage de la vitesse du moteur (valeur signée)
    lcd_gotoxy(1,2);
    printf_lcd("SpeedSetting %3d", pData -> SpeedSetting);
    // Affichage de la vitesse du moteur (valeur absolue)
    lcd_gotoxy(1,3);
    printf_lcd("absSpeed %7d", pData -> absSpeed);
    // Affichage de l'angle du servomoteur (valeur signée)
    lcd_gotoxy(1,4);
    printf_lcd("Angle %10d", pData -> AngleSetting);
}

// Execution PWM et gestion moteur à partir des info dans structure
void GPWM_ExecPWM(S_pwmSettings *pData)
{
    //Variables pour rapports cycliques des OCs (initialisées à 0)
    uint32_t OC2_DutyCycle = 0;
    uint32_t OC3_DutyCycle = 0;
    
    //Gestion de la direction du pont en H en fonction du signe de la vitesse
    //Dans la structure S_pwmSettings, SpeedSetting correspond au signe de la
    //  vitesse
    
    if(pData -> SpeedSetting > 0)
    {
        //Sens anti-horaire
        AIN1_HBRIDGE_W = 0;
        AIN2_HBRIDGE_W = 1;
        STBY_HBRIDGE_W = 1;
    }
    else if (pData -> SpeedSetting < 0)
    {
        //Sens horaire
        AIN1_HBRIDGE_W = 1;
        AIN2_HBRIDGE_W = 0;
        STBY_HBRIDGE_W = 1;
    }
    else
    {
        //STOP
        AIN1_HBRIDGE_W = 0;
        AIN2_HBRIDGE_W = 0;
        STBY_HBRIDGE_W = 0;    
    }
    
    //Calcul de la vitesse absSpeed en % pour obtention du rapport cyclique
    OC2_DutyCycle = 20 * pData -> absSpeed; //-0.5 = valeur minimum du Servo, nbre de tics
    //2000 correspond à la valeur utilisée dans le MHC + 1
    DRV_OC0_PulseWidthSet(OC2_DutyCycle);
    //Calcul de l'angle absAngle en nombre d'impulsions
    OC3_DutyCycle = (1124 * pData -> AngleSetting)/90 + 1874;
    //8750 correspond à la valeur utilisée dans le MHC + 1
    //180 correspond à la plage de l'angle
    DRV_OC1_PulseWidthSet(OC3_DutyCycle);
}

// Execution PWM software
void GPWM_ExecPWMSoft(S_pwmSettings *pData)
{
    //Fonction premettant de génlrer par comptage un PWM de 100 cycles de 35us
    
    //Variable de comptage (statique car on veut conserver la valeur)
    static uint8_t CounterPWM = 0;
    
    CounterPWM = (CounterPWM + 1) % 100; //Modulo 100 pour pourcentage
   
    if ( CounterPWM < pData->absSpeed)
    {
        BSP_LEDOn(BSP_LED_2);
    }
    else
    {
        BSP_LEDOff(BSP_LED_2);
    }
    if(CounterPWM == 100)
        CounterPWM = 0;
}