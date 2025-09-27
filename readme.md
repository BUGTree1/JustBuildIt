<p align="center">
<img src="BuildIt.png" alt="drawing" width="200"/>
</p>

> [!WARNING]
> This is absolutely not production ready this is only hobbyist project

# Just Build It

Is a simple Python build system for C/C++ that:
- Is designed for windows MinGW and Linux (But can adapt to many other toolchains)
- Is multithreaded
- Can build a project without any arguments in one command
- Can automatically create config for vscode
- Works on projects written in YAML
- Helps with maintaining git repositories

# Dependencies

[PyYAML](https://pypi.org/project/PyYAML/)

# Usage

```
JustBuildIt [-h] [-t <template_name>] [-r] [-i <remote_url>] [-s <commit_name>] [-l] [-d] [project_dir]

Simple build system for C/C++

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

PS. Generating Templates is just coping a template project (directory) from `./project_templates/<template name>` to the project directory

## Quick Start

```console
$ make build
$ buildit -t c test
```

### Example `buildme.yaml`

```
create_vscode_settings : true                      # Create vscode workspace settings based on eg. include paths
run_after_build        : true                      # Run the executable after succesful build
auto_out_file_ext      : true                      # Add the correct file exension to the build files
build_type             : "executable"              # Build type can be: executable, static (library), dynamic (library)
proj_name              : "TEMPLATE"                # Project name
proj_version           : "0.1.0.0"                 # Project version
file_name              : "TEMPLATE"                # Output executable name (on windows .exe is autmatically added with auto_out_file_ext)
compiler               : "gcc"                     # Compiler for c and c++ (you can add c_compiler or cxx_compiler for different compilers)
linker                 : "gcc"                     # Linker for linking all object files
flags                  : ['-Wall','-Wextra','-O3'] # Flags for linker and compiler (you can specifiy compiler_flags and linker_flags)
output_dir             : "bin"                     # Directory for the executable (and all objects in subdirectory specified by obj_dir default is obj)
source_dir             : "src"                     # Directory with the source code (can be in subdirectories)
libs                   : []                        # List of Libraries to link with
pkgconf_libs           : []                        # List of Libraries passed to pkgconf
lib_dirs               : []                        # List of Directories with libraries
include_dirs           : []                        # List of Directories with headers
run_args               : []                        # Arguments to pass to the executable when run_after_build is True

exec_postbuild         : []                        # List of Command lines to run after a successful build
exec_prebuild          : []                        # List of Command lines to run before the build

```