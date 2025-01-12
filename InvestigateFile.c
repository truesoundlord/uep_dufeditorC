/* 
 * File:   investigateFile.c
 * Author: Dimitri "Hurukan" <soundlord@gmail.com>
 *
 * Créé le January 2, 2025, 11:53 PM
 */

// ****************************************************************************
// HISTORIQUE
// ****************************************************************************

// 11 jan 2025
// Implementing

// ****************************************************************************
// SECTION : fichiers d'inclusions
// ****************************************************************************

#include "uep_dufeditor.h"

// ****************************************************************************
// SECTION : déclaration des constantes/types/structures/énumérations/...
// ****************************************************************************

#define VLDEXTREGEX ".duf$"

// ****************************************************************************
// SECTION : déclaration des variables globales/externes
// ****************************************************************************

extern FILE *logFile;
extern PositionCadre ext_MainWindow;

char				*ReadBuffer;

// ****************************************************************************
// SECTION : implémentation des fonctions
// ****************************************************************************

void InvestigateFile(char *duffile,char *tmpdirforunzip)
{
	// duffile is expected to be a .duf file...
	// reading .duf files is not a solution, first we have to unzip it... (zip format I think)
	
	char *newname=NULL;
	char *destname=NULL;
	
	long taillefichier=0L;
	FILE	*readFILE;
	char	*LogMsg=calloc(160,1);
	
	if(!duffile) return;
	if(!tmpdirforunzip) 
	{
		mkdir("./tmp",S_IRWXU);
		tmpdirforunzip=calloc(255,1);
		strcpy(tmpdirforunzip,"tmp");
	}
	
	// is this file is valid ???
	
	regex_t regexcomp;
	int rtcd = regcomp(&regexcomp,VLDEXTREGEX,REG_EXTENDED);
	
	if(rtcd == -1)
	{
		regerror(rtcd,&regexcomp,LogMsg,160);
		Log(logFile,LogMsg);
		return;
	}
					
	rtcd = regexec(&regexcomp,duffile,0,NULL,0);
	
	if(rtcd == -1)
	{
		regerror(rtcd,&regexcomp,LogMsg,160);
		Log(logFile,LogMsg);
		return;
	}
	
	if(rtcd!=REG_NOMATCH) // the file is a .duf extension file
	{
		sprintf(LogMsg,"[%s] %s is a .duf file...",__func__,duffile);
		Log(logFile,LogMsg);
		
		Log(logFile,"\tuncompressing...");
		
		// BUG: si le répertoire existe déjà ça foire...
		
		char *command=calloc(255,1);
		char *pSeek=NULL;
		
		
		sprintf(command,"ark -b ./%s \0",duffile);
		system(command);		
		
		sprintf(LogMsg,"\t%s file uncompressed",duffile);
		Log(logFile,LogMsg);
		
		/*
			sprintf(command,"ark -b ./%s -o ./%s\0",duffile,tmpdirforunzip);
			system(command);		
			 
		 */
		
		// Habituellement les fichiers archivés portent juste l'extension .duf en moins...
		
		pSeek=strstr(duffile,".duf");
		if(pSeek)
		{
			newname=calloc(strlen(duffile)+1,1);
			destname=calloc(FILENAME_MAX,1);
			
			strncpy(newname,duffile,(pSeek-duffile));
			//AddToMessageBoxEx(newname,&ext_MainWindow);
			
			pSeek=strchr(newname,'/');
			while(pSeek)
			{
				newname=pSeek+1;
				pSeek=strchr(newname,'/');
			}
			
			sprintf(LogMsg,"%s is being copied in %s...",newname,tmpdirforunzip);
			AddToMessageBoxEx(LogMsg,&ext_MainWindow);
			
			if(chdir(tmpdirforunzip)==-1)
			{
				mkdir(tmpdirforunzip,S_IRWXU|S_IRWXG|S_IRWXO);
			}
			else
			{
				chdir("..");
			}
			
			sprintf(destname,"%s/%s",tmpdirforunzip,newname);
			
			FILE *srcFile=fopen(newname,"r");
			FILE *destFile=fopen(destname,"w");
			
			if(!srcFile || !destFile)
			{
				sprintf(LogMsg,"[%s] -- Error %s",__func__,strerror(errno));
				Log(logFile,LogMsg);
				return;
			}
			
			fseek(srcFile,0L,SEEK_END);
			taillefichier=ftell(srcFile);
			fseek(srcFile,0L,SEEK_CUR);
			
			// on va copier le tout d'un bloc bordel de merde
			
			char *BufferOneBlock=calloc(taillefichier+1,sizeof(wchar_t));
			
			// fread((char*)BufferOneBlock,sizeof(wchar_t),taillefichier,srcFile);
			// fwrite((char*)BufferOneBlock,sizeof(char),taillefichier,destFile);
			
			long reste=taillefichier;
			while(reste>0)
			{
				wchar_t car=fgetwc(srcFile);
				fputwc(car,destFile);
				reste--;
			}
			
			fflush(destFile);
			
			fclose(destFile);
			fclose(srcFile);
			
			free(BufferOneBlock);
			BufferOneBlock=NULL;
		}
		
		// Copies are done !!
		
		chdir(tmpdirforunzip);
		
		readFILE=fopen(newname,"rt");
		if(!readFILE)
		{
			sprintf(LogMsg,"[%s] -- Error %s",__func__,strerror(errno));
			Log(logFile,LogMsg);
			return;
		}
			
		ReadBuffer=calloc(taillefichier,sizeof(char));
		fread(ReadBuffer,sizeof(char),taillefichier,readFILE);
		
		sprintf(LogMsg,"\t %16ld MBytes read from file",(long)((taillefichier/1024)/1024));
		Log(logFile,LogMsg);
		
		
		
		chdir(".."); // ne pas oublier évidemment ^^
	}
}

// ****************************************************************************
// SECTION : TODO (ce qui reste à faire)
// ****************************************************************************


