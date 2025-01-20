/* 
 * File:   VerifyParameters.c
 * Author: Dimitri "Hurukan" <soundlord@gmail.com>
 *
 * Créé le January 3, 2025, 12:00 AM
 */

// ****************************************************************************
// HISTORIQUE
// ****************************************************************************


// ****************************************************************************
// SECTION : fichiers d'inclusions
// ****************************************************************************

#include "uep_dufeditor.h"

// ****************************************************************************
// SECTION : déclaration des constantes/types/structures/énumérations/...
// ****************************************************************************
// ****************************************************************************
// SECTION : déclaration des variables globales/externes
// ****************************************************************************

// ****************************************************************************
// SECTION : implémentation des fonctions
// ****************************************************************************

unsigned short VerifyParameters(LinkedList *param)
{
	unsigned short us_NbVerified=0;
	lc_Datas	*pExtracted=param->pHead;
	
	while(pExtracted!=NULL)
	{
		char *pUnwrap = (char*)pExtracted->value;
		
		if(strcmp("-w",pUnwrap)==0)
		{
			us_NbVerified|=WORKING;
		}
		if(strcmp("-t",pUnwrap)==0)
		{
			us_NbVerified|=TEMP;
		}
		if(strcmp("-f",pUnwrap)==0)
		{
			us_NbVerified|=FILES;
			pExtracted=pExtracted->pNext;											// skip -f
			
			while(pExtracted != NULL)
			{
				pUnwrap=(char*)pExtracted->value;
				pExtracted=pExtracted->pNext;
			}
			if(!pExtracted) break;
		}
		pExtracted=pExtracted->pNext;
	}
	if(us_NbVerified==0) return 0;
	return us_NbVerified;
}

// ****************************************************************************
// SECTION : TODO (ce qui reste à faire)
// ****************************************************************************


