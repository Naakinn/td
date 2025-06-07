## td (ToDo) - fast and simple CLI task manager

> [!WARNING]
> Highly beta

`td` works using sqlite3 database, so the one requirement is sqlite3.

### Requirements

- sqlite3
- make(to actually bulid project)

### Quick start
Dowload the repo and run 
```
make
```
to build `td`. 
#### If you haven't sqlite3 library installed.
You can run `setup_external.sh` script to download sqlite3 source code. 
Then compile `td` using `make EXTERNAL_SQLITE3=ON`

### Getting help
```
Usage: td [options]
Simple ToDo task manager. With no command lists all tasks.
Allowed characters in tasks' names and notes are 'a-zA-Z,.<space>&!'

COMMANDS:
    -p --push Push a task to database.
    -i --info <ID> Get information about specific task, such as note.
    -d --drop <ID> Delete task.
    -a --amend <ID> Amend a task's name or note.
OPTIONS & HELPERS:
    -n --no-confirm Do not confirm user before amending or deleting a task.
    -v --version Print td's version
    -h --help Display this help page.
```
### Goals
- [x] Use cli args with commands
- [ ] Local .td database
