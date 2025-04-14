#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define N 15

int chessboard[N + 1][N + 1] = {0};
int player_number;

void printChessboard() {
    system("clear");
    for(int i = 0; i <= N; i++) {
        for(int j = 0; j <= N; j++) {
            if(i == 0) printf("%3d", j);
            else if(j == 0) printf("%3d", i);
            else if(chessboard[i][j] == 1) printf("  X");
            else if(chessboard[i][j] == 2) printf("  O");
            else printf("  *");
        }
        printf("\n");
    }
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main(int argc, char *argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;

    if(argc != 3) {
        printf("Usage: %s <IP> <PORT>\n", argv[0]);
        return -1;
    }

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        perror("invalid address");
        return -1;
    }

    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connection failed");
        return -1;
    }

    recv(sock, &player_number, sizeof(player_number), 0);
    printf("You are Player %d\n", player_number);
    printChessboard();

    while(1) {
        char buffer[10] = {0};
        recv(sock, buffer, sizeof(buffer), 0);

        if(strcmp(buffer, "YOUR_TURN") == 0) {
            int x, y, input_valid;
            do {
                printf("Your turn (row column, 1-%d): ", N);
                input_valid = scanf("%d %d", &x, &y);
                clear_input_buffer();

                if(input_valid != 2) {
                    printf("Invalid input! Please enter two numbers.\n");
                    continue;
                }

                if(x < 1 || x > N || y < 1 || y > N) {
                    printf("Coordinates must be between 1 and %d!\n", N);
                    input_valid = 0;
                } else if(chessboard[x][y] != 0) {
                    printf("Position (%d, %d) is occupied!\n", x, y);
                    input_valid = 0;
                }
            } while(input_valid != 2);

            send(sock, &x, sizeof(x), 0);
            send(sock, &y, sizeof(y), 0);

            // Wait for server confirmation
            recv(sock, buffer, sizeof(buffer), 0);
            if(strcmp(buffer, "INVALID") == 0) {
                printf("Server rejected move. Try again.\n");
                continue;
            }
        } 
        else if(strcmp(buffer, "MOVE") == 0) {
            int x, y, player;
            recv(sock, &x, sizeof(x), 0);
            recv(sock, &y, sizeof(y), 0);
            recv(sock, &player, sizeof(player), 0);
            chessboard[x][y] = player + 1;
            printChessboard();
        } 
        else if(strcmp(buffer, "GAME_OVER") == 0) {
            int winner;
            recv(sock, &winner, sizeof(winner), 0);
            printf("Player %d wins!\n", winner);
            
            int choice;
            do {
                printf("Play again? (1=Yes, 0=No): ");
                scanf("%d", &choice);
                clear_input_buffer();
            } while(choice != 0 && choice != 1);
            
            send(sock, &choice, sizeof(choice), 0);
            
            if(choice) {
                memset(chessboard, 0, sizeof(chessboard));
                printChessboard();
            } else {
                break;
            }
        }
    }

    close(sock);
    return 0;
}