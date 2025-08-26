<p align="center">
<img src="BuildIt.png" alt="drawing" width="200"/>
</p>

> [!WARNING]
> This is absolutely not production ready this is only hobbyist project

# Just Build It

Is a simple [D](https://dlang.org) build system for C/C++ that:
- Is designed for windows MinGW and Linux (But can adapt to many other toolchains)
- Works on projects written in YAML
- Can build a project without any arguments in one command
- Helps with maintaining git repositories
- All its config options can contain bash like commands (e.g. "$(echo Hello)") so you can have any other program "print" values as for example the compiler arguments

# Usage

```
Usage: buildit [options] [project_path]

Simple D build system for C/C++

positional arguments:
  project_path                  path to the project (if not specified will use current working directory)

options:
  -h, --help                    show this help message and exit
  -t, --template TEMPLATE_NAME  generate template
  -c, --clean                   clean any build files
  -r, --rebuild                 rebuild whole project
  -i, --init REMOTE_URL         init a git repo with remote
  -s, --push COMMIT_NAME        (-s - save) push to remote repo with commit name
  -l, --pull                    (-l - load) pull lastest changes from remote repo
  -d, --discard                 discard local changes and reset to remote repo

Available default templates are: cpp
```

PS. Generating Templates is just coping a template project (directory) from `./cpp_project_templates/<template name>` to the project directory

## Quick Start

```console
$ make
$ buildit -t cpp test
```

### Example `buildme.yaml`

```
create_vscode_settings : True                      # Create vscode workspace settings based on eg. include paths
run_after_build        : True                      # Run the executable after succesful build
proj_name              : "test"                    # Project name
proj_version           : "0.1.0.0"                 # Project version
file_name              : "test"                    # Output executable name (on windows .exe is autmatically added)
compiler               : "g++"                     # Compiler for c and c++ (you can add c_compiler or cxx_compiler for different compilers)
linker                 : "g++"                     # Linker for linking all object files
flags                  : ['-Wall','-Wextra','-O3'] # Flags for linker and compiler (you can specifiy compiler_flags and linker_flags)
output_path            : "bin"                     # Directory for the executable (and all objects in subdirectory specified by obj_dir default is obj)
source_path            : "src"                     # Directory with the source code (can be in subdirectories)
libs                   : []                        # List of Libraries to link with
pkgconf_libs           : []                        # List of Libraries passed to pkgconf
lib_paths              : []                        # List of Directories with libraries
include_paths          : []                        # List of Directories with headers
run_args               : []                        # Arguments to pass to the executable when run_after_build is True

exec_postbuild         : []                        # List of Command lines to run after a successful build
exec_prebuild          : []                        # List of Command lines to run before the build

```