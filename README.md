<p align="center">
<img src="BuildIt.png" alt="icon" width="200"/>
</p>

> [!WARNING]
> This is absolutely not production ready this is only hobbyist project

# Just Build It

Is a simple build system for C/C++ made in C++ that:
- Can be used from a (stb style) single header
- Abstracts OS specific API
- Recompiles itself when modified
- Is crossplatform
- Is multithreaded
- Can automatically create configs for vscode and clangd

Also has a helper app that:
- Can build a project without any arguments in one command
- Can bootstrap the build system
- Simplifies maintaining git repositories
- Can copy templates (copy a folder from `<helper app path>/project_templates/<template name>`)

## Quick Start

```console
$ make
$ buildit -t c test
```

## Helper Usage

```
buildit [-h] [-t <template_name>] [-r] [-i <remote_url>] [-s <commit_name>] [-l] [-d] [project_dir]

positional arguments:
  project_dir                     Path to the project

options:
  -h, --help                      show this help message and exit
  -t, --template <template_name>  generate template
  -r, --rebuild                   rebuild whole project
  -i, --init <remote_url>         init a git repo with remote
  -s, --push <commit_name>        (-s - save) push to remote repo with commit name
  -l, --pull                      (-l - load) pull lastest changes from remote repo
  -d, --discard                   discard local changes and reset to remote repo

Available default templates are: c, cpp
```

## Example `buildit.cpp`

```
TODO:
```
