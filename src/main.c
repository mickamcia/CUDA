#include <stdio.h>
#include <stdlib.h>
#define U64 unsigned long long
#define m_set_bit(word, index) ((word) |= (1ULL << (U64)(index)))
#define m_get_bit(word, index) ((word) & (1ULL << (U64)(index)))
#define m_pop_bit(word, index) ((word) &= ~(1ULL << (U64)(index)))

#define key_size 29 //4GB
#define hash_size (1 << key_size)
#define max_move_count 76
#define initial_piece_count 32
#define board_legal_mask 0xffe3e3808080e3e3ULL
//#define board_start_position 0x001c1c7f7f7f1c14ULL 
  #define board_start_position 0x001c1c7f777f1c1cULL 
//#define board_start_position 0x001c1c7f777f1c0cULL

U64 table_masks[max_move_count];
U64 table_moves[max_move_count];
typedef struct Stack{
    U64 boards[initial_piece_count * max_move_count];
    int index;
} Stack;

typedef struct Entry{
    U64 board;

} Entry;
Entry* transposition_table;

U64 flipHorizontal (U64 x) {
   const U64 k1 = 0x5555555555555555ULL;
   const U64 k2 = 0x3333333333333333ULL;
   const U64 k4 = 0x0f0f0f0f0f0f0f0fULL;
   x = ((x >> 1) & k1) | ((x & k1) << 1);
   x = ((x >> 2) & k2) | ((x & k2) << 2);
   x = ((x >> 4) & k4) | ((x & k4) << 4);
   return x >> 1;
}
U64 flipDiagonal(U64 x) {
   U64 t;
   const U64 k1 = 0x5500550055005500ULL;
   const U64 k2 = 0x3333000033330000ULL;
   const U64 k4 = 0x0f0f0f0f00000000ULL;
   t  = k4 & (x ^ (x << 28));
   x ^=       t ^ (t >> 28) ;
   t  = k2 & (x ^ (x << 14));
   x ^=       t ^ (t >> 14) ;
   t  = k1 & (x ^ (x <<  7));
   x ^=       t ^ (t >>  7) ;
   return x;
}

static inline int population_count(U64 word)
{
    int count = 0;
    while (word){
        count++;
        word &= word - 1;
    }
    return count;
}
static inline unsigned int key_gen(U64 word){
    U64 key = word;
    key ^= word >> key_size;
    return (unsigned int)(key & (hash_size - 1));
}
void print_board(U64);
void generate_tables();
void Stack_push(Stack*, U64);
U64 Stack_pop(Stack*);
void Move_gen(Stack*, U64);


void print_board(U64 board){
    for(int i = 0; i < 8; i++){
        printf("\n");
        for(int j = 0; j < 8; j++){
            int index = i * 8 + j;
            if(m_get_bit(board_legal_mask, index) == 0ULL){
                if(m_get_bit(board, index) == 0ULL){
                    printf(". ");
                }
                else{
                    printf("O ");
                }
            }
            else{
                if(m_get_bit(board, index) == 0ULL){
                    printf("+ ");
                }
                else{
                    printf("E ");
                }
            }
        }
    }
    printf("\n%llx\n", board);
}
void generate_tables(){
    int dirs[] = {+1, +8, -1, -8};
    int index = 0;
    for(int i = 0; i < 64; i++){
        for(int j = 0; j < 4; j++){
            if(i + 2 * dirs[j] > 63 || i + 2 * dirs[j] < 0 || i > 63 || i < 0) continue;
            U64 word = 0x0ULL;
            m_set_bit(word, i);
            m_set_bit(word, i + dirs[j]);
            m_set_bit(word, i + 2 * dirs[j]);
            if((word & board_legal_mask) == 0ULL){
                table_masks[index] = word;
                m_pop_bit(word, i);
                table_moves[index] = word;
                index++;
            }
        }
    }
    if(index != max_move_count){
        printf("incorrect max_move_count");
    }
}

void Stack_push(Stack* stack, U64 board){
    stack->boards[(stack->index)++] = board;
}

U64 Stack_pop(Stack* stack){
    return stack->boards[--(stack->index)];
}

void Move_gen(Stack* stack, U64 board){
    for(int i = 0; i < 76; i++){
        if((board & table_masks[i]) == table_moves[i]){
            Stack_push(stack, board^table_masks[i]);
        }
    }
}



typedef struct Boardlist{
    U64 boards[initial_piece_count];
    int index;
} Boardlist;
void print_movelist(Boardlist list){
    for(int i = 0; i < initial_piece_count; i++){
        print_board(list.boards[i]);
    }
}
void fill_symmetries(U64 board, U64* symmetries){
    symmetries[0] = flipDiagonal(board);
    symmetries[1] = flipHorizontal(symmetries[0]);
    symmetries[2] = flipDiagonal(symmetries[1]);
    symmetries[3] = flipHorizontal(symmetries[2]);
    symmetries[4] = flipDiagonal(symmetries[3]);
    symmetries[5] = flipHorizontal(symmetries[4]);
    symmetries[6] = flipDiagonal(symmetries[5]);
    symmetries[7] = flipHorizontal(symmetries[6]);
}
int main()
{
    U64 solutions_found = 0;
    U64 positions_searched = 0;
    U64 table_hits = 0;
    U64 table_miss = 0;
    U64 table_fill = 0;
    generate_tables();
    Boardlist list = {.boards = {0x0ULL}, .index = 0};
    list.boards[(list.index)++] = board_start_position;
    transposition_table = (Entry*)malloc(hash_size * sizeof(Entry));
    for(int i = 0; i < hash_size; i++){
        transposition_table[i].board = 0x0ULL;
    }
    if(transposition_table == NULL) printf("no malloc");
    Stack stack = {.index = 0, .boards = {0ULL}};
    Stack_push(&stack, board_start_position);
    U64 board;
    U64 symmetries[8];
    print_board(board_start_position);
    while(stack.index > 0){
        board = Stack_pop(&stack);
        fill_symmetries(board, symmetries);
        int flag = 0;
        for(int i = 0; i < 8; i++){
            positions_searched++;
            list.index = initial_piece_count - population_count(board);
            list.boards[list.index] = board;
            unsigned int key = key_gen(symmetries[i]);
            if(transposition_table[key].board != symmetries[i]){
                if(transposition_table[key].board != 0ULL){
                    table_miss++;
                }
                else{
                    table_fill++;
                }
                transposition_table[key].board = symmetries[i];
                
            }
            else{
                table_hits++;
                flag = 1;
            }
            if(flag == 0){
                Move_gen(&stack, board);
                if(population_count(board) == 1){
                    solutions_found++;
                    //print_board(board);
                    print_movelist(list);
                }
            }
        }
        if(positions_searched % 100000000 == 0){
            //print_board(board);
            printf("Solutions:%4llu Searched:%12llu TT hits:%10lf TT ratio:%10lf TT fill:%10lf\n", solutions_found, positions_searched, (double)table_hits / positions_searched, ((double)table_hits) / (table_hits + table_miss), (double)table_fill/hash_size);
        }
    }
    printf("Solutions:%4llu Searched:%12llu TT hits:%10lf TT ratio:%10lf TT fill:%10lf\n", solutions_found, positions_searched, (double)table_hits / positions_searched, ((double)table_hits) / (table_hits + table_miss), (double)table_fill/hash_size);
    free(transposition_table);
    return 0;
}