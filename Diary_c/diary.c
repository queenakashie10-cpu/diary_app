#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LEN 512
#define MAX_MORSE_LEN 4096
#define MAX_ENTRIES 500

typedef struct {
    char character;
    char code[8];
} MorseEntry;

typedef struct {
    char text[MAX_LINE_LEN];
} DiaryEntry;

static const MorseEntry MORSE_TABLE[] = {
    {'A', ".-"}, {'B', "-..."}, {'C', "-.-."}, {'D', "-.."},
    {'E', "."}, {'F', "..-."}, {'G', "--."}, {'H', "...."},
    {'I', ".."}, {'J', ".---"}, {'K', "-.-"}, {'L', ".-.."},
    {'M', "--"}, {'N', "-."}, {'O', "---"}, {'P', ".--."},
    {'Q', "--.-"}, {'R', ".-."}, {'S', "..."}, {'T', "-"},
    {'U', "..-"}, {'V', "...-"}, {'W', ".--"}, {'X', "-..-"},
    {'Y', "-.--"}, {'Z', "--.."},
    {'0', "-----"}, {'1', ".----"}, {'2', "..---"}, {'3', "...--"},
    {'4', "....-"}, {'5', "....."}, {'6', "-...."}, {'7', "--..."},
    {'8', "---.."}, {'9', "----."}
};

static const int MORSE_TABLE_SIZE = sizeof(MORSE_TABLE) / sizeof(MORSE_TABLE[0]);

void clearScreen(void) {
    system("cls||clear");
}

void waitEnter(void) {
    char buffer[8];
    printf("\nPress Enter to return...");
    fgets(buffer, sizeof(buffer), stdin);
}

int readInt(const char *prompt) {
    char line[64];
    char *endPtr;
    long value;

    while (1) {
        printf("%s", prompt);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            return -1;
        }

        line[strcspn(line, "\n")] = '\0';

        if (strlen(line) == 0) {
            printf("Invalid option. Please enter a number.\n");
            continue;
        }

        value = strtol(line, &endPtr, 10);

        if (*endPtr != '\0') {
            printf("Invalid option. Please enter a valid number.\n");
            continue;
        }

        return (int)value;
    }
}

const char *charToMorse(char c) {
    int i;
    for (i = 0; i < MORSE_TABLE_SIZE; i++) {
        if (MORSE_TABLE[i].character == c) {
            return MORSE_TABLE[i].code;
        }
    }
    return NULL;
}

char morseToChar(const char *code) {
    int i;
    for (i = 0; i < MORSE_TABLE_SIZE; i++) {
        if (strcmp(MORSE_TABLE[i].code, code) == 0) {
            return MORSE_TABLE[i].character;
        }
    }
    return '?';
}

void encodeToMorse(const char *input, char *output) {
    int i;
    int firstLetter = 1;

    output[0] = '\0';

    for (i = 0; input[i] != '\0'; i++) {
        char c = (char)toupper((unsigned char)input[i]);
        const char *morse;

        if (c == ' ') {
            strcat(output, "       ");
            firstLetter = 1;
            continue;
        }

        morse = charToMorse(c);
        if (morse == NULL) {
            continue;
        }

        if (!firstLetter) {
            strcat(output, "   ");
        }

        strcat(output, morse);
        firstLetter = 0;
    }
}

void decodeFromMorse(const char *input, char *output) {
    int i = 0;
    int outIndex = 0;
    char currentCode[16];
    int codeIndex = 0;
    int spaceCount = 0;

    output[0] = '\0';

    while (1) {
        char ch = input[i];

        if (ch == '.' || ch == '-') {
            if (spaceCount > 0) {
                if (spaceCount >= 7) {
                    if (codeIndex > 0) {
                        currentCode[codeIndex] = '\0';
                        output[outIndex++] = morseToChar(currentCode);
                        codeIndex = 0;
                    }

                    if (outIndex > 0 && output[outIndex - 1] != ' ') {
                        output[outIndex++] = ' ';
                    }
                } else if (spaceCount >= 3) {
                    if (codeIndex > 0) {
                        currentCode[codeIndex] = '\0';
                        output[outIndex++] = morseToChar(currentCode);
                        codeIndex = 0;
                    }
                }

                spaceCount = 0;
            }

            if (codeIndex < (int)sizeof(currentCode) - 1) {
                currentCode[codeIndex++] = ch;
            }
        } else if (ch == ' ') {
            spaceCount++;
        } else if (ch == '\0') {
            if (codeIndex > 0) {
                currentCode[codeIndex] = '\0';
                output[outIndex++] = morseToChar(currentCode);
            }
            break;
        }

        i++;
    }

    output[outIndex] = '\0';
}

void addEntry(const char *filename) {
    char input[MAX_LINE_LEN];
    char morse[MAX_MORSE_LEN];
    FILE *f;

    clearScreen();
    printf("=== ADD ENTRY ===\n");

    printf("Enter text:\n> ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Input error.\n");
        waitEnter();
        return;
    }

    input[strcspn(input, "\n")] = '\0';

    if (strlen(input) == 0) {
        printf("Empty entry.\n");
        waitEnter();
        return;
    }

    encodeToMorse(input, morse);

    f = fopen(filename, "a");
    if (f == NULL) {
        printf("File error.\n");
        waitEnter();
        return;
    }

    fprintf(f, "%s\n", morse);
    fclose(f);

    printf("Saved.\n");
    waitEnter();
}

void viewEntries(const char *filename) {
    FILE *f;
    char line[MAX_MORSE_LEN];
    char decoded[MAX_LINE_LEN];
    int index = 1;

    clearScreen();
    printf("=== VIEW ENTRIES ===\n");

    f = fopen(filename, "r");
    if (f == NULL) {
        printf("No file.\n");
        waitEnter();
        return;
    }

    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\n")] = '\0';

        if (strlen(line) == 0) {
            continue;
        }

        decodeFromMorse(line, decoded);
        printf("[%d] %s\n", index, decoded);
        index++;
    }

    fclose(f);

    if (index == 1) {
        printf("Empty.\n");
    }

    waitEnter();
}

void searchEntries(const char *filename) {
    FILE *f;
    DiaryEntry entries[MAX_ENTRIES];
    char line[MAX_MORSE_LEN];
    char key[MAX_LINE_LEN];
    char keyLower[MAX_LINE_LEN];
    int total = 0;
    int i, j;
    int found = 0;

    clearScreen();
    printf("=== SEARCH ENTRIES ===\n");

    f = fopen(filename, "r");
    if (f == NULL) {
        printf("No file.\n");
        waitEnter();
        return;
    }

    while (fgets(line, sizeof(line), f) != NULL && total < MAX_ENTRIES) {
        line[strcspn(line, "\n")] = '\0';

        if (strlen(line) == 0) {
            continue;
        }

        decodeFromMorse(line, entries[total].text);
        total++;
    }

    fclose(f);

    if (total == 0) {
        printf("No entries.\n");
        waitEnter();
        return;
    }

    printf("Keyword: ");
    if (fgets(key, sizeof(key), stdin) == NULL) {
        printf("Input error.\n");
        waitEnter();
        return;
    }

    key[strcspn(key, "\n")] = '\0';

    if (strlen(key) == 0) {
        printf("Empty keyword.\n");
        waitEnter();
        return;
    }

    for (i = 0; key[i] != '\0'; i++) {
        keyLower[i] = (char)tolower((unsigned char)key[i]);
    }
    keyLower[i] = '\0';

    for (i = 0; i < total; i++) {
        char temp[MAX_LINE_LEN];

        for (j = 0; entries[i].text[j] != '\0'; j++) {
            temp[j] = (char)tolower((unsigned char)entries[i].text[j]);
        }
        temp[j] = '\0';

        if (strstr(temp, keyLower) != NULL) {
            printf("[%d] %s\n", i + 1, entries[i].text);
            found = 1;
        }
    }

    if (!found) {
        printf("No matches.\n");
    }

    waitEnter();
}

void printMainMenu(void) {
    printf("=== DIARY MENU ===\n");
    printf("1. Add Entry\n");
    printf("2. View Entries\n");
    printf("3. Search Entries\n");
    printf("0. Exit\n");
}

int main(int argc, char *argv[]) {
    const char *filename = (argc > 1) ? argv[1] : "diary.txt";
    int choice;

    do {
        clearScreen();
        printMainMenu();
        choice = readInt("Choice: ");

        switch (choice) {
            case 1:
                addEntry(filename);
                break;
            case 2:
                viewEntries(filename);
                break;
            case 3:
                searchEntries(filename);
                break;
            case 0:
                clearScreen();
                printf("Goodbye!\n");
                break;
            default:
                printf("Invalid option.\n");
                waitEnter();
                break;
        }

    } while (choice != 0);

    return 0;
}
