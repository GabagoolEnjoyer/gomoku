#include "gomoku.h"  // Contains game constants (N=15, PORT) and declarations

// Global game state
int chessboard[N + 1][N + 1] = {0};  // 16x16 board (1-15 usable, 0 for labels)
int player_number;                    // 1 or 2 indicating player identity
int first_turn_flag = 2;              // Controls initial instruction display (2=first time)

/* Prints the game board with row/column labels */
void print_chessboard() {
    system("clear");  // Clear terminal screen
    // Show instructions on first two turns
    if (first_turn_flag > 0) {
        printf("Controls: Input row and column from 1 to 15 with a space inbetween\n");
        printf("Hint: Input -1 -1 to quit\n");
        first_turn_flag = first_turn_flag == 2? 1 : 0;  // Update flag state
    }
    // Print board with coordinates
    for(int i = 0; i <= N; i++) {
        for(int j = 0; j <= N; j++) {
            if(i == 0) printf("%3d", j);          // Top row (column numbers)
            else if(j == 0) printf("%3d", i);     // Left column (row numbers)
            else if(chessboard[i][j] == 1) printf("  X");  // Player 1's pieces
            else if(chessboard[i][j] == 2) printf("  O");  // Player 2's pieces
            else printf("  *");                   // Empty spots
        }
        printf("\n");
    }
}

/* Main client function handling network communication */
int main(int argc, char *argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Validate command line arguments
    if(argc != 3) {
        printf("Usage: %s <IP> <PORT>\n", argv[0]);
        return -1;
    }

    // Create TCP socket
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        return -1;
    }

    // Configure server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));  // Convert port to network byte order

    // Convert IP address to binary form
    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        perror("invalid address");
        return -1;
    }

    // Connect to game server
    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connection failed");
        return -1;
    }

    // Receive player number from server (1 or 2)
    recv(sock, &player_number, sizeof(player_number), 0);
    system("clear");
    printf("You are Player %d\n", player_number);

    print_chessboard();  // Initial board display

    // Main game loop
    while(1) {
        char buffer[10] = {0};
        recv(sock, buffer, sizeof(buffer), 0);  // Wait for server command

        if(strcmp(buffer, "YOUR_TURN") == 0) {
            // Client's turn to make a move
            int x, y;
            printf("Your turn: ");
            // Input validation loop
            while (1) {
                if (scanf("%d %d", &x, &y) == 2) {
                    if (x == -1 && y == -1) {  // Quit command
                        break;
                    }
                    // Check valid coordinates and empty spot
                    if (x > 0 && x <= 15 && y > 0 && y <= 15) {
                        if (chessboard[x][y] == 0) {
                            break;
                        }
                    }
                }
                printf("Incorrect input. \n");
                while (getchar() != '\n'); // очистка буфера
            }
            // Send move to server
            send(sock, &x, sizeof(x), 0);
            send(sock, &y, sizeof(y), 0);
        } 
        else if(strcmp(buffer, "MOVE") == 0) {
            // Update board with opponent's move
            int x, y, player;
            recv(sock, &x, sizeof(x), 0);
            recv(sock, &y, sizeof(y), 0);
            recv(sock, &player, sizeof(player), 0);
            chessboard[x][y] = player + 1;  // Convert 0/1 index to 1/2 player numbers
            print_chessboard();
        } 
        else if(strcmp(buffer, "GAME_OVER") == 0) {
            // End game scenario
            int winner;
            recv(sock, &winner, sizeof(winner), 0);
            if (winner != -1) {
                printf("Player %d wins!\n", winner);
            }
            else {
                printf("Game ended pramaturely.\n");
            }
            
            sleep(4);  // Give time to read message
            break;     // Exit game loop
        }
    }

    system("clear");
    close(sock);  // Cleanup network connection
    return 0;
}