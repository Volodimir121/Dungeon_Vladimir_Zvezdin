#include "statushero.h"
#include <stdio.h>
#include <string.h>

typedef struct
{
   const char* name;
   const char* label;
   unsigned int flag;
} StatusEntry;

static const StatusEntry STATUS_TABLE[] = {
   { "poison",    "poisoned",  POISONED  },
   { "blind",     "blinded",   BLINDED   },
   { "invisible", "invisible", INVISIBLE },
   { "petrify",   "petrified", PETRIFIED },
   { "burn",      "burning",   BURNING   },
   { "sleep",     "sleeping",  SLEEPING  },
};

static const size_t STATUS_COUNT =
    sizeof(STATUS_TABLE) / sizeof(STATUS_TABLE[0]);

void print_statuses(unsigned int states)
{
   printf("States:\n");

   if (states == 0)
   {
      printf("- none\n");
      return;
   }

   for (size_t i = 0; i < STATUS_COUNT; i++)
   {
      if (states & STATUS_TABLE[i].flag)
      {
         printf("- %s\n", STATUS_TABLE[i].label);
      }
   }
}

int process_status_command(const char* command, unsigned int* states)
{

   int removing = (strncmp(command, "un", 2) == 0);
   const char* name = removing ? command + 2 : command;

   for (size_t i = 0; i < STATUS_COUNT; i++)
   {
      if (strcmp(name, STATUS_TABLE[i].name) != 0)
      {
         continue;
      }

      if (removing)
      {
         *states &= ~STATUS_TABLE[i].flag;
         printf("%s removed.\n", STATUS_TABLE[i].label);
      }
      else
      {
         *states |= STATUS_TABLE[i].flag;
         printf("You are now %s.\n", STATUS_TABLE[i].label);
      }

      return 1;
   }

   return 0;
}