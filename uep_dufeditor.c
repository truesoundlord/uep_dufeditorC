/* 
 * File:   uep_dufeditor.c.c
 * Author: Dimitri "Hurukan" <soundlord@gmail.com>
 *
 * Créé le December 12, 2024, 9:32 PM
 */

// ****************************************************************************
// HISTORIQUE
// ****************************************************************************

// **************************************************************************** 
// Fichiers de définitions
// **************************************************************************** 

#include "uep_dufeditor.h"

// **************************************************************************** 
// Déclaration des constantes symboliques
// **************************************************************************** 

// **************************************************************************** 
// Déclaration des variables globales, externes, ...
// **************************************************************************** 

PositionCadre topStatusBar;
PositionCadre downStatusBar;

extern PositionCadre ext_MainWindow={0};

LinkedList		*Parametres;
LinkedList		*Fichiers;

extern FILE		*logFile=NULL;

char					*logMessage;
char					*localWorkingDir=NULL;
char					*localTempDir=NULL;

unsigned short	us_Width;
unsigned short	us_Height;

// ****************************************************************************
// SECTION : prototypes des fonctions en test pour CE code source
// ****************************************************************************



/*
 * FONCTION PRINCIPALE
 */
int main(int argc,char** argv)
{
	logFile=InitLog("./logs","dufeditor.log",LOGW);
	InitUEPWIDE("fr_BE.UTF8");
	
	logMessage=calloc(80,sizeof(char));
	
	EffacerEcran();
	
	Parametres=lc_init();
	Fichiers=lc_init();
	
	sprintf(logMessage,"-- uep_dufeditor STARTS --");
	Log(logFile,logMessage);
	
	if(argc>1)
	{
		int cpt=argc-1;
		while(cpt>=1)
		{
			lc_insert(argv[cpt],Parametres,uepuserdef,strlen(argv[cpt])+1);
			sprintf(logMessage,"Parameter found:\t%s",argv[cpt]);
			Log(logFile,logMessage);
			cpt--;
		}	
	}
	if(Parametres->NbElem==1)
	{
		wprintf(L"No file to process...\n");
		return(EXIT_SUCCESS);
	}
	
	sprintf(logMessage,"Parameters to process: %02d",Parametres->NbElem);
	Log(logFile,logMessage);
	
	// Verify the different parameters...
	
	// USAGE:
	// uep_dufeditor -w <working folder> -t <temporary folder> -f <filename1> <filename2>
	// uep_dufeditor -t <temporary folder> -f <filename1> <filename2>
	
	unsigned params=VerifyParameters(Parametres);
	if(!params)
	{
		DisplayUsage();
	}
	
	// Computing params
	
	if ((params & TEMP) == TEMP)
	{
		int tempID=lc_FindByValue(Parametres,"-t",compareme);
		if(tempID>1000)
		{
			lc_Datas *tempdir=lc_search(Parametres,tempID-1);				// risqué mais pas le choix
			localTempDir=(char*)tempdir->value;
		}
	}
	if ((params & WORKING) == WORKING)
	{
		int workingID=lc_FindByValue(Parametres,"-w",compareme);
		if(workingID>1000)
		{
			lc_Datas *workingdir=lc_search(Parametres,workingID-1);				// risqué mais pas le choix
			localWorkingDir=(char*)workingdir->value;
		}
	}
	
	GetConsoleDimensions(&us_Width,&us_Height);
	topStatusBar=DrawTitledBoxWithRGB(1,1,2,us_Width,(struct s_RGB){0,170,0,true},"{Current directory}",(struct s_RGB){0,255,127,false},COLORED_SEPARATE);
	
	if(localWorkingDir!=NULL)
	{
		if(chdir(localWorkingDir)==-1)
		{
			mkdir(localWorkingDir,S_IRWXU|S_IRWXG|S_IRWXO);
			chdir(localWorkingDir);
		}
		//DisplayXY(localWorkingDir,FenetrePrincipale.FirstPrintableX,FenetrePrincipale.FirstPrintableY);
		AddToMessageBoxEx(localWorkingDir,&topStatusBar);
	}
	
	if(localTempDir!=NULL)
	{
		AddToMessageBoxEx(localTempDir,&topStatusBar);
	}
	
	downStatusBar=DrawTitledBoxWithRGB(1,us_Height-14,12,us_Width,(struct s_RGB){0,170,0,true},"{files}",(struct s_RGB){0,255,127,false},COLORED_SEPARATE);
		
	if ((params & FILES) == FILES)
	{
		// Normalement les fichiers sont en fin de liste
		// -f indique la fin de la liste des fichiers à traiter
		
		// * est pris en charge (?)
		
		lc_Datas *candidat=Parametres->pTail;
		while(candidat!=NULL)
		{
			char *nomfichier=(char*)candidat->value;
			if(strcmp(nomfichier,"-f")==0) break;
			AddToMessageBoxEx(nomfichier,&downStatusBar);
			lc_insert(nomfichier,Fichiers,uepuserdef,strlen(nomfichier)+1);
			candidat=candidat->pPrevious;
		}
	}
	else
	{
		DisplayUsage();
	}
	
	ext_MainWindow=DrawBoxWithRGB(1,topStatusBar.Last,us_Height-((topStatusBar.Height+downStatusBar.Height)+7),us_Width,(struct s_RGB){0,255,127,true});					// 3 cadres et une ligne (donc 7)
	
	while(Fichiers->NbElem>0)
	{
		lc_Datas *candidat=lc_pop(Fichiers);
		char *fileName=(char*)candidat->value;
		InvestigateFile(fileName,localTempDir);														// may be next threaded
	}
	
	lc_empty(Parametres);
	Parametres=NULL;
	
	SetCursor(downStatusBar.FirstPrintableX,downStatusBar.Last);
	
	sprintf(logMessage,"-- uep_dufeditor ENDS --");
	Log(logFile,logMessage);
	
	return(EXIT_SUCCESS);
}

// ****************************************************************************
// SECTION : implémentation des fonctions
// ****************************************************************************

bool compareme(void *candidat,void *target)
{
	char *strTarget=(char*)target;
	char *strCandidat=(char*)candidat;
	
	if(strcmp(strCandidat,strTarget)==0) return true;
	return false;
}

void DisplayUsage(void)
{
	EffacerEcran();
	wprintf(L"USAGE:\n");
	wprintf(L"\tuep_dufeditor -w <working directory> -t <temp directory> -f <filename1> <filename2>\n");
	wprintf(L"\tuep_dufeditor -w <working directory> -f <filename1> <filename2>\n");
	wprintf(L"\tuep_dufeditor -t <temp directory> -f <filename1> <filename2>\n");
	exit(EXIT_FAILURE);
}
