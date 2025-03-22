## td (ToDo) - fast and simple CLI task manager

> [!WARNING]
> Highly beta

`td` works using sqlite3 database, so the one requirement is sqlite3
### Requirements

- sqlite3
- make( to actually bulid project)
### Quick start
Dowload the repo and run 
```
make
```
to build `td`. 

### Commands summary
- `td` - by default td lists all tasks found in local data.db file
- `td info` - distplays detailed information for a task

### Goals
- [x] `info` command
- [x] per-user database
- [x] `push` command
- [ ] `amend` command
- [ ] `drop` command
