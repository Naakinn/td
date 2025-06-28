## td (ToDo) - fast and simple CLI task manager
`td` is a fast and reliable task manager. You can use it to manage your tasks. Now, it supports UTF-8!!!

### Requirements

- sqlite3(optional, see below)
- make(to actually bulid project)

### Quick start
Dowload the repo and run 
```
make
```
to build `td`. Then, you can move it to user binary directories, such as `~/.local/bin` and others.
#### If you haven't sqlite3 library installed.
You can run `setup_external.sh` script to download sqlite3 source code. 
Then compile `td` using `make EXTERNAL_SQLITE3=ON`

### Examples

#### With `td` you can:

List all tasks by invoking `td`. 
```
$ td   
{26} Buy milk
{28} Привет, Мир!
{29} Check out the note
```
Check out the note for task with id 29!
```
$ td -i 29
{29} Check out the note: Meow meow moewwwwww
```
Delete a task
```
$ td -d 28
Delete task? (y/n) y
Task deleted
```
Add new one
```
$ td -p   
Enter a name(skip to abort): Buy coffee
Enter a note(skip for NULL): 
Created task 'Buy coffee'
```
Amend a task
```
$ td -a 26
{26} Buy milk: (null)
What to change: n(A)me/n(O)te: a
New name: Buy milk and cheese
Amend task? (y/n) y
Task amended
```

And more! See docs below for other commands and options!

### Getting help
Run `td --help`:
```
Usage: td [options]
Simple ToDo task manager. With no command lists all tasks.
td relies on task database, which is by default located in $HOME/.td directory.
When td is invoked, it recursively finds nearest to the current directory task database.
Starting from version v1.2.0 td supports UTF-8 string format. The only exception is confirmation. See --no-confirm below.

COMMANDS:
    -p --push Push a task to database.
    -i --info <ID> Get information about specific task, such as note.
    -d --drop <ID> Delete task.
    -a --amend <ID> Amend a task's name or note.
    -l --local Initialize task database in the current directory.
OPTIONS & HELPERS:
    -n --no-confirm Do not confirm user before amending or deleting a task.
    -v --version Print td's version
    -h --help Display this help page.
```
