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
// Implementing and testing

// 20 jan 2025
// still testing

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

long	copy;
char	*ReadBuffer;
char	safety[4096];	// DEBUG

// **************************************************************************** 
// Threads
// **************************************************************************** 

#ifdef THREADED
pthread_mutex_t LockInvestigate;
pthread_mutex_t LockIO;
pthread_mutex_t LockMsg;
#endif

// ****************************************************************************
// SECTION : implémentation des fonctions
// ****************************************************************************

void InvestigateFile(char *duffile,char *tmpdirforunzip)
{
	// duffile is expected to be a .duf file...
	// reading .duf files is not a solution, first we have to unzip it... (zip format I think)
		
	long taillefichier=0L;
	long *offset;
	long *offsetend;
	

	FILE	*readFILE;
		
	LinkedList *ll_positionsDebut=lc_init();
	LinkedList *ll_positionsFin=lc_init();
	
	char	*newname=NULL;
	char	*destname=NULL;
	char	*LogMsg=calloc(160,1);
	
	// **************************************************************************
	// PRE PROCESS
	// **************************************************************************
	
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
	
	// **************************************************************************
	// END
	// **************************************************************************
		
	if(rtcd!=REG_NOMATCH) // the file is a .duf extension file
	{
		sprintf(LogMsg,"[%s] %s is a .duf file...",__func__,duffile);
		Log(logFile,LogMsg);
		
		Log(logFile,"\tuncompressing...");
		
		// BUG: si le répertoire existe déjà ça foire...
		
		// **************************************************************************
		// UNCOMPRESSING .duf FILE
		// **************************************************************************
			
		char *command=calloc(255,1);
		char *pSeek=NULL;
		
		
		sprintf(command,"ark -b ./%s \0",duffile);
		system(command);		
		
		sprintf(LogMsg,"\t%s file uncompressed",duffile);
		Log(logFile,LogMsg);
		
		free(command);
		
		// **************************************************************************
		// END
		// **************************************************************************
			
		// Habituellement les fichiers archivés portent juste l'extension .duf en moins...
		
		pSeek=strstr(duffile,".duf");
		if(pSeek)
		{
			newname=calloc(strlen(duffile)+1,1);
			destname=calloc(FILENAME_MAX,1);
			
			strncpy(newname,duffile,(pSeek-duffile));
			
			pSeek=strchr(newname,'/');
			while(pSeek)
			{
				newname=pSeek+1;
				pSeek=strchr(newname,'/');
			}
			
			// **************************************************************************
			// COPYING to tmpdirfirunzip
			// **************************************************************************
				
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
			
			// **************************************************************************
			// END
			// **************************************************************************
		}
		else return;
		
		// Copies are done !!
			
		chdir(tmpdirforunzip);
		
		// **************************************************************************
		// MARKING Camera blocks
		// **************************************************************************
		
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
			offsetend=calloc(1,sizeof(long));
			
			pSeek=strstr(ReadBuffer,PTRN_ID);
			if(!pSeek) continue;
			
			pSeek+=strlen(PTRN_ID)+1;
			ReadBuffer=pSeek;
			
			pSeek=strstr(ReadBuffer,PTRN_LABEL);
			pSeek+=strlen(PTRN_LABEL);
			
			char *pBegin=pSeek;
			char *pEnd;

			pEnd=strstr(ReadBuffer,PTRN_END_LBL);
			char *CameraName=calloc((pEnd-pBegin)+1,1);
			strncpy(CameraName,pBegin,(pEnd-pBegin));
			
			sprintf(LogMsg,"\t\t [ %s ]",CameraName);
			Log(logFile,LogMsg);
			
			// On a détecté la caméra... maintenant on va faire comment ?
			// une fois le pattern obtenu on va faire un reverse
			
			while(*ReadBuffer!='{')
			{
				ReadBuffer--;
			}
			
			// une fois ici on est au début de l'élément "caméra"
			
			pBegin=ReadBuffer+1;
			
			*offset=(long)ReadBuffer-copy;
			
			pEnd=strstr(pBegin,PTRN_END_ITEM);
			pEnd+=strlen(PTRN_END_ITEM);
			
			*offsetend=(long)pEnd-copy;
			*offsetend-=1;
					
			sprintf(LogMsg,"\t\t\t offset begin %ld offset end %ld",*offset,*offsetend);
			Log(logFile,LogMsg);
			
			lc_add(offset,ll_positionsDebut,ueplong,sizeof(long));
			lc_add(offsetend,ll_positionsFin,ueplong,sizeof(long));
			
			sleep(1);
			
			ReadBuffer=pEnd;
			
			free(CameraName);
			//free(newname);
			
		}while(pSeek);
		
		// **************************************************************************
		// END
		// **************************************************************************
		
#ifdef DEBUG
		sleep(1);
#endif
	
		// Nous avons les offsets...
		// Bon !!
		
		// on va relire le fichier caractère par caractère et former un "pseudo fichier"
		// qui va "bypasser" tout ce qui concerne une camera... 
		// je me demande si je ne devrais pas détecter le début ET la fin d'une caméra...
		// du moins dans la liste avoir le début ET la fin (?)
		
		// **************************************************************************
		// SAVING modified FILE
		// **************************************************************************
			
		ReadBuffer=(char*)copy;
			
		pSeek=strstr(duffile,".duf");
		if(pSeek)
		{
			strncpy(destname,duffile,(pSeek-duffile));
			strcat(destname,".processed");
			
			pSeek=strchr(destname,'/');
			while(pSeek)
			{
				destname=pSeek+1;
				pSeek=strchr(destname,'/');
			}
			
			FILE *destFile=fopen(destname,"w+t");
			long positiondanslebuffer=0L;
			
			lc_Datas *tmpElemBegin=lc_pop(ll_positionsDebut);
			lc_Datas *tmpElemEnd=lc_pop(ll_positionsFin);
			
			long lBegin=*(long*)tmpElemBegin->value;
			long lEnd=*(long*)tmpElemEnd->value;
			
			while(positiondanslebuffer<taillefichier)
			{
				char carAEcrire=ReadBuffer[positiondanslebuffer];
				if(positiondanslebuffer<lBegin || positiondanslebuffer>lEnd) 
				{
					fputc(carAEcrire,destFile);
				}
				else
				{
					fputc(carAEcrire,logFile);
				}
				if(positiondanslebuffer==lBegin)
				{
					sprintf(LogMsg,"DEBUG\t[%c (%03d)] BEGIN -> %08ld",carAEcrire,carAEcrire,positiondanslebuffer);
					Log(logFile,LogMsg);
				}
				if(positiondanslebuffer==lEnd)
				{
					sprintf(LogMsg,"DEBUG\t[%c (%03d)] END -> %08ld",carAEcrire,carAEcrire,positiondanslebuffer);
					Log(logFile,LogMsg);
					tmpElemBegin=lc_pop(ll_positionsDebut);
					tmpElemEnd=lc_pop(ll_positionsFin);
					
					if(tmpElemBegin && tmpElemEnd)
					{
						lBegin=*(long*)tmpElemBegin->value;
						lEnd=*(long*)tmpElemEnd->value;
					}
					else
					{
						lBegin=taillefichier;
						lEnd=0L;
					}
				}
				positiondanslebuffer++;
			}
			
			free(tmpElemBegin);
			free(tmpElemEnd);
			
			fflush(destFile);
			fclose(destFile);
			
		} // endif
		
		// **************************************************************************
		// END
		// **************************************************************************
		
		// TODO:
		
		// Pour après: 
		// zip -r -0 <FILE>.duf <FILE> 
		// retirer les 71 premiers caractères ajoutés par zip du fichier .duf
		
		// cat <FILE> | gzip > <FILE>.duf
		// a l'air de fonctionner mais les données sont compressées (la taille change)
		
		// **************************************************************************
		// COMPRESSING IN .duf FORMAT 
		// **************************************************************************
				
		newname=NULL;
		newname=calloc(FILENAME_MAX,1);
		
		// renommer .processed 
		
		pSeek=strstr(destname,".processed");
		if(pSeek)
		{
			strncpy(newname,destname,(pSeek-destname));
		}
		
		unlink(newname);
		rename(destname,newname);
		
		// préparer l'archivage avec l'extension .duf
		
		sprintf(destname,"%s.duf",newname);
		unlink(destname);
		
		// archiver au format .duf
		
		command=calloc(255,1);
		//sprintf(command,"zip -r -0 %s.duf %s",newname,newname);
		sprintf(command,"cat %s | gzip %s.duf",newname,newname);
		
		system(command);
		
#ifdef DEBUG
		
		// TODO:
		// ouvrir le fichier .duf et lire le contenu
		// effacer le fichier .duf
		// écrire sur le fichier .duf le contenu à partir du 68ème byte
		
		readFILE=fopen(destname,"r");
		if(!readFILE)
		{
			sprintf(LogMsg,"[%s] Error -> %s",__func__,strerror(errno));
			Log(logFile,LogMsg);
			return;
		}
		
		unlink(newname);		
		
		FILE *writeFile=fopen(newname,"w");
		
		fseek(readFILE,0L,SEEK_END);
		taillefichier=ftell(readFILE);
		fseek(readFILE,68L,SEEK_SET);	// on lit à partir du 68ème byte 
		
		ReadBuffer=NULL;
		ReadBuffer=calloc(taillefichier+1,sizeof(char));

		// Ici on est en BINAIRE plus en TEXTE
		// Apparemment il ne faut pas !!! 23 jan 2025
		
		//fread(ReadBuffer,taillefichier,1,readFILE);			
		//fwrite(ReadBuffer,taillefichier-171L,1,writeFile);
		
		long positiondanslefichier=0L;
		while(positiondanslefichier<taillefichier-171L)
		{
			char unCaractere=fgetc(readFILE);
			if(unCaractere==EOF) break;
			fputc(unCaractere,writeFile);
			positiondanslefichier++;
		}
				
		fflush(writeFile);
		
		fclose(readFILE);
		fclose(writeFile);
		
		// **************************************************************************
		// END
		// **************************************************************************
		
		unlink(destname);
		rename(newname,destname);
#endif
		
		pSeek=NULL;
		destname=NULL;
		newname=NULL;
		
		chdir(".."); // ne pas oublier évidemment ^^
		
		free(ReadBuffer);
		free(ll_positionsDebut);
		free(ll_positionsFin);
	}
}

// ****************************************************************************
// SECTION : TODO (ce qui reste à faire)
// ****************************************************************************


