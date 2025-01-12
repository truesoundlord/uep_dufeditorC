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
	long *offset;
	long copy;

	FILE	*readFILE;
	char	*LogMsg=calloc(160,1);
	
	LinkedList *ll_positions=lc_init();
	
	
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
			fseek(srcFile,0L,SEEK_SET); // pas SEEK_CUR CONNARD !!!!
			
			
			// on va copier le tout d'un bloc bordel de merde
			
			char *BufferOneBlock=calloc(taillefichier+1,sizeof(char));
			char car=0;
			
			car=fgetc(srcFile);
			while(car!=EOF)
			{
				fputc(car,destFile);
				car=fgetc(srcFile);
			}
			
			fflush(destFile);
			
			fclose(destFile);
			fclose(srcFile);
			
			free(BufferOneBlock);
			BufferOneBlock=NULL;
		}
		
		// Copies are done !!
		
		chdir(tmpdirforunzip);
		
		readFILE=fopen(newname,"r");
		if(!readFILE)
		{
			sprintf(LogMsg,"[%s] -- Error %s",__func__,strerror(errno));
			Log(logFile,LogMsg);
			return;
		}
			
		ReadBuffer=calloc(taillefichier,sizeof(char));
		
		unsigned long cursor=0;
		char car=fgetc(readFILE);
		while(car!=EOF)
		{
			ReadBuffer[cursor]=car;
			car=fgetc(readFILE);
			cursor++;
		}
		
		sprintf(LogMsg,"\t %16ld MBytes read from file",(long)((taillefichier/1024)/1024));
		Log(logFile,LogMsg);
		
		copy = (long)ReadBuffer;
		
		do
		{
			offset=calloc(1,sizeof(long));
			
			pSeek=strstr(ReadBuffer,PTRN_ID);
			if(!pSeek) continue;
			// *offset+=(pSeek-ReadBuffer);
			
			pSeek+=strlen(PTRN_ID)+1;
			ReadBuffer=pSeek;
			
			pSeek=strstr(ReadBuffer,PTRN_LABEL);
			pSeek+=strlen(PTRN_LABEL);
			
			//*offset+=(pSeek-ReadBuffer);

			char *pBegin=pSeek;
			char *pEnd;

			pEnd=strstr(ReadBuffer,PTRN_END_LBL);
			char *CameraName=calloc((pEnd-pBegin)+1,1);
			strncpy(CameraName,pBegin,(pEnd-pBegin));
			
			//*offset+=(pEnd-ReadBuffer);

			sprintf(LogMsg,"\t\t [ %s ]",CameraName);
			Log(logFile,LogMsg);
			
			// On a détecté la caméra... maintenant on va faire comment ?
			
			// une fois le pattern obtenu on va faire un reverse
			
			while(*ReadBuffer!='{')
			{
				ReadBuffer--;
				//(*offset)--;
			}
			
			// une fois ici on est au début de l'élément "caméra"
			
			pBegin=ReadBuffer+1;
			
			*offset=(long)ReadBuffer-copy;
			
			pEnd=strstr(pBegin,PTRN_END_ITEM);
			pEnd+=strlen(PTRN_END_ITEM);
					
			sprintf(LogMsg,"\t\t\t offset %ld",*offset);
			Log(logFile,LogMsg);
			
			lc_insert(offset,ll_positions,ueplong,sizeof(long));
			
			sleep(1);
			
			ReadBuffer=pEnd;
			
		}while(pSeek);
		
		sleep(1);
			
		chdir(".."); // ne pas oublier évidemment ^^
	}
}

// ****************************************************************************
// SECTION : TODO (ce qui reste à faire)
// ****************************************************************************


