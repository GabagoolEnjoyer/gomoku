#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define N 15
#define PORT 8080

int chessboard[N + 1][N + 1] = {0};
int player_sockets[2] = {0};

int judge(int x, int y) {
    const int step[4][2] = {{-1,0}, {0,-1}, {1,1}, {1,0}};
    for(int i = 0; i < 4; ++i) {
        const int d[2] = {-1, 1};
        int count = 1;
        for(int j = 0; j < 2; ++j) {
            for(int k = 1; k <= 4; ++k) {
                int row = x + k*d[j]*step[i][0];
                int col = y + k*d[j]*step[i][1];
                if(row >= 1 && row <= N && col >= 1 && col <= N &&
                   chessboard[x][y] == chessboard[row][col])
                    count++;
                else
                    break;
            }
        }
        if(count >= 5) return 1;
    }
    return 0;
}

void reset_game() {
    memset(chessboard, 0, sizeof(chessboard));
}

void handle_game(int current_player) {
    int x, y, winner = 0;
    int other_player = current_player ^ 1; // Toggle between 0 and 1

    // Notify current player to move
    send(player_sockets[current_player], "YOUR_TURN", 10, 0);
    
    // Receive move
    recv(player_sockets[current_player], &x, sizeof(x), 0);
    recv(player_sockets[current_player], &y, sizeof(y), 0);

    // Update board
    chessboard[x][y] = current_player + 1;

    // Check win
    if(judge(x, y)) {
        winner = current_player + 1;
    }

    // Send update to both players
    char msg[10] = "MOVE";
    for(int i = 0; i < 2; i++) {
        send(player_sockets[i], msg, 10, 0);
        send(player_sockets[i], &x, sizeof(x), 0);
        send(player_sockets[i], &y, sizeof(y), 0);
        send(player_sockets[i], &current_player, sizeof(current_player), 0);
        if(winner) {
            send(player_sockets[i], "GAME_OVER", 10, 0);
            send(player_sockets[i], &winner, sizeof(winner), 0);
        }
    }
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create socket
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind
    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen
    if(listen(server_fd, 2) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for players...\n");

    // Accept two players
    for(int i = 0; i < 2; i++) {
        if((player_sockets[i] = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }
        int player_num = i + 1;
        send(player_sockets[i], &player_num, sizeof(player_num), 0);
        printf("Player %d connected\n", player_num);
    }

    // Game loop
    while(1) {
        for(int current_player = 0; current_player < 2; current_player++) {
            handle_game(current_player);
            
            // Check for winner
            if(chessboard[0][0] == -1) { // Simple win flag
                int responses[2];
                for(int i = 0; i < 2; i++) {
                    recv(player_sockets[i], &responses[i], sizeof(responses[i]), 0);
                }

                if(responses[0] && responses[1]) {
                    reset_game();
                    for(int i = 0; i < 2; i++) {
                        send(player_sockets[i], "RESTART", 8, 0);
                    }
                } else {
                    printf("Game ending...\n");
                    close(server_fd);
                    exit(0);
                }
            }
        }
    }

    close(server_fd);
    return 0;
}