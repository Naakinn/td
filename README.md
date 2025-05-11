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
Just run `td help` or check it in [source](https://github.com/Naakinn/td/blob/main/src/main.c#L74)

### Goals
- [ ] Use cli args with commands
- [ ] Local .td database
