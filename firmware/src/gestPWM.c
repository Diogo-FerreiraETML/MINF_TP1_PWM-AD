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



#include "GestPWM.h"

S_pwmSettings PWMData;      // pour les settings

void GPWM_Initialize(S_pwmSettings *pData)
{
   // Init les data 
    
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
    uint16_t TabValADC1[10] = {0}; //AD1
    uint16_t TabValADC2[10] = {0}; //AD2

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
    
    PWMData.SpeedSetting = ValBruteADC - 99;
    if(PWMData.SpeedSetting < 0)
    {
        PWMData.absSpeed = PWMData.SpeedSetting * -1;
    }
    else
    {
        PWMData.absSpeed = PWMData.SpeedSetting;
    }
    
    //Conversion Angle
    PWMData.AngleSetting = ((Moyenne2*180) / 1023) - 90;
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
    
}

// Execution PWM software
void GPWM_ExecPWMSoft(S_pwmSettings *pData)
{
    
}


