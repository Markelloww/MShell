#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <mach/mach.h>

#define MAX_HISTORY 100 // Число команд (с конца), которые будут записаны в файл
#define HISTORY_FILE "command_history.txt" // Название файла для хранения истории


void sighup_handler(int sig) { // Обработчик сигнала SIGHUP
    printf("Configuration reloaded\n");
    fflush(stdout);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach/mach.h>

void dump_process_memory(pid_t pid) {
    char filename[256];
    snprintf(filename, sizeof(filename), "dump_%d.bin", pid);

    // Получение task-порта для процесса
    mach_port_t task;
    kern_return_t kr = task_for_pid(mach_task_self(), pid, &task);
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "Error: Failed to access process %d (task_for_pid: %s)\n", pid, mach_error_string(kr));
        return;
    }

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Error: Could not create dump file");
        return;
    }

    // Чтение регионов памяти процесса
    vm_address_t address = 0; // Начало чтения с нулевого адреса
    vm_size_t size;
    vm_region_basic_info_data_t info;
    mach_msg_type_number_t info_count = VM_REGION_BASIC_INFO_COUNT;
    mach_port_t object_name;

    while (1) {
        info_count = VM_REGION_BASIC_INFO_COUNT;
        kr = vm_region(task, &address, &size, VM_REGION_BASIC_INFO,
                       (vm_region_info_t)&info, &info_count, &object_name);
        if (kr != KERN_SUCCESS) {
            break;
        }

        // Выделение буфера для чтения
        void *buffer = malloc(size);
        if (!buffer) {
            fprintf(stderr, "Error: Could not allocate memory for buffer\n");
            break;
        }

        mach_msg_type_number_t read_bytes = 0;
        kr = vm_read_overwrite(task, address, size, (vm_address_t)buffer, &read_bytes);
        if (kr == KERN_SUCCESS) {
            fwrite(buffer, 1, read_bytes, fp);
        } else {
            fprintf(stderr, "Error reading memory at address 0x%lx: %s\n", (unsigned long)address, mach_error_string(kr));
        }

        free(buffer);

        // Переход к следующему региону
        address += size;
    }

    fclose(fp);
    printf("Memory dump saved to %s\n", filename);
}

int main() {
    signal(SIGHUP, sighup_handler);

    char input[100];
    char history[MAX_HISTORY][100];
    int history_index = 0;
    int commands_stored = 0; // Количество сохранённых команд

    while (1) {
        printf("$ ");
        fflush(stdout); // Запись буфера вывода в консоль
        if (fgets(input, 100, stdin) == NULL) { // Проверка на ошибку
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

        if (strncmp(input, "echo", 4) == 0) { // Обработка команды echo
            printf("%s\n", input + 5);
            continue;
        }
        else if (strncmp(input, "\\e", 2) == 0) { // Обработка команды вывода переменной окружения \e
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
        else if (strcmp(input, "exit") == 0 || strcmp(input, "\\q") == 0) { // Обработка команд exit и \q
            break;
        }
        else if (input[0] == '!') { // Обработка команды выполнения бинарного файла
            char command[100];
            strcpy(command, input + 1); // Копирование строки без первого символа
            int result = system(command);
            printf("Return code: %d\n", result);
            continue;
        }
        else if (strncmp(input, "\\l", 2) == 0) { // Обработка команды \l (macOS)
            char *device = input + 3;
            char command[150];
            snprintf(command, sizeof(command), "diskutil list %s", device);
            int result = system(command);
            if (result != 0) {
                printf("Error: Failed to retrieve information for device %s\n", device);
            }
            continue;
            /*
             * На макосе нет sda, потому я проверял на \l /dev/disk0
             */
        }
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
        else if (strncmp(input, "\\mem", 4) == 0) {
            char *pid_str = input + 5; // Получаем строку с PID
            pid_t pid = atoi(pid_str);

            if (pid <= 0) {
                printf("Error: Invalid process ID '%s'\n", pid_str);
                continue;
            }

            printf("Dumping memory of process %d...\n", pid);
            dump_process_memory(pid);
            continue;
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