#include <stdio.h>
#include <stdlib.h>
#include "data.h"
#define m_set_bit(word, index) ((word) |= (1ULL << (U64)(index)))
#define m_get_bit(word, index) ((word) & (1ULL << (U64)(index)))
#define m_pop_bit(word, index) ((word) &= ~(1ULL << (U64)(index)))
#define board_mask (0xffe3e3808080e3e3ULL)

typedef struct Node
{
    U64 board;
    U64 boards[76];
    struct Node* next[76];
} Node;

void print_board(U64);
void print_hexa(U64);
void print_node(Node* node);
void move_mask_print();
void move_check_print();
static inline int get_population_count(U64);
static inline Node* move_gen(U64);
static inline Node* tree_gen(U64, int);
void tree_free(Node*);
void iterative_deepening(U64);
void tree_count_leaves(Node*, int*);

void tree_count_leaves(Node* node, int* count){
    if(node == NULL) return;
    (*count)++;
    for(int i = 0; i < 76; i++){
        tree_count_leaves(node->next[i], count);
    }
}
void iterative_deepening(U64 board){
    int counts[40];
    for(int i = 1; i < 40; i++){
        Node* tree = tree_gen(board, i);
        tree_count_leaves(tree, counts + i);
        tree_free(tree);
        printf("Depth: %d\t Count: %d\n", i, counts[i]);
    }
}
void tree_free(Node* node){
    if(node == NULL) return;
    for(int i = 0; i < 76; i++){
        tree_free(node->next[i]);
    }
    free(node);
}
static inline Node* tree_gen(U64 board, int depth){
    if(depth < 0) return NULL;
    Node* moves = move_gen(board);
    if(moves == NULL) return NULL;
    for(int i = 0; i < 76; i++){
        moves->next[i] = tree_gen(moves->boards[i], depth - 1);
    }
    return moves;
}

void print_node(Node* node){
    for(int i = 0; i < 76; i++){
        if(node->boards[i]){
            print_board(node->boards[i]);
        }
    }
}
static inline Node* move_gen(U64 board){
    if(board == 0x0ULL) return NULL;
    Node* moves = malloc(sizeof(Node));
    moves->board = board;
    for(int i = 0; i < 76; i++){
        if((board & move_mask[i]) == move_check[i]){
            moves->boards[i] = board^move_mask[i];
        }
        else{
            moves->boards[i] = 0x0ULL;
        }
    }
    return moves;
}
static inline int get_population_count(U64 word)
{
    int count = 0;
    while (word){
        count++;
        word &= word - 1;
    }
    return count;
}

int main(){
    if(sizeof(U64) != 8){
        printf("wrong ull length\n");
        return 0;
    }
    U64 board = 0x001c1c7f777f1c1cULL;
    iterative_deepening(board);
    return 0;
}

void print_board(U64 board){
    printf("\n");
    for(int i = 0; i < 7; i++){
        for(int j = 0; j < 7; j++){
            printf("%c", m_get_bit(board, i * 8 + j) ? 'O' : '-');
        }
        printf("\n");
    }
    printf("\n");
}

void print_hexa(U64 word){
    printf("0x%llx", word);
}

void move_mask_print(){
    int dirs[4] = {1, 8, -1, -8};
    for(int i = 0; i < 7; i++){
        for(int j = 0; j < 7; j++){
            for(int d = 0; d < 4; d++){
                U64 move = 0ULL;
                m_set_bit(move, i * 8 + j);
                m_set_bit(move, i * 8 + j + dirs[d]);
                m_set_bit(move, i * 8 + j + 2*dirs[d]);
                if((move & board_mask) != 0x0ULL) continue;
                print_hexa(move);
                printf("ULL,\n");
            }
        }
    }
}
void move_check_print(){
    int dirs[4] = {1, 8, -1, -8};
    for(int i = 0; i < 7; i++){
        for(int j = 0; j < 7; j++){
            for(int d = 0; d < 4; d++){
                U64 move = 0ULL;
                m_set_bit(move, i * 8 + j);
                m_set_bit(move, i * 8 + j + dirs[d]);
                m_set_bit(move, i * 8 + j + 2*dirs[d]);
                if((move & board_mask) != 0x0ULL) continue;
                m_pop_bit(move, i * 8 + j);
                print_hexa(move);
                printf("ULL,\n");
            }
        }
    }
}