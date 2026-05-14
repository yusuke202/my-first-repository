#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_WORDS 46725
#define WORD_LEN 100
#define MAX_LINES 100
#define LINE_LEN  64
#define CHANGE_SCORE(x) (total_score += x)


//グローバル変数
int variable = 0;
//int total_score = 0;    
char file_ranking[MAX_LINES][LINE_LEN];
int ranking[MAX_LINES];  
char dictionary[MAX_WORDS][WORD_LEN];
char forbidden_letter[3];
//bool skip = false;
int git = 0;

//関数プロトタイプ
int stricmp(const char *a, const char *b);
int load_words(char dictionary[][WORD_LEN]);
int word_length_restriction();
void forbidden_letter_restriction();
char lucky_alphabet();
void record();
void load_ranking();
void swap(int *x, int *y);
void print_ranking(int ranking[]);
void timeout(int sig);
void word_chain_game();
void git_method1();

// 大文字小文字を無視して比較
int stricmp(const char *a, const char *b) {

    while (*a && *b) {
        if (tolower(*a) != tolower(*b))
            return 0;
        a++;
        b++;
    }

    while(*a){
        if(*a != '\0'){
            return 0;
        }
    }

    while(*b){
        if(*b != '\0'){
            return 0;
        }
    }

    return 1;
}

// 存在する英単語を取得
int load_words(char dictionary[][WORD_LEN]) {

    const char *filename = "Englishwords.txt";

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("ファイル '%s' が開けません。\n", filename);
        return 0;
    }

    int count_A = 0;

    while (fgets(dictionary[count_A], WORD_LEN, fp)) {

        // 改行文字を削除
        dictionary[count_A][strcspn(dictionary[count_A], "\n")] = '\0';

        count_A++;

        if (count_A >= MAX_WORDS)
            break;
    }

    fclose(fp);

    return count_A;
}

//列挙体


// 難易度選択
typedef struct {
    bool word_length_restriction;
    bool forbidden_letter_restriction;
} rule;

// 文字数指定
int word_length_restriction() {
    return rand() % 5 + 4;
}

// 使用禁止文字指定
void forbidden_letter_restriction() {
    char table[] = {"abcdefghijklmnnnopqrrrrrstttuvwxyyyz"};//末尾で多いアルファベットであるrを5倍、n,t,yを3倍出やすく
    for (int i  = 0;i < 3;i++){
        forbidden_letter[i] = table[rand()%(sizeof(table - 1))];
    }
}

//加点するアルファベット
char lucky_alphabet(){
    return 'a' + (rand() % 26);
}

// 記録
void record() {

    const char *filename = "ranking.txt";

    FILE *fp = fopen(filename, "a");
    if (!fp) {
        printf("ファイル '%s' が開けません。\n", filename);
        return;
    }

    fprintf(fp, "%d", total_score);
    fprintf(fp, "\n");

    fclose(fp);
}

// ランキングを配列に保存
void load_ranking() {

    const char *filename = "ranking.txt";

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("ファイル '%s' が開けません。\n", filename);
        return;
    }

    int count_B = 0;

    while (fgets(file_ranking[count_B], LINE_LEN, fp)) {

        if (file_ranking[count_B][0]== '\n'){break;}

        ranking[count_B] = atoi(file_ranking[count_B]);

        count_B++;

        if (count_B >= MAX_LINES)
            break;
    }

    fclose(fp);

    variable = count_B;
}

// int型の値交換（*xと*yの値を交換）
void swap(int *x, int *y) {

    int tmp = 0;

    tmp = *x;
    *x = *y;
    *y = tmp;
}

// ランキングを表示
void print_ranking(int ranking[]) {

    int i = variable;
    int rank = 1;

    for (int j = 1; j < i; j++) {
        for (int k = i; k > j - 1; k--) {
            if (ranking[k-1] < ranking[k]) {
                swap(&ranking[k-1], &ranking[k]);
            }
        }
    }

    for (int x = 1; x < i; x++) {
        if (ranking[x] > total_score) {
            rank++;
        }
    }

    printf("第１位： %d pt\n",ranking[0]);
    printf("第２位： %d pt\n",ranking[1]);
    printf("第３位： %d pt\n",ranking[2]);
    printf("あなたのスコア %d pt は第 %d 位です。\n", total_score, rank);
}

// 時間制限が来た後の表示
void timeout(int sig) {

    printf("時間切れ！\n");
    printf("スコアは%dptです。\n", total_score);

    record();

    load_ranking();

    print_ranking(ranking);

    exit(0);
}

// ゲーム
void word_chain_game(rule player) {

    char rule[4][10] = {"", "easy", "normal", "hard"};

    char result[1000][100];

    int word_count = load_words(dictionary);

    int difficulty = 0;
    int found_a = 0;
    int found_b = 0;
    int i = 0;
    char word[100];
    char last = 'a';
    int x;
    char y1;
    char y2;
    char y3;

    //難易度選択
    do {
        printf("難易度を選択してください（半角数字）： 1 → easy,2 → normal（使用禁止文字）,3 → hard（文字数指定、使用禁止文字）");
        scanf("%d", &difficulty);

        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
        continue;
    } while(difficulty < 1 || difficulty > 3);

    if (difficulty == EASY) {
        player.word_length_restriction = false;
        player.forbidden_letter_restriction = false;
    }
    else if (difficulty == NORMAL) {
        player.word_length_restriction = false;
        player.forbidden_letter_restriction = true;
    }
    else {
        player.word_length_restriction = true;
        player.forbidden_letter_restriction = true;
    }

    // ゲーム開始
    printf("<<ルール>>\n");
    printf("文字数指定や使用禁止文字のルールを破った場合はその単語は無効で、-1ptとなります。\n");
    printf("@を入力するとスキップできます。また、次の文字を指定できます。ただし、-10ptとなります。\n");
    printf("ラッキーアルファベットは１文字含むごとに+5ptとなります。");
    printf("難易度：%s\n", rule[difficulty]);
    printf("Enterキーを押すと、ゲーム開始！(”a”から始まる単語でスタート)\n");

    int cha;
    while ((cha = getchar()) != '\n');

    signal(SIGALRM, timeout);
    alarm(90);

    while (true) {

        int c;

        char z = lucky_alphabet();

        printf("現在のスコア：%d\n", total_score);

        if(i>=1){
            printf("これまでに使用した単語：\n");
            for(int z = 0;z < i;z++){
                printf("%s\n",result[z]);
            }
        }

        printf("ラッキーアルファベット：%c\n",z);

        if (player.word_length_restriction == true) {
            x = word_length_restriction();
            printf("文字数は%d文字指定です。\n", x);
        }

        if (player.forbidden_letter_restriction == true) {
            forbidden_letter_restriction();
            y1 = forbidden_letter[0];
            y2 = forbidden_letter[1];
            y3 = forbidden_letter[2];

            if((y1 == y2)&&(y2 == y3)){
                printf("使用禁止文字は%cです。\n", y1);
            }

            else if(y1 == y2){
                printf("使用禁止文字は%c,%cです。\n", y1,y3);
            }

            else if(y2 == y3){
                printf("使用禁止文字は%c,%cです。\n", y1,y2);
            }

            else if(y1 == y3){
                printf("使用禁止文字は%c,%cです。\n", y1,y2);
            }

            else{
                printf("使用禁止文字は%c,%c,%cです。\n", y1,y2,y3);
            }

        }

        printf("%cから始まる英単語を入力してください。\n", last);
        scanf("%99s", word);

        //スキップ
        if(strcmp(word,"@") == 0){
            char skip_word;
            skip = true;
            do{
                printf("スキップします。次に入力する英単語の頭文字を入力してください。（半角）\n");
                scanf("%1s", &skip_word);
            }while(skip_word<'a' || skip_word>'z');
            last = skip_word;
            CHANGE_SCORE(-10);
            continue;
        }

        do{
            // 存在しない英単語を入力した場合
            strcpy(result[i], word);
            found_a = 0;
            for (int p = 0; p < word_count; p++) {
                if (stricmp(dictionary[p], result[i]) == 1) {
                    found_a = 1;
                    break;
                }
            }
            if (found_a == 0) {
                printf("%sは存在しません。\n", result[i]);
                strcpy(result[i], "");
                break;
            }

            // 同じ単語を二度入力した場合
            strcpy(result[i], word);
            found_b = 0;
            for (int p = 0; p < i; p++) {
                if (stricmp(result[i], result[p]) == 1) {
                    found_b = 1;
                    break;
                }
            }
            if (found_b == 1) {
                printf("%sは既に入力されています。\n", result[i]);
                strcpy(result[i], word);
            }

        }while(false);

        //文字数指定の判定
        if (player.word_length_restriction == true) {
            if (strlen(result[i]) != x) {
                printf("不適切な文字数です。\n");
                CHANGE_SCORE(-1);
                continue;
            }
        }

        //使用禁止文字の判定
        if (player.forbidden_letter_restriction == true) {
            bool forbidden_letter_flag = false;
            for (int a = 0; a < strlen(result[i]); a++) {
                if ((result[i][a] == y1)||(result[i][a] == y2)||(result[i][a] == y3)) {
                    printf("使用禁止文字が含まれています。\n");
                    forbidden_letter_flag = true;
                    break;
                }
            }

            if(forbidden_letter_flag == true){
                CHANGE_SCORE(-3);
                continue;
            }
        }

        //文字数指定と使用禁止文字のルールをクリアした場合
        if ((found_a == 1) && (found_b == 0)) {

            for (int b = 0; b < strlen(result[i]); b++) {
                if (result[i][b] == z) {
                    total_score+=5;
                }
            }
            last = word[strlen(result[i]) - 1];
            CHANGE_SCORE(strlen(result[i]));
            i++;
        }

    }

}

// メインメソッド
int main(void) {

    rule player;
    srand((unsigned int)time(NULL));

    word_chain_game(player);

    return 0;
}