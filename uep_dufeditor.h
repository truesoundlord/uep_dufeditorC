/* 
 * File:   uep_dufeditor.h
 * Author: Dimitri "Hurukan" <soundlord@gmail.com>
 *
 * Créé le December 16, 2024, 7:15 PM
 *  
 */

#ifndef UEP_DUFEDITOR_H
#define UEP_DUFEDITOR_H

// ****************************************************************************
// SECTION : fichiers d'inclusions
// ****************************************************************************

#include <stdio.h>
#include <stdlib.h>

#include <stdbool.h>      
#include <unistd.h>
#include <string.h>

#include <wchar.h>
#include <dirent.h>

#include <uepdebugtool.h>
#include <uep_wide.h>
#include <uepthreads.h>
#include <linkedlist.h>
#include <sys/stat.h>
#include <regex.h>
#include <errno.h>


// **************************************************************************** 
// Définitions des constantes symboliques
// **************************************************************************** 

#define PTRN_ID				"\"type\" : \"camera\","
#define PTRN_LABEL		"\t\t\t\"label\" : \""
#define PTRN_END_LBL	"\","
#define PTRN_END_ITEM "]\n\t\t},\n\t\t"


// **************************************************************************** 
// Définition(s) des structures, types et énumérations
// **************************************************************************** 

typedef enum e_paramset { NONE=0, TEMP=2, WORKING=4, FILES=8 }t_paramset;

// **************************************************************************** 
// Définition(s) des variables statiques/externes
// **************************************************************************** 

// **************************************************************************** 
// Définition(s) des fonctions
// **************************************************************************** 

unsigned short VerifyParameters(LinkedList*);
bool compareme(void*,void*);
void DisplayUsage(void);
void InvestigateFile(char*,char*);

#endif /* UEP_DUFEDITOR_H */
