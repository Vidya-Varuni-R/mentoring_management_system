/*
    structures.h
    All struct definitions for the Mentoring Management System.
    Include this in any .c file: #include "structures.h"
*/

#ifndef STRUCTURES_H
#define STRUCTURES_H

struct Mentee {
    char regno[50];
    char name[50];
    char dept[50];
    struct Mentee *next;
};

struct Meeting {
    char regno[50];
    char date[20];
    char time[20];
    char mode[20];
    char agenda[100];
    struct Meeting *next;
};

struct Request {
    char regno[50];
    char message[100];
    struct Request *next;
};
struct Note {
    char regno[50];
    char note[200];
    struct Note *next;
};
#endif