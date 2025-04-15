#include "gomoku.h"

// Global game state
int chessboard[N + 1][N + 1] = {0};  // Shared game board
int player_sockets[2] = {0};          // Stores connected players' socket descriptors
int empty_spaces = 1;

/* Checks if last move caused a win (5 in a row) */
int judge(int x, int y) {
    const int step[4][2] = {{-1,0}, {0,-1}, {1,1}, {1,0}};  // Check directions: 
    // Vertical, Horizontal, Diagonal \, Diagonal /
    for(int i = 0; i < 4; ++i) {
        const int d[2] = {-1, 1};  // Check both directions from current position
        int count = 1;  // Start counting current stone
        
        for(int j = 0; j < 2; ++j) {  // For both directions
            for(int k = 1; k <= 4; ++k) {  // Check up to 4 steps
                int row = x + k*d[j]*step[i][0];
                int col = y + k*d[j]*step[i][1];
                // Check boundaries and same stone type
                if(row >= 1 && row <= N && col >= 1 && col <= N &&
                   chessboard[x][y] == chessboard[row][col])
                    count++;
                else
                    break;
            }
        }
        if(count >= 5) return 1;  // Winning condition met
    }
    return 0;  // No win detected
}

/* Handles a single player's move and game state update */
void handle_game(int current_player) {
    int x, y, winner = 0, quit_flag = 0;

    // Notify current player to move
    send(player_sockets[current_player], "YOUR_TURN", 10, 0);
    
    // Receive move coordinates
    recv(player_sockets[current_player], &x, sizeof(x), 0);
    recv(player_sockets[current_player], &y, sizeof(y), 0);

    check_field_fullness(); // Check if there is space left

    // Handle quit command
    if ((x == -1 && y == -1) || (empty_spaces == 0)) {
        quit_flag = 1;
        winner = -1;
        x = 0; y = 0;  // Use dummy coordinates
        chessboard[0][0] = -1;  // Special flag for termination
    }

    if (!quit_flag) {
        // Update game board (current_player 0 becomes 1, 1 becomes 2)
        chessboard[x][y] = current_player + 1;

        // Check for winning condition
        if(judge(x, y)) {
            winner = current_player + 1;  // Return player number (1/2)
            chessboard[0][0] = -1;
        }

    }

    // Broadcast update to both players
    char msg[10] = "MOVE";
    for(int i = 0; i < 2; i++) {
        send(player_sockets[i], msg, 10, 0);
        send(player_sockets[i], &x, sizeof(x), 0);
        send(player_sockets[i], &y, sizeof(y), 0);
        send(player_sockets[i], &current_player, sizeof(current_player), 0);
        
        if(winner || quit_flag) {
            send(player_sockets[i], "GAME_OVER", 10, 0);
            send(player_sockets[i], &winner, sizeof(winner), 0);
        }
    }
}

void check_field_fullness() {
    int empty_spaces_local = 0;
    for (int i = 1; i < N + 1; i++) {
        for (int j = 1; j < N + 1; j++) {
            if (chessboard[i][j] == 0) {
                empty_spaces_local++;
            }
        }
    }

    empty_spaces = empty_spaces_local;
}

/* Main server function managing connections and game flow */
int main() {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create server socket
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // Bind to all interfaces
    address.sin_port = htons(PORT);        // Use defined port

    // Bind socket to port
    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    if(listen(server_fd, 2) < 0) {  // Queue up to 2 connections
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for players...\n");

    // Accept two players sequentially
    for(int i = 0; i < 2; i++) {
        if((player_sockets[i] = accept(server_fd, (struct sockaddr *)&address, 
            (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }
        int player_num = i + 1;  // Assign player numbers 1 and 2
        send(player_sockets[i], &player_num, sizeof(player_num), 0);
        printf("Player %d connected\n", player_num);
    }

    // Main game loop (alternate between players)
    while(1) {
        for(int current_player = 0; current_player < 2; current_player++) {
            handle_game(current_player);
            
            // Check termination flag (set when player quits)
            if(chessboard[0][0] == -1) {
                printf("Game ending...\n");
                sleep(6);  // Allow clients to receive final message
                close(server_fd);
                exit(0);
            }
        }
    }

    return 0;
}