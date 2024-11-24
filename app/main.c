#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_HISTORY 100 // Число команд (с конца), которые будут записаны в файл
#define HISTORY_FILE "command_history.txt" // Название файла для хранения истории

/*
 * (9) По сигналу SIGHUP вывести "Configuration reloaded"
 */
void sighup() {
    signal(SIGHUP, sighup);
    printf("Configuration reloaded\n");
}

int main() {
    signal(SIGHUP, sighup);

    char input[100]; // Введенная строка с клавиатуры

    char history[MAX_HISTORY][100];
    int history_index = 0;
    int commands_stored = 0;

    while (1) {

        /*
         * (1) + (2) Печатает введенную строку в цикле.
         */
        printf("> ");

        /*
         * Используем fflush для корректного вызова printf.
         */
        fflush(stdout);

        /*
         * (2) Выходим из цикла по Ctrl+D
         * Читаем введенную строку с клавиатуры и записываем ее в input.
         * Максимальный размер строки равен 100.
         * Если чтение было успешным, то указатель пойдет на начало строки,
         * иначе будет равен NULL, тогда программа завершится.
         */
        if (fgets(input, 100, stdin) == NULL) {
            break;
        }

        /*
         * Из полученной пользователем строки удаляем \n.
         * Т.е. длина будет уменьшена на единицу.
         */
        unsigned long n = strlen(input);
        input[n - 1] = '\0';

        /*
         * (4) Здесь происходит обработка истории введенных пользователем команд.
         * История будет хранить последние MAX_HISTORY команд,
         * и если количество команд превышает MAX_HISTORY,
         * то новые команды будут перезаписывать старые команды в истории.
         */
        strcpy(history[history_index % MAX_HISTORY], input);
        history_index++;
        if (commands_stored < MAX_HISTORY) {
            commands_stored++;
        }

        /*
         * (3) Обрабатываем команды выхода exit и \q.
         */
        if (strcmp(input, "exit") == 0 || strcmp(input, "\\q") == 0) {
            break;
        }

        /*
         * (5) Обрабатываем команду echo.
         * strncmp берет из input 4 символа и если они равны "echo",
         * то возвращает 0 => printf выведет в консоль оставшиеся введенные символы.
         */
        else if (strncmp(input, "echo", 4) == 0) {
            printf("%s\n", input + 5);
            continue;
        }

        /*
         * (7) Обработка команды вывода переменной окружения \e.
         */
        else if (strncmp(input, "\\e", 2) == 0) {
            char * var_name = input + 3; // Получаем имя переменной окружения
            char * value = getenv(var_name); // Получаем значение, если еге нет, то вернет NULL
            if (value != NULL) {
                printf("%s\n", value);
            }
            else {
                printf("Error: environment variable '%s' not found\n", var_name);
            }
            continue;
        }

        /*
         * (8) Выполняем указанный бинарник из /bin/ !
         */
        else if (strncmp(input, "!", 1) == 0) {
            char command[100];
            char * binary_file_name = input + 1; // Получаем имя двоичного файла
            snprintf(command, sizeof(command), "/bin/%s", binary_file_name); // Формируем command
            int result = system(command);
            if (result != 0) {
                printf("Error: Failed to run binary file %s\n", binary_file_name);
            }
            continue;
        }

        /*
         * (10) Получаем информацию о разделе системы (для macOS) \l
         */
        else if (strncmp(input, "\\l", 2) == 0) {
            char command[100];
            char * device = input + 3; // Имя дискового устройства
            // snprintf записывает строку в массив command с заменой %s на значение переменной device
            snprintf(command, sizeof(command), "diskutil list %s", device);
            int result = system(command);
            if (result != 0) {
                printf("Error: Failed to retrieve information for device %s\n", device);
            }
            continue;
            /*
             * На macOS нет SDA, потому я проверял на \l /dev/disk0
             */
        }

        /*
         * (11) По \cron подключить VFS в /tmp/vfs со списком задач в планировщике
         */
        else if (strcmp(input, "\\cron") == 0) { // Обработка команды \cron
            const char * vfs_dir = "/tmp/vfs";
            const char * tasks_file = "/tmp/vfs/tasks";

            // Создаем виртуальную файловую систему (каталог /tmp/vfs)
            if (access(vfs_dir, F_OK) == -1) {
                if (mkdir(vfs_dir, 0755) != 0) {
                    perror("Error creating VFS directory");
                    continue;
                }
                printf("VFS mounted at %s\n", vfs_dir);
            }
            else {
                printf("VFS already mounted at %s\n", vfs_dir);
            }

            // Открываем файл для записи списка задач
            FILE * fp = fopen(tasks_file, "w");
            if (fp == NULL) {
                perror("Error creating tasks file in VFS");
                continue;
            }

            // Получаем список задач из crontab
            FILE * crontab_output = popen("crontab -l", "r");
            if (crontab_output == NULL) {
                perror("Error reading crontab");
                fclose(fp);
                continue;
            }

            // Читаем вывод команды crontab -l и записываем в файл
            char line[256];
            while (fgets(line, sizeof(line), crontab_output) != NULL) {
                fprintf(fp, "%s", line);
            }

            pclose(crontab_output);
            fclose(fp);

            printf("Tasks saved to %s\n", tasks_file);
            continue;
        }

        /*
         * (6) Если после проверки введенной команды она не будет найденной,
         * то пользователь будет об этом уведомлен.
         */
        printf("%s: command not found\n", input);
    }

    /*
     * (4) Здесь происходит сохранение истории введенных команд в файл.
     */
    FILE * fp = fopen(HISTORY_FILE, "w");
    for (int i = 0; i < commands_stored; i++) {
        fprintf(fp, "%s\n", history[(history_index - commands_stored + i) % MAX_HISTORY]);
    }
    fclose(fp);

    return 0;
}