#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define N 15
#define PORT 8080

int judge(int x, int y);
void handle_game(int current_player);
void print_chessboard();
void check_field_fullness();