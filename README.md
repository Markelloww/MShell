# MShell - Command Interpreter in C

**Language / Язык:**  
[English](#english) | [Русский](#russian)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

<div id="english"></div>

## 🇺🇸 English Version

This project is a command interpreter (shell) written in C that supports basic commands and process management functions.

## Functionality

### Main Commands:

1. **Basic I/O** - cyclic command input with `>` prompt
2. **Exit** - `exit` or `\q` commands to terminate the shell
3. **Echo** - `echo` command for text output
4. **Command History** - saves last 100 commands to `command_history.txt` file
5. **Environment Variables** - `\e <VAR>` command to display environment variable values
6. **Binary Execution** - `!<program>` command to run programs from `/bin/`
7. **SIGHUP Signal** - signal handling with "Configuration reloaded" output
8. **Disk Information** - `\l <device>` command to get partition information (macOS)
9. **Scheduler VFS** - `\cron` command to create VFS in `/tmp/vfs/` with cron tasks

## Makefile

### Main Commands:

1. `make all` - build **MShell**
2. `make clean` - remove all build artifacts
3. `make install` - install to /usr/local/bin
4. `make run` - build and run **MShell**
5. `make help` - show available commands

### Compilation and Execution

```bash
make run
```

### If errors occur
```bash
make clean
make run
```

<div id="russian"></div>

## 🇷🇺 Русская версия

Проект представляет собой командный интерпретатор (shell), написанный на языке C, который поддерживает основные команды и функции управления процессами.

## Функциональность

### Основные команды:

1. **Базовый ввод/вывод** — циклический ввод команд с приглашением `>`
2. **Выход** — команды `exit` или `\q` для завершения работы
3. **Эхо** — команда `echo` для вывода текста
4. **История команд** — сохранение последних 100 команд в файл `command_history.txt`
5. **Переменные окружения** - команда `\e <VAR>` для вывода значений переменных окружения
6. **Запуск бинарных файлов** - команда `!<program>` для запуска программ из `/bin/`
7. **Сигнал SIGHUP** - обработка сигнала с выводом "Configuration reloaded"
8. **Информация о дисках** - команда `\l <device>` для получения информации о разделах (macOS)
9. **Виртуальная ФС планировщика** - команда `\cron` для создания VFS в `/tmp/vfs/` с задачами cron

## Makefile

### Основные команды:

1. `make all` — собрать **MShell**
2. `make clean` — удалить все артефкаты сборки
3. `make install` — установить в /usr/local/bin
4. `make run` — собрать и запустить **MShell**
5. `make help` — показать доступные команды

### Компиляция и запуск

```bash
make run
```

### При возникновении ошибок
```bash
make clean
make run
```
