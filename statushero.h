#ifndef DUNGEON_VLADIMIR_ZVEZDIN_STATUSHERO_H
#define DUNGEON_VLADIMIR_ZVEZDIN_STATUSHERO_H

#define POISONED   (1u << 0)
#define BLINDED    (1u << 1)
#define INVISIBLE  (1u << 2)
#define PETRIFIED  (1u << 3)
#define BURNING    (1u << 4)
#define SLEEPING   (1u << 5)

void print_statuses(unsigned int states);

int process_status_command(const char* command, unsigned int* states);

#endif