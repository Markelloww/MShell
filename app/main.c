#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_HISTORY 100 // Число команд (с конца), которые будут записаны в файл
#define HISTORY_FILE "command_history.txt" // Название файла для хранения истории

int main() {
    char input[100];
    char history[MAX_HISTORY][100];
    int history_index = 0;
    int commands_stored = 0; // Количество сохранённых команд

    while (1) {
        printf("$ ");
        fflush(stdout);
        if (fgets(input, 100, stdin) == NULL) {
            break;
        }

        unsigned long n = strlen(input);
        input[n - 1] = '\0'; // Удаление символа новой строки

        // Сохранение команды в историю
        strcpy(history[history_index % MAX_HISTORY], input);
        history_index++;
        if (commands_stored < MAX_HISTORY) {
            commands_stored++;
        }

        // Обработка команды echo
        if (strncmp(input, "echo", 4) == 0) {
            printf("%s\n", input + 5);
            continue;
        }

        // Обработка команды вывода переменной окружения \e
        if (strncmp(input, "\\e", 2) == 0) {
            char * var_name = input + 3; // Получаем имя переменной окружения
            char * value = getenv(var_name);
            if (value != NULL) {
                printf("%s\n", value);
            }
            else {
                printf("Error: environment variable '%s' not found\n", var_name);
            }
            continue;
        }

        // Обработка команд exit и \q
        if (strcmp(input, "exit") == 0 || strcmp(input, "\\q") == 0) {
            break;
        }

        printf("%s: command not found\n", input);
    }

    // Запись истории в файл (история удаляется после повторной записи в файл)
    FILE *fp = fopen(HISTORY_FILE, "w");
    for (int i = 0; i < commands_stored; i++) {
        fprintf(fp, "%s\n", history[(history_index - commands_stored + i) % MAX_HISTORY]);
    }
    fclose(fp);

    return 0;
}